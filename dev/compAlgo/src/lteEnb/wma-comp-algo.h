#pragma once

#include <algorithm>

#include "icomp-decision-algo.h"


class WMACompAlgo : public ICompSchedulingAlgo
{
public:
  WMACompAlgo(CsiJournal* j) : ICompSchedulingAlgo(j) {}

  CellId redefineBestCell(CellId lastScheduled) override
  {
      double probesCount = 0;
      std::map<CellId, double> wmaSignalLvl;

      for (const auto &csiPair : *mCsiJournal)
        {

          double aveSignal = calcWMA(csiPair.second);
          wmaSignalLvl[csiPair.first] =  aveSignal;

          writeScore(csiPair.first, aveSignal, csiPair.second.back().second); // logging

          probesCount += csiPair.second.size();
        }

      probesCount /= mCsiJournal->size();
      if (probesCount < 3.0)
        return lastScheduled;

      CellId maxSignalCellId = lastScheduled;

      if (mUseForecast)
        return makeForecast(wmaSignalLvl, lastScheduled);


      double maxSignal = wmaSignalLvl[lastScheduled];
      for (const auto &cellSignalPair : wmaSignalLvl)
        {
          if (cellSignalPair.second > maxSignal + 0.6)
            {
              maxSignal = cellSignalPair.second;
              maxSignalCellId = cellSignalPair.first;
            }
        }

      return maxSignalCellId;
  }

private:
  bool mUseForecast = true;

  CellId makeForecast(std::map<CellId, double> &wmaSignalLvl, CellId lastScheduled)
  {
    const double hysteresis = .2;

    std::map<CellId, bool> breaksUpwardsTrig;
    std::map<CellId, double> signalForecast;

    for (const auto &cellAverage : wmaSignalLvl)
      {
        const CellId cellId = cellAverage.first;
        const CsiArray &csiArray = (*mCsiJournal)[cellId];
        if (!csiArray.size())
          continue;

        breaksUpwardsTrig[cellId] = csiArray.back().second > cellAverage.second + hysteresis;

        const int diffCount = (csiArray.size() > 2)? 2 : 1;

        for (int k = 0; k < diffCount; k++)
          {
            signalForecast[cellId] += csiArray[csiArray.size() - 1 - k].second
                                      - csiArray[csiArray.size() - 1 - k - 1].second;
          }
        signalForecast[cellId] = signalForecast[cellId] / double(diffCount) + csiArray.back().second;
        if (isLastOutlier(cellId))
          signalForecast[cellId] = wmaSignalLvl[cellId];
      }

    CellId nextDecision = lastScheduled;
    double estimatedBestSignal = signalForecast[lastScheduled];
    bool choosed = false;
    for (auto &cellTriggered : breaksUpwardsTrig)
      {
        bool isQualityRises = cellTriggered.second;
        bool isWmaBetter = wmaSignalLvl[cellTriggered.first] > wmaSignalLvl[lastScheduled];
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
        nextDecision = std::max_element(wmaSignalLvl.begin(), wmaSignalLvl.end(),
                                        [] (const std::pair<CellId, double> &pairL,
                                            const std::pair<CellId, double> &pairR)
        {
            return pairL.second < pairR.second;
        })->first;
      }

    return nextDecision;
  }


  double calcWMA(const CsiArray &csiArray)
  {
    const size_t len = csiArray.size();
    int64_t signalSum = 0;
    for (size_t i = 0; i < len; i++)
      {
        signalSum += (i + 1) * csiArray[i].second;
      }

    return (signalSum + 0.0) / ((1 + len) * len / 2); // WMA
  }

  double calcSMM(const CsiArray &csiDeque) // simple moving median
  {
    auto csiArray = csiDeque;
    const size_t len = csiArray.size();

    int k = len / 2;
    std::nth_element(csiArray.begin(), csiArray.begin() + k, csiArray.end(), [] (const CsiUnit &l, const CsiUnit &r)
    {
      return l.second < r.second;
    });

    return csiArray[k].second;
  }
};


