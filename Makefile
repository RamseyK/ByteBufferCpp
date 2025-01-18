# Makefile for ByteBuffer and accompanying test programs
# (C) Ramsey Kant 2011-2025

CXX = clang++
# Debug Flags
DEBUGFLAGS = -g -O0 
# Production Flags
PRODFLAGS = -O3

CXXFLAGS = -DBB_UTILITY=1 -std=c++23 -Wall -Wextra -Wno-sign-compare -Wno-missing-field-initializers -pedantic $(DEBUGFLAGS)

TEST_H   = src/ByteBuffer.hpp
TEST_SRC = src/ByteBuffer.cpp src/test.cpp

PACKETS_H   = src/ByteBuffer.hpp
PACKETS_SRC = src/ByteBuffer.cpp src/examples/packets/packets.cpp

HTTP_H   = src/ByteBuffer.hpp src/examples/http/HTTPMessage.h src/examples/http/HTTPRequest.h src/examples/http/HTTPResponse.h
HTTP_SRC = src/ByteBuffer.cpp src/examples/http/http.cpp src/examples/http/HTTPMessage.cpp src/examples/http/HTTPRequest.cpp src/examples/http/HTTPResponse.cpp

test: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) -o bin/$@ $(TEST_SRC)

packets: $(PACKETS_SRC)
	$(CXX) $(CXXFLAGS) -o bin/$@ $(PACKETS_SRC)

http: $(HTTP_SRC)
	$(CXX) $(CXXFLAGS) -o bin/$@ $(HTTP_SRC)

.PHONY: clean
clean:
	rm -f bin/test
	rm -f bin/packets
	rm -f bin/http
	rm -Rf *.dSYM
