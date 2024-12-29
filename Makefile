#CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lrt

SRC = drm_input.c
OBJ = $(SRC:.c=.o)
EXEC = drm_input

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)

.PHONY: all clean