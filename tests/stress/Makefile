CC = gcc
CFLAGS = -g -Wall
TEST_ROOT = $(PWD)/..
INCLUDE_ROOT = $(PWD)/../../include

INCLUDES = -I$(TEST_ROOT)/common/mock -I$(TEST_ROOT)/common

SRCS = connection_interruption_test.c \
       $(TEST_ROOT)/common/test_framework.c \
       $(TEST_ROOT)/common/test_runner.c \
       $(TEST_ROOT)/common/mock/anarchy-device.c \
       $(TEST_ROOT)/common/utils/test_utils.c

OBJS = $(SRCS:.c=.o)

all: connection_interruption_test

connection_interruption_test: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) connection_interruption_test

.PHONY: all clean
