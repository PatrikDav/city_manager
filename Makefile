# City Manager - Makefile

CC = gcc

CFLAGS = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude -g

SRCDIR = src

OBJDIR = obj

INCDIR = include

TARGET  = city_manager
MONITOR = monitor_reports
SCORER  = scorer

SRCS = src/main.c \
       src/args.c \
       src/permissions.c \
       src/district.c \
       src/logger.c \
       src/report_io.c \
       src/symlinks.c \
       src/filter.c \
       src/remove_district.c \
       src/monitor_notify.c

OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

MONITOR_SRCS = src/monitor_reports.c
MONITOR_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(MONITOR_SRCS))

SCORER_SRCS = src/scorer.c
SCORER_OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SCORER_SRCS))

all: $(TARGET) $(MONITOR) $(SCORER)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(MONITOR): $(MONITOR_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SCORER): $(SCORER_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET) $(MONITOR) $(SCORER)

rebuild: clean all

.PHONY: all clean rebuild
