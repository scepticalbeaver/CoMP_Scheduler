#pragma once

#include "itrend-indicator.h"

class ApproximationIndicator : public ITrendIndicator
{
public:
  enum Method
  {
    chebyshevPolynomials
    , polyRegressionFitting
    , fromConfig
  };


  ApproximationIndicator(CsiJournalPtr j, Method type = fromConfig);


  double forecast(CellId cellId);

protected:
  Method mApproximationType;

  double updateHook(CellId cellId);

  double forecastLagrange(CellId cellId);

  //! approximation by chebyshev polynomials
  static double calcChebyshev(const CsiArray &csiArray, int64_t lPointer);
  //! @brief polynomial least-square method
  static double calcPolyRegression(const CsiArray &csiArray, int64_t lPointer);

  std::function<double (const CsiArray&, int64_t)> mCalcApprxFunc;

private:
  static double calcMeanTime(const CsiArray &csiArray, int64_t lPointer);
  static double calcStdDevTime(const CsiArray &csiArray, int64_t lPointer, double meanTime);
};

using UniqApproximationIndicator = std::unique_ptr<ApproximationIndicator>;

