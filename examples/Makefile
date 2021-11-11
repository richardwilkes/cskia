SRC = $(wildcard *.c)
OBJ = ${SRC:.c=.o}
CC = clang

OUT = hello_world

GLFW_LIB=`pkg-config --libs glfw3 --static`
LIBS=-framework OpenGL 
LDFLAGS=$(GLFW_LIB) $(LIBS)
CFLAGS= -Wall -std=c99 -pedantic

.PHONY: all run

all: $(OUT) run

$(OUT): $(OBJ)
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@printf "\e[33mLinking\e[90m %s\e[0m\n" $@
	@printf "\e[34mDone!\e[0m\n"

run: $(OUT)
	./$(OUT)	

%.o: %.c
	@$(CC) -c $< -o $@
	@printf "\e[36mCompile\e[90m %s\e[0m\n" $@

clean:
	@rm -f $(OUT) $(OBJ)
	@printf "\e[34mAll clear!\e[0m\n"
