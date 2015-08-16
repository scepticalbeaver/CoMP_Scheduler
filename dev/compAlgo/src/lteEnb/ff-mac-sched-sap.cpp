#include "ff-mac-sched-sap.h"

#include <assert.h>

void FfMacSchedSapUser::schedDlConfigInd(int cellId, const SchedDlConfigIndParameters &params)
{
  mDecisions[cellId].push_back(std::make_pair(SimTimeProvider::getTime() + macToChannelDelay, params));
  if (mDecisions[cellId].size() < 10)
    return;

  const Time currentTime = SimTimeProvider::getTime();
  SchedulerDecisions& decisions = mDecisions[cellId];
  while (decisions.size() >= 2 && decisions[0].first < currentTime && decisions[1].first < currentTime)
    decisions.pop_front();
}

bool FfMacSchedSapUser::getDciDecision(int cellId, bool peek)
{
  bool lastAvailableDecision = false;
  const Time currentTime = SimTimeProvider::getTime();

  const auto copyOfCellDecisions = mDecisions[cellId];

  SchedulerDecisions& decisions = mDecisions[cellId];
  while(!decisions.empty() && decisions.front().first <= currentTime)
    {
      lastAvailableDecision = decisions.front().second.dciDecision;
      if (decisions.size() > 1)
        {
          decisions.pop_front(); // next decisions will be without delay
        }
      else
        {
          break;
        }
    }

  if (peek)
    mDecisions[cellId] = copyOfCellDecisions;

  return lastAvailableDecision;
}

int FfMacSchedSapUser::getDirectCellId()
{
  const bool peek = true;
  std::vector<int> decisions {getDciDecision(1, peek), getDciDecision(2, peek), getDciDecision(3, peek)};
  int cellId = -1;
  for (unsigned int i = 0; i < decisions.size(); i++)
    {
      int decision = decisions[i];
      if (!decision)
        continue;
      if (cellId == -1)
        cellId = i;
      else
        {
          ERR("@" << SimTimeProvider::getTime() << "\tASSERT:\tDual transmission");
        }
    }
  return cellId;
}

bool FfMacSchedSapUser::peekDciDecision(int cellId)
{
  SchedulerDecisions& decisions = mDecisions[cellId];
  return (decisions.empty())? false : decisions.front().second.dciDecision;
}

Time FfMacSchedSapUser::getMacToChannelDelay() const
{
  return macToChannelDelay;
}
