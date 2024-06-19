CC = clang
CFLAGS = -Wall -Wpedantic  -Wextra -Werror -g

SRCS = main.c fillStructs.c info.c list.c get.c
OBJS = $(SRCS:.c=.o)
TARGET = main

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFALGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(TARGET)
