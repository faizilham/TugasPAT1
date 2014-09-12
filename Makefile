# generic makefile

CC = g++
EXT = cpp

# modules
MODULES = $(wildcard src/*.$(EXT))

LIB = 
INCLUDE = 

MODE = debug
# release or debug

FLAGS = 

ifeq ($(MODE), release)
FLAGS += -O2
else
FLAGS += -O0 -g
endif

# Everything after this is generic, no need to edit

OBJS := ${MODULES:src/%.$(EXT)=bin/%.o}

.PHONY: all run clean
  
all: $(OBJS)
	$(CC) -o bin/main $(OBJS) $(LIB)

run:
	bin/main
	
clean:
	rm -f bin/main bin/*.o

bin/%.o : src/%.$(EXT)
	$(CC) -c $< -o $@ $(FLAGS) $(INCLUDE)

