#pragma once

#include <string>

struct DlMacPacket
{
  std::string dlMacStatLine;
};

struct CSIMeasurementReport
{
  int targetCellId;
  int rsrp;
};

class L2Mac
{
public:
  L2Mac();

  void makeScheduleDecision(DlMacPacket packet);
  void recvMeasurementsReport(CSIMeasurementReport report);


};

