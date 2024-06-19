CC = clang
CFLAGS = -Wall -Wpedantic  -Wextra -Werror -g
LDFLAGS = -lpthread

SRCS = main.c queue.c threads.c mlfq.c
OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFALGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
