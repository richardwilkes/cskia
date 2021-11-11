CSKIA_DIST := ../dist
CC := clang
SRC = $(wildcard *.c)
OBJ = ${SRC:.c=.o}

OUT = hello_world

UNAME := $(shell uname -s)
ifeq ($(UNAME), Darwin)	# Mac OS X
	CSKIA_LIB=-L$(CSKIA_DIST)/lib/darwin
	LIBS=-framework OpenGL 
else
$(error Unsupported OS: >$(UNAME)<)	
endif

CSKIA_HEADER=-I$(CSKIA_DIST)/include
CFLAGS= -Wall -std=c99 -pedantic $(CSKIA_HEADER)

GLFW_LIB=`pkg-config --libs glfw3 --static`
SKIA_LIB=-lskia -lc++
LDFLAGS=$(CSKIA_LIB) $(SKIA_LIB) $(GLFW_LIB) $(LIBS)

.PHONY: all run

all: $(OUT) run

$(CSKIA_DIST):
	@printf "\e[31mError: folder >$(CSKIA_DIST)< does not exist, build CSkia first...\e[0m\n"
	exit 1

$(OUT): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)
	@printf "\e[33mLinking\e[90m %s\e[0m\n" $@
	@printf "\e[34mDone!\e[0m\n"

run: $(OUT)
	./$(OUT)	

%.o: %.c
	@$(CC) $(CFLAGS) -c $< -o $@
	@printf "\e[36mCompile\e[90m %s\e[0m\n" $@

clean:
	@rm -f $(OUT) $(OBJ)
	@printf "\e[34mAll clear!\e[0m\n"
