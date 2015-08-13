#include "ff-mac-scheduler.h"
#include "x2-channel.h"
#include "../simulator.h"

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
}

void FfMacScheduler::schedDlTriggerReq()
{
  Time currentTime = Simulator::instance()->getTime();
  bool canAlreadyTx = mIsDirectParticipant && currentTime > mTxTrafficAfter;
  bool canStillTx = !mIsDirectParticipant && currentTime < mTxTrafficUntil;

  mMacSapUser->schedDlConfigInd({canAlreadyTx || canStillTx});
}

void FfMacScheduler::schedDlCqiInfoReq(int tCellId, CsiUnit csi)
{
  if (!mIsLeader && mLeaderCellId != -1)
    {
      CSIMeasurementReport measReport;
      measReport.csi = csi;
      measReport.targetCellId = tCellId;

      X2Message msg;
      msg.type = X2Message::measuresInd;
      msg.report = measReport;

      X2Channel::instance()->send(mLeaderCellId, msg);
      return;
    }

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

  // todo: make something cool algo here
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
      mTxTrafficUntil = Simulator::instance()->getTime() + X2Channel::instance()->getLatency() / 2;
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
      mTxTrafficAfter = Simulator::instance()->getTime() + X2Channel::instance()->getLatency();
      mIsDirectParticipant = true;
    }

  mDirectParticipantCellId = cellId;
}
