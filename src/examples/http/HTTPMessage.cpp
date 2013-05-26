/**
	httpserver
	HTTPMessage.cpp
	Copyright 2011-2012 Ramsey Kant

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

#include "HTTPMessage.h"

HTTPMessage::HTTPMessage() : ByteBuffer(4096) {
    this->init();
}

HTTPMessage::HTTPMessage(std::string sData) : ByteBuffer(sData.size()+1) {
    putBytes((uint8_t*)sData.c_str(), sData.size()+1);
    this->init();
}

HTTPMessage::HTTPMessage(uint8_t* pData, uint32_t len) : ByteBuffer(pData, len) {
    this->init();
}

HTTPMessage::~HTTPMessage() {
    headers->clear();
    delete headers;
}

void HTTPMessage::init() {
	parseErrorStr = "";
	
	data = NULL;
	dataLen = 0;
	
	version = HTTP_VERSION; // By default, all create() will indicate the version is whatever HTTP_VERSION is
    
    headers = new std::map<std::string, std::string>();
}

/**
 * Put Line
 * Append a line (string) to the backing ByteBuffer at the current position
 *
 * @param str String to put into the byte buffer
 * @param crlf_end If true (default), end the line with a \r\n
 */
void HTTPMessage::putLine(std::string str, bool crlf_end) {
    // Terminate with crlf if flag set
    if(crlf_end)
        str += "\r\n";
    
    // Put the entire contents of str into the buffer
    putBytes((uint8_t*)str.c_str(), str.size());
}

/**
 * Put Headers
 * Write all headers currently in the 'headers' map to the ByteBuffer.
 * 'Header: value'
 */
void HTTPMessage::putHeaders() {
    std::map<std::string,std::string>::const_iterator it;
    for(it = headers->begin(); it != headers->end(); it++) {
		std::string final = it->first + ": " + it->second;
		putLine(final, true);
	}
	
	// End with a blank line
	putLine();
}

/**
 * Get Line
 * Retrive the entire contents of a line: string from current position until CR or LF, whichever comes first, then increment the read position
 * until it's past the last CR or LF in the line
 *
 * @return Contents of the line in a string (without CR or LF)
 */
std::string HTTPMessage::getLine() {
	std::string ret = "";
	int32_t startPos = getReadPos();
	bool newLineReached = false;
	char c = 0;

	// Append characters to the return std::string until we hit the end of the buffer, a CR (13) or LF (10)
	for(uint32_t i = startPos; i < size(); i++) {
		// If the next byte is a \r or \n, we've reached the end of the line and should break out of the loop
		c = peek();
		if((c == 13) || (c == 10)) {
			newLineReached = true;
			break;
		}

		// Otherwise, append the next character to the std::string
		ret += getChar();
	}

	// If a line termination was never reached, discard the result and conclude there are no more lines to parse
	if(!newLineReached) {
		setReadPos(startPos); // Reset the position to before the last line read that we are now discarding
		ret = "";
		return ret;
	}

	// Increment the read position until the end of a CR or LF chain, so the read position will then point32_t to the next line
	// Also, only read a maximum of 2 characters so as to not skip a blank line that is only \r\n
	uint32_t k = 0;
	for(uint32_t i = getReadPos(); i < size(); i++) {
		if(k++ >= 2)
			break;
		c = getChar();
		if((c != 13) && (c != 10)) {
			// Set the Read position back one because the retrived character wasn't a LF or CR
			setReadPos(getReadPos()-1);
			break;
		}
	}

	return ret;
}

/**
 * getStrElement
 * Get a token from the current buffer, stopping at the delimiter. Returns the token as a string
 *
 * @param delim The delimiter to stop at when retriving the element. By default, it's a space
 * @return Token found in the buffer. Empty if delimiter wasn't reached
 */
std::string HTTPMessage::getStrElement(char delim) {
    std::string ret = "";
    int32_t startPos = getReadPos();
    uint32_t size = 0;
    int32_t endPos = find(delim, startPos);

	// Calculate the size based on the found ending position
    size = (endPos+1) - startPos;

    if((endPos == -1) || (size <= 0))
        return "";
    
    // Grab the std::string from the byte buffer up to the delimiter
    char *str = new char[size];
    getBytes((uint8_t*)str, size);
	str[size-1] = 0x00; // NULL termination
    ret.assign(str);
    
    // Increment the read position PAST the delimiter
    setReadPos(endPos+1);
    
    return ret;
}

/**
 * Parse Headers
 * When an HTTP message (request & response) has reached the point32_t where headers are present, this method
 * should be called to parse and populate the internal map of headers.
 * Parse headers will move the read position past the blank line that signals the end of the headers
 */
void HTTPMessage::parseHeaders() {
	std::string hline = "", app = "";
	
	// Get the first header
	hline = getLine();

	// Keep pulling headers until a blank line has been reached (signaling the end of headers)
	while(hline.size() > 0) {
		// Case where values are on multiple lines ending with a comma
		app = hline;
		while(app[app.size()-1] == ',') {
			app = getLine();
			hline += app;
		}
		
		addHeader(hline);
		hline = getLine();
	}
}

/**
 * Parse Body
 * Parses everything after the headers section of an HTTP message. Handles chuncked responses/requests
 *
 * @return True if successful. False on error, parseErrorStr is set with a reason
 */
bool HTTPMessage::parseBody() {
	// Content-Length should exist (size of the Body data) if there is body data
	std::string hlenstr = "";
	uint32_t contentLen = 0;
	hlenstr = getHeaderValue("Content-Length");
	
	// No body data to read:
	if(hlenstr.empty())
		return true;

	contentLen = atoi(hlenstr.c_str());
	
	// contentLen should NOT exceed the remaining number of bytes in the buffer
	// Add 1 to bytesRemaining so it includes the byte at the current read position
	if(contentLen > bytesRemaining()+1) {
		/*
		// If it exceeds, read only up to the number of bytes remaining
		dataLen = bytesRemaining();
		*/
		// If it exceeds, there's a potential security issue and we can't reliably parse
		std::stringstream pes;
		pes << "Content-Length (" << hlenstr << ") is greater than remaining bytes (" << bytesRemaining() << ")";
		parseErrorStr = pes.str();
		return false;
	} else {
		// Otherwise, we ca probably trust Content-Length is valid and read the specificed number of bytes
		dataLen = contentLen;
	}

	// Create a big enough buffer to store the data
	uint32_t dIdx = 0, s = size();
	data = new uint8_t[dataLen];
	
	// Grab all the bytes from the current position to the end
	for(uint32_t i = getReadPos(); i < s; i++) {
		data[dIdx] = get(i);
		dIdx++;
	}
	
	// TODO: Handle chuncked Request/Response parsing (with footers) here
	
	return true;
}

/**
 * Add Header to the Map from string
 * Takes a formatted header string "Header: value", parse it, and put it into the std::map as a key,value pair.
 *
 * @param string containing formatted header: value
 */
void HTTPMessage::addHeader(std::string line) {
	std::string key = "", value = "";
	size_t kpos;
	int32_t i = 0;
	kpos = line.find(':');
	if(kpos == std::string::npos) {
		printf("Could not addHeader: %s\n", line.c_str());
		return;
	}
	key = line.substr(0, kpos);
	value = line.substr(kpos+1, line.size()-kpos-1);
	
	// Skip all leading spaces in the value
	while(value.at(i) == 0x20) {
		i++;
	}
	value = value.substr(i, value.size());
	
	// Add header to the map
	addHeader(key, value);
}

/**
 * Add header key-value std::pair to the map
 * 
 * @param key String representation of the Header Key
 * @param value String representation of the Header value
 */
void HTTPMessage::addHeader(std::string key, std::string value) {
    headers->insert(std::pair<std::string, std::string>(key, value));
}

/**
 * Add header key-value std::pair to the map (Integer value)
 * Integer value is converted to a string
 * 
 * @param key String representation of the Header Key
 * @param value Integer representation of the Header value
 */
void HTTPMessage::addHeader(std::string key, int32_t value) {
	std::stringstream sz;
	sz << value;
    headers->insert(std::pair<std::string, std::string>(key, sz.str()));
}

/**
 * Get Header Value
 * Given a header name (key), return the value associated with it in the headers map
 *
 * @param key Key to identify the header
 */
std::string HTTPMessage::getHeaderValue(std::string key) {
    // Lookup in map
    std::map<std::string, std::string>::const_iterator it;
    it = headers->find(key);
    
    // Key wasn't found, return a blank value
    if (it == headers->end())
        return "";
    
    // Otherwise, return the value
    return it->second;
}

/**
 * Get Header String
 * Get the full formatted header string "Header: value" from the headers map at position index
 * 
 * @ret Formatted string with header name and value
 */
std::string HTTPMessage::getHeaderStr(int32_t index) {
	int32_t i = 0;
	std::string ret = "";
    std::map<std::string,std::string>::const_iterator it;
    for(it = headers->begin(); it != headers->end(); it++) {
		if(i == index) {
			ret = it->first + ": " + it->second;
			break;
		}

		i++;
	}
	return ret;
}

/**
 * Get Number of Headers
 * Return the number of headers in the headers map
 *
 * @return size of the map
 */
int32_t HTTPMessage::getNumHeaders() {
	return headers->size();
}

/**
 * Clear Headers
 * Removes all headers from the internal map
 */
void HTTPMessage::clearHeaders() {
    headers->clear();
}

