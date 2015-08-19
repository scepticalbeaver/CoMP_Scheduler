#pragma once

#include <fstream>

#include "../helpers.h"
#include "trendIndicators/wma-indicator.h"
#include "trendIndicators/kama-indicator.h"


class CompSchedulingAlgo
{
public:
  CompSchedulingAlgo(CsiJournalPtr j, CellIdVectorPtr compGroup);

  void setJournal(CsiJournalPtr j);
  void setCompGroup(CellIdVectorPtr cg);

  ~CompSchedulingAlgo();

  void update(CellId cellId);
  CellId redefineBestCell(CellId lastScheduled);

private:
    CompSchedulingAlgo& operator=(const CompSchedulingAlgo&) = delete;
    CompSchedulingAlgo(const CompSchedulingAlgo&) = delete;

    CsiJournalPtr mCsiJournal;
    CellIdVectorPtr mCompGroup;
    std::fstream mMovingScoreLogger;

    UniqWmaIndicator mWmaIndicator;
    UniqKamaIndicator mKamaIndicator;


    void writeScore(CellId cellId, double aveValue, double rawValue);
    void removeOldValues();

    bool haveTooLittleValues();

    CellId predictorPureRawForecast(CellId lastScheduled);

    CellId predictorWeightedForecast(CellId lastScheduled);
    double weightedLastValue(CellId cellId);
    double weightedForecast(CellId cellId);

};

using UniqCompSchedulingAlgo = std::unique_ptr<CompSchedulingAlgo>;
