#pragma once

using Time = uint64_t;


class Converter
{
  static uint64_t seconds(int64_t s) { return s * 1000 * 1000; }
  static uint64_t milliseconds(int64_t s) { return s * 1000; }
  static uint64_t microseconds(int64_t s) { return s; }
};
