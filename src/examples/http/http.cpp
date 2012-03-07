/**
 ByteBuffer
 http.cpp
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

#include <iostream>
#include <stdlib.h>
#include "../../ByteBuffer.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"

using namespace std;

int main() {
	HTTPMessage *msg = new HTTPMessage("line1\r\nline2\nline3");
    HTTPRequest *req = new HTTPRequest("POST /sample/path.html HTTP/1.1\r\nHeader1: value1\r\nHeader2: value2\r\nHeader3: value3\r\nContent-Length: 5\r\n\r\ndata");
	HTTPRequest *req2 = new HTTPRequest();
	HTTPRequest *req3 = NULL;
    //HTTPResponse *resp = new HTTPResponse();

	// Test getLine() in HTTPMessage

	string l1 = "", l2 = "", l3 = "", l4 = "";
	l1 = msg->getLine(); // Expected: test1
	l2 = msg->getLine(); // Expected: test2
	l3 = msg->getLine(); // Expected: 
	l4 = msg->getLine(); // Expected: 

    printf("%s (%u)\n%s (%u)\n%s (%u)\n%s (%u)\n\n", l1.c_str(), (unsigned int)l1.size(), l2.c_str(), (unsigned int)l2.size(), l3.c_str(), (unsigned int)l3.size(), l4.c_str(), (unsigned int)l4.size());

	// Test HTTPRequest parse()
	req->parse();
	if(req->hasParseError()) {
		printf("HTTPRequest had a parse error: %s\n", req->getParseError().c_str());
	}
	printf("HTTPRequest: %i %s\n", req->getMethod(), req->getVersion().c_str());
	byte *data = req->getData();
	printf("Data (%i):\n", req->getDataLength());
	for(unsigned int i = 0; i < req->getDataLength(); i++) {
		printf("0x%02x ", data[i]);
	}
	printf("\n");

	// Populate vars in an HTTPRequest to test create()
	string req2Content = "var=2";
	char req2ContentLen[8];
	sprintf(req2ContentLen, "%u", (unsigned int)req2Content.size());
	byte* req2Ret = NULL;
	unsigned int req2Size = 0;
	req2->setMethod(Method(POST));
	req2->setRequestUri("/dir/test.php");
	req2->addHeader("From", "user@example.com");;
	req2->addHeader("User-Agent", "ByteBuffer/1.0");
	req2->addHeader("Content-Type", "text/html");
	req2->addHeader("Content-Length", req2ContentLen);
	req2->addHeader("Multi-Test", "line1,\r\nline2,\r\nline3");
	req2->setData((byte*)req2Content.c_str(), req2Content.size());
	req2Ret = req2->create();
	req2Size = req2->size();

	printf("\n\n");
	
	// Have req3 take the entire data from req2 and parse it
	printf("Parsing req2 with req3:\n");
	req3 = new HTTPRequest(req2Ret, req2Size);
	req3->parse();
	if(req3->hasParseError()) {
		printf("req3 parse error: %s\n", req3->getParseError().c_str());
	} else {
		string req3Header = req3->methodIntToStr(req3->getMethod()) + " " + req3->getRequestUri() + " " + req3->getVersion();
		printf("%s\n", req3Header.c_str());
		printf("req3 headers (%i):\n", req3->getNumHeaders());
		for(int i = 0; i < req3->getNumHeaders(); i++) {
			printf("%s\n", req3->getHeaderStr(i).c_str());
		}
		printf("req3 data(%i):\n", req3->getDataLength());
		byte* req3Data = req3->getData();
		for(unsigned int i = 0; i < req3->getDataLength(); i++) {
			printf("0x%02x ", req3Data[i]);
		}
		printf("\n\n");
	}
    
	delete msg;
    delete req;
	delete req2;
	delete req3;
    //delete resp;
    return 0;
}
