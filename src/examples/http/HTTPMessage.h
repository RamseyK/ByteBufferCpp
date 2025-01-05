/**
    ByteBuffer
    HTTPMessage.h
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
#ifndef _HTTPMESSAGE_H_
#define _HTTPMESSAGE_H_

#include <map>
#include <string>
#include <sstream>

#include "../../ByteBuffer.hpp"

// Constants
constexpr std::string HTTP_VERSION_10 = "HTTP/1.0";
constexpr std::string HTTP_VERSION_11 = "HTTP/1.1";
constexpr std::string DEFAULT_HTTP_VERSION = HTTP_VERSION_11;
constexpr int32_t NUM_METHODS = 9;

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
    std::map<std::string, std::string> headers;

protected:
    std::string parseErrorStr = "";

    std::string version = DEFAULT_HTTP_VERSION; // By default, all create() will indicate the version is whatever DEFAULT_HTTP_VERSION is

    // Message Body Data (Resource in the case of a response, extra parameters in the case of a request)
    uint8_t* data = nullptr;
    uint32_t dataLen = 0;

public:
    HTTPMessage();
    explicit HTTPMessage(std::string const& sData);
    explicit HTTPMessage(const uint8_t* pData, uint32_t len);
    ~HTTPMessage() override = default;

    virtual uint8_t* create() = 0;
    virtual bool parse() = 0;

    // Create helpers
    void putLine(std::string str = "", bool crlf_end = true);
    void putHeaders();

    // Parse helpers
    std::string getLine();
    std::string getStrElement(char delim = 0x20); // 0x20 = "space"
    void parseHeaders();
    bool parseBody();

    // Header Map manipulation
    void addHeader(std::string const& line);
    void addHeader(std::string const& key, std::string const& value);
    void addHeader(std::string const& key, int32_t value);
    std::string getHeaderValue(std::string const& key) const;
    std::string getHeaderStr(int32_t index) const;
    uint32_t getNumHeaders() const;
    void clearHeaders();

    // Getters & Setters

    std::string getParseError() const {
        return parseErrorStr;
    }

    void setVersion(std::string_view v) {
        version = v;
    }

    std::string getVersion() const {
        return version;
    }

    void setData(uint8_t* d, uint32_t len) {
        data = d;
        dataLen = len;
    }

    uint8_t* getData() {
        return data;
    }

    uint32_t getDataLength() const {
        return dataLen;
    }
};

#endif
