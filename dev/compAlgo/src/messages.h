#pragma once

#include <string>
#include <queue>
#include <deque>
#include <map>
#include <memory>

using Time = uint64_t;
using CsiUnit = std::pair<Time, int>; //< time of measurements received, rsrp
using CsiArray = std::deque<CsiUnit>;

using CellId = int;
using CellIdVector = std::vector<CellId>;
using CellIdVectorPtr = std::shared_ptr<CellIdVector>;

using CsiJournal = std::map<CellId, CsiArray>;
using CsiJournalPtr = std::shared_ptr<CsiJournal>;

struct DlRlcPacket
{
  std::string dlRlcStatLine;
};

struct CSIMeasurementReport
{
  CellId targetCellId;
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
  Time applyDirectMembership;
  CellId leaderCellId;
};


enum class EventType
{
  scheduleAttempt
  , csiIndicator
  , x2Message
  , stopSimulation
  , l2Timeout
};


struct Event
{
  EventType eventType;
  Time atTime;
  CellId cellId;

  DlRlcPacket packet;
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




