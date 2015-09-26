#include "simulator.h"


int main()
{
  Simulator::instance()->run();

  Simulator::destroy();

  return 0;
}

