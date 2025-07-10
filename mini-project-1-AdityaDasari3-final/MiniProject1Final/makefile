CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = 

SRCS = main.c GetDirectory.c ChangeDirectory.c Reveal.c log.c Proclore.c \
       Seek.c IORedirection.c myshrc.c piping.c activities.c fgbg.c \
       neonate.c iman.c ping.c

OBJS = $(SRCS:.c=.o)

TARGET = myshell

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

main.o: main.c $(wildcard *.h)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean