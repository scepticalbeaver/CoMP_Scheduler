#include "comp-decision-algo.h"

CompSchedulingAlgo::CompSchedulingAlgo(CsiJournalPtr j, CellIdVectorPtr compGroup)
  : mCsiJournal(j)
  , mCompGroup(compGroup)
  , mWmaIndicator(new WmaIndicator(j))
  , mKamaIndicator(new KamaIndicator(j))
{
  assert(j && compGroup);

  mMovingScoreLogger.open("output/moving_score.log", std::ios_base::out | std::ios_base::trunc);
  assert(mMovingScoreLogger.is_open());
  mMovingScoreLogger << "% time [us]\tcellId\tcellId\tvalue\n";
}

CompSchedulingAlgo::~CompSchedulingAlgo()
{
  mMovingScoreLogger.flush();
  mMovingScoreLogger.close();
}

void CompSchedulingAlgo::update(CellId cellId)
{
  removeOldValues();
  mWmaIndicator->update(cellId);
  mKamaIndicator->update(cellId);
  writeScore(cellId, mWmaIndicator->lastValueFor(cellId), mCsiJournal->at(cellId).back().second);
}

CellId CompSchedulingAlgo::redefineBestCell(CellId lastScheduled)
{
  if (haveTooLittleValues())
    return lastScheduled;

  const double hysteresis = .2;

  std::map<CellId, double> signalForecast;

  CellId nextDecision = lastScheduled;
  double estimatedBestSignal = signalForecast[lastScheduled];
  bool choosed = false;

  for (auto cellId : *mCompGroup)
    {
      const double lastWeightedVal = mWmaIndicator->lastValueFor(cellId);
      const CsiArray &csiArray = mCsiJournal->at(cellId);
      if (!csiArray.size())
        continue;


      const int diffCount = (csiArray.size() > 2)? 2 : 1;

      for (int k = 0; k < diffCount; k++)
        {
          signalForecast[cellId] += csiArray[csiArray.size() - 1 - k].second
              - csiArray[csiArray.size() - 1 - k - 1].second;
        }
      signalForecast[cellId] = signalForecast[cellId] / double(diffCount) + csiArray.back().second;
      if (mWmaIndicator->isLastOutlier(cellId))
        signalForecast[cellId] = lastWeightedVal;

      bool isQualityRises = mWmaIndicator->isCurrentBreaksUpwards(cellId);
      bool isWmaBetter = mWmaIndicator->lastValueFor(cellId) > mWmaIndicator->lastValueFor(lastScheduled);
      bool isCurForecastBetterScheduledSignificantly =
          signalForecast[cellId] > signalForecast[lastScheduled] + hysteresis * 9;
      bool isCurForecastBetterScheduled = signalForecast[cellId] > signalForecast[lastScheduled] + hysteresis;

      if (isQualityRises
          && isCurForecastBetterScheduled
          && (isWmaBetter || isCurForecastBetterScheduledSignificantly)
          && signalForecast[cellId] > estimatedBestSignal)
        {
          nextDecision = cellId;
          estimatedBestSignal = signalForecast[cellId];
          choosed = true;
        }
    }

  if (!choosed)
    {
      double maxSignal = 0;
      for (auto cellId : *mCompGroup)
        {
          if (mWmaIndicator->lastValueFor(cellId) > maxSignal)
            {
              maxSignal = mWmaIndicator->lastValueFor(cellId);
              nextDecision = cellId;
            }
        }
    }

  return nextDecision;
}


bool CompSchedulingAlgo::haveTooLittleValues()
{
  double probesCount = 0.0;
  for (auto &pair : *mCsiJournal)
    probesCount += pair.second.size();
  probesCount /= mCsiJournal->size();

  return probesCount < 2.9;
}


void CompSchedulingAlgo::writeScore(CellId cellId, double aveValue, double rawValue)
{
  mMovingScoreLogger << SimTimeProvider::getTime() << "\t" << cellId << "\t" << cellId << "\t"
                     << rawValue << "\n";
  mMovingScoreLogger << SimTimeProvider::getTime() << "\t" << cellId + 10 << "\t" << cellId + 10
                     << "\t" << aveValue << "\n";
}

void CompSchedulingAlgo::removeOldValues()
{
  const auto windowDuration = std::max(mKamaIndicator->windowDuration(), mWmaIndicator->windowDuration());
  assert(windowDuration);
  for (auto &csiPair : *mCsiJournal)
    {
      CsiArray &array = csiPair.second;
      while (array.size() > 1 && array.front().first < SimTimeProvider::getTime() - windowDuration)
        array.pop_front();
    }
}
