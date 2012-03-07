/**
 ByteBuffer
 HTTPRequest.h
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

#ifndef _HTTPRequest_h
#define _HTTPRequest_h

#include "HTTPMessage.h"

class HTTPRequest : public HTTPMessage {
private:
    int method;
    string requestUri;
    string version;
	byte* createRetData;
	byte* data;
	unsigned int dataLen;

	bool createCached;
    
protected:
    virtual void init();
    
public:
    HTTPRequest();
    HTTPRequest(string sData);
    HTTPRequest(byte *pData, unsigned int len);
    virtual ~HTTPRequest();
    
    virtual byte *create(bool freshCreate=false);
    virtual void parse();

	// Helper functions

	int methodStrToInt(string name);
    string methodIntToStr(unsigned int mid);
    
    // Info getters  & setters
    
    void setMethod(int m) {
        method = m;
    }
    
    int getMethod() {
        return method;
    }
    
    void setRequestUri(string u) {
        requestUri = u;
    }
    
    string getRequestUri() {
        return requestUri;
    }
    
    void setVersion(string v) {
        version = v;
    }
    
    string getVersion() {
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
