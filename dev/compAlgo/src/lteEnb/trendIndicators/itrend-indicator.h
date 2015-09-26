#pragma once

#include "../../helpers.h"

class ITrendIndicator
{
public:
  ITrendIndicator(const std::string &id, CsiJournalPtr j);
  virtual ~ITrendIndicator();

  void setJournal(CsiJournalPtr j) { mCsiJournal = j; }
  void setPreventiveAnalysis(bool value) { mApplyAnalysOnForecast = value; }

  void update(CellId cellId);
  virtual double lastValueFor(CellId cellId);

  virtual double forecast(CellId cellId);

  virtual bool isUpgoingTrend(CellId cellId);
  virtual bool isDescendingTrend(CellId cellId);
  //! @brief speed of growth decreases / speed of fading increases
  //! @arg useFading not change this param
  bool isFadingTrend(CellId cellId, bool useFading = true);
  //! @brief speed of growth increases / speed of fading decreases
  bool isRisingTrend(CellId cellId);

  bool isCurrentBreaksUpwards(CellId cellId);
  bool isCurrentBreaksDescending(CellId cellId);

  Time windowDuration() const;
  size_t windowSize() const;

protected:
  const double crossHysteresis = 0.2;
  const Time measuremetnsInterval = Converter::milliseconds(SimConfig::timeInterval);

  CsiJournalPtr mCsiJournal;
  bool mApplyAnalysOnForecast = false;
  bool mIsShadowValueUsed = false;

  Time mWindowDuration = Converter::milliseconds(0);
  size_t mWindowSize = 0;

  using ResValuesJournal = std::map<CellId, std::deque<double>>;

  ResValuesJournal mWeightedSignals;
  ResValuesJournal mSignalDiffs;
  ResValuesJournal mWValuesDiffs;


  virtual double updateHook(CellId cellId) = 0;

  bool isUpgoingTrendWeighted(CellId cellId, std::function<bool(double, double)> f, double hysteresis);
  bool isCurrentBreaksWeighted(CellId cellId, std::function<bool(double, double)> f, double hysteresis);
  void updateWeightedJournal(CellId cellId, double value);
  void updateSignalDiffs(CellId cellId);
  void updateWeightedValuesDiffs(CellId cellId);
  void updateErrorStatistics(double newVal, CellId cellId);
  int64_t lPointerCsiFromWindowSize(CellId cellId);

private:
  double mLastPrediction = 0;
  Statistics<double> mErrorStats;
  std::string mIdentity;
};

