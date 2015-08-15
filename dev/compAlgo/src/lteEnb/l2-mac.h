#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "../helpers.h"
#include "ff-mac-scheduler.h"
#include "ff-mac-sched-sap.h"

class L2Mac
{
public:
  L2Mac();
  ~L2Mac();

  void activateDlCompFeature();

  void makeScheduleDecision(int cellId, const DlRlcPacket &packet);
  void recvMeasurementsReport(int cellId, const CSIMeasurementReport &report);
  void recvX2Message(int cellId, const X2Message &message);
  void l2Timeout(int cellId);

private:
  FfMacSchedSapUser *mMacSapUser;

  const int compMembersCount = 3;

  std::vector<FfMacScheduler> mSchedulers;
  TimeMeasurement mTimeMeasurement;
  std::fstream mResultRlcStats;
  std::fstream mResultMeasurements;
  size_t mMissedFrameCounter = 0;

  L2Mac(const L2Mac &) = delete;
  L2Mac& operator=(const L2Mac &) = delete;

  void printMacTimings();
};

