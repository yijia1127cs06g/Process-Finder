# Compiler
CC = /usr/bin/gcc

# Name of program
PROG = ps_command

# The name of the object files
OBJS = main.o

# All the header and c files
SRCS = main.c

# Add -I to the dir the curl include files are in
CFLAGS = -g -std=c99 -w -static

# Build the executable file
$(PROG): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROG)

# Seperately compile each .c file
main.o: main.c
	$(CC) $(CFLAGS) -c main.c


# Clean up crew
clean: 
	rm -fv core* $(PROG) $(OBJS)

cleaner: clean
	rm -fv #* *~

