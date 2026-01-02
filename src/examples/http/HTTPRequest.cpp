/**
 ByteBuffer
 HTTPRequest.cpp
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

#include "HTTPMessage.h"
#include "HTTPRequest.h"

#include <format>
#include <memory>
#include <print>


HTTPRequest::HTTPRequest() : HTTPMessage() {
}

HTTPRequest::HTTPRequest(std::string const& sData) : HTTPMessage(sData) {
}

HTTPRequest::HTTPRequest(const uint8_t* pData, uint32_t len) : HTTPMessage(pData, len) {
}

/**
 * Takes the method name and converts it to the corresponding method
 * id detailed in the Method enum
 *
 * @param name String representation of the Method
 * @return Corresponding Method ID, INVALID_METHOD if unable to find the method
 */
uint32_t HTTPRequest::methodStrToInt(std::string_view name) const {
    // Method name cannot must be between 1 and 10 characters. Anything outside those bounds shouldn't be compared at all
    if (name.empty() || (name.size() >= 10))
        return INVALID_METHOD;

    // Loop through requestMethodStr array and attempt to match the 'name' with a known method in the array
    uint32_t ret = INVALID_METHOD;
    for (uint32_t i = 0; i < NUM_METHODS; i++) {
        if (name.compare(requestMethodStr[i]) == 0) {
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
std::string HTTPRequest::methodIntToStr(uint32_t mid) const {
    // ID is out of bounds of the possible requestMethodStr indexes
    if (mid >= NUM_METHODS)
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
std::unique_ptr<uint8_t[]> HTTPRequest::create() {
    // Clear the bytebuffer in the event this isn't the first call of create()
    clear();

    // Insert the initial line: <method> <path> <version>\r\n
    std::string mstr = "";
    mstr = methodIntToStr(method);
    if (mstr.empty()) {
        std::print("Could not create HTTPRequest, unknown method id: {}\n", method);
        return nullptr;
    }
    putLine(std::format("{} {} {}", mstr, requestUri, version));

    // Put all headers
    putHeaders();

    // If theres body data, add it now
    if ((this->data != nullptr) && this->dataLen > 0) {
        putBytes(this->data, this->dataLen);
    }

    // Allocate space for the returned byte array and return it
    auto sz = size();
    auto createRetData = std::make_unique<uint8_t[]>(sz);
    setReadPos(0);
    getBytes(createRetData.get(), sz);

    return createRetData;
}

/**
 * Parse
 * Populate internal HTTPRequest variables by parsing the HTTP data
 *
 * @param True if successful. If false, sets parseErrorStr for reason of failure
 */
bool HTTPRequest::parse() {
    // Get elements from the initial line: <method> <path> <version>\r\n
    std::string methodName = getStrElement();
    if (methodName.empty()) {
        parseErrorStr = "Empty method";
        return false;
    }

    // Convert the name to the internal enumeration number
    method = methodStrToInt(methodName);
    if (method == INVALID_METHOD) {
        parseErrorStr = "Invalid Method";
        return false;
    }

    requestUri = getStrElement();
    if (requestUri.empty()) {
        parseErrorStr = "No request URI";
        return false;
    }

    version = getLine(); // End of the line, pull till \r\n
    if (version.empty()) {
        parseErrorStr = "HTTP version string was empty";
        return false;
    }

    if (!version.starts_with("HTTP/1")) {
        parseErrorStr = "HTTP version was invalid";
        return false;
    }

    // Optional - Validate the HTTP version. If there is a mismatch, discontinue parsing
    // if (strcmp(version.c_str(), HTTP_VERSION) != 0) {
    //     parseErrorStr = "Supported HTTP version does not match";
    //     return false;
    // }

    // Parse and populate the headers map using the parseHeaders helper
    parseHeaders();

    // Only POST and PUT can have Content (data after headers)
    if ((method != POST) && (method != PUT))
        return true;

    // Parse the body of the message
    if (!parseBody())
        return false;

    return true;
}

