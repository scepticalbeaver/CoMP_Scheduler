#pragma once


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
  SchedDlConfigIndParameters mParams;
};
