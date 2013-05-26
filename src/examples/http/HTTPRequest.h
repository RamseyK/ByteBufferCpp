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

#ifndef _HTTPREQUEST_H_
#define _HTTPREQUEST_H_

#include "HTTPMessage.h"

class HTTPRequest : public HTTPMessage {
private:
    int32_t method;
    std::string requestUri;
    
protected:
    virtual void init();
    
public:
    HTTPRequest();
    HTTPRequest(std::string sData);
    HTTPRequest(uint8_t *pData, uint32_t len);
    virtual ~HTTPRequest();
    
    virtual uint8_t *create();
    virtual bool parse();

	// Helper functions

	int32_t methodStrToInt(std::string name);
    std::string methodIntToStr(uint32_t mid);
    
    // Info getters  & setters
    
    void setMethod(int32_t m) {
        method = m;
    }
    
    int32_t getMethod() {
        return method;
    }
    
    void setRequestUri(std::string u) {
        requestUri = u;
    }
    
    std::string getRequestUri() {
        return requestUri;
    }
};

#endif
