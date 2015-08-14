#include "l2-mac.h"

#include "x2-channel.h"

L2Mac::L2Mac()
{
  X2Channel::instance()->configurate(compMembersCount);
  mMacSapUser = new FfMacSchedSapUser;
  for (int i = 0; i < compMembersCount; i++)
    {
      mSchedulers.push_back(FfMacScheduler(i + 1));
      mSchedulers.back().setFfMacSchedSapUser(mMacSapUser);
    }

  std::string resultLocation = "../../../output/DlMacStats.txt";
  mResultMacStats.open(resultLocation, std::ios_base::out | std::ios_base::trunc);
  mResultMacStats << "% time	cellId	IMSI	frame	sframe	RNTI	mcsTb1	sizeTb1	mcsTb2	sizeTb2\n";
}

L2Mac::~L2Mac()
{
  delete mMacSapUser;
  mResultMacStats.flush();
  mResultMacStats.close();

  printMacTimings();
}

void L2Mac::activateDlCompFeature()
{
  mSchedulers.front().setLeader(true);
  for (auto &scheduler : mSchedulers)
    {
      scheduler.schedDlTriggerReq();
    }
}

void L2Mac::makeScheduleDecision(int cellId, const DlMacPacket &packet)
{
  if (mMacSapUser->getDciDecision(cellId))
    {
      mResultMacStats << packet.dlMacStatLine << "\n";
    }
}

void L2Mac::recvMeasurementsReport(int cellId, const CSIMeasurementReport &report)
{
  const std::string fname = "recvMeasurementsReport" + std::to_string(cellId);
  mTimeMeasurement.start(fname);

  mSchedulers[cellId - 1].schedDlCqiInfoReq(report.targetCellId, report.csi);

  mTimeMeasurement.stop(fname);
}

void L2Mac::recvX2Message(int cellId, const X2Message &message)
{
  switch (message.type)
    {
    case X2Message::changeScheduleModeInd:
      {
        mSchedulers[cellId].setTrafficActivity(message.mustSendTraffic);
        break;
      }
    case X2Message::measuresInd:
    {
        recvMeasurementsReport(cellId, message.report);
        break;
      }
    case X2Message::leadershipInd:
      {
        mSchedulers[cellId].setLeader(message.leaderCellId);
        break;
      }
    }
}

void L2Mac::printMacTimings()
{
  LOG("Mac simulation statistics:");
  LOG("\tSchedDlTriggerReq timings [us]:");
  for (int i = 0; i < compMembersCount; i++)
    {
      const std::string index = "makeScheduleDecision" + std::to_string(i + 1);
      LOG("\tcellId = " << i + 1
          << "\tave: " << mTimeMeasurement.average(index) << "\tmin: "<< mTimeMeasurement.minimum(index)
          << "\tmax: " << mTimeMeasurement.maximum(index));
    }

  LOG("\tSchedDlCqiInfoReq + SchedDlTriggerReq  timings [us]:");
  for (int i = 0; i < compMembersCount; i++)
    {
      const std::string index = "recvMeasurementsReport" + std::to_string(i + 1);
      LOG("\tcellId = " << i + 1
          << "\tave: " << mTimeMeasurement.average(index) << "\tmin: "<< mTimeMeasurement.minimum(index)
          << "\tmax: " << mTimeMeasurement.maximum(index));
    }
}
