# City Manager - Makefile

CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -Iinclude -g

SRCDIR = src

OBJDIR = obj

INCDIR = include

TARGET = city_manager

SRCS = $(wildcard $(SRCDIR)/*.c)

OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild
