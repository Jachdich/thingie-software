SIM_LIBS := mcufont st7789

SIM_SRC := $(shell find src -name "*.c")
SW_SRC := $(shell find ../src -name "*.c")
LIB_SRC := $(foreach dir,$(SIM_LIBS),$(shell find ../lib/$(dir) -name "*.c"))

SIM_HEADERS := $(shell find include -name "*.h")
SW_HEADERS := $(shell find ../include -name "*.h")

SIM_OBJS := $(patsubst src/%,obj/%,$(SIM_SRC:.c=.o))
SW_OBJS := $(patsubst ../src/%,sw_obj/%,$(SW_SRC:.c=.o))
# SW_OBJS := $(shell python3 create_obj_paths.py $(SW_SRC))
LIB_OBJS := $(shell python3 create_obj_paths.py $(LIB_SRC))
#$(pathsubst ../lib/%/%,lib_obj/%/%,$(LIB_SRC:.c=.o))

CFLAGS := -Iinclude -g -DTESTING -Wall
LIBRARIES := -lm -lSDL2 -lpthread
EXE := main

obj/%.o: src/%.c $(SIM_HEADERS)
	@mkdir -p obj
	gcc -c -o $@ $< $(CFLAGS)

sw_obj/%.o: ../src/%.c $(SIM_HEADERS) $(SW_HEADERS)
	@mkdir -p sw_obj
	gcc -c -o $@ $< $(CFLAGS)

$(EXE): $(SIM_OBJS) $(SW_OBJS) $(LIB_OBJS)
	@echo $(LIB_OBJS)
	gcc -o $@ $(SIM_OBJS) $(SW_OBJS) $(LIB_OBJS) $(LIBRARIES)

define GEN_RULE
lib_obj/$(dir)/%.o: ../lib/$(dir)/%.c $(SIM_HEADERS) $(SW_HEADERS)
	@mkdir -p lib_obj/$(dir)
	gcc -c -o $$@ $$< $(CFLAGS)
endef

$(foreach dir,$(SIM_LIBS),$(eval $(GEN_RULE)))

.PHONY: clean
clean:
	rm -rf obj sw_obj main
