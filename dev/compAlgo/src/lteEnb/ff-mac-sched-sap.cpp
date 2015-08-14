#include "ff-mac-sched-sap.h"




void FfMacSchedSapUser::schedDlConfigInd(int cellId, const FfMacSchedSapUser::SchedDlConfigIndParameters params)
{
  mDecisions[cellId].push(std::make_pair(SimTimeProvider::getTime() + macToChannelDelay, params));
}

bool FfMacSchedSapUser::getDciDecision(int cellId)
{
  bool lastAvailableDecision = false;
  const Time currentTime = SimTimeProvider::getTime();
  SchedulerDecisions& decisions = mDecisions[cellId];
  while(!decisions.empty() && decisions.front().first <= currentTime)
    {
      lastAvailableDecision = decisions.front().second.dciDecision;
      if (decisions.size() > 1)
        {
          decisions.pop(); // next decisions will be without delay
        }
      else
        {
          break;
        }
    }

  return lastAvailableDecision;
}

Time FfMacSchedSapUser::getMacToChannelDelay() const
{
  return macToChannelDelay;
}
