#include "ff-mac-scheduler.h"

#include <algorithm>
#include <functional>

#include "x2-channel.h"
#include "../simulator.h"
#include "wma-comp-algo.h"

FfMacScheduler::FfMacScheduler(CellId cellId)
  : mCellId(cellId)
  , mIsLeader(false)
  , mWindowDuration(Converter::milliseconds(20))
  , mDirectParticipantCellId(-1)
  , mIsDirectParticipant(false)
  , mLeaderCellId(-1)
  , mCompAlgo(new WMACompAlgo(&mCsiHistory))
  , mLastScheduledCellId(cellId)
{
}

FfMacScheduler::FfMacScheduler(FfMacScheduler &&scheduler)
  : mCellId(scheduler.mCellId)
  , mIsLeader(scheduler.mIsLeader)
  , mWindowDuration(scheduler.mWindowDuration)
  , mDirectParticipantCellId(scheduler.mDirectParticipantCellId)
  , mIsDirectParticipant(scheduler.mIsDirectParticipant)
  , mLeaderCellId(scheduler.mLeaderCellId)
  , mCsiHistory(std::move(scheduler.mCsiHistory))
  , mMacSapUser(scheduler.mMacSapUser)
  , mCompAlgo(std::move(scheduler.mCompAlgo))
  , mInternalEvents(std::move(scheduler.mInternalEvents))
  , mScheduledDirectCellId(scheduler.mScheduledDirectCellId)
  , mLastScheduledCellId(scheduler.mLastScheduledCellId)
  , mlCellSwitchWatch(scheduler.mlCellSwitchWatch)
  , mlHistoryLenCounter(std::move(scheduler.mlHistoryLenCounter))
{
  mCompAlgo->setJournal(&mCsiHistory);
}

FfMacScheduler::~FfMacScheduler()
{
  if (mIsLeader || mlCellSwitchCounter)
    {
      LOG("MAC scheduler cell-switch statistics:");
      LOG("\tIntervals between switch [ms]:");

      LOG("\tave: " << mlCellSwitchWatch.average(mlCellSwitchIndex) / 1000.0
          << "\tmin: "<< mlCellSwitchWatch.minimum(mlCellSwitchIndex) / 1000.0
          << "\tmax: " << mlCellSwitchWatch.maximum(mlCellSwitchIndex) / 1000.0
          << "\tTotal switches: " << mlCellSwitchCounter);


      LOG("\tCell measurements history length on decision moment:\n\taverage per cell:");
      for (auto &cellHistoryStats : mlHistoryLenCounter)
        {
          double average = cellHistoryStats.second.first / (cellHistoryStats.second.second + 0.0);
          LOG("\tid = " << cellHistoryStats.first << " : " << average);
        }
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
  if (mlCellSwitchCounter++ != 0)
    {
      mlCellSwitchWatch.stop(mlCellSwitchIndex, currentTime);
    }
  mlCellSwitchWatch.start(mlCellSwitchIndex, currentTime);
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
  if (SimTimeProvider::getTime() < mLastSwichTime + Converter::milliseconds(1))
    return;

  for (const auto &cellHistory : mCsiHistory)
    {
      const auto sumOfLen = mlHistoryLenCounter[cellHistory.first].first + cellHistory.second.size();
      const auto counts = mlHistoryLenCounter[cellHistory.first].second + 1;
      mlHistoryLenCounter[cellHistory.first] = std::make_pair(sumOfLen, counts);
    }

  int cellIdNext = mCompAlgo->redefineBestCell(mDirectParticipantCellId);

  if (cellIdNext != mLastScheduledCellId)
    {
      DEBUG("@" << SimTimeProvider::getTime()
          << "  cell switch from " << mLastScheduledCellId << " to " << cellIdNext);
      switchDirectCell(cellIdNext);
    }
}

