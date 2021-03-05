SCALENE=../scalene

all: leak.so x
	LD_PRELOAD=./leak.so ./x

leak.so: leak.cxx
	g++ -g --std=c++17 -fPIC -I. -I$(SCALENE)/include -o $@ $< -shared -ldl

x: x.cxx
	g++ -g --std=c++17 -rdynamic -o $@ $<

clean:
	-rm -f x leak.so
