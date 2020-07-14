#ifndef HL_EXACTLYONE_H
#define HL_EXACTLYONE_H

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

/**
 * @class ExactlyOne
 * @brief Creates a singleton of type CLASS, accessed through ().
 * @author Emery Berger <http://www.emeryberger.org>
 */

#include <new>

namespace HL {

  template <class CLASS>
    class ExactlyOne {
  public:

    inline CLASS& operator()() {
      // We store the singleton in a double buffer to force alignment.
      static double buf[(sizeof(CLASS) + sizeof(double) - 1) / sizeof(double)];
      static CLASS * theOneTrueInstancePtr = new (buf) CLASS;
      return *theOneTrueInstancePtr;
    }

  };

}

#endif

