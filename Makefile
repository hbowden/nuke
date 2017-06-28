
SOURCES = 
INCLUDES =
LIBS = 
BINARY_NAME = nuke
MAIN = nuke.c
CFLAGS += -fpic -fno-strict-aliasing -Wall -O3
LDFLAGS += 

all:
	clang $(INCLUDES) $(LIBS) $(CFLAGS) -o $(BINARY_NAME) $(MAIN) $(SOURCES) $(LDFLAGS)