#pragma once

#include "ma-comp-algo.h"

//! @class KamaCompAlgo is Kaufman's Adaptive Moving Average algorithm
class KamaCompAlgo : public MaCompAlgo
{
public:
  KamaCompAlgo(CsiJournal* j);

  virtual CellId redefineBestCell(CellId lastScheduled) override;

protected:

  double calcAMA(const CsiArray &csiArray, CellId cellId);

  const int n = 10; // window size for efficincy ratio calculation
  const int f = 2; // window for fast moving average
  const int s = 5; // window for slow MA



};


