/**
 ByteBuffer
 HTTPRequest.cpp
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
#include "HTTPRequest.h"

HTTPRequest::HTTPRequest() : HTTPMessage() {
    this->init();
}

HTTPRequest::HTTPRequest(std::string sData) : HTTPMessage(sData) {
    this->init();
}

HTTPRequest::HTTPRequest(byte* pData, unsigned int len) : HTTPMessage(pData, len) {
    this->init();
}

HTTPRequest::~HTTPRequest() {
}

void HTTPRequest::init() {
    method = 0;
    requestUri = "";
}

/**
 * Takes the method name and converts it to the corresponding method
 * id detailed in the Method enum
 *
 * @param name String representation of the Method
 * @return Corresponding Method ID, -1 if unable to find the method
 */
int HTTPRequest::methodStrToInt(std::string name) {
    // Method name cannot must be between 1 and 10 characters. Anything outside those bounds shouldn't be compared at all
    if(name.empty() || (name.size() >= 10))
        return -1;
    
    // Loop through requestMethodStr array and attempt to match the 'name' with a known method in the array
	int ret = -1;
	for(unsigned int i = 0; i < NUM_METHODS; i++) {
		if(strcmp(requestMethodStr[i], name.c_str()) == 0) {
			ret = i;
			break;
		}
	}
	return ret;
}

/**
 * Takes the method ID in the Method enum and returns the corresponding std::string representation
 * @param mid Method ID to lookup
 * @return The method name in the from of a std::string. Blank if unable to find the method
 */
std::string HTTPRequest::methodIntToStr(unsigned int mid) {
    // ID is out of bounds of the possible requestMethodStr indexes
    if(mid >= NUM_METHODS)
        return "";
    
    // Return the std::string matching the id
    return requestMethodStr[mid];
}

/**
 * Create
 * Create and return a byte array of an HTTP request, built from the variables of this HTTPRequest
 *
 * @return Byte array of this HTTPRequest to be sent over the wire
 */
byte* HTTPRequest::create() {
    // Clear the bytebuffer in the event this isn't the first call of create()
	clear();
    
    // Insert the initial line: <method> <path> <version>\r\n
    std::string mstr = "";
    mstr = methodIntToStr(method);
    if(mstr.empty()) {
        printf("Could not create HTTPRequest, unknown method id: %i\n", method);
        return NULL;
    }
    putLine(mstr + " " + requestUri + " " + version);
    
    // Put all headers
    putHeaders();
    
    // If theres body data, add it now
    if((data != NULL) && dataLen > 0) {
        putBytes(data, dataLen);
    }
    
    // Allocate space for the returned byte array and return it
	byte* createRetData = new byte[size()];
	setReadPos(0);
	getBytes(createRetData, size());
    
    return createRetData;
}

/**
 * Parse
 * Populate internal HTTPRequest variables by parsing the HTTP data
 *
 * @param True if successful. If false, sets parseErrorStr for reason of failure
 */
bool HTTPRequest::parse() {
	std::string initial = "", methodName = "";

	// Get elements from the initial line: <method> <path> <version>\r\n
	methodName = getStrElement();
	requestUri = getStrElement();
	version = getLine(); // End of the line, pull till \r\n

	// Convert the name to the internal enumeration number
	method = methodStrToInt(methodName);
	if(method == -1) {
		parseErrorStr = "Invalid Method: " + methodName;
		return false;
	}

	// Validate the HTTP version. If there is a mismatch, discontinue parsing
	if(strcmp(version.c_str(), HTTP_VERSION) != 0) {
		parseErrorStr = "Supported HTTP version does not match";
		return false;
	}

	// Parse and populate the headers map using the parseHeaders helper
	parseHeaders();

	// Only POST and PUT can have Content (data after headers)
	if((method != POST) && (method != PUT))
		return true;
	
	// Parse the body of the message
	if(!parseBody())
		return false;
	
	return true;
}

