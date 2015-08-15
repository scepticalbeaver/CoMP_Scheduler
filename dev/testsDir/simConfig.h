#pragma once

#include "ns3/vector.h"

class UeConfig
{
public:
  static constexpr double xPosStart = 740.0;
  static constexpr double yPosStart = 9.0;
  static constexpr double zPosStart = 1.5;

  static constexpr double xVel = 0.7483;
  static constexpr double yVel = 0.5;

  static constexpr double xPosEnd(double seconds)
  {
    return xPosStart + xVel * seconds;
  }

  static constexpr double yPosEnd(double seconds)
  {
    return yPosStart + yVel * seconds;
  }

  static ns3::Vector ueVelocity()
  {
    return ns3::Vector(xVel, yVel, 0.0);
  }

  static ns3::Vector ueStartPos()
  {
    return ns3::Vector(xPosStart, yPosStart, zPosStart);
  }
};


const double simTime = 20.0;
