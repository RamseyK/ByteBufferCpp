# Makefile for ByteBuffer

CC = g++
DEBUG = -g -fpermissive
WARN = -Wall
# Debug Flags
DBGFLAGS = $(DEBUG) $(WARN)
# Production Flags
PRODFLAGS = $(WARN)

TEST_H   = src/ByteBuffer.h
TEST_SRC = src/ByteBuffer.cpp src/test.cpp

PACKETS_H   = src/ByteBuffer.h
PACKETS_SRC = src/ByteBuffer.cpp src/examples/packets/packets.cpp

HTTP_H   = src/ByteBuffer.h src/examples/http/HTTPMessage.h src/examples/http/HTTPRequest.h src/examples/http/HTTPResponse.h
HTTP_SRC = src/ByteBuffer.cpp src/examples/http/http.cpp src/examples/http/HTTPMessage.cpp src/examples/http/HTTPRequest.cpp src/examples/http/HTTPResponse.cpp

test: $(TEST_SRC) $(TEST_H)
	$(CC) $(DBGFLAGS) -o bin/$@ $(TEST_SRC)

packets: $(PACKETS_SRC) $(PACKETS_H)
	$(CC) $(DBGFLAGS) -o bin/$@ $(PACKETS_SRC)

http: $(HTTP_SRC) $(HTTP_H)
	$(CC) $(DBGFLAGS) -o bin/$@ $(HTTP_SRC)

clean:
	rm -f *.o bin/*.exe *~ \#*

macClean: clean
	rm -Rf *.dSYM
