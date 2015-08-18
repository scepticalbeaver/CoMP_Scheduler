#pragma once

#include <algorithm>
#include <functional>

#include "icomp-decision-algo.h"


class MaCompAlgo : public ICompSchedulingAlgo
{
public:
  enum MovingAverageAlgo
  {
    weightedMovingAverage
    , simpleMovingMedian
  };

  MaCompAlgo(CsiJournal* j, MovingAverageAlgo type);
  virtual ~MaCompAlgo() {}

  virtual void update(CellId cellId) override;
  virtual CellId redefineBestCell(CellId lastScheduled) override;

protected:
  MaCompAlgo(CsiJournal* j);
  MaCompAlgo(const MaCompAlgo&) = delete;
  MaCompAlgo& operator=(const MaCompAlgo&) = delete;

  std::map<CellId, std::deque<double>> mWeightedSignalsJournal;
  std::function<double (const CsiArray&, CellId)> mCalcMaFunc;

  virtual CellId makeForecast(CellId lastScheduled);

  void updateWeightedJournal(CellId cellId, double value);
  double lastWeightedValue(CellId cellId);
  bool haveTooLittleValues();



  static double calcWMA(const CsiArray &csiArray, CellId);
  static double calcSMM(const CsiArray &csiDeque, CellId);
};


