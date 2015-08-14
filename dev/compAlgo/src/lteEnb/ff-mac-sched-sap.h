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

  void schedDlConfigInd (int cellId, const SchedDlConfigIndParameters params);

  bool getDciDecision(int cellId);

  Time getMacToChannelDelay() const;

private:
  const Time macToChannelDelay = Converter::milliseconds(1);
  using SchedulerDecisions = std::queue<std::pair<Time, SchedDlConfigIndParameters>>;

  std::map<int, SchedulerDecisions> mDecisions;
};
