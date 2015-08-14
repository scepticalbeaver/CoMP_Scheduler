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

  void makeScheduleDecision(int cellId, const DlMacPacket &packet);
  void recvMeasurementsReport(int cellId, const CSIMeasurementReport &report);
  void recvX2Message(int cellId, const X2Message &message);

private:
  FfMacSchedSapUser *mMacSapUser;

  const int compMembersCount = 3;

  std::vector<FfMacScheduler> mSchedulers;
  RealtimeMeasurement mTimeMeasurement;
  std::fstream mResultMacStats;
  std::fstream mResultMeasurements;

  void printMacTimings();
};

