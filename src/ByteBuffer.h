/**
   ByteBuffer
   ByteBuffer.h
   Copyright 2011 Ramsey Kant

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef _BYTEBUFFER_H_
#define _BYTEBUFFER_H_

// Default number of bytes to allocate in the backing buffer if no size is provided
#define DEFAULT_SIZE 4096

// If defined, utility functions within the class are enabled
#define BB_UTILITY

#include <cstdlib>
#include <cstring>
#include <vector>

#ifdef BB_UTILITY
#include <iostream>
#include <stdio.h>
#endif

typedef unsigned char byte;

class ByteBuffer {
private:
	unsigned int rpos, wpos;
	std::vector<byte> buf;

#ifdef BB_UTILITY
	std::string name;
#endif

    template <typename T> T read() {
		T data = read<T>(rpos);
		rpos += sizeof(T);
		return data;
	}
	
	template <typename T> T read(unsigned int index) const {
		if(index + sizeof(T) <= buf.size())
			return *((T*)&buf[index]);
		return 0;
	}

	template <typename T> void append(T data) {
		unsigned int s = sizeof(data);

		if (size() < (wpos + s))
			buf.resize(wpos + s);
		memcpy(&buf[wpos], (byte*)&data, s);
		//printf("writing %c to %i\n", (byte)data, wpos);

		wpos += s;
	}
	
	template <typename T> void insert(T data, unsigned int index) {
		if((index + sizeof(data)) > size())
			return;

		memcpy(&buf[index], (byte*)&data, sizeof(data));
		wpos = index+sizeof(data);
	}

public:
	ByteBuffer(unsigned int size = DEFAULT_SIZE);
	ByteBuffer(byte* arr, unsigned int size);
	virtual ~ByteBuffer();

	unsigned int bytesRemaining(); // Number of bytes from the current read position till the end of the buffer
	void clear(); // Clear our the vector and reset read and write positions
	ByteBuffer* clone(); // Return a new instance of a bytebuffer with the exact same contents and the same state (rpos, wpos)
	//ByteBuffer compact(); // TODO?
	bool equals(ByteBuffer* other); // Compare if the contents are equivalent
	void resize(unsigned int newSize);
	unsigned int size(); // Size of internal vector
    
    // Basic Searching (Linear)
    template <typename T> int find(T key, unsigned int start=0) {
        int ret = -1;
        unsigned int len = buf.size();
        for(unsigned int i = start; i < len; i++) {
            T data = read<T>(i);
            // Wasn't actually found, bounds of buffer were exceeded
            if((key != 0) && (data == 0))
                break;
            
            // Key was found in array
            if(data == key) {
                ret = i;
                break;
            }
        }
        return ret;
    }
    
    // Replacement
    void replace(byte key, byte rep, unsigned int start = 0, bool firstOccuranceOnly=false);
	
	// Read

	byte peek(); // Relative peek. Reads and returns the next byte in the buffer from the current position but does not increment the read position
	byte get(); // Relative get method. Reads the byte at the buffers current position then increments the position
	byte get(unsigned int index); // Absolute get method. Read byte at index
	void getBytes(byte* buf, unsigned int len); // Absolute read into array buf of length len
	char getChar(); // Relative
	char getChar(unsigned int index); // Absolute
	double getDouble();
	double getDouble(unsigned int index);
	float getFloat();
	float getFloat(unsigned int index);
	int getInt();
	int getInt(unsigned int index);
	long getLong();
	long getLong(unsigned int index);
	short getShort();
	short getShort(unsigned int index);

	// Write

	void put(ByteBuffer* src); // Relative write of the entire contents of another ByteBuffer (src)
	void put(byte b); // Relative write
	void put(byte b, unsigned int index); // Absolute write at index
	void putBytes(byte* b, unsigned int len); // Relative write
	void putBytes(byte* b, unsigned int len, unsigned int index); // Absolute write starting at index
	void putChar(char value); // Relative
	void putChar(char value, unsigned int index); // Absolute
	void putDouble(double value);
	void putDouble(double value, unsigned int index);
	void putFloat(float value);
	void putFloat(float value, unsigned int index);
	void putInt(int value);
	void putInt(int value, unsigned int index);
	void putLong(long value);
	void putLong(long value, unsigned int index);
	void putShort(short value);
	void putShort(short value, unsigned int index);

	// Buffer Position Accessors & Mutators

	void setReadPos(unsigned int r) {
		rpos = r;
	}

	int getReadPos() {
		return rpos;
	}

	void setWritePos(unsigned int w) {
		wpos = w;
	}

	int getWritePos() {
		return wpos;
	}

	// Utility Functions
#ifdef BB_UTILITY
	void setName(std::string n);
	std::string getName();
	void printInfo();
	void printAH();
	void printAscii();
	void printHex();
	void printPosition();
#endif
};

#endif
