#pragma once

#include <queue>
#include <deque>

#include "helpers.h"
#include "lteEnb/l2-mac.h"
#include "lteEnb/x2-channel.h"

enum class EventType
{
  scheduleAttempt
  , csiIndicator
  , x2Message
  , stopSimulation
};


struct Event
{
  EventType eventType;
  Time atTime;
  int cellId;

  DlMacPacket packet;
  CSIMeasurementReport report;
  X2Message message;

  bool operator <(const Event &other) const
  {
    return atTime < other.atTime;
  }
  bool operator >(const Event &other) const
  {
    return atTime > other.atTime;
  }

  Event() {}
  Event(EventType type, uint64_t time) : eventType(type), atTime(time) {}
};

using EventQueue = std::priority_queue<Event, std::deque<Event>, std::greater<Event>>;




