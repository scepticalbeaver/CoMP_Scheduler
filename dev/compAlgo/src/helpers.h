#pragma once

#include <iostream>

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
