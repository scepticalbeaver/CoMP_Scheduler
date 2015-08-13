#pragma once

#include "eventQueue.h"
#include "lteEnb/l2-mac.h"

class Simulator
{
public:
  static Simulator* instance();
  static void destroy();

  void run();

  void scheduleEvent(Event event);
  Time getTime() const;

private:
  static Simulator* mSimulator;
  EventQueue mEventQueue;
  Time mCurrentTime = Converter::microseconds(0);
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

