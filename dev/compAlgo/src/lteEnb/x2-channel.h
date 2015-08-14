#pragma once

#include "../helpers.h"

class X2Channel
{
public:
  static X2Channel* instance();
  static void destroy();
  void configurate(int compGroupSize);

  Time getLatency() const;

  void send(int tCellId, X2Message msg);


private:
  int mCompGroupSize = 0;
  const Time delay = Converter::milliseconds(2);

  static X2Channel* mInstance;

  X2Channel() = default;
};

