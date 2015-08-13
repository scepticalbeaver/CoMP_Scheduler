#include "simulator.h"


int main()
{
  Simulator::instance();

  Simulator::instance()->run();


  Simulator::destroy();

  return 0;
}

