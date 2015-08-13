#include "l2-mac.h"

L2Mac::L2Mac()
  : mMacSapUser(new FfMacSchedSapUser)
{
}

L2Mac::~L2Mac()
{
  delete mMacSapUser;
}

void L2Mac::makeScheduleDecision(int cellId, DlMacPacket packet)
{

}

void L2Mac::recvMeasurementsReport(int cellId, CSIMeasurementReport report)
{

}

void L2Mac::recvX2Message(int cellId, X2Message message)
{

}
