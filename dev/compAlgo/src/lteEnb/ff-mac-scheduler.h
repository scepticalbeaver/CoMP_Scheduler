#pragma once

#include <initializer_list>

#include "../helpers.h"
#include "ff-mac-sched-sap.h"
#include "comp-decision-algo.h"


class FfMacScheduler
{
public:
  FfMacScheduler(CellId cellId);
  FfMacScheduler(FfMacScheduler &&scheduler);
  ~FfMacScheduler();
  void setLeader(CellId cellId);
  void setCompGroup(std::initializer_list<CellId> list);
  void setFfMacSchedSapUser(FfMacSchedSapUser *user);

  void setTrafficActivity(bool mustSend, Time applyTime);

  void schedDlTriggerReq();
  void schedDlCqiInfoReq(CellId tCellId, CsiUnit csi);

  void onTimeout();

private:
  int const mCellId;
  bool mIsLeader;
  int mDirectParticipantCellId;
  bool mIsDirectParticipant;
  int mLeaderCellId;

  CellIdVectorPtr mCompGroup = std::make_shared<CellIdVector>();

  using CsiArray = std::deque<CsiUnit>;
  CsiJournalPtr mCsiHistory = std::make_shared<CsiJournal>();

  FfMacSchedSapUser *mMacSapUser = nullptr;

  UniqCompSchedulingAlgo mCompAlgo;

  void setTimeout(Time when);
  enum class SchedulerEvent
  {
    updateDciDecision
    , startTx
    , stopTx
  };

  std::map<Time, std::queue<SchedulerEvent>> mInternalEvents;
  std::map<Time, int> mScheduledDirectCellId;
  int mLastScheduledCellId;
  Time mLastSwichTime = Converter::milliseconds(0);


  //- Logging stuff -----------------------------------------
  TimeMeasurement mlCellSwitchWatch;
  size_t mlCellSwitchCounter = 0;
  const std::string mlCellSwitchIndex = "switchDirectCell";

  std::map<int, std::pair<size_t, uint64_t>> mlHistoryLenCounter; //< (value history len sum, number of probes)


  //----------------------------------------------------------

  void enqueueTx(Time start);
  void enqueueTxStop(Time stop, int scheduledDirectCellId);


  void processREChanges();
  void switchDirectCell(int cellId);
};




