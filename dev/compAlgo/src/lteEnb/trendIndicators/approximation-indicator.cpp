#include "approximation-indicator.h"

#include <algorithm>

#include <gsl/gsl_fit.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_chebyshev.h>

namespace
{
  struct GslFunctionParams
  {
    CsiArray const * arrayPtr;
    int64_t left;

    GslFunctionParams(CsiArray const * arrayPtr, int64_t lPtr) : arrayPtr(arrayPtr), left(lPtr) {}
  };

  double getNearestValueFromJournal(double x, void* params)
  {
    const auto gslParams = reinterpret_cast<GslFunctionParams *>(params);
    assert(gslParams != nullptr);

    auto low = gslParams->arrayPtr->begin();
    std::advance(low, gslParams->left);
    auto end = gslParams->arrayPtr->end();


    auto resIter =  std::upper_bound (low, end, x, [] (double x, const CsiUnit &unit )
    {
      return x < unit.first;
    });

    return (resIter == end)? gslParams->arrayPtr->back().second : resIter->second;
  }
}

ApproximationIndicator::ApproximationIndicator(CsiJournalPtr j, Method type)
  : ITrendIndicator("approximation-ind", j)
  , mApproximationType(type)
{
  switch (SimConfig::algoType)
    {
    case SimConfig::chebyshevApprx:
      mApproximationType = chebyshevPolynomials;
      mCalcApprxFunc = calcChebyshev;
      break;
    case SimConfig::leastSquaresRegression:
      mApproximationType = polyRegressionFitting;
      mCalcApprxFunc = calcPolyRegression;
      break;
    default:
      DEBUG2("Other indicator in use. Makeing stub..");
      mCalcApprxFunc = [] (const CsiArray &, int64_t) -> double { return 0.0; };
      return;
    }
  mWindowSize = SimConfig::approxAlgoWindowSize;
}

double ApproximationIndicator::updateHook(CellId cellId)
{
  return forecast(cellId);
}

double ApproximationIndicator::calcChebyshev(const CsiArray &csiArray, int64_t lPointer)
{
  const auto dataSize = csiArray.size();
  assert(dataSize);
  if (dataSize == 1)
    return csiArray.front().second;

  auto chebSeries = gsl_cheb_alloc (6);

  gsl_function gslFunction;
  gslFunction.function = getNearestValueFromJournal;

  GslFunctionParams gslParams {&csiArray, lPointer};
  gslFunction.params = &gslParams;

  gsl_cheb_init (chebSeries, &gslFunction, csiArray[lPointer].first, csiArray.back().first);

  const double x = csiArray.back().first + SimConfig::approxAlgoXOffset;

  double result = gsl_cheb_eval (chebSeries, x);
  gsl_cheb_free (chebSeries);
  return result;
}

double ApproximationIndicator::calcPolyRegression(const CsiArray &csiArray, int64_t lPointer)
{
  const int64_t dataSize = csiArray.size();

  static int64_t eqOne = 0;
  static int64_t eqElse = 0;
  if (dataSize == 1)
    {
      DEBUG("win size: "<< ++eqOne << "\t" << eqElse);
      return calcChebyshev(csiArray, lPointer);
    }
  else
    DEBUG("win size: "<< eqOne << "\t" << ++eqElse);


  const int n = dataSize - lPointer;
  const size_t p = 2; /* linear fit */
  gsl_matrix *Xmatrix, *cov;
  gsl_vector *xset, *yset, *coeff;


  Xmatrix = gsl_matrix_alloc (n, p);
  xset = gsl_vector_alloc (n);
  yset = gsl_vector_alloc (n);

  coeff = gsl_vector_alloc (p);
  cov = gsl_matrix_alloc (p, p);

  for (auto i = lPointer; i < dataSize; i++)
    {
      gsl_vector_set (xset, i - lPointer, csiArray[i].first);
      gsl_vector_set (yset, i - lPointer, csiArray[i].second);
    }

  /* construct design matrix X for linear fit */
  const auto meanTime = calcMeanTime(csiArray, lPointer);
  const auto stdDev = calcStdDevTime(csiArray, lPointer, meanTime);
  for (int i = 0; i < n; ++i)
    {
      double xi = (gsl_vector_get(xset, i) - meanTime) / stdDev;

      gsl_matrix_set (Xmatrix, i, 0, 1.0);
      gsl_matrix_set (Xmatrix, i, 1, xi);
    }

  gsl_multifit_robust_workspace* work = gsl_multifit_robust_alloc (gsl_multifit_robust_bisquare, n, p);
  gsl_multifit_robust (Xmatrix, yset, coeff, cov, work);
  gsl_multifit_robust_free (work);


  const auto intercept = gsl_vector_get(coeff, 0);
  const auto slope = gsl_vector_get(coeff, 1);
  // Y = intecept + slope * x

  const double x = csiArray.back().first + SimConfig::approxAlgoXOffset;

  gsl_matrix_free (Xmatrix);
  gsl_vector_free (xset);
  gsl_vector_free (yset);
  gsl_vector_free (coeff);
  gsl_matrix_free (cov);
  return intercept + slope * x;
}

double ApproximationIndicator::calcMeanTime(const CsiArray &csiArray, int64_t lPointer)
{
  const int64_t size = csiArray.size();
  double mean = 0;
  for (int64_t i = lPointer; i < size; i++)
    {
      mean += csiArray[i].first;
    }
  return mean / double(size - lPointer);
}

double ApproximationIndicator::calcStdDevTime(const CsiArray &csiArray, int64_t lPointer, double meanTime)
{
  const int64_t size = csiArray.size();
  double stdDev = 0;
  for (int64_t i = lPointer; i < size; i++)
    {
      stdDev += std::pow(csiArray[i].first - meanTime, 2);
    }
  return std::sqrt(stdDev / (double(size - lPointer)));
}

double ApproximationIndicator::forecast(CellId cellId)
{
  auto &array = mCsiJournal->at(cellId);
  return mCalcApprxFunc(array, lPointerCsiFromWindowSize(cellId));
}
