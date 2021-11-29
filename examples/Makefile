EXAMPLES := $(shell  ls -d */ | sed -e 's^/^^' -e'/xcode/d')
CSKIA_DIST := ../dist

UNAME := $(shell uname -s)
ifeq ($(UNAME), Darwin)	# Mac OS X
	CC := clang
	CSKIA_LIB:=-L$(CSKIA_DIST)/lib/darwin
	LIBS:=-framework OpenGL
	GLFW_LIB:=`pkg-config --libs glfw3 --static`
else ifeq ($(UNAME), Linux)	# Linux
	CC := gcc
	CSKIA_LIB:=-L$(CSKIA_DIST)/lib/linux
	LIBS:=-lGL -lpthread -lstdc++ -lfreetype  -lfontconfig
	GLFW_LIB:=`pkg-config --libs glfw3 --static`
else
$(error Unsupported OS: >$(UNAME)<)	
endif

SRC = $(wildcard */*.c)
OBJ = ${SRC:.c=.o}
OUT = $(foreach e, $(EXAMPLES), $(addprefix $(e)/,$(e)) )

CSKIA_HEADER:=-I$(CSKIA_DIST)/include
CFLAGS:= -Wall -std=c99 -pedantic $(CSKIA_HEADER)

SKIA_LIB:=-lskia -lc++
LDFLAGS:=$(CSKIA_LIB) $(SKIA_LIB) $(GLFW_LIB) $(LIBS)

.PHONY: all run

all: $(OUT)

%.o: %.c
	@printf "\e[36mCompiling\e[90m %s\e[0m\n" $@
	@$(CC) $(CFLAGS) -c $< -o $@

$(OUT): $(OBJ) 
	@printf "\e[33mLinking\e[90m %s\e[0m\n" $@
	@$(CC) $(filter $(dir $@)%,$^) -o $@ $(LDFLAGS)

clean:
	@rm -f $(OUT) $(OBJ)
	@printf "\e[34mAll clear!\e[0m\n"

