#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "x2-channel.h"
#include "ff-mac-scheduler.h"

class L2Mac
{
public:
  L2Mac();
  ~L2Mac();

  void makeScheduleDecision(int cellId, const DlMacPacket &packet);
  void recvMeasurementsReport(int cellId, const CSIMeasurementReport &report);
  void recvX2Message(int cellId, const X2Message &message);

private:
  FfMacSchedSapUser *mMacSapUser;

  const int compMembersCount = 3;

  std::vector<FfMacScheduler> mSchedulers;
  RealtimeMeasurement mTimeMeasurement;
  std::fstream mResultMacStats;

  void printMacTimings();
};

