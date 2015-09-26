#include "interpolation-indicator.h"

InterpolationIndicator::InterpolationIndicator(CsiJournalPtr j, Method type)
  : ITrendIndicator("interpolation-ind", j)
  , mInterpolationType(type)
{
  mWindowSize = SimConfig::approxAlgoWindowSize;
}

double InterpolationIndicator::updateHook(CellId cellId)
{
  return forecast(cellId);
}

double InterpolationIndicator::forecastLagrange(CellId cellId)
{
  CsiArray &data = mCsiJournal->at(cellId);
  const int64_t dataSize = data.size();
  if (dataSize <= 1)
    return data.back().second;

  const auto left = std::max(int64_t(dataSize - mWindowSize), int64_t(0));

  double x = data.back().first - data[dataSize - 2].first;
  if (dataSize > 2)
    x = 0.5 * (x + data[dataSize - 2].first - data[dataSize - 3].first);

  x = data.back().first + SimConfig::approxAlgoXOffset;

  double result = 0.0;
  for (auto j = left; j < dataSize; j++)
    {
      double product = 1.0;
      for (auto m = left; m < dataSize; m++)
        {
          if (m == j)
            continue;
          product *= (x - double(data[m].first)) / double(data[j].first - data[m].first);
        }
      result += data[j].second * product;
    }
  return (result + data.back().second) / 2.0;
}

double InterpolationIndicator::forecast(CellId cellId)
{
  return forecastLagrange(cellId);
}
