#pragma once

#include <iostream>
#include <time.h>
#include <fstream>
#include <unordered_map>
#include <utility>
#include <assert.h>

#include "messages.h"

#define LOG(x)  std::clog << "LOG: " << x << "\n";
#define WARN(x) std::cerr << __FILE__ << "\tWARN: " << x << "\n";
#define ERR(x)  std::cerr << __FILE__ << "\tERR: " << x << "\n"; \
                assert(false); \
                std::terminate();

#ifndef NDEBUG
    #define DEBUG(x) do { std::cerr << "DEBUG: " << x << "\n"; } while (false)
    #define DEBUG2(x) do { std::cerr << "DEBUG: " << __func__ << ":" << #x << ": " << x << "\n"; } while (false)
#else
    #define DEBUG(x) do {} while (false)
    #define DEBUG2(x) do {} while(false)
#endif

#define UNUSED(x) [&x] {} ()


class SimConfig
{
public:
  static constexpr int timeInterval = 10; // 5 / 10 / 20 / 40
  enum DecisionAlgo
  {
    naive
    , interpolation
    , wmaRaw
    , smmRaw
    , kamaRaw
    , kamaPure
    , hybrid
    , chebyshevApprx
    , leastSquaresRegression
  };

  static constexpr DecisionAlgo algoType = naive;


  static constexpr int wmaSmmDuration = 21;


  static constexpr int approxAlgoWindowSize   =  10  /*10*/      ;
  static constexpr int approxAlgoXOffset =  1000   /*2000*/   ;


  static constexpr int kamaN =      10    ;
  static constexpr int kamaF =    1    ;
  static constexpr int kamaS =  20    ;

};

class Converter
{
public:
  static constexpr uint64_t seconds(int64_t s) { return s * 1000 * 1000; }
  static constexpr uint64_t milliseconds(int64_t s) { return s * 1000; }
  static constexpr uint64_t microseconds(int64_t s) { return s; }
};

class Simulator;
class SimTimeProvider
{
public:
  static Time getTime();

  friend class Simulator;
private:
  static Time mCurrentTime;

  static void setTime(Time newTime);
};


template <typename T>
class Statistics
{
public:
  void add(const std::string &index, const T &val)
  {
    mSum[index] += val;
    if (!mCounter[index] || val > mMax[index])
      mMax[index] = val;

    if (!mCounter[index] || val < mMin[index])
      mMin[index] = val;

    ++mCounter[index];
  }

  double average(const std::string &index)
  {
    return (mSum[index]) / static_cast<double>(mCounter[index]);
  }

  T minimum(const std::string &index) { return mMin[index]; }
  T maximum(const std::string &index) { return mMax[index]; }

private:
  std::unordered_map<std::string, T> mMin, mMax;
  std::unordered_map<std::string, decltype(std::declval<T>() + std::declval<T>())> mSum;
  std::unordered_map<std::string, uint64_t> mCounter;
};


class TimeMeasurement
{
public:
  void start(const std::string &index);
  void start(const std::string &index, Time time); //< manually

  void stop(const std::string &index);
  void stop(const std::string &index, Time time); //< manually

  double average(const std::string &index) { return mStatistics.average(index); }

  int64_t minimum(const std::string &index) { return mStatistics.minimum(index); }
  int64_t maximum(const std::string &index) { return mStatistics.maximum(index); }

private:
//  typedef decltype(std::chrono::high_resolution_clock::now()) RealTimePoint;
//  std::unordered_map<std::string, RealTimePoint> mStartTime, mStopTime;
  std::unordered_map<std::string, clock_t> mStartTime, mStopTime;
  std::unordered_map<std::string, Time> mStartTimeManual, mStopTimeManual;
  Statistics<uint64_t> mStatistics;
};


class FileLogger
{
public:
  template <typename T>
  static void write(T &stream)
  {
    if (!mInitiated)
      {
        mConcreteFileLogger.open("./output/log.log", std::ios_base::out | std::ios_base::trunc);
        assert(mConcreteFileLogger.is_open());
        mInitiated = true;
      }
    mConcreteFileLogger << stream << "\n";
    mConcreteFileLogger.flush();
  }


private:
  static std::fstream mConcreteFileLogger;
  static bool mInitiated;
};


