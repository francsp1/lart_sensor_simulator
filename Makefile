PROGRAM_OPT =args

TARGET_SRV =bin/server
TARGET_CLI =bin/client

CFLAGS =-std=c11 -Wall -Wextra -Wpedantic -pedantic -pedantic-errors -Wmissing-declarations -Wmissing-include-dirs -Wundef -Wfloat-equal -ggdb -D_POSIX_C_SOURCE=200809L# -Werror -pg

SRC_SRV =$(wildcard src/srv/*.c)
OBJ_S   =$(SRC_SRV:src/srv/%.c=obj/srv/%.o)
OBJ_SRV =$(OBJ_S) obj/common.o obj/srv/args/$(PROGRAM_OPT).o

SRC_CLI =$(wildcard src/cli/*.c) 
OBJ_C   =$(SRC_CLI:src/cli/%.c=obj/cli/%.o) 
OBJ_CLI =$(OBJ_C) obj/common.o obj/cli/args/$(PROGRAM_OPT).o

.PHONY: all default clean

all: obj/srv/args/$(PROGRAM_OPT).o obj/cli/args/$(PROGRAM_OPT).o $(TARGET_SRV) $(TARGET_CLI)

run: clean default
	./$(TARGET_SRV) --file "./mynewdb.db" --ip "127.0.0.1" --port 8080 --new 
#	./$(TARGET_CLI) 127.0.0.1
#	kill -9 $$(pidof dbserver)

default: clean $(TARGET_SRV) $(TARGET_CLI)

clean:
	rm -f inc/srv/args/$(PROGRAM_OPT).h 
	rm -f src/srv/args/$(PROGRAM_OPT).c
	rm -f obj/srv/args/$(PROGRAM_OPT).o
	rm -f inc/cli/args/$(PROGRAM_OPT).h 
	rm -f src/cli/args/$(PROGRAM_OPT).c
	rm -f obj/cli/args/$(PROGRAM_OPT).o
	rm -f obj/srv/*.o
	rm -f obj/cli/*.o
	rm -f obj/*.o
	rm -f bin/*
#	rm -f *.db

# Dependencies
# Server
obj/srv/main.o: src/srv/main.c inc/srv/main.h inc/common.h inc/srv/args/$(PROGRAM_OPT).h inc/srv/socket.h inc/srv/threads.h
obj/srv/args/$(PROGRAM_OPT).o: src/srv/args/$(PROGRAM_OPT).c inc/srv/args/$(PROGRAM_OPT).h src/srv/args/$(PROGRAM_OPT).ggo
obj/srv/socket.o: src/srv/socket.c inc/srv/socket.h inc/common.h
obj/srv/thread.o: src/srv/thread.c inc/srv/thread.h inc/common.h 
# Client
obj/cli/main.o: src/cli/main.c inc/cli/main.h inc/common.h inc/cli/args/$(PROGRAM_OPT).h 
obj/cli/args/$(PROGRAM_OPT).o: src/cli/args/$(PROGRAM_OPT).c inc/cli/args/$(PROGRAM_OPT).h src/cli/args/$(PROGRAM_OPT).ggo
# Common
obj/common.o: src/common.c inc/common.h

# Generates command line arguments code from gengetopt .ggo configuration file
# Server
src/srv/args/$(PROGRAM_OPT).c inc/srv/args/$(PROGRAM_OPT).h: src/srv/args/$(PROGRAM_OPT).ggo
	gengetopt --input=src/srv/args/$(PROGRAM_OPT).ggo --file-name=$(PROGRAM_OPT) --header-output-dir=inc/srv/args --src-output-dir=src/srv/args
# Client
src/cli/args/$(PROGRAM_OPT).c inc/cli/args/$(PROGRAM_OPT).h: src/cli/args/$(PROGRAM_OPT).ggo
	gengetopt --input=src/cli/args/$(PROGRAM_OPT).ggo --file-name=$(PROGRAM_OPT) --header-output-dir=inc/cli/args --src-output-dir=src/cli/args

# Generate gengetopt .o files with no warnings
# Server
obj/srv/args/$(PROGRAM_OPT).o: src/srv/args/$(PROGRAM_OPT).c inc/srv/args/$(PROGRAM_OPT).h
	gcc -ggdb -std=c11 -pedantic -c $< -o $@ -Iinc -Iinc/srv -Iinc/srv/args
# Client
obj/cli/args/$(PROGRAM_OPT).o: src/cli/args/$(PROGRAM_OPT).c inc/cli/args/$(PROGRAM_OPT).h
	gcc -ggdb -std=c11 -pedantic -c $< -o $@ -Iinc -Iinc/cli -Iinc/cli/args


obj/common.o: src/common.c inc/common.h
	gcc $(CFLAGS) -c $< -o $@ -Iinc

# Compile Server
$(TARGET_SRV): $(OBJ_SRV) 
	gcc -o $@ $^

# Generate .o files from every .c file in src/srv
$(OBJ_S): obj/srv/%.o: src/srv/%.c
	gcc $(CFLAGS) -c $< -o $@ -Iinc -Iinc/srv -Iinc/srv/args

# Compile Client
$(TARGET_CLI): $(OBJ_CLI) 
	gcc -o $@ $^

# Generate .o files from every .c file in src/cli
$(OBJ_C): obj/cli/%.o: src/cli/%.c
	gcc $(CFLAGS) -c $< -o $@ -Iinc -Iinc/cli -Iinc/cli/args