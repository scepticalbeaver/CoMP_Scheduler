#pragma once

#include <string>
#include <queue>
#include <deque>

using Time = uint64_t;
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

struct X2Message
{
  enum X2MsgType
  {
    measuresInd
    , changeScheduleModeInd
    , leadershipInd
  };

  X2MsgType type;
  CSIMeasurementReport report;
  bool mustSendTraffic;
  int leaderCellId;
};


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




