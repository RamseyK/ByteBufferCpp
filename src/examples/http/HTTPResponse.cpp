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
    reason = "";
}

/**
 * Get the status string representation of the Response's set status code.
 * To be read by humans, non standard representation
 *
*/
string HTTPResponse::getStatusStr() {
    // Place the status code as part of the human representation.
    stringstream ret;
    ret << status << " - ";
    
    switch(status) {
        case Status(CONTINUE):
            ret << "Continue";
            break;
        case Status(OK):
            ret << "OK";
            break;
        case Status(BAD_REQUEST):
        	ret << "Bad Request";
        	break;
        case Status(NOT_FOUND):
            ret << "Not Found";
            break;
        case Status(SERVER_ERROR):
            ret << "Internal Server Error";
            break;
        case Status(NOT_IMPLEMENTED):
            ret << "Method not implemented";
            break;
        default:
            break;
    }
    
    return ret.str();
}

/**
 * Create
 * Create and return a byte array of an HTTP response, built from the variables of this HTTPResponse
 *
 * @param freshCreate If true, force a rebuild of the packet even if there's a previously cached version
 * @return Byte array of this HTTPResponse to be sent over the wire
 */
byte* HTTPResponse::create(bool freshCreate) {
    // Clear the bytebuffer in the event this isn't the first call of create(), or if a fresh create is desired
	if(!createCached || freshCreate) {
		clear();
	} else { // Otherwise, return the already created data
		return createRetData;
	}
	
	if(createRetData != NULL) {
		delete createRetData;
		createRetData = NULL;
	}
	
    // Insert the status line: <version> <status code> <reason>\r\n
    stringstream sline;
    sline << version << " " << status << " " << getStatusStr();
    putLine(sline.str());
    
    // Put all headers
    putHeaders();
    
    // If theres body data, add it now
    if((data != NULL) && dataLen > 0) {
        putBytes(data, dataLen);
    }
    
    // Allocate space for the returned byte array and return it
	createRetData = new byte[size()];
	setReadPos(0);
	getBytes(createRetData, size());

	// createCached should now be true, because a create was successfully performed.
	createCached = true;
    
    return createRetData;
}

/**
 * Parse
 * Populate internal HTTPResponse variables by parsing the HTTP data
 *
 * @param True if successful. If false, sets parseErrorStr for reason of failure
 */
bool HTTPResponse::parse() {
	string statusstr;
	
	// Get elements from the status line: <version> <status code> <reason>\r\n
	version = getStrElement();
	statusstr = getStrElement();
	status = atoi(statusstr.c_str());
	reason = getLine(); // Pull till \r\n termination
	
	// Validate the HTTP version. If there is a mismatch, discontinue parsing
	if(strcmp(version.c_str(), HTTP_VERSION) != 0) {
		parseErrorStr = "Supported HTTP version does not match";
		return false;
	}
	
	// Parse and populate the headers map using the parseHeaders helper
	parseHeaders();
	
	// If the body of the message
	if(!parseBody())
		return false;
	
	return true;
}


