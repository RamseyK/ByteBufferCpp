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

#include <cstring>
#include <format>
#include <memory>
#include <print>

using namespace std;

static int failures = 0;

static void check(bool cond, string_view msg) {
    if (!cond) {
        std::print("  FAIL: {}\n", msg);
        ++failures;
    }
}

int32_t main() {

    // --- getLine() ---
    std::print("== getLine() ==\n");
    {
        auto msg = std::make_unique<HTTPRequest>("line1\r\nline2\nline3\n");
        string l1 = msg->getLine();
        string l2 = msg->getLine();
        string l3 = msg->getLine();
        string l4 = msg->getLine();
        check(l1 == "line1", "getLine CRLF terminator: \"line1\"");
        check(l2 == "line2", "getLine LF terminator:   \"line2\"");
        check(l3 == "line3", "getLine LF terminator:   \"line3\"");
        check(l4.empty(),    "getLine at end of buffer returns empty string");
        // Sizes
        check(l1.size() == 5, "l1 length == 5");
        check(l2.size() == 5, "l2 length == 5");
        check(l3.size() == 5, "l3 length == 5");
        check(l4.size() == 0, "l4 length == 0");
    }

    // --- HTTPRequest parse(): POST with body ---
    std::print("== HTTPRequest parse() POST ==\n");
    {
        auto req = std::make_unique<HTTPRequest>(
            "POST /sample/path.html HTTP/1.1\r\n"
            "Header1: value1\r\n"
            "Header2: value2\r\n"
            "Header3: value3\r\n"
            "Content-Length: 5\r\n"
            "\r\n"
            "data\0");

        check(req->parse(), std::format("parse() succeeded (error: {})", req->getParseError()));
        check(req->getMethod()     == POST,             "method == POST");
        check(req->getRequestUri() == "/sample/path.html", "request URI");
        check(req->getVersion()    == "HTTP/1.1",       "version == HTTP/1.1");
        check(req->getNumHeaders() == 4,                "4 headers parsed");
        check(req->getHeaderValue("Header1")        == "value1", "Header1: value1");
        check(req->getHeaderValue("Header2")        == "value2", "Header2: value2");
        check(req->getHeaderValue("Header3")        == "value3", "Header3: value3");
        check(req->getHeaderValue("Content-Length") == "5",      "Content-Length: 5");

        uint8_t* data = req->getData();
        check(data != nullptr,                                    "body data pointer non-null");
        check(req->getDataLength() == 5,                          "body data length == 5");
        check(std::strncmp((const char*)data, "data\0", 5) == 0,  "body data content matches");
    }

    // --- HTTPRequest header value lookup (case-insensitive) ---
    std::print("== HTTPRequest header lookup ==\n");
    {
        auto req2 = std::make_unique<HTTPRequest>();
        string req2Content = "var=2";
        string req2ContentLen = std::format("{}", req2Content.size());
        req2->setMethod(Method(POST));
        req2->setRequestUri("/dir/test.php");
        req2->addHeader("From", "user@example.com");
        req2->addHeader("user-agent", "ByteBuffer/1.0");
        req2->addHeader("Content-Type", "text/html");
        req2->addHeader("Content-Length", req2ContentLen);
        req2->addHeader("Multi-Test", "line1,\r\nline2,\r\nline3");
        req2->setData((uint8_t*)req2Content.c_str(), req2Content.size());

        // Exact case lookup
        check(req2->getHeaderValue("Content-Length") == req2ContentLen,
              "Content-Length value retrieval (exact case)");
        // Case-insensitive lookup
        check(req2->getHeaderValue("User-Agent") == "ByteBuffer/1.0",
              "User-Agent value retrieval (case-insensitive)");
        check(req2->getHeaderValue("FROM") == "user@example.com",
              "From value retrieval (all caps key)");
        check(req2->getHeaderValue("content-type") == "text/html",
              "Content-Type retrieval (lowercase key)");
        // Missing header
        check(req2->getHeaderValue("X-Missing").empty(),
              "missing header returns empty string");

        // --- HTTPRequest create() then re-parse (round-trip) ---
        std::print("== HTTPRequest create()/parse() round-trip ==\n");
        auto req2Ret  = req2->create();
        uint32_t req2Size = req2->size();
        check(req2Ret != nullptr, "create() returns non-null");
        check(req2Size > 0,       "create() produces non-empty output");

        auto req3 = std::make_unique<HTTPRequest>(req2Ret.get(), req2Size);
        check(req3->parse(), std::format("req3 parse() succeeded (error: {})", req3->getParseError()));
        check(req3->getMethod()     == POST,          "req3 method == POST");
        check(req3->getRequestUri() == "/dir/test.php", "req3 request URI");
        check(req3->getVersion()    == "HTTP/1.1",    "req3 version");
        check(req3->getNumHeaders() == 5,             "req3 has 5 headers");
        check(req3->getHeaderValue("From")           == "user@example.com", "req3 From header");
        check(req3->getHeaderValue("user-agent")     == "ByteBuffer/1.0",   "req3 user-agent header");
        check(req3->getHeaderValue("Content-Type")   == "text/html",        "req3 Content-Type header");
        check(req3->getHeaderValue("Content-Length") == req2ContentLen,     "req3 Content-Length header");
        uint8_t* req3Data = req3->getData();
        check(req3Data != nullptr,                                                         "req3 body data non-null");
        check(req3->getDataLength() == (uint32_t)req2Content.size(),                      "req3 body length matches");
        check(std::strncmp((const char*)req3Data, req2Content.c_str(), req2Content.size()) == 0,
              "req3 body content matches");
    }

    // --- HTTPRequest parse(): GET (no body) ---
    std::print("== HTTPRequest parse() GET ==\n");
    {
        auto getReq = std::make_unique<HTTPRequest>(
            "GET /index.html HTTP/1.1\r\n"
            "Accept: text/html\r\n"
            "Connection: keep-alive\r\n"
            "\r\n");
        check(getReq->parse(),                                       "GET request parses successfully");
        check(getReq->getMethod()     == GET,                        "GET method");
        check(getReq->getRequestUri() == "/index.html",              "GET request URI");
        check(getReq->getVersion()    == "HTTP/1.1",                 "GET version");
        check(getReq->getNumHeaders() == 2,                          "GET has 2 headers");
        check(getReq->getHeaderValue("Accept")     == "text/html",   "GET Accept header");
        check(getReq->getHeaderValue("Connection") == "keep-alive",  "GET Connection header");
        check(getReq->getDataLength() == 0, "GET has no body");
    }

    // --- HTTPRequest parse(): invalid/malformed inputs ---
    std::print("== HTTPRequest parse() invalid inputs ==\n");
    {
        // Empty body
        auto emptyReq = std::make_unique<HTTPRequest>("");
        check(!emptyReq->parse(), "empty request fails to parse");
        check(!emptyReq->getParseError().empty(), "empty request has parse error message");

        // Unknown method
        auto badMethod = std::make_unique<HTTPRequest>("BADMETHOD /path HTTP/1.1\r\n\r\n");
        check(!badMethod->parse(), "unknown method fails to parse");
        check(!badMethod->getParseError().empty(), "unknown method has parse error message");

        // Missing URI
        auto noUri = std::make_unique<HTTPRequest>("GET HTTP/1.1\r\n\r\n");
        check(!noUri->parse(), "request without URI fails to parse");
    }

    // --- HTTPResponse parse() ---
    std::print("== HTTPResponse parse() ==\n");
    {
        const string responseBody =
            "<html><body>\n"
            "<h2>No Host: header received</h2>\n"
            "HTTP 1.1 requests must include the Host: header.\n"
            "</body></html>";
        auto res = std::make_unique<HTTPResponse>(
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 111\r\n"
            "\r\n" + responseBody);

        check(res->parse(), std::format("HTTPResponse parse() succeeded (error: {})", res->getParseError()));
        check(res->getVersion() == "HTTP/1.1",  "response version");
        check(res->getReason()  == "Bad Request", "response reason");
        check(res->getNumHeaders() == 2,         "response has 2 headers");
        check(res->getHeaderValue("Content-Type")   == "text/html", "response Content-Type");
        check(res->getHeaderValue("Content-Length") == "111",       "response Content-Length");
        uint8_t* resData = res->getData();
        check(resData != nullptr,                    "response body data non-null");
        check(res->getDataLength() == 111,           "response body length == 111");
        check(std::strncmp((const char*)resData, responseBody.c_str(), responseBody.size()) == 0,
              "response body content matches");
    }

    // --- HTTPResponse create()/parse() round-trip ---
    std::print("== HTTPResponse create()/parse() round-trip ==\n");
    {
        string body = "Hello, World!";
        auto builtRes = std::make_unique<HTTPResponse>();
        builtRes->setStatus(Status(OK));
        builtRes->addHeader("Content-Type", "text/plain");
        builtRes->addHeader("Content-Length", (int32_t)body.size());
        builtRes->setData((uint8_t*)body.c_str(), body.size());

        auto raw  = builtRes->create();
        uint32_t sz = builtRes->size();
        check(raw != nullptr, "HTTPResponse create() returns non-null");
        check(sz > 0,         "HTTPResponse create() produces non-empty output");

        auto parsedRes = std::make_unique<HTTPResponse>(raw.get(), sz);
        check(parsedRes->parse(), std::format("round-trip parse() succeeded (error: {})", parsedRes->getParseError()));
        check(parsedRes->getVersion() == "HTTP/1.1", "round-trip response version");
        check(parsedRes->getReason()  == "OK",       "round-trip response reason");
        check(parsedRes->getHeaderValue("Content-Type")   == "text/plain",            "round-trip Content-Type");
        check(parsedRes->getHeaderValue("Content-Length") == std::format("{}", body.size()),
              "round-trip Content-Length");
        uint8_t* parsedData = parsedRes->getData();
        check(parsedData != nullptr,                                                    "round-trip body non-null");
        check(parsedRes->getDataLength() == (uint32_t)body.size(),                     "round-trip body length");
        check(std::strncmp((const char*)parsedData, body.c_str(), body.size()) == 0,   "round-trip body content");
    }

    if (failures == 0) {
        std::print("\nAll tests PASSED\n");
        return 0;
    }
    std::print("\n{} test(s) FAILED\n", failures);
    return 1;
}
