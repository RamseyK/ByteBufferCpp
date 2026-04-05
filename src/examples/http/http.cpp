/**
 ByteBuffer
 http.cpp
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

#include "../../ByteBuffer.hpp"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

#include <format>
#include <memory>
#include <print>
#include <utility>

#include <cstring>

using namespace std;

int32_t main() {
    bool testFailed = false;

    std::print("HTTP Test Cases:\n");

    // Test getLine() in HTTPMessage
    auto msg = std::make_unique<HTTPRequest>("line1\r\nline2\nline3\n");
    string l1 = msg->getLine(); // Expected: "line1"
    if (strcmp(l1.c_str(), "line1") != 0) {
        std::print("l1 mismatch. Got: {}. Expected: line1\n", l1);
        testFailed = true;
    }
    string l2 = msg->getLine(); // Expected: "line2"
    if (strcmp(l2.c_str(), "line2") != 0) {
        std::print("l2 mismatch. Got: {}. Expected: line2\n", l2);
        testFailed = true;
    }
    string l3 = msg->getLine(); // Expected: "line3"
    if (strcmp(l3.c_str(), "line3") != 0) {
        std::print("l3 mismatch. Got: {}. Expected: line3\n", l3);
        testFailed = true;
    }
    string l4 = msg->getLine(); // Expected empty string
    if (!l4.empty()) {
        std::print("l4 mismatch. Got: {}. Expected to be blank\n", l4);
        testFailed = true;
    }

    std::print("{} ({})\n{} ({})\n{} ({})\n{} ({})\n\n", l1, l1.size(), l2, l2.size(), l3, l3.size(), l4, l4.size());

    // Test HTTPRequest parse()
    auto req = std::make_unique<HTTPRequest>("POST /sample/path.html HTTP/1.1\r\nHeader1: value1\r\nHeader2: value2\r\nHeader3: value3\r\nContent-Length: 5\r\n\r\ndata");

    if (!req->parse()) {
        std::print("HTTPRequest (req) had a parse error: {}\n", req->getParseError());
        testFailed = true;
    } else {
        std::print("HTTPRequest(req): {} {}\n", req->getMethod(), req->getVersion());
        uint8_t *data = req->getData();
        if (data == nullptr || std::strncmp((const char*)data, "data", 4) != 0) {
            std::print("HTTPRequest(req) expected data failed\n");
            testFailed = true;
        }
        std::print("Data (len={}):\n", req->getDataLength());
        for (uint32_t i = 0; i < req->getDataLength(); i++) {
            std::print("0x{:02x} ", data[i]);
        }
        std::print("\n");
        for (uint32_t i = 0;i < req->getDataLength(); i++) {
            std::print("{}", static_cast<char>(data[i]));
        }
        std::print("\n");
    }

    // Populate vars in an HTTPRequest to test create()
    string req2Content = "var=2";
    string req2ContentLen = std::format("{}", req2Content.size());
    uint32_t req2Size = 0;
    auto req2 = std::make_unique<HTTPRequest>();
    req2->setMethod(Method(POST));
    req2->setRequestUri("/dir/test.php");
    req2->addHeader("From", "user@example.com");;
    req2->addHeader("user-agent", "ByteBuffer/1.0");
    req2->addHeader("Content-Type", "text/html");
    req2->addHeader("Content-Length", req2ContentLen);
    if (req2->getHeaderValue("Content-Length") != req2ContentLen) {
        std::print("Content-Length value retrieval failed\n");
        testFailed = true;
    }
    // Tests lower-case check
    if (req2->getHeaderValue("User-Agent") != "ByteBuffer/1.0") {
        std::print("User-Agent value retrieval failed\n");
        testFailed = true;
    }
    req2->addHeader("Multi-Test", "line1,\r\nline2,\r\nline3");
    req2->setData((uint8_t*)req2Content.c_str(), req2Content.size());
    auto req2Ret = req2->create();
    req2Size = req2->size();
    std::print("\n\n");

    // Have req3 take the entire data from req2 and parse it
    std::print("Parsing req2 with req3:\n");
    auto req3 = std::make_unique<HTTPRequest>(req2Ret.get(), req2Size);
    if (!req3->parse()) {
        std::print("req3 parse error: {}\n", req3->getParseError());
        testFailed = true;
    } else {
        string req3Header = req3->methodIntToStr(req3->getMethod()) + " " + req3->getRequestUri() + " " + req3->getVersion();
        std::print("HTTPResponse(res3): {}\n", req3Header);
        std::print("req3 headers ({}):\n", req3->getNumHeaders());
        for (int32_t i = 0; i < req3->getNumHeaders(); i++) {
            std::print("[{}] {}\n", i, req3->getHeaderStr(i));
        }
        if (req3->getNumHeaders() != 5) {
            std::print("req3 - Expected 5 headers and got {}\n", req3->getNumHeaders());
            testFailed = true;
        }
        std::print("req3 data(len={}):\n", req3->getDataLength());
        if (req3->getData() == nullptr || std::strncmp((const char*)req3->getData(), "var=2", 5) != 0) {
            std::print("req3 expected data failed\n");
            testFailed = true;
        }
        uint8_t *req3Data = req3->getData();
        for (uint32_t i = 0; i < req3->getDataLength(); i++) {
            std::print("0x{:02x} ", req3Data[i]);
        }
        std::print("\n");
        for (uint32_t i = 0;i < req3->getDataLength(); i++) {
            std::print("{}", static_cast<char>(req3Data[i]));
        }
        std::print("\n\n");
    }

    // Test HTTPResponse(res) parse()
    auto res = std::make_unique<HTTPResponse>("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: 111\r\n\r\n<html><body>\n<h2>No Host: header received</h2>\nHTTP 1.1 requests must include the Host: header.\n</body></html>");
    if (!res->parse()) {
        std::print("res parse error: {}\n", res->getParseError());
        testFailed = true;
    } else {
        std::print("HTTPResponse(res): {} {}\n", res->getVersion(), res->getReason());
        std::print("res headers ({}):\n", res->getNumHeaders());
        for (int32_t i = 0; i < res->getNumHeaders(); i++) {
            std::print("{}\n", res->getHeaderStr(i));
        }
        if (res->getData() == nullptr || std::strncmp((const char*)res->getData(), "<html><body>\n<h2>No Host: header received</h2>\nHTTP 1.1 requests must include the Host: header.\n</body></html>", 110) != 0) {
            std::print("req3 expected data failed\n");
            testFailed = true;
        }
        std::print("res data(len={}):\n", res->getDataLength());
        uint8_t *resData = res->getData();
        for (uint32_t i = 0; i < res->getDataLength(); i++) {
            std::print("0x{:02x} ", resData[i]);
        }
        std::print("\n");
        for (uint32_t i = 0;i < res->getDataLength(); i++) {
            std::print("{}", static_cast<char>(resData[i]));
        }
        std::print("\n\n");
    }

    if (testFailed) {
        std::print("TEST PROGRAM FAILED: Read through output carefully to find point of failure\n");
        return 1;
    } else {
        std::print("TEST PROGRAM PASSED\n");
    }

    return 0;
}
