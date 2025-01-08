/**
 ByteBuffer
 HTTPRequest.h
 Copyright 2011-2025 Ramsey Kant

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

class HTTPRequest final : public HTTPMessage {
private:
    uint32_t method = 0;
    std::string requestUri = "";

public:
    HTTPRequest();
    explicit HTTPRequest(std::string const& sData);
    explicit HTTPRequest(const uint8_t* pData, uint32_t len);
    ~HTTPRequest() override = default;

    uint8_t* create() override;
    bool parse() override;

    // Helper functions

    uint32_t methodStrToInt(std::string_view name) const;
    std::string methodIntToStr(uint32_t mid) const;

    // Info getters  & setters
    void setMethod(uint32_t m) {
        method = m;
    }

    uint32_t getMethod() const {
        return method;
    }

    void setRequestUri(std::string_view u) {
        requestUri = u;
    }

    std::string getRequestUri() const {
        return requestUri;
    }
};

#endif
