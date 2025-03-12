SIM_SRC := $(shell find src -name "*.c")
SW_SRC := $(shell find ../src -name "*.c")
SIM_HEADERS := $(shell find include -name "*.h")
SW_HEADERS := $(shell find ../include -name "*.h")

SIM_OBJS := $(patsubst src/%,obj/%,$(SIM_SRC:.c=.o))
SW_OBJS := $(patsubst ../src/%,sw_obj/%,$(SW_SRC:.c=.o))


CFLAGS := -Iinclude -g
LIBRARIES := -lm -lSDL2
EXE := main

obj/%.o: src/%.c $(SIM_HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< $(CFLAGS)

sw_obj/%.o: ../src/%.c $(SIM_HEADERS) $(SW_HEADERS)
	@mkdir -p sw_obj
	gcc -c -o $@ $< $(CFLAGS)

$(EXE): $(SIM_OBJS) $(SW_OBJS)
	gcc -o $@ $< $(LIBRARIES)