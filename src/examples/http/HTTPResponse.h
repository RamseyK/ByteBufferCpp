/**
 ByteBuffer
 HTTPResponse.h
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

#ifndef _HTTPResponse_h
#define _HTTPResponse_h

#include <ctime>

#include "../../ByteBuffer.h"

class HTTPResponse : public HTTPMessage {
private:    
    // Response variables
    int status;

protected:
	virtual void init();
    
public:
    HTTPResponse();
    HTTPResponse(string sData);
    HTTPResponse(byte *pData, unsigned int len);
    virtual ~HTTPResponse();
    
    virtual byte* create(bool freshCreate=false);
    virtual void parse();
    
    // Accessors & Mutators
    
    string getStatusStr();
    void setStatus (int scode) {
        status = scode;
    }
};

#endif
