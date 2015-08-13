#pragma once

#include "eventQueue.h"

class Simulator
{
public:
  static Simulator* instance();
  static void destroy();

  void run();

  void sheduleEvent(Event &event);
  Time getTime() const;

private:
  static Simulator* mSimulator;
  EventQueue mEventQueue;
  Time mCurrentTime = 0;
  const Time stopTime = Converter::seconds(10);

  Simulator();
  ~Simulator();
  Simulator(const Simulator &) = delete;
  Simulator& operator =(const Simulator &) = delete;

  void parseMacTraffic();
  void parseMeasurements();
};

