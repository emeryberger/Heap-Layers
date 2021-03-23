# for heapredirect and dependencies
SCALENE=../scalene

CXX=clang++
CXXFLAGS=-std=c++14 -O3 -g -DNDEBUG -fno-builtin-malloc -fPIC

ifeq ($(shell uname -s),Darwin)
  LD_PRELOAD=DYLD_INSERT_LIBRARIES
  WRAPPER=wrappers/macwrapper.cpp
  WRAPPER_FLAGS=
else
  LD_PRELOAD=LD_PRELOAD
  WRAPPER=wrappers/gnuwrapper.cpp
  WRAPPER_FLAGS=-D'CUSTOM_PREFIX(x)=xx\#\#x' -fvisibility=hidden -Bsymbolic
endif

all: leak.so x
	$(LD_PRELOAD)=./leak.so ./x

LEAK_SRC=leak.cxx $(WRAPPER)
leak.so: $(LEAK_SRC)
	$(CXX) $(CXXFLAGS) $(WRAPPERFLAGS) -I. -I$(SCALENE)/include -o $@ $(LEAK_SRC) -shared -ldl

x: x.cxx
	$(CXX) $(CXXFLAGS) -rdynamic -o $@ $< -lpthread

clean:
	-rm -f x leak.so
	-rm -rf *.dSYM
