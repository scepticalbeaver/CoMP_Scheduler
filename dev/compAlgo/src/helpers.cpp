#include "helpers.h"


Time SimTimeProvider::mCurrentTime = Converter::milliseconds(0);

Time SimTimeProvider::getTime()
{
  return mCurrentTime;
}

void SimTimeProvider::setTime(Time newTime)
{
  mCurrentTime = newTime;
}

void TimeMeasurement::start(const std::string &index)
{
  mStartTime[index] = std::chrono::high_resolution_clock::now();
}

void TimeMeasurement::start(const std::string &index, Time time)
{
  mStartTimeManual[index] = time;
}

void TimeMeasurement::stop(const std::string &index)
{
  mStopTime[index] = std::chrono::high_resolution_clock::now();
  uint64_t const elapsed =
      std::chrono::duration_cast<std::chrono::microseconds>(mStopTime[index] - mStartTime[index]).count();

  mSumElapsed[index] += elapsed;
  if (!mCounter[index] || elapsed > mMaxElapsed[index])
    mMaxElapsed[index] = elapsed;

  if (!mCounter[index] || elapsed < mMinElapsed[index])
    mMinElapsed[index] = elapsed;

  ++mCounter[index];
}

void TimeMeasurement::stop(const std::string &index, Time time)
{
  mStopTimeManual[index] = time;
  auto const elapsed = mStopTimeManual[index] - mStartTimeManual[index];

  mSumElapsed[index] += elapsed;
  if (!mCounter[index] || elapsed > mMaxElapsed[index])
    mMaxElapsed[index] = elapsed;

  if (!mCounter[index] || elapsed < mMinElapsed[index])
    mMinElapsed[index] = elapsed;

  ++mCounter[index];
}

double TimeMeasurement::average(const std::string &index)
{
  return (mSumElapsed[index] + 0.0) / mCounter[index];
}
