objs = main.o mpc.o rl.o lval.o
MPCPATH = /home/samurai/Libraries/mpc
CXXFLAGS = -g --std=c++17 -I$(MPCPATH)

vpath %.c $(MPCPATH)

app: $(objs) -lreadline
	g++ -o $@ $^

lval.o: lval.hpp

.PHONY: clean
clean:
	-rm app $(objs)
