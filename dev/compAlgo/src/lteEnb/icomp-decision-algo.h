#pragma once

#include <map>
#include <queue>
#include <memory>
#include <fstream>
#include <algorithm>

#include "../helpers.h"

using CsiArray = std::deque<CsiUnit>;
using CsiJournal = std::map<CellId, CsiArray>;

class ICompSchedulingAlgo
{
public:
  ICompSchedulingAlgo(CsiJournal* j)
    : mCsiJournal(j)
  {
    mMovingScoreLogger.open("output/moving_score.log", std::ios_base::out | std::ios_base::trunc);
    assert(mMovingScoreLogger.is_open());
    mMovingScoreLogger << "% time [us]\tcellId\tcellId\tvalue\n";
  }
  void setJournal(CsiJournal* j) { mCsiJournal = j; }

  virtual ~ICompSchedulingAlgo()
  {
    mMovingScoreLogger.flush();
    mMovingScoreLogger.close();
  }

  virtual CellId redefineBestCell(CellId lastScheduled) = 0;

protected:
    CsiJournal* mCsiJournal;
    std::fstream mMovingScoreLogger;


    bool isLastOutlier(CellId cellId)
    {
      const double order = 2.0;
      CsiArray array = (*mCsiJournal)[cellId];
      const size_t size = array.size();
      if (size <= 2)
        return false;

      int k = (size % 2 == 0)? size / 2 + 1 : size / 2;
      std::vector<CsiUnit> values(array.begin(), array.end());
      std::nth_element(values.begin(), values.begin() + k, values.end(), [] (const CsiUnit &l, const CsiUnit &r)
      {
        return l.second < r.second;
      });
      const auto median = values[k].second;

      std::vector<int> diffs(size);
      for (size_t i = 0; i < size; i++)
        {
          diffs[i] = std::abs(array[i].second - median);
        }

      std::nth_element(diffs.begin(), diffs.begin() + k, diffs.end());
      const double diffsMedian = diffs[k];

      return diffs.back() / diffsMedian > order;
    }

    void writeScore(CellId cellId, double val, double rawValue)
    {
      mMovingScoreLogger << SimTimeProvider::getTime() << "\t" << cellId << "\t" << cellId << "\t"
                         << rawValue << "\n";
      mMovingScoreLogger << SimTimeProvider::getTime() << "\t" << cellId + 10 << "\t" << cellId + 10
                         << "\t" << val << "\n";
    }
};

using UniqCompSchedulingAlgo = std::unique_ptr<ICompSchedulingAlgo>;
