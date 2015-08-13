#pragma once

#include "../helpers.h"

struct X2Message
{
  enum X2MsgType
  {
    measuresInd
    , changeScheduleMode
  };

  X2MsgType type;
  CSIMeasurementReport report;
  bool mustSendTraffic;
};


class X2Channel
{
public:
  static X2Channel* instance();
  static void destroy();

  Time getLatency() const;

  void send(int tCellId, X2Message msg);


private:
  const Time delay = Converter::milliseconds(2);

  static X2Channel* mInstance;

  X2Channel() = default;
};

