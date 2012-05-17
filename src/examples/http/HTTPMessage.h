/**
 ByteBuffer
 HTTPMessage.h
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

#ifndef _HTTPMessage_h
#define _HTTPMessage_h

#include <map>
#include <string>
#include <sstream>

#include "../../ByteBuffer.h"

// Constants
#define HTTP_VERSION "HTTP/1.1"
#define NUM_METHODS 9

// HTTP Methods (Requests)

enum Method {
	HEAD = 0,
	GET = 1,
	POST = 2,
	PUT = 3,
	DEL = 4, // DELETE is taken, use DEL instead
	TRACE = 5,
	OPTIONS = 6,
	CONNECT = 7,
	PATCH = 8
};

const static char* const requestMethodStr[NUM_METHODS] = {
	"HEAD", // 0
	"GET", // 1
	"POST", // 2
	"PUT", // 3
	"DELETE", // 4
	"TRACE", // 5
	"OPTIONS", // 6
	"CONNECT", // 7
	"PATCH" // 8
};


// HTTP Response Status codes
enum Status {
    // 1xx Informational
    CONTINUE = 100,
    
    // 2xx Success
    OK = 200,
    
    // 3xx Redirection
    
    // 4xx Client Error
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    
    // 5xx Server Error
    SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501
};

class HTTPMessage : public ByteBuffer {
private:
    std::map<std::string, std::string> *headers;

protected:
	std::string parseErrorStr;
	
	std::string version;
	
	// Message Body Data (Resource in the case of a response, extra parameters in the case of a request)
	byte* data;
	unsigned int dataLen;

protected:
    virtual void init();
    
public:
    HTTPMessage();
    HTTPMessage(std::string sData);
    HTTPMessage(byte *pData, unsigned int len);
    virtual ~HTTPMessage();
    
	virtual byte* create() = 0;
	virtual bool parse() = 0;
    
    // Create helpers
    void putLine(std::string str = "", bool crlf_end=true);
    void putHeaders();
    
    // Parse helpers
	std::string getLine();
    std::string getStrElement(char delim = 0x20); // 0x20 = "space"
    void parseHeaders();
    bool parseBody();
    
    // Header Map manipulation
	void addHeader(std::string line);
    void addHeader(std::string key, std::string value);
	void addHeader(std::string key, int value);
    std::string getHeaderValue(std::string key);
	std::string getHeaderStr(int index);
	int getNumHeaders();
    void clearHeaders();
    
    // Getters & Setters

	std::string getParseError() {
		return parseErrorStr;
	}
	
    void setVersion(std::string v) {
        version = v;
    }
    
    std::string getVersion() {
        return version;
    }
    
	void setData(byte* d, unsigned int len) {
		data = d;
		dataLen = len;
	}

	byte* getData() {
		return data;
	}

	unsigned int getDataLength() {
		return dataLen;
	}
};

#endif
