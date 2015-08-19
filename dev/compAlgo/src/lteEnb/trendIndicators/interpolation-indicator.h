#pragma once

#include "itrend-indicator.h"

class InterpolationIndicator : public ITrendIndicator
{
public:
  enum Method
  {
    lagrangePolynomials
  };

  InterpolationIndicator(CsiJournalPtr j, Method type = lagrangePolynomials);


  double forecast(CellId cellId);

protected:
  Method mInterpolationType;

  double updateHook(CellId cellId);

  double forecastLagrange(CellId cellId);
};

using UniqInterpolationIndicator = std::unique_ptr<InterpolationIndicator>;

