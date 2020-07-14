/*

  Heap Layers: An Extensible Memory Allocation Infrastructure
  
  Copyright (C) 2000-2020 by Emery Berger
  http://www.emeryberger.com
  emery@cs.umass.edu
  
  Heap Layers is distributed under the terms of the Apache 2.0 license.

  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0

*/

#if !defined(_REAP_H_)
#define _REAP_H_

/**
 * @file reap.h
 * @brief The Reap API (plus the deprecated region-named API).
 * @author Emery Berger <http://www.cs.umass.edu/~emery>
 */

#if defined(_cplusplus)
extern "C" {
#endif

void reapcreate (void ** reap, void ** parent);
void * reapmalloc (void ** reap, size_t sz);
void reapfree (void ** reap, void * ptr);
void reapclear (void ** reap);
void reapdestroy (void ** reap);

int  regionFind (void ** reap, void * ptr);
void regionCreate (void ** reap, void ** parent);
void * regionAllocate (void ** reap, size_t sz);
void regionFree (void ** reap, void * ptr);
void regionFreeAll (void ** reap);
void regionDestroy (void ** reap);

#if defined(_cplusplus)
}
#endif

#endif
