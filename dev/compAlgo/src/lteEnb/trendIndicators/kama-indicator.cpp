#include "kama-indicator.h"

#include <math.h>
#include <algorithm>

KamaIndicator::KamaIndicator(CsiJournalPtr j)
  : ITrendIndicator(j)
{
    mWindowSize = std::max(n, s + 1); // at start
}


double KamaIndicator::updateHook(CellId cellId)
{
  const auto &array = mCsiJournal->at(cellId);
  double result =  calcAMA(array, cellId);
  updateFilter(cellId);
  return result;
}

double KamaIndicator::calcAMA(const CsiArray &csiArray, CellId cellId)
{
  const int closeSize = csiArray.size();
  const int left = std::max(closeSize - n, 0);

  const auto direction = std::abs(csiArray.back().second - csiArray[left].second);

  // volatility
  const int realCount = closeSize - left - 1;
  int volatility = 0;
  for (int i = 0; i < realCount - 1; i++)
    volatility += std::abs(csiArray[closeSize - i].second - csiArray[closeSize - i - 1].second);


  mLatestEfficiencyRatio = (volatility)? double(direction) / volatility : 1;

  const auto fastest = 2 / double(f + 1);
  const auto slowest = 2 / double(s + 1);
  const auto smooth = mLatestEfficiencyRatio * (fastest - slowest) + slowest;

  const auto amaCoeff = smooth * smooth;

  return amaCoeff * csiArray.back().second + (1.0 - amaCoeff) * lastValueFor(cellId);
}

void KamaIndicator::updateFilter(CellId cellId)
{
  const auto &dArray = mWValuesDiffs[cellId];
  if (!dArray.size())
    return;
  const auto dLeft = std::max(int64_t(dArray.size()) - n, int64_t(0));
  const auto count = dArray.size() - dLeft;
  const double expectedValue = 1 / count * std::accumulate(dArray.begin() + dLeft, dArray.end(), 0.0);

  const double stdDev = std::sqrt(1 / count * std::accumulate(dArray.begin() + dLeft, dArray.end(), 0.0,
      [&expectedValue] (double init, double val)
      {
        return init + std::pow(val - expectedValue, 2);
      }));

  const double magicK = 0.2;
  mLatestFilter = magicK * stdDev;
}

double KamaIndicator::minAmaLatest(CellId cellId)
{
  return minmaxAmaLatest(cellId, true);
}

double KamaIndicator::maxAmaLatest(CellId cellId)
{
  return minmaxAmaLatest(cellId, false);
}

double KamaIndicator::minmaxAmaLatest(CellId cellId, bool useMin)
{
  const auto &array = mWeightedSignals[cellId];
  const auto &deltaArray = mWValuesDiffs[cellId];
  const auto size = array.size();
  const auto dsize = deltaArray.size();

  auto left = std::max(int64_t(size) - s, int64_t(0));
  auto dleft = std::max(int64_t(dsize) - s + 1, int64_t(0));

  std::function<bool(double, double)> compBeforeZero;
  if (useMin)
    compBeforeZero = std::less<double>();
  else
    compBeforeZero = std::greater<double>();

  std::function<bool(double, double)> compAfterZero;
  if (useMin)
    compAfterZero = std::greater<double>();
  else
    compAfterZero = std::less<double>();

  size_t offset = 0;
  for (size_t i = dleft; i < dsize - 1; i++)
    {
      if (compBeforeZero(deltaArray[i], 0.0) && compAfterZero(deltaArray[i + 1], 0.0))
        offset = i - dleft;
    }

  auto result = std::min_element(array.begin() + left + offset, array.end());
  return (result != array.end())? *result : array.back();
}


bool KamaIndicator::isUpgoingTrend(CellId cellId)
{
  return lastValueFor(cellId) - minAmaLatest(cellId) > mLatestFilter;
}

bool KamaIndicator::isDescendingTrend(CellId cellId)
{
  return maxAmaLatest(cellId) - lastValueFor(cellId) > mLatestFilter;
}
