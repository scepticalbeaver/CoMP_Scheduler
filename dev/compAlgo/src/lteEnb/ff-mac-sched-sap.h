#pragma once

#include <map>
#include <queue>



class FfMacSchedSapUser
{
public:
  FfMacSchedSapUser()
    : mParams({false})
  {}

  struct SchedDlConfigIndParameters
  {
    bool dciDecision;
  };

  void schedDlConfigInd (const SchedDlConfigIndParameters params)
  {
    mParams = params;
  }

  bool getDciDecision()
  {
    return mParams.dciDecision;
  }


private:
  //typedef
  SchedDlConfigIndParameters mParams;
  //std::map<int, SchedDlConfigIndParameters> mParams;
};
