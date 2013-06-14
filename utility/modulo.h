// -*- C++ -*-

#ifndef HL_MODULO_H
#define HL_MODULO_H

/// A templated class that provides faster modulo functions when the
/// argument is a power of two.

#include "checkpoweroftwo.h"

namespace HL {

  template <unsigned long Modulus>
  class Modulo;

  template <unsigned long Modulus>
  class Modulo {
  public:
    static unsigned long mod (unsigned long m) {
      if (IsPowerOfTwo<Modulus>::VALUE) {
	return m & (Modulus - 1);
      } else {
	return m % Modulus;
      }
    }
  };

}

#endif
