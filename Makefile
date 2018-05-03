CC = gcc
CXXFLAGS=-w -Wall
CXX_DEBUG_FLAGS=-g
CXX_RELEASE_FLAGS=-O3 -DNO_LOG
 
EXEC = server
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
STD_INCLUDES = 
LIBS = -levent_openssl -levent -lcrypto -lssl
INCLUDES = 

all: CXXFLAGS+=$(CXX_DEBUG_FLAGS)
all: INCLUDES=$(STD_INCLUDES)
all: $(EXEC)

release: CXXFLAGS+=$(CXX_RELEASE_FLAGS)
release: INCLUDES+=$(STD_INCLUDES)
release: $(EXEC)

# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) $(LIBS)
 
# To obtain object files
%.o: %.c
	$(CC) -c $(CXXFLAGS) $< $(INCLUDES) -o $@
 
# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)
