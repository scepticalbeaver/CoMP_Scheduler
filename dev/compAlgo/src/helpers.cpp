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
  mStartTime[index] = clock();
//  mStartTime[index] = std::chrono::high_resolution_clock::now();
}

void TimeMeasurement::start(const std::string &index, Time time)
{
  mStartTimeManual[index] = time;
}

void TimeMeasurement::stop(const std::string &index)
{
  mStopTime[index] = clock(); // std::chrono::high_resolution_clock::now();
//  uint64_t const elapsed =
//      std::chrono::duration_cast<std::chrono::microseconds>(mStopTime[index] - mStartTime[index]).count();
//  mStopTime[index] = clock();
  uint64_t const elapsed =
      static_cast<u_int64_t>(double(mStopTime[index] - mStartTime[index]) / CLOCKS_PER_SEC * 1000 * 1000);

  mStatistics.add(index, elapsed);
}

void TimeMeasurement::stop(const std::string &index, Time time)
{
  mStopTimeManual[index] = time;
  auto const elapsed = mStopTimeManual[index] - mStartTimeManual[index];
  mStatistics.add(index, elapsed);
}


bool FileLogger::mInitiated = false;
std::fstream FileLogger::mConcreteFileLogger;
