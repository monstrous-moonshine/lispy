objs = main.o mpc.o rl.o
MPCPATH = /home/samurai/Libraries/mpc
CXXFLAGS = -g -I$(MPCPATH)

vpath %.c $(MPCPATH)

app: $(objs) -lreadline
	g++ -o $@ $^

.PHONY: clean
clean:
	-rm app $(objs)
