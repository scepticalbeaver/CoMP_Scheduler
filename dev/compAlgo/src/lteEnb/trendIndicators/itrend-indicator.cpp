#include "itrend-indicator.h"


ITrendIndicator::ITrendIndicator(CsiJournalPtr j)
  : mCsiJournal(j)
{
}

void ITrendIndicator::update(CellId cellId)
{
  if (mIsShadowValueUsed)
    {
      if (!mWeightedSignals[cellId].empty())
        mWeightedSignals[cellId].pop_back();
      if (!mWValuesDiffs[cellId].empty())
        mWValuesDiffs[cellId].pop_front();
      if (!mSignalDiffs[cellId].empty())
        mSignalDiffs[cellId].pop_front();
      mIsShadowValueUsed = false;
    }

  updateSignalDiffs(cellId);
  auto value = updateHook(cellId);
  updateWeightedJournal(cellId, value);
  updateWeightedValuesDiffs(cellId);

  if (mApplyAnalysOnForecast)
    {
      if (!mIsShadowValueUsed)
        {
          // add shadow value
          auto shadowVal = forecast(cellId);
          mWeightedSignals[cellId].push_back(shadowVal);
          updateWeightedValuesDiffs(cellId);
          mSignalDiffs[cellId].push_back(mWValuesDiffs[cellId].back());
          mIsShadowValueUsed = true;
        }
      else
        {
          assert(false);
        }
    }
}

double ITrendIndicator::lastValueFor(CellId cellId)
{
  if (mWeightedSignals[cellId].empty())
    {
      if (mCsiJournal->at(cellId).size())
        return mCsiJournal->at(cellId).back().second;
      else
        return 0.0;
    }
  return mWeightedSignals[cellId].back();
}

double ITrendIndicator::forecast(CellId cellId)
{
  double delta = 0.0;
  if (isFadingTrend(cellId) || isRisingTrend(cellId))
    delta = 1.2 * mWValuesDiffs[cellId].back();
  else
    {
      const auto size = mWValuesDiffs[cellId].size();
      if (size >= 2)
        delta = 0.5 * (mWValuesDiffs[cellId].back() + mWValuesDiffs[cellId][size - 2]);
      else if (size == 1)
        delta = 0.7 * mWValuesDiffs[cellId].back();
      else if (!size)
        delta = 0;
    }

  return lastValueFor(cellId) + delta;
}

bool ITrendIndicator::isUpgoingTrend(CellId cellId)
{
  return isUpgoingTrendWeighted(cellId, std::greater<double>(), crossHysteresis / 2);
}

bool ITrendIndicator::isDescendingTrend(CellId cellId)
{
  return isUpgoingTrendWeighted(cellId, std::less<double>(), -crossHysteresis / 2);
}

bool ITrendIndicator::isFadingTrend(CellId cellId, bool useFading)
{
  bool fading = true;
  const auto size = mWValuesDiffs[cellId].size();

  std::function<bool(double, double)> f;
  if (useFading)
    f = std::less<double>();
  else
    f = std::greater<double>();
  const double eps = (useFading)? -0.01 : 0.01;

  if (size == 2)
    return f(mWValuesDiffs[cellId][size - 1], mWValuesDiffs[cellId][size - 2] + eps);
  else if (size <= 1)
    return false;

  for (int i = 0; i < 2; i++)
    fading = fading && f(mWValuesDiffs[cellId][size - i - 1], mWValuesDiffs[cellId][size - i - 2] + eps);

  return fading;
}

bool ITrendIndicator::isRisingTrend(CellId cellId)
{
  return isFadingTrend(cellId, false);
}


bool ITrendIndicator::isCurrentBreaksUpwards(CellId cellId)
{
  return isCurrentBreaksWeighted(cellId, std::greater<double>(), crossHysteresis);
}

bool ITrendIndicator::isCurrentBreaksDescending(CellId cellId)
{
  return isCurrentBreaksWeighted(cellId, std::less<double>(), -crossHysteresis);
}

Time ITrendIndicator::windowDuration() const
{
  return mWindowDuration? mWindowDuration : mWindowSize * Converter::milliseconds(5) + 1;
}

size_t ITrendIndicator::windowSize() const
{
  return mWindowSize ? mWindowSize : mWindowDuration / Converter::milliseconds(5) + 1;
}

bool ITrendIndicator::isUpgoingTrendWeighted(CellId cellId, std::function<bool (double, double)> f, double hysteresis)
{
  auto &deque = mWValuesDiffs[cellId];
  const auto size = deque.size();
  if (!size || size == 1)
    return false;
  return f(deque.back(), deque[deque.size() - 2] + hysteresis);
}

bool ITrendIndicator::isCurrentBreaksWeighted(CellId cellId, std::function<bool (double, double)> f, double hysteresis)
{
  return f(mCsiJournal->at(cellId).back().second, lastValueFor(cellId) + hysteresis);
}


void ITrendIndicator::updateWeightedJournal(CellId cellId, double value)
{
  auto &deque = mWeightedSignals[cellId];
  deque.push_back(value);

  assert(mWindowDuration || mWindowSize);
  if (mWindowDuration)
    mWindowSize = mWindowDuration / Converter::milliseconds(5) + 1;

  while (deque.size() > mWindowSize)
    {
      deque.pop_front();
      mSignalDiffs[cellId].pop_front();
      mWValuesDiffs[cellId].pop_front();
    }
}

void ITrendIndicator::updateSignalDiffs(CellId cellId)
{
  auto &array = mCsiJournal->at(cellId);
  const auto size = array.size();
  if (size <= 1)
    mSignalDiffs[cellId].push_back(0);

  mSignalDiffs[cellId].push_back(array[size - 1].second - array[size - 2].second);
}

void ITrendIndicator::updateWeightedValuesDiffs(CellId cellId)
{
  auto &array = mWeightedSignals[cellId];
  const auto size = array.size();
  if (size <= 1)
    {
      mWValuesDiffs[cellId].push_back(0);
      return;
    }

  mWValuesDiffs[cellId].push_back(array[size - 1] - array[size - 2]);
}

