#include "kama-comp-algo.h"

#include <functional>

KamaCompAlgo::KamaCompAlgo(CsiJournal *j)
  : MaCompAlgo(j)
{
  mCalcMaFunc = std::bind(&KamaCompAlgo::calcAMA, this, std::placeholders::_1, std::placeholders::_2);
  mWindowSize = s + 1; // at start
}

CellId KamaCompAlgo::redefineBestCell(CellId lastScheduled)
{
  CellId nextDecision = lastScheduled;
  double maxSignal = mWeightedSignalsJournal[lastScheduled].back();
  for (auto &pair : mWeightedSignalsJournal)
    {
      if (pair.second.back() > maxSignal)
        {
          maxSignal = pair.second.back();
          nextDecision = pair.first;
        }
    }
  return nextDecision;
}



double KamaCompAlgo::calcAMA(const CsiArray &csiArray, CellId cellId)
{
  const int closeSize = csiArray.size();
  const int left = std::max(closeSize - n, 0);

  const auto direction = std::abs(csiArray.back().second - csiArray[left].second);

  // volatility
  const int realCount = closeSize - left - 1;
  int volatility = 0;
  for (int i = 0; i < realCount - 1; i++)
    volatility += std::abs(csiArray[closeSize - i].second - csiArray[closeSize - i - 1].second);


  const double efficiencyRatio = (volatility)? double(direction) / volatility : 1;

  const auto fastest = 2 / double(f + 1);
  const auto slowest = 2 / double(s + 1);
  const auto smooth = efficiencyRatio * (fastest - slowest) + slowest;

  const auto amaCoeff = smooth * smooth;

  return amaCoeff * csiArray.back().second + (1.0 - amaCoeff) * lastWeightedValue(cellId);
}
