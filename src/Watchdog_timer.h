#ifndef NEURONWIRED_WATCHDOG_TIMER_H
#define NEURONWIRED_WATCHDOG_TIMER_H

class Watchdog_timer
{
 public:
  void init(void);
  void reset(void);
};
void Watchdog_timer::init(void)
{
  // Empty function, we are not using the watchdog timer.
}
void Watchdog_timer::reset(void)
{
  // Empty function, we are not using the watchdog timer.
}

#endif  //NEURONWIRED_WATCHDOG_TIMER_H
