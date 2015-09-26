#include "wma-indicator.h"

#include <algorithm>

WmaIndicator::WmaIndicator(CsiJournalPtr j)
  : ITrendIndicator("wma-ind", j)
{

  switch (SimConfig::algoType)
    {
    case SimConfig::smmRaw:
      mCalcMaFunc = calcSMM;
      break;
    default:
      mCalcMaFunc = calcWMA;
    }
  mWindowDuration = Converter::milliseconds(SimConfig::wmaSmmDuration);
}


double WmaIndicator::updateHook(CellId cellId)
{
  auto &array = mCsiJournal->at(cellId);
  size_t lPointer = 0;
  const size_t size = array.size();
  while (lPointer < size && array[lPointer].first < array.back().first - mWindowDuration)
    lPointer++;

  return mCalcMaFunc(array, lPointer);
}



bool WmaIndicator::isLastOutlier(CellId cellId, size_t lPointer)
{
  const double order = 2.0;
  CsiArray array = mCsiJournal->at(cellId);
  const size_t size = array.size();
  if (size <= 2)
    return false;

  int k = (size % 2 == 0)? size / 2 + 1 : size / 2;
  std::vector<CsiUnit> values(array.begin() + lPointer, array.end());
  std::nth_element(values.begin(), values.begin() + k, values.end(), [] (const CsiUnit &l, const CsiUnit &r)
  {
      return l.second < r.second;
    });
  const auto median = values[k].second;

  std::vector<int> diffs(size);
  for (size_t i = lPointer; i < size; i++)
    {
      diffs[i] = std::abs(array[i].second - median);
    }

  std::nth_element(diffs.begin(), diffs.begin() + k, diffs.end());
  const double diffsMedian = diffs[k];

  return diffs.back() / diffsMedian > order;
}

double WmaIndicator::calcWMA(const CsiArray &csiArray, size_t lPointer)
{
  const size_t len = csiArray.size() - lPointer;
  int64_t signalSum = 0;
  for (size_t i = 0; i < len; i++)
    {
      signalSum += (i + 1) * csiArray[i + lPointer].second;
    }

  return (signalSum + 0.0) / ((1 + len) * len / 2); // WMA
}

double WmaIndicator::calcSMM(const CsiArray &csiDeque, size_t lPointer) // simple moving median
{
  auto csiArray = csiDeque;
  const size_t len = csiArray.size() - lPointer;

  const int k = len / 2 + lPointer;
  std::nth_element(csiArray.begin() + lPointer, csiArray.begin() + k, csiArray.end(),
                   [] (const CsiUnit &l, const CsiUnit &r)
  {
      return l.second < r.second;
    });

  return csiArray[k].second;
}

