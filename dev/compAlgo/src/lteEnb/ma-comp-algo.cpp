#include "ma-comp-algo.h"


MaCompAlgo::MaCompAlgo(CsiJournal *j, MaCompAlgo::MovingAverageAlgo type)
  : ICompSchedulingAlgo(j)
{
  switch (type)
    {
    case MovingAverageAlgo::simpleMovingMedian:
      mCalcMaFunc = calcSMM;
      break;
    case MovingAverageAlgo::weightedMovingAverage:
      mCalcMaFunc = calcWMA;
      break;
    }
  mWindowDuration = Converter::milliseconds(16);
}

void MaCompAlgo::update(CellId cellId)
{
  removeOldValues();

  const auto &array = (*mCsiJournal)[cellId];
  double aveSignal = mCalcMaFunc(array, cellId);

  updateWeightedJournal(cellId, aveSignal);
  writeScore(cellId, aveSignal, array.back().second); // logging
}

MaCompAlgo::MaCompAlgo(CsiJournal *j) : ICompSchedulingAlgo(j) {}


CellId MaCompAlgo::redefineBestCell(CellId lastScheduled)
{
  return makeForecast(lastScheduled);
}

void MaCompAlgo::updateWeightedJournal(CellId cellId, double value)
{
  auto &deque = mWeightedSignalsJournal[cellId];
  deque.push_back(value);
  const int64_t localLen = deque.size();
  const int64_t csiJournalLen = (*mCsiJournal)[cellId].size();
  for (int64_t i = 0; i < localLen  - csiJournalLen; i++)
    deque.pop_front();
}

double MaCompAlgo::lastWeightedValue(CellId cellId)
{
  if (mWeightedSignalsJournal[cellId].empty())
    return 0.0;
  return mWeightedSignalsJournal[cellId].back();
}

bool MaCompAlgo::haveTooLittleValues()
{
  double probesCount = 0.0;
  for (auto &pair : *mCsiJournal)
    probesCount += pair.second.size();
  probesCount /= mCsiJournal->size();

  return probesCount < 3.0;
}

CellId MaCompAlgo::makeForecast(CellId lastScheduled)
{
  if (haveTooLittleValues())
    return lastScheduled;

  const double hysteresis = .2;

  std::map<CellId, bool> breaksUpwardsTrig;
  std::map<CellId, double> signalForecast;

  for (const auto &cellAverage : mWeightedSignalsJournal)
    {
      const CellId cellId = cellAverage.first;
      const double lastWeightedVal = cellAverage.second.back();
      const CsiArray &csiArray = (*mCsiJournal)[cellId];
      if (!csiArray.size())
        continue;

      breaksUpwardsTrig[cellId] = csiArray.back().second > lastWeightedVal + hysteresis;

      const int diffCount = (csiArray.size() > 2)? 2 : 1;

      for (int k = 0; k < diffCount; k++)
        {
          signalForecast[cellId] += csiArray[csiArray.size() - 1 - k].second
              - csiArray[csiArray.size() - 1 - k - 1].second;
        }
      signalForecast[cellId] = signalForecast[cellId] / double(diffCount) + csiArray.back().second;
      if (isLastOutlier(cellId))
        signalForecast[cellId] = lastWeightedVal;
    }

  CellId nextDecision = lastScheduled;
  double estimatedBestSignal = signalForecast[lastScheduled];
  bool choosed = false;
  for (auto &cellTriggered : breaksUpwardsTrig)
    {
      bool isQualityRises = cellTriggered.second;
      bool isWmaBetter = lastWeightedValue(cellTriggered.first) > lastWeightedValue(lastScheduled);
      bool isCurForecastBetterScheduledSignificantly =
          signalForecast[cellTriggered.first] > signalForecast[lastScheduled] + hysteresis * 9;
      bool isCurForecastBetterScheduled =
          signalForecast[cellTriggered.first] > signalForecast[lastScheduled] + hysteresis;

      if (isQualityRises
          && isCurForecastBetterScheduled
          && (isWmaBetter || isCurForecastBetterScheduledSignificantly)
          && signalForecast[cellTriggered.first] > estimatedBestSignal)
        {
          nextDecision = cellTriggered.first;
          estimatedBestSignal = signalForecast[cellTriggered.first];
          choosed = true;
        }
    }

  if (!choosed)
    {
      double maxSignal = 0;
      for (auto &pair : mWeightedSignalsJournal)
        {
          if (pair.second.back() > maxSignal)
            {
              maxSignal = pair.second.back();
              nextDecision = pair.first;
            }
        }
    }

  return nextDecision;
}

double MaCompAlgo::calcWMA(const CsiArray &csiArray, CellId)
{
  const size_t len = csiArray.size();
  int64_t signalSum = 0;
  for (size_t i = 0; i < len; i++)
    {
      signalSum += (i + 1) * csiArray[i].second;
    }

  return (signalSum + 0.0) / ((1 + len) * len / 2); // WMA
}

double MaCompAlgo::calcSMM(const CsiArray &csiDeque, CellId) // simple moving median
{
  auto csiArray = csiDeque;
  const size_t len = csiArray.size();

  const int k = len / 2;
  std::nth_element(csiArray.begin(), csiArray.begin() + k, csiArray.end(),
      [] (const CsiUnit &l, const CsiUnit &r)
      {
        return l.second < r.second;
      });

  return csiArray[k].second;
}
