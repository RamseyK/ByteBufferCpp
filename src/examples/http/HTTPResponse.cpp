/**
 ByteBuffer
 HTTPResponse.cpp
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

#include "HTTPMessage.h"
#include "HTTPResponse.h"

HTTPResponse::HTTPResponse() : HTTPMessage() {
    this->init();
}

HTTPResponse::HTTPResponse(string sData) : HTTPMessage(sData) {
    this->init();
}

HTTPResponse::HTTPResponse(byte* pData, unsigned int len) : HTTPMessage(pData, len) {
    this->init();
}

HTTPResponse::~HTTPResponse() {
}

void HTTPResponse::init() {
    status = 0;
}

/**
 * Get the status string representation of the Response's set status code.
 * To be read by humans, non standard representation
 *
*/
string HTTPResponse::getStatusStr() {
    string ret = "";
    
    // Place the status code as part of the human representation.
    char strcode[8];
    sprintf(strcode, "%d - ", status);
    ret += strcode;
    
    switch(status) {
        case Status(CONTINUE):
            ret = "Continue";
            break;
        case Status(OK):
            ret = "OK";
            break;
        case Status(NOT_FOUND):
            ret = "Not Found";
            break;
        case Status(SERVER_ERROR):
            ret = "Internal Server Error";
            break;
        case Status(NOT_IMPLEMENTED):
            ret = "Method not implemented";
            break;
        default:
            ret = "";
            break;
    }
    
    return ret;
}

byte* HTTPResponse::create(bool freshCreate) {
	return NULL;
}

void HTTPResponse::parse() {
}

/*
string HTTPResponse::generateResponse() {
    stringstream resp;
    
    // Add common headers
    addHeader("Server", SERVER_HEAD);
    
    // Date Header.  Ex: Date: Fri, 31 Dec 1999 23:59:59 GMT
    char date[80];
    time_t tm;
    struct tm *gmt;
    time(&tm);
    gmt = gmtime(&tm);
    strftime(date, 80, "%a, %d %b %Y %H:%M:%S GMT", gmt);
    addHeader("Date", date);
    
    // Status Line: HTTP-Version SP Status-Code SP Reason-Phrase CRLF
    resp << HTTP_VERSION << " " << status << " " << getStatusStr() << "\r\n";
    
    // Loop through headers and dump each line by line
    
    // Message Body
    
    // End with a new line
    resp << "\n";
    
    return resp.str();
}*/

