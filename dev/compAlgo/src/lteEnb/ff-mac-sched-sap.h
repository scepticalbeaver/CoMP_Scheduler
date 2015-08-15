#pragma once

#include <map>
#include <deque>

#include "../helpers.h"


class FfMacSchedSapUser
{
public:
  FfMacSchedSapUser() = default;

  struct SchedDlConfigIndParameters
  {
    bool dciDecision;
  };

  void schedDlConfigInd (int cellId, const SchedDlConfigIndParameters &params);

  bool getDciDecision(int cellId, bool peek = false);
  int getDirectCellId(); //< for debugging purpose

  bool peekDciDecision(int cellId);

  Time getMacToChannelDelay() const;

private:
  const Time macToChannelDelay = Converter::milliseconds(1);
  using SchedulerDecisions = std::deque<std::pair<Time, SchedDlConfigIndParameters>>;

  std::map<int, SchedulerDecisions> mDecisions;
};
