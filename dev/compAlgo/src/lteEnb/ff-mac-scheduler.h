#pragma once

#include <map>
#include <deque>

#include "../helpers.h"
#include "ff-mac-sched-sap.h"


class FfMacScheduler
{
public:
  FfMacScheduler(int cellId, bool isLeader);
  void setLeader(bool isLeader);
  void setFfMacSchedSapUser(FfMacSchedSapUser *user);

  void setTrafficActivity(bool mustSend);

  void schedDlTriggerReq();
  void schedDlCqiInfoReq(int tCellId, CsiUnit csi);

private:
  int const mCellId;
  bool mIsLeader;
  bool mIsDirectParticipant;
  Time mWindowDuration;
  int mDirectParticipantCellId;

  Time mTxTrafficUntil = Converter::milliseconds(0);
  Time mTxTrafficAfter = Converter::milliseconds(0);

  using CsiArray = std::deque<CsiUnit>;
  std::map<int, CsiArray> mCsiHistory;

  FfMacSchedSapUser *mMacSapUser = nullptr;

  void switchDirectCell(int cellId);
};




