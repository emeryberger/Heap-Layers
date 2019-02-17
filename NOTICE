[Heap Layers: An Extensible Memory Allocation Infrastructure](http://www.heaplayers.org)
-----------------------------------------------------------

Copyright (C) 2000 - 2018 by [Emery Berger](http://www.emeryberger.com)

Heap Layers is distributed under the terms of the Apache 2.0 license.

## Introduction ##

Heap Layers provides a flexible infrastructure for composing
high-performance memory allocators out of C++ _layers_. Heap Layers
makes it easy to write high-quality custom and general-purpose
memory allocators. Heap Layers has been used successfully to build
a number of high-performance allocators, including [Hoard](http://www.hoard.org) and [DieHard](http://www.diehard-software.org/).

## Using Heap Layers ##

For an introduction to Heap Layers, read the article [Policy-Based
Memory Allocation](http://www.drdobbs.com/policy-based-memory-allocation/184402039), by Andrei
Alexandrescu and Emery Berger. It's a good overview.

Heap Layers contains a number of ready-made heap components that can
be plugged together with minimal effort, and the result is often
faster than hand-built allocators. The PLDI 2001 paper [Composing
High-Performance Memory
Allocators](http://www.cs.umass.edu/~emery/pubs/berger-pldi2001.pdf)
has plenty of examples.

## Performance ##

Heap Layers can substantially outperform its only real competitor,
the Vmalloc package from AT&T. The OOPSLA 2002 paper [Reconsidering
Custom Memory
Allocation](http://www.cs.umass.edu/~emery/pubs/berger-oopsla2002.pdf)
paper has more details.

Not only is Heap Layers much higher level and simpler to use, but
its use of templates also improves performance. Heap Layers both
eliminates the function call overhead imposed by Vmalloc layers and
yields higher quality code by exposing more optimization
opportunities.

## Citation ##

To reference Heap Layers in an academic paper, please cite as follows:

```@inproceedings{DBLP:conf/pldi/BergerZM01,
  author    = {Emery D. Berger and
               Benjamin G. Zorn and
               Kathryn S. McKinley},
  title     = {Composing High-Performance Memory Allocators},
  booktitle = {Proceedings of the 2001 {ACM} {SIGPLAN} Conference on Programming
               Language Design and Implementation (PLDI), Snowbird, Utah, USA, June
               20-22, 2001},
  pages     = {114--124},
  year      = {2001},
  crossref  = {DBLP:conf/pldi/2001},
  url       = {https://doi.org/10.1145/378795.378821},
  doi       = {10.1145/378795.378821},
  timestamp = {Wed, 14 Nov 2018 10:54:59 +0100},
  biburl    = {https://dblp.org/rec/bib/conf/pldi/BergerZM01},
  bibsource = {dblp computer science bibliography, https://dblp.org}
}

@proceedings{DBLP:conf/pldi/2001,
  editor    = {Michael Burke and
               Mary Lou Soffa},
  title     = {Proceedings of the 2001 {ACM} {SIGPLAN} Conference on Programming
               Language Design and Implementation (PLDI), Snowbird, Utah, USA, June
               20-22, 2001},
  publisher = {{ACM}},
  year      = {2001},
  url       = {http://dl.acm.org/citation.cfm?id=378795},
  isbn      = {1-58113-414-2},
  timestamp = {Mon, 21 May 2012 16:19:53 +0200},
  biburl    = {https://dblp.org/rec/bib/conf/pldi/2001},
  bibsource = {dblp computer science bibliography, https://dblp.org}
}
```

