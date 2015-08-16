#pragma once

#include "icomp-decision-algo.h"

class WMACompAlgo : public ICompSchedulingAlgo
{
public:
  WMACompAlgo(CsiJournal const *j) : ICompSchedulingAlgo(j) {}

  CellId redefineBestCell(CellId lastScheduled) override
  {
      double aveProbes = 0;
      std::map<CellId, double> signalLvl;
      for (const auto &csiPair : *mCsiJournal)
        {
          const size_t len = csiPair.second.size();
          int64_t signalSum = 0;
          for (size_t i = 0; i < len; i++)
            {
              signalSum += (i + 1) * csiPair.second[i].second;
            }

          double aveSignal = (signalSum + 0.0) / ((1 + len) * len / 2); // WMA
          signalLvl.insert(std::make_pair(csiPair.first, aveSignal));

          aveProbes += csiPair.second.size();
        }

      aveProbes /= mCsiJournal->size();
      if (aveProbes < 3.0)
        return lastScheduled;

      CellId cellIdNext = lastScheduled;
      double maxSignal = signalLvl[lastScheduled];
      for (const auto &cellSignalPair : signalLvl)
        {
          if (cellSignalPair.second > maxSignal + 0.6)
            {
              maxSignal = cellSignalPair.second;
              cellIdNext = cellSignalPair.first;
            }
        }
      return cellIdNext;
  }
};


