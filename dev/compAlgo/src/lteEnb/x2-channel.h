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
  X2Channel* instance();
  void destroy();

  uint64_t delay() const;

  void send(int tCellId, X2Message msg);


private:
  const uint64_t delay = Converter::milliseconds(2);

  static X2Channel* mInstance;

  X2Channel();
  ~X2Channel();
};

