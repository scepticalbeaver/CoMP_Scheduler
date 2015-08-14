#pragma once

#include "helpers.h"
#include "lteEnb/l2-mac.h"

class Simulator
{
public:
  static Simulator* instance();
  static void destroy();

  void run();

  void scheduleEvent(Event event);

private:
  using EventQueue = std::priority_queue<Event, std::deque<Event>, std::greater<Event>>;

  static Simulator* mSimulator;
  EventQueue mEventQueue;
  L2Mac mL2MacFlat;
  Time mStopTime = Converter::seconds(0);
  RealtimeMeasurement mTimeMeasurement;

  Simulator();
  ~Simulator();
  Simulator(const Simulator &) = delete;
  Simulator& operator =(const Simulator &) = delete;

  void parseMacTraffic();
  void parseMeasurements();

  void postProcessing();
};

