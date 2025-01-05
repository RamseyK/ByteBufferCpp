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

#include <iostream>
#include <stdlib.h>
#include "../../ByteBuffer.hpp"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

using namespace std;

int32_t main() {
    bool testFailed = false;
    HTTPMessage *msg = new HTTPRequest("line1\r\nline2\nline3");
    HTTPRequest *req = new HTTPRequest("POST /sample/path.html HTTP/1.1\r\nHeader1: value1\r\nHeader2: value2\r\nHeader3: value3\r\nContent-Length: 5\r\n\r\ndata");
    HTTPRequest *req2 = new HTTPRequest();
    HTTPRequest *req3 = NULL;
    HTTPResponse *res = new HTTPResponse("HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\nContent-Length: 111\r\n\r\n<html><body>\n<h2>No Host: header received</h2>\nHTTP 1.1 requests must include the Host: header.\n</body></html>");
    
    printf("HTTP Test Cases:\n");

    // Test getLine() in HTTPMessage

    string l1 = "", l2 = "", l3 = "", l4 = "";
    l1 = msg->getLine(); // Expected: line1
    if(strcmp(l1.c_str(), "line1") != 0) {
        printf("l1 mismatch. Got: %s. Expected: line1\n", l1.c_str());
        testFailed = true;
    }
    l2 = msg->getLine(); // Expected: line2
    if(strcmp(l2.c_str(), "line2") != 0) {
        printf("l2 mismatch. Got: %s. Expected: line2\n", l2.c_str());
        testFailed = true;
    }
    l3 = msg->getLine(); // Expected: 
    if(!l3.empty()) {
        printf("l3 mismatch. Got: %s. Expected to be blank\n", l3.c_str());
        testFailed = true;
    }
    l4 = msg->getLine(); // Expected: 
    if(!l4.empty()) {
        printf("l4 mismatch. Got: %s. Expected to be blank\n", l4.c_str());
        testFailed = true;
    }

    printf("%s (%u)\n%s (%u)\n%s (%u)\n%s (%u)\n\n", l1.c_str(), (unsigned int)l1.size(), l2.c_str(), (unsigned int)l2.size(), l3.c_str(), (unsigned int)l3.size(), l4.c_str(), (unsigned int)l4.size());

    // Test HTTPRequest parse()
    if(!req->parse()) {
        printf("HTTPRequest (req) had a parse error: %s\n", req->getParseError().c_str());
        testFailed = true;
    } else {
        printf("HTTPRequest(req): %i %s\n", req->getMethod(), req->getVersion().c_str());
        uint8_t *data = req->getData();
        printf("Data (%i):\n", req->getDataLength());
        for(uint32_t i = 0; i < req->getDataLength(); i++) {
            printf("0x%02x ", data[i]);
        }
        printf("\n");
        for(uint32_t i = 0;i < req->getDataLength(); i++) {
            printf("%c", data[i]);
        }
        printf("\n");
    }

    // Populate vars in an HTTPRequest to test create()
    string req2Content = "var=2";
    char req2ContentLen[8];
    snprintf(req2ContentLen, sizeof(req2ContentLen), "%zu", req2Content.size());
    uint8_t *req2Ret = NULL;
    uint32_t req2Size = 0;
    req2->setMethod(Method(POST));
    req2->setRequestUri("/dir/test.php");
    req2->addHeader("From", "user@example.com");;
    req2->addHeader("User-Agent", "ByteBuffer/1.0");
    req2->addHeader("Content-Type", "text/html");
    req2->addHeader("Content-Length", req2ContentLen);
    req2->addHeader("Multi-Test", "line1,\r\nline2,\r\nline3");
    req2->setData((uint8_t*)req2Content.c_str(), req2Content.size());
    req2Ret = req2->create();
    req2Size = req2->size();
    printf("\n\n");
    
    // Have req3 take the entire data from req2 and parse it
    printf("Parsing req2 with req3:\n");
    req3 = new HTTPRequest(req2Ret, req2Size);
    if(!req3->parse()) {
        printf("req3 parse error: %s\n", req3->getParseError().c_str());
        testFailed = true;
    } else {
        string req3Header = req3->methodIntToStr(req3->getMethod()) + " " + req3->getRequestUri() + " " + req3->getVersion();
        printf("HTTPResponse(res3): %s\n", req3Header.c_str());
        printf("req3 headers (%i):\n", req3->getNumHeaders());
        for(int32_t i = 0; i < req3->getNumHeaders(); i++) {
            printf("%s\n", req3->getHeaderStr(i).c_str());
        }
        printf("req3 data(%i):\n", req3->getDataLength());
        uint8_t *req3Data = req3->getData();
        for(uint32_t i = 0; i < req3->getDataLength(); i++) {
            printf("0x%02x ", req3Data[i]);
        }
        printf("\n");
        for(uint32_t i = 0;i < req3->getDataLength(); i++) {
            printf("%c", req3Data[i]);
        }
        printf("\n\n");
    }
    
    // Test HTTPResponse(res) parse()
    
    if(!res->parse()) {
        printf("res parse error: %s\n", res->getParseError().c_str());
        testFailed = true;
    } else {
        printf("HTTPResponse(res): %s %s\n", res->getVersion().c_str(), res->getReason().c_str());
        printf("res headers (%i):\n", res->getNumHeaders());
        for(int32_t i = 0; i < res->getNumHeaders(); i++) {
            printf("%s\n", res->getHeaderStr(i).c_str());
        }
        printf("res data(%i):\n", res->getDataLength());
        uint8_t *resData = res->getData();
        for(uint32_t i = 0; i < res->getDataLength(); i++) {
            printf("0x%02x ", resData[i]);
        }
        printf("\n");
        for(uint32_t i = 0;i < res->getDataLength(); i++) {
            printf("%c", resData[i]);
        }
        printf("\n\n");
    }
    
    delete msg;
    delete req;
    delete req2;
    delete req3;
    delete res;
    
    if(testFailed) {
        printf("TEST PROGRAM FAILED: Read through output carefully to find point32_t of failure\n");
    } else {
        printf("TEST PROGRAM PASSED\n");
    }
    
    return 0;
}
