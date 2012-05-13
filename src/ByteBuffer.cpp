/**
   ByteBuffer
   ByteBuffer.cpp
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

#include "ByteBuffer.h"

/**
 * ByteBuffer constructor
 * Reserves specified size in internal vector
 * 
 * @param size Size (in bytes) of space to preallocate internally. Default is set in DEFAULT_SIZE
 */
ByteBuffer::ByteBuffer(unsigned int size) {
	buf.reserve(size);
	clear();
#ifdef BB_UTILITY
	name = "";
#endif
}

/**
 * ByteBuffer constructor
 * Consume an entire byte array of length len in the ByteBuffer
 * 
 * @param arr byte array of data (should be of length len)
 * @param size Size of space to allocate
 */
ByteBuffer::ByteBuffer(byte* arr, unsigned int size) {
	// If the provided array is NULL, allocate a blank buffer of the provided size
	if(arr == NULL) {
		buf.reserve(size);
		clear();
	} else { // Consume the provided array
		buf.reserve(size);
		clear();
		putBytes(arr, size);
	}
	
#ifdef BB_UTILITY
	name = "";
#endif
}

/**
 * ByteBuffer Deconstructor
 *
 */
ByteBuffer::~ByteBuffer() {
}

/**
 * Bytes Remaining
 * Returns the number of bytes from the current read position till the end of the buffer
 *
 * @return Number of bytes from rpos to the end (size())
 */
unsigned int ByteBuffer::bytesRemaining() {
	return size()-rpos;
}

/**
 * Clear
 * Clears out all data from the internal vector (original preallocated size remains), resets the positions to 0
 */
void ByteBuffer::clear() {
	rpos = 0;
	wpos = 0;
	buf.clear();
}

/**
 * Clone
 * Allocate an exact copy of the ByteBuffer on the heap and return a pointer
 *
 * @return A pointer to the newly cloned ByteBuffer. NULL if no more memory available
 */
ByteBuffer* ByteBuffer::clone() {
	ByteBuffer* ret = new ByteBuffer(buf.size());

	// Copy data
	for(unsigned int i = 0; i < buf.size(); i++) {
		ret->put(i, (byte)get(i));
	}

	// Reset positions
	ret->setReadPos(0);
	ret->setWritePos(0);

	return ret;
}

/**
 * Equals, test for data equivilancy
 * Compare this ByteBuffer to another by looking at each byte in the internal buffers and making sure they are the same
 *
 * @param other A pointer to a ByteBuffer to compare to this one
 * @return True if the internal buffers match. False if otherwise
 */
bool ByteBuffer::equals(ByteBuffer* other) {
	// If sizes aren't equal, they can't be equal
	if(size() != other->size())
		return false;

	// Compare byte by byte
	unsigned int len = size();
	for(unsigned int i = 0; i < len; i++) {
		if((byte)get(i) != (byte)other->get(i))
			return false;
	}

	return true;
}

/**
 * Resize
 * Reallocates memory for the internal buffer of size newSize. Read and write positions will also be reset
 *
 * @param newSize The amount of memory to allocate
 */
void ByteBuffer::resize(unsigned int newSize) {
	buf.resize(newSize);
	rpos = 0;
	wpos = 0;
}

/**
 * Size
 * Returns the size of the internal buffer...not necessarily the length of bytes used as data!
 *
 * @return size of the internal buffer
 */
unsigned int ByteBuffer::size() {
	return buf.size();
}

// Replacement

/**
 * Replace
 * Replace occurance of a particular byte, key, with the byte rep
 *
 * @param key Byte to find for replacement
 * @param rep Byte to replace the found key with
 * @param start Index to start from. By default, start is 0
 * @param firstOccuranceOnly If true, only replace the first occurance of the key. If false, replace all occurances. False by default
 */
void ByteBuffer::replace(byte key, byte rep, unsigned int start, bool firstOccuranceOnly) {
    unsigned int len = buf.size();
    for(unsigned int i = start; i < len; i++) {
        byte data = read<byte>(i);
        // Wasn't actually found, bounds of buffer were exceeded
        if((key != 0) && (data == 0))
            break;
        
        // Key was found in array, perform replacement
        if(data == key) {
            buf[i] = rep;
            if(firstOccuranceOnly)
                return;
        }
    }
}

// Read Functions

byte ByteBuffer::peek() {
	return read<byte>(rpos);
}

byte ByteBuffer::get() {
	return read<byte>();
}

byte ByteBuffer::get(unsigned int index) {
	return read<byte>(index);
}

void ByteBuffer::getBytes(byte* buf, unsigned int len) {
	for(unsigned int i = 0; i < len; i++) {
		buf[i] = read<byte>();
	}
}

char ByteBuffer::getChar() {
	return read<char>();
}

char ByteBuffer::getChar(unsigned int index) {
	return read<char>(index);
}

double ByteBuffer::getDouble() {
	return read<double>();
}

double ByteBuffer::getDouble(unsigned int index) {
	return read<double>(index);
}

float ByteBuffer::getFloat() {
	return read<float>();
}

float ByteBuffer::getFloat(unsigned int index) {
	return read<float>(index);
}

int ByteBuffer::getInt() {
	return read<int>();
}

int ByteBuffer::getInt(unsigned int index) {
	return read<int>(index);
}

long ByteBuffer::getLong() {
	return read<long>();
}

long ByteBuffer::getLong(unsigned int index) {
	return read<long>(index);
}

short ByteBuffer::getShort() {
	return read<short>();
}

short ByteBuffer::getShort(unsigned int index) {
	return read<short>(index);
}


// Write Functions

void ByteBuffer::put(ByteBuffer* src) {
	int len = src->size();
	for(int i = 0; i < len; i++)
		append<byte>(src->get(i));
}

void ByteBuffer::put(byte b) {
	append<byte>(b);
}

void ByteBuffer::put(byte b, unsigned int index) {
	insert<byte>(b, index);
}

void ByteBuffer::putBytes(byte* b, unsigned int len) {
	// Insert the data one byte at a time into the internal buffer at position i+starting index
	for(unsigned int i = 0; i < len; i++)
		append<byte>(b[i]);
}

void ByteBuffer::putBytes(byte* b, unsigned int len, unsigned int index) {
	wpos = index;

	// Insert the data one byte at a time into the internal buffer at position i+starting index
	for(unsigned int i = 0; i < len; i++)
		append<byte>(b[i]);
}

void ByteBuffer::putChar(char value) {
	append<char>(value);
}

void ByteBuffer::putChar(char value, unsigned int index) {
	insert<char>(value, index);
}

void ByteBuffer::putDouble(double value) {
	append<double>(value);
}

void ByteBuffer::putDouble(double value, unsigned int index) {
	insert<double>(value, index);
}
void ByteBuffer::putFloat(float value) {
	append<float>(value);
}

void ByteBuffer::putFloat(float value, unsigned int index) {
	insert<float>(value, index);
}

void ByteBuffer::putInt(int value) {
	append<int>(value);
}

void ByteBuffer::putInt(int value, unsigned int index) {
	insert<int>(value, index);
}

void ByteBuffer::putLong(long value) {
	append<long>(value);
}

void ByteBuffer::putLong(long value, unsigned int index) {
	insert<long>(value, index);
}

void ByteBuffer::putShort(short value) {
	append<short>(value);
}

void ByteBuffer::putShort(short value, unsigned int index) {
	insert<short>(value, index);
}

// Utility Functions
#ifdef BB_UTILITY
void ByteBuffer::setName(std::string n) {
	name = n;
}

std::string ByteBuffer::getName() {
	return name;
}

void ByteBuffer::printInfo() {
	unsigned int length = buf.size();
	std::cout << "ByteBuffer " << name.c_str() << " Length: " << length << ". Info Print" << std::endl;
}

void ByteBuffer::printAH() {
	unsigned int length = buf.size();
	std::cout << "ByteBuffer " << name.c_str() << " Length: " << length << ". ASCII & Hex Print" << std::endl;
	for(unsigned int i = 0; i < length; i++) {
		printf("0x%02x ", buf[i]);
	}
	printf("\n");
	for(unsigned int i = 0; i < length; i++) {
		printf("%c ", buf[i]);
	}
	printf("\n");
}

void ByteBuffer::printAscii() {
	unsigned int length = buf.size();
	std::cout << "ByteBuffer " << name.c_str() << " Length: " << length << ". ASCII Print" << std::endl;
	for(unsigned int i = 0; i < length; i++) {
		printf("%c ", buf[i]);
	}
	printf("\n");
}

void ByteBuffer::printHex() {
	unsigned int length = buf.size();
	std::cout << "ByteBuffer " << name.c_str() << " Length: " << length << ". Hex Print" << std::endl;
	for(unsigned int i = 0; i < length; i++) {
		printf("0x%02x ", buf[i]);
	}
	printf("\n");
}

void ByteBuffer::printPosition() {
	unsigned int length = buf.size();
	std::cout << "ByteBuffer " << name.c_str() << " Length: " << length << " Read Pos: " << rpos << ". Write Pos: " << wpos << std::endl;
}
#endif
