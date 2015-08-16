#pragma once

#include <map>
#include <queue>
#include <memory>

#include "../helpers.h"

using CsiArray = std::deque<CsiUnit>;
using CsiJournal = std::map<CellId, CsiArray>;

class ICompSchedulingAlgo
{
public:
  ICompSchedulingAlgo(CsiJournal const *j) : mCsiJournal(j) {}
  void setJournal(CsiJournal const *j) { mCsiJournal = j; }

  virtual ~ICompSchedulingAlgo() {}

  virtual CellId redefineBestCell(CellId lastScheduled) = 0;

protected:
    CsiJournal const *mCsiJournal;
};

using UniqCompSchedulingAlgo = std::unique_ptr<ICompSchedulingAlgo>;
