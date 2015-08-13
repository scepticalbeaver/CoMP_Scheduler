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
  std::clog << "log: start parsing mac traffic..." << std::endl;
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
          std::cerr << "warn: drop line: " << line << "\n";
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

  std::clog << "log: parsing mac traffic done" << std::endl;
}

void Simulator::parseMeasurements()
{
  std::clog << "log: start parsing measurements..." << std::endl;
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
          std::cerr << "warn: drop line: \"" << line << "\"\n";
          continue;
        }

      std::stringstream stream(line);

      Time timeUSec;
      int sCellId, tCellId, rsrp;
      stream >> timeUSec >> sCellId >> tCellId >> rsrp;

      CSIMeasurementReport report;
      report.targetCellId = tCellId;
      report.rsrp = rsrp;

      Event event(EventType::csiIndicator, timeUSec);
      event.cellId = sCellId;
      event.report = report;

      mEventQueue.push(event);
    }

  std::clog << "log: measurements parsing done" << std::endl;
}

Simulator::~Simulator()
{

}

void Simulator::destroy()
{
  delete mSimulator;
  mSimulator = nullptr;
}

void Simulator::run()
{

}

void Simulator::scheduleEvent(Event &event)
{
  assert(event.atTime >= mCurrentTime);
  mEventQueue.push(event);
}

Time Simulator::getTime() const
{
  return mCurrentTime;
}
