#pragma once

#include <map>
#include <queue>

#include "../helpers.h"
#include "ff-mac-sched-sap.h"


class FfMacScheduler
{
public:
  FfMacScheduler(int cellId);
  ~FfMacScheduler();
  void setLeader(int cellId);
  void setFfMacSchedSapUser(FfMacSchedSapUser *user);

  void setTrafficActivity(bool mustSend, Time applyTime);

  void schedDlTriggerReq();
  void schedDlCqiInfoReq(int tCellId, CsiUnit csi);

  void onTimeout();

private:
  int const mCellId;
  bool mIsLeader;
  Time mWindowDuration;
  int mDirectParticipantCellId;
  bool mIsDirectParticipant;
  int mLeaderCellId;

  using CsiArray = std::deque<CsiUnit>;
  std::map<int, CsiArray> mCsiHistory;

  FfMacSchedSapUser *mMacSapUser = nullptr;

  TimeMeasurement mCellSwitchWatch;
  size_t mCellSwitchCounter = 0;
  const std::string mCellSwitchIndex = "switchDirectCell";

  void setTimeout(Time when);
  enum class SchedulerEvent
  {
    updateDciDecision
    , startTx
    , stopTx
  };

  std::map<Time, std::queue<SchedulerEvent>> mInternalEvents;
  std::map<Time, int> mScheduledDirectCellId;
  int mLastScheduledCellId = mCellId;
  Time mLastSwichTime = Converter::milliseconds(0);

  void enqueueTx(Time start);
  void enqueueTxStop(Time stop, int scheduledDirectCellId);



  void processREChanges();
  void switchDirectCell(int cellId);

  void simpleDecisionAlgo();
  void movingAverageDecisionAlgo();
};




