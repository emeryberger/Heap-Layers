/* -*- C++ -*- */

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

#ifndef HL_TIMER_H
#define HL_TIMER_H

/**
 * @class Timer
 * @brief A portable class for high-resolution timing.
 *
 * This class simplifies timing measurements across a number of platforms.
 * 
 * @code
 *  Timer t;
 *  t.start();
 *  // do some work
 *  t.stop();
 *  cout << "That took " << (double) t << " seconds." << endl;
 * @endcode
 *
 */

/* Updated to use new C++11 high-resolution timer classes. */

#if (__cplusplus < 201103)
#include "timer-old.h"
#else

#include <chrono>

namespace HL {

  class Timer {
  public:

    Timer() {
      _start = _end;
      _elapsed = 0.0;
    }

    void start() {
      _start = std::chrono::high_resolution_clock::now();
      _end = _start;
    }

    void stop() {
      _end = std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> elapsed_seconds = _end - _start;
      _elapsed = elapsed_seconds.count();
    }

    operator double() {
      return _elapsed;
    }

  private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _start, _end;
    double _elapsed;
  };

}

#endif // version of C++

#endif
