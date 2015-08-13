#include "x2-channel.h"

#include "../simulator.h"

X2Channel* X2Channel::mInstance = nullptr;

X2Channel::X2Channel()
{
}

X2Channel *X2Channel::instance()
{
  if (!mInstance)
    mInstance = new X2Channel;
  return mInstance;
}

void X2Channel::destroy()
{
  delete mInstance;
  mInstance = nullptr;
}

uint64_t X2Channel::delay() const
{
  return delay;
}

void X2Channel::send(int tCellId, X2Message msg)
{
  Event msgEvent(EventType::x2Message, Simulator::instance()->getTime() + delay);
  msgEvent.cellId = tCellId;
  msgEvent.message = msg;
  Simulator::instance()->scheduleEvent(msgEvent);
}

