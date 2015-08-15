#include "ff-mac-scheduler.h"

#include <algorithm>
#include <functional>

#include "x2-channel.h"
#include "../simulator.h"

FfMacScheduler::FfMacScheduler(int cellId)
  : mCellId(cellId)
  , mIsLeader(false)
  , mWindowDuration(Converter::milliseconds(16))
  , mDirectParticipantCellId(-1)
  , mIsDirectParticipant(false)
  , mLeaderCellId(-1)
{
}

FfMacScheduler::~FfMacScheduler()
{
  if (mIsLeader || mCellSwitchCounter)
    {
      LOG("MAC scheduler cell-switch statistics:");
      LOG("\tintervals between switch [ms]:");

      LOG("\tave: " << mCellSwitchWatch.average(mCellSwitchIndex) / 1000.0
          << "\tmin: "<< mCellSwitchWatch.minimum(mCellSwitchIndex) / 1000.0
          << "\tmax: " << mCellSwitchWatch.maximum(mCellSwitchIndex) / 1000.0
          << "\tTotal switches: " << mCellSwitchCounter);
    }
}

void FfMacScheduler::setLeader(int cellId)
{
  mLeaderCellId = cellId;

  if (mIsLeader == (mCellId == cellId))
    return;

  mIsLeader = mCellId == cellId;

  if (mIsLeader)
    {
      mIsDirectParticipant = true;
      mDirectParticipantCellId = mCellId;

      X2Message message;
      message.type = X2Message::leadershipInd;
      message.leaderCellId = mCellId;
      X2Channel::instance()->send(-1, message);
    }
  schedDlTriggerReq();
}

void FfMacScheduler::setFfMacSchedSapUser(FfMacSchedSapUser *user)
{
  mMacSapUser = user;
}

void FfMacScheduler::setTrafficActivity(bool mustSend, Time applyTime)
{
  if (mustSend)
    enqueueTx(applyTime);
  else
    enqueueTxStop(applyTime, -1);
}

void FfMacScheduler::schedDlTriggerReq()
{
  mMacSapUser->schedDlConfigInd(mCellId, {mIsDirectParticipant});
}

void FfMacScheduler::schedDlCqiInfoReq(int tCellId, CsiUnit csi)
{
  if (!mIsLeader)
    {
      if (mLeaderCellId != -1)
        {
          CSIMeasurementReport measReport;
          measReport.csi = csi;
          measReport.targetCellId = tCellId;

          X2Message msg;
          msg.type = X2Message::measuresInd;
          msg.report = measReport;

          X2Channel::instance()->send(mLeaderCellId, msg);
        }
      return;
    }
  // This cell is leader:


  if (mCsiHistory[tCellId].empty())
    {
      mCsiHistory[tCellId].push_back(csi);
      return;
    }

  CsiArray& array = mCsiHistory[tCellId];
  if (csi.first < array.back().first) // drop older CSIs
    return;

  array.push_back(csi);
  while (array.back().first - array.front().first > mWindowDuration)
    array.pop_front();

  if (SimTimeProvider::getTime() > Converter::milliseconds(150))
    processREChanges();
}

void FfMacScheduler::onTimeout()
{
  const Time currentTime = SimTimeProvider::getTime();
  auto &executionQ = mInternalEvents[currentTime];
  while (!executionQ.empty())
    {
      const auto event = executionQ.front();
      executionQ.pop();
      switch (event)
        {
        case SchedulerEvent::startTx:
          mIsDirectParticipant = true;
          mDirectParticipantCellId = mCellId;
          schedDlTriggerReq();
          break;
        case SchedulerEvent::stopTx:
          mIsDirectParticipant = false;
          mDirectParticipantCellId = mScheduledDirectCellId[currentTime];
          mScheduledDirectCellId.erase(currentTime);
          schedDlTriggerReq();
          break;
        case SchedulerEvent::updateDciDecision:
          schedDlTriggerReq();
          break;
        }
    }
  mInternalEvents.erase(currentTime);
}

void FfMacScheduler::switchDirectCell(int cellId)
{
  Time currentTime = SimTimeProvider::getTime();
  Time applyChanges = currentTime + X2Channel::instance()->getLatency() + Converter::microseconds(10);
  // switch traffic OFF at old cell
  if (mLastScheduledCellId != mCellId)
    {
      // not this cell
      X2Message msgOff;
      msgOff.type = X2Message::changeScheduleModeInd;
      msgOff.mustSendTraffic = false;
      msgOff.applyDirectMembership = applyChanges;

      X2Channel::instance()->send(mLastScheduledCellId, msgOff);
    }
  else
    {
      // this cell
      enqueueTxStop(applyChanges, cellId);
    }

  // switch traffic ON on new cell
  if (cellId != mCellId)
    {
      // not this cell
      X2Message msgOn;
      msgOn.type = X2Message::changeScheduleModeInd;
      msgOn.mustSendTraffic = true;
      msgOn.applyDirectMembership = applyChanges;

      X2Channel::instance()->send(cellId, msgOn);
    }
  else
    {
      // this cell
      enqueueTx(applyChanges);
    }

  mLastScheduledCellId = cellId;
  mLastSwichTime = currentTime;
  // logging
  if (mCellSwitchCounter++ != 0)
    {
      mCellSwitchWatch.stop(mCellSwitchIndex, currentTime);
    }
  mCellSwitchWatch.start(mCellSwitchIndex, currentTime);
}


void FfMacScheduler::setTimeout(Time when)
{
  Event event { EventType::l2Timeout, when };
  event.cellId = mCellId;
  Simulator::instance()->scheduleEvent(event);
}

void FfMacScheduler::enqueueTx(Time start)
{
  mInternalEvents[start].push(SchedulerEvent::startTx);
  setTimeout(start);
}

void FfMacScheduler::enqueueTxStop(Time stop, int scheduledDirectCellId)
{
  mInternalEvents[stop].push(SchedulerEvent::stopTx);
  mScheduledDirectCellId[stop] = scheduledDirectCellId;
  setTimeout(stop);
}



void FfMacScheduler::processREChanges()
{
  if (SimTimeProvider::getTime() > mLastSwichTime + Converter::milliseconds(1))
    movingAverageDecisionAlgo();
}

// Algo examples

void FfMacScheduler::simpleDecisionAlgo()
{
  const Time currentTime = SimTimeProvider::getTime();
  if (currentTime > Converter::seconds(3) && currentTime < Converter::seconds(6) && mDirectParticipantCellId != 2)
    switchDirectCell(2);
  else if (currentTime > Converter::seconds(6) && mDirectParticipantCellId != 3)
    switchDirectCell(3);
}

void FfMacScheduler::movingAverageDecisionAlgo() // weighted
{
  int cellIdNext = mDirectParticipantCellId;

  double aveProbes = 0;
  std::map<int, double> signalLvl;
  for (const auto &csiPair : mCsiHistory)
    {
      const size_t len = csiPair.second.size();
      int64_t signalSum = 0;
      for (size_t i = 0; i < len; i++)
        {
          signalSum += (i + 1) * csiPair.second[i].second;
        }

      double aveSignal = (signalSum + 0.0) / ((1 + len) * len / 2); // WMA
      signalLvl.insert(std::make_pair(csiPair.first, aveSignal));

      aveProbes += csiPair.second.size();
    }

  aveProbes /= mCsiHistory.size();
  if (aveProbes < 3.0)
    return;

  double maxSignal = signalLvl[mDirectParticipantCellId];
  for (const auto &cellSignalPair : signalLvl)
    {
      if (cellSignalPair.second > maxSignal + 0.6)
        {
          maxSignal = cellSignalPair.second;
          cellIdNext = cellSignalPair.first;
        }
    }

  if (cellIdNext != mLastScheduledCellId)
    {
      LOG("@" << SimTimeProvider::getTime()
          << "  cell switch from " << mLastScheduledCellId << " to " << cellIdNext);
      switchDirectCell(cellIdNext);
    }
}

