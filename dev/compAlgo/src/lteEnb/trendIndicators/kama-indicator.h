#pragma once

#include "itrend-indicator.h"

//! @class KamaIndicator is Kaufman's Adaptive Moving Average algorithm
class KamaIndicator : public ITrendIndicator
{
public:
  KamaIndicator(CsiJournalPtr j);

  //! @brief efficiencyRatio shows either market is more volatile or trend
  //! @return ER = 0 if absolutely volatile, 1 for stable situation
  double efficiencyRatio() const { return mLatestEfficiencyRatio; }

  bool isUpgoingTrend(CellId cellId) override;
  bool isDescendingTrend(CellId cellId) override;

private:
  const int n = SimConfig::kamaN; // window size for efficincy ratio calculation
  const int f = SimConfig::kamaF; // window for fast moving average
  const int s = SimConfig::kamaS; // window for slow MA

  double mLatestEfficiencyRatio = 1;
  double mLatestFilter = 0;

  double updateHook(CellId cellId) override;

  double calcAMA(const CsiArray &csiArray, CellId cellId);

  void updateFilter(CellId cellId);

  double minAmaLatest(CellId cellId);
  double maxAmaLatest(CellId cellId);

  double minmaxAmaLatest(CellId cellId, bool useMin);
};

using UniqKamaIndicator = std::unique_ptr<KamaIndicator>;
