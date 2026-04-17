# City Manager - Makefile

CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude -g

SRCDIR = src

OBJDIR = obj

INCDIR = include

TARGET = city_manager

SRCS = src/main.c \
       src/args.c \
       src/permissions.c \
       src/district.c \
       src/logger.c \
       src/report_io.c \
       src/symlinks.c \
       src/filter.c

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
