#include "simulator.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <assert.h>

Simulator* Simulator::mSimulator = nullptr;

Simulator *Simulator::instance()
{
  if (!mSimulator)
    mSimulator = new Simulator;

  return mSimulator;
}

Simulator::Simulator()
{
  parseMacTraffic();
  parseMeasurements();

  scheduleEvent(Event(EventType::stopSimulation, stopTime));
}

void Simulator::parseMacTraffic()
{
  LOG("start parsing mac traffic...");
  std::string location = "../../../input/DlMacStats.txt";
  std::fstream macStats;
  macStats.open(location, std::ios_base::in);
  assert(macStats.is_open());

  std::string line;
  std::getline(macStats, line); // first line dummy

  while (std::getline(macStats, line))
    {
      if (line.size() < 15)
        {
          WARN("drop line: " << line);
          continue;
        }

      std::stringstream stream(line);

      double time; // seconds
      int cellId;
      stream >> time >> cellId;
      Time timeUSec = static_cast<uint64_t>(time * 1000 * 1000);

      DlMacPacket packet;
      packet.dlMacStatLine = line;

      Event event(EventType::scheduleAttempt, timeUSec);
      event.cellId = cellId;
      event.packet = packet;

      mEventQueue.push(event);
    }

  LOG("parsing mac traffic done");
}

void Simulator::parseMeasurements()
{
  LOG("start parsing measurements...");
  std::string location = "../../../input/measurements.log";
  std::fstream measurements;
  measurements.open(location, std::ios_base::in);
  assert(measurements.is_open());

  std::string line;
  std::getline(measurements, line); // first line dummy

  while (std::getline(measurements, line))
    {
      if (line.size() < 7)
        {
          WARN("warn: drop line: \"" << line << "\"");
          continue;
        }

      std::stringstream stream(line);

      Time timeUSec;
      int sCellId, tCellId, rsrp;
      stream >> timeUSec >> sCellId >> tCellId >> rsrp;

      CSIMeasurementReport report;
      report.targetCellId = tCellId;
      report.csi = std::make_pair(timeUSec, rsrp);

      Event event(EventType::csiIndicator, timeUSec);
      event.cellId = sCellId;
      event.report = report;

      mEventQueue.push(event);
    }

  LOG("measurements parsing done");
}

void Simulator::postProcessing()
{
  LOG("Stop event was reached\n" << "\tStill scheduled in queue " << mEventQueue.size() << " events");
}

Simulator::~Simulator()
{
  X2Channel::destroy();
}

void Simulator::destroy()
{
  delete mSimulator;
  mSimulator = nullptr;
}

void Simulator::run()
{
  while (!mEventQueue.empty())
    {
      Event event = mEventQueue.top();
      mEventQueue.pop();
      switch(event.eventType)
        {
          case EventType::stopSimulation:
          {
            postProcessing();
            return;
          }
        case EventType::x2Message:
          {
            mL2MacFlat.recvX2Message(event.cellId, event.message);
            break;
          }
        case EventType::csiIndicator:
          {
            mL2MacFlat.recvMeasurementsReport(event.cellId, event.report);
            break;
          }
        case EventType::scheduleAttempt:
          {
            mL2MacFlat.makeScheduleDecision(event.cellId, event.packet);
            break;
          }
        }
    }
}

void Simulator::scheduleEvent(Event event)
{
  assert(event.atTime >= mCurrentTime);
  mEventQueue.push(event);
}

Time Simulator::getTime() const
{
  return mCurrentTime;
}
