#pragma once

#include <map>
#include <queue>
#include <memory>
#include <fstream>


#include "../helpers.h"
#include "trendIndicators/wma-indicator.h"
#include "trendIndicators/kama-indicator.h"


class CompSchedulingAlgo
{
public:
  CompSchedulingAlgo(CsiJournalPtr j, CellIdVectorPtr compGroup);

  void setJournal(CsiJournalPtr j) { assert(mCsiJournal = j); }
  void setCompGroup(CellIdVectorPtr cg) { assert(mCompGroup = cg); }

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
};

using UniqCompSchedulingAlgo = std::unique_ptr<CompSchedulingAlgo>;
