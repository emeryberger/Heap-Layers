# for heapredirect and dependencies
SCALENE=../scalene

CXX=clang++
CXXFLAGS=-std=c++14 -O3 -g -DNDEBUG -fno-builtin-malloc -fPIC
LEAKCXXFLAGS=-I. -I$(SCALENE)/include
LEAKLDFLAGS=-shared
LEAKLDLIBS=-ldl

LIBBACKTRACE=../libbacktrace
ifdef LIBBACKTRACE
  LEAKCXXFLAGS+=-DUSE_LIBBACKTRACE=1 -I$(LIBBACKTRACE)
  LEAKLDLIBS+=$(LIBBACKTRACE)/.libs/libbacktrace.a
endif

ifeq ($(shell uname -s),Darwin)
  LD_PRELOAD=DYLD_INSERT_LIBRARIES
  WRAPPER=wrappers/macwrapper.cpp
else
  LD_PRELOAD=LD_PRELOAD
  WRAPPER=wrappers/gnuwrapper.cpp
  LEAKCXXFLAGS+=-D'CUSTOM_PREFIX(x)=xx\#\#x' -fvisibility=hidden
  LEAKLDFLAGS+=-Bsymbolic
endif

all: leak.so x
	$(LD_PRELOAD)=./leak.so ./x

LEAK_SRC=leak.cxx $(WRAPPER)
leak.so: $(LEAK_SRC)
	$(CXX) $(CXXFLAGS) $(LEAKCXXFLAGS) -o $@ $(LEAK_SRC) $(LEAKLDFLAGS) $(LEAKLDLIBS)

x: x.cxx
	$(CXX) $(CXXFLAGS) -rdynamic -o $@ $< -lpthread

clean:
	-rm -f x leak.so
ifeq ($(shell uname -s),Darwin)
	-rm -rf *.dSYM
endif
