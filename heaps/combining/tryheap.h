// -*- C++ -*-

/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2012 by Emery Berger
  http://www.cs.umass.edu/~emery
  emery@cs.umass.edu
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  
*/

#ifndef HL_TRYHEAP_H
#define HL_TRYHEAP_H

#include <cstddef>

namespace HL {

  template <class Heap1, class Heap2>
  class TryHeap : public Heap2 {
  public:

    TryHeap (void)
    {}

    inline void * malloc (size_t sz) {
      void * ptr = heap1.malloc (sz);
      if (ptr == NULL)
        ptr = Heap2::malloc (sz);
      return ptr;
    }

    inline void free (void * ptr) {
      heap1.free (ptr);
    }

  private:
    Heap1 heap1;
  };

}

#endif
