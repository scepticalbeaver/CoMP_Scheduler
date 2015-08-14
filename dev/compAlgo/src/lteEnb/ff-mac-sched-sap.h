#pragma once

#include <map>
#include <queue>

#include "../helpers.h"


class FfMacSchedSapUser
{
public:
  FfMacSchedSapUser()
  {}

  struct SchedDlConfigIndParameters
  {
    bool dciDecision;
  };

  void schedDlConfigInd (int cellId, const SchedDlConfigIndParameters params)
  {
    mDecisions[cellId].push(std::make_pair(SimTimeProvider::getTime() + macToChannelDelay, params));
  }

  bool getDciDecision(int cellId)
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


private:
  const Time macToChannelDelay = Converter::milliseconds(1);
  using SchedulerDecisions = std::queue<std::pair<Time, SchedDlConfigIndParameters>>;

  std::map<int, SchedulerDecisions> mDecisions;
};
