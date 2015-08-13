#pragma once

#include <string>
#include <vector>

#include "x2-channel.h"
#include "ff-mac-scheduler.h"

class L2Mac
{
public:
  L2Mac();
  ~L2Mac();

  void makeScheduleDecision(int cellId, DlMacPacket packet);
  void recvMeasurementsReport(int cellId, CSIMeasurementReport report);
  void recvX2Message(int cellId, X2Message message);


private:
  FfMacSchedSapUser *mMacSapUser;

  std::vector<FfMacScheduler> mShedulers;

};

