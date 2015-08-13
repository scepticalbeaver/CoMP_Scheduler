#pragma once

#include <iostream>
#include <chrono>
#include <unordered_map>

#define LOG(x) std::clog << "log: " << x << "\n";
#define WARN(x) std::cerr << "warn: " << x << "\n";

using Time = uint64_t;


class Converter
{
public:
  static uint64_t seconds(int64_t s) { return s * 1000 * 1000; }
  static uint64_t milliseconds(int64_t s) { return s * 1000; }
  static uint64_t microseconds(int64_t s) { return s; }
};


using CsiUnit = std::pair<Time, int>; //< time of measurements received, rsrp

struct DlMacPacket
{
  std::string dlMacStatLine;
};


struct CSIMeasurementReport
{
  int targetCellId;
  CsiUnit csi;
};


class RealtimeMeasurement
{
public:
  void start(const std::string &index)
  {
    mStartTime[index] = std::chrono::high_resolution_clock::now();
  }

  void stop(const std::string &index)
  {
    mStopTime[index] = std::chrono::high_resolution_clock::now();
    auto const elapsed =
        std::chrono::duration_cast<std::chrono::microseconds>(mStopTime[index] - mStartTime[index]).count();

    mSumElapsed[index] += elapsed;
    if (!mCounter[index] || elapsed > mMaxElapsed[index])
      mMaxElapsed[index] = elapsed;

    if (!mCounter[index] || elapsed < mMinElapsed[index])
      mMinElapsed[index] = elapsed;

    ++mCounter[index];
  }

  double average(const std::string &index)
  {
    return (mSumElapsed[index] + 0.0) / mCounter[index];
  }


  int64_t minimum(const std::string &index) { return mMinElapsed[index]; }
  int64_t maximum(const std::string &index) { return mMaxElapsed[index]; }

private:
  typedef decltype(std::chrono::high_resolution_clock::now()) RealTimePoint;

  std::unordered_map<std::string, RealTimePoint> mStartTime, mStopTime;
  std::unordered_map<std::string, int64_t> mSumElapsed, mMinElapsed, mMaxElapsed;
  std::unordered_map<std::string, int64_t> mCounter;
};
