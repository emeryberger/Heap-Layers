# for heapredirect and dependencies
SCALENE=../scalene

ifeq ($(shell uname -s),Darwin)
  LD_PRELOAD=DYLD_INSERT_LIBRARIES
  WRAPPER=wrappers/macwrapper.cpp
else
  LD_PRELOAD=LD_PRELOAD
  WRAPPER=wrappers/gnuwrapper.cpp
endif

all: leak.so x
	$(LD_PRELOAD)=./leak.so ./x

leak.so: leak.cxx
	g++ -g --std=c++17 -fPIC -I. -I$(SCALENE)/include -o $@ $< $(WRAPPER) -shared -ldl

x: x.cxx
	g++ -g --std=c++17 -rdynamic -o $@ $<

clean:
	-rm -f x leak.so
	-rm -rf *.dSYM
