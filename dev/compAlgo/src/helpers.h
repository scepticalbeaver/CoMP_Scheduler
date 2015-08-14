#pragma once

#include <iostream>
#include <chrono>
#include <unordered_map>

#include "messages.h"

#define LOG(x) std::clog << "log: " << x << "\n";
#define WARN(x) std::cerr << "warn: " << x << "\n";

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



class RealtimeMeasurement
{
public:
  void start(const std::string &index);

  void stop(const std::string &index);

  double average(const std::string &index);


  int64_t minimum(const std::string &index) { return mMinElapsed[index]; }
  int64_t maximum(const std::string &index) { return mMaxElapsed[index]; }

private:
  typedef decltype(std::chrono::high_resolution_clock::now()) RealTimePoint;

  std::unordered_map<std::string, RealTimePoint> mStartTime, mStopTime;
  std::unordered_map<std::string, int64_t> mSumElapsed, mMinElapsed, mMaxElapsed;
  std::unordered_map<std::string, int64_t> mCounter;
};
