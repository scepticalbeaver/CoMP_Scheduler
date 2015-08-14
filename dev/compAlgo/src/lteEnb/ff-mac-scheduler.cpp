#include "ff-mac-scheduler.h"

#include <algorithm>
#include <functional>

#include "x2-channel.h"

FfMacScheduler::FfMacScheduler(int cellId)
  : mCellId(cellId)
  , mIsLeader(false)
  , mIsDirectParticipant(false)
  , mWindowDuration(Converter::milliseconds(500))
  , mDirectParticipantCellId(-1)
  , mLeaderCellId(-1)
{
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
}

void FfMacScheduler::setFfMacSchedSapUser(FfMacSchedSapUser *user)
{
  mMacSapUser = user;
}

void FfMacScheduler::setTrafficActivity(bool mustSend)
{
  mIsDirectParticipant = mustSend;
  if (mIsDirectParticipant)
    {
      mDirectParticipantCellId = mCellId;
      mTxTrafficAfter = Converter::milliseconds(0);
    }
  else
    {
      mDirectParticipantCellId = -1;
      mTxTrafficUntil = Converter::milliseconds(0);
    }
  schedDlTriggerReq();
}

void FfMacScheduler::schedDlTriggerReq()
{
  Time currentTime = SimTimeProvider::getTime();
  bool canAlreadyTx = mIsDirectParticipant && currentTime >= mTxTrafficAfter;
  bool canStillTx = !mIsDirectParticipant && currentTime < mTxTrafficUntil;

  mMacSapUser->schedDlConfigInd(mCellId, {canAlreadyTx || canStillTx});
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
  if (array.back().first - array.front().first > mWindowDuration)
    array.pop_front();

  if (SimTimeProvider::getTime() > Converter::seconds(1))
    processREChanges();

  schedDlTriggerReq();
}

void FfMacScheduler::switchDirectCell(int cellId)
{
  // switch traffic OFF at old cell
  if (mDirectParticipantCellId != mCellId)
    {
      // not this cell
      X2Message msg;
      msg.type = X2Message::changeScheduleModeInd;
      msg.mustSendTraffic = false;

      X2Channel::instance()->send(mDirectParticipantCellId, msg);
    }
  else
    {
      // this cell
      mTxTrafficUntil =
          SimTimeProvider::getTime() + X2Channel::instance()->getLatency() - mMacSapUser->getMacToChannelDelay();
      mIsDirectParticipant = false;
    }


  // switch traffic ON on new cell
  if (cellId != mCellId)
    {
      // not this cell
      X2Message msgOn;
      msgOn.type = X2Message::changeScheduleModeInd;
      msgOn.mustSendTraffic = true;

      X2Channel::instance()->send(cellId, msgOn);
      mDirectParticipantCellId = cellId;
    }
  else
    {
      // this cell
      mTxTrafficAfter =
          SimTimeProvider::getTime() + X2Channel::instance()->getLatency() - mMacSapUser->getMacToChannelDelay() / 2;
      mIsDirectParticipant = true;
    }

  mDirectParticipantCellId = cellId;
}




void FfMacScheduler::processREChanges()
{
  simpleDecisionAlgo();
}


void FfMacScheduler::simpleDecisionAlgo()
{
  const Time currentTime = SimTimeProvider::getTime();
  if (currentTime > Converter::seconds(3) && currentTime < Converter::seconds(6) && mDirectParticipantCellId != 2)
    switchDirectCell(2);
  else if (currentTime > Converter::seconds(6) && mDirectParticipantCellId != 3)
    switchDirectCell(3);
}

void FfMacScheduler::movingAverageDecisionAlgo()
{
  int cellIdNext = mDirectParticipantCellId;
  int maxSignal = 0;
  for (const auto &csiPair : mCsiHistory)
    {
      auto signal = std::accumulate(csiPair.second.begin(), csiPair.second.end(), 0,
                                    [] (int acc, const CsiUnit &unit) { return acc + unit.second; } );
      if (signal > maxSignal)
        {
          cellIdNext = csiPair.first;
          maxSignal = signal;
        }
    }

  if (cellIdNext != mDirectParticipantCellId)
    switchDirectCell(cellIdNext);
}

