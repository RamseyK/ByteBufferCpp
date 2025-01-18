/**
    ByteBuffer
    HTTPMessage.cpp
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

#include <string>
#include <format>
#include <iostream>

HTTPMessage::HTTPMessage() : ByteBuffer(4096) {
}

HTTPMessage::HTTPMessage(std::string const& sData) : ByteBuffer(sData.size() + 1) {
    putBytes((uint8_t*)sData.c_str(), sData.size() + 1);
}

HTTPMessage::HTTPMessage(const uint8_t* pData, uint32_t len) : ByteBuffer(pData, len) {
}

/**
 * Put Line
 * Append a line (string) to the backing ByteBuffer at the current position
 *
 * @param str String to put into the ByteBuffer
 * @param crlf_end If true (default), end the line with a \r\n
 */
void HTTPMessage::putLine(std::string str, bool crlf_end) {
    // Terminate with crlf if flag set
    if (crlf_end)
        str += "\r\n";

    // Put the entire contents of str into the buffer
    putBytes((uint8_t*)str.c_str(), str.size());
}

/**
 * Put Headers
 * Write all headers currently in the 'headers' map to the ByteBuffer.
 * 'Header: value'
 */
void HTTPMessage::putHeaders() {
    for (auto const &[key, value] : headers) {
        putLine(std::format("{}: {}", key, value), true);
    }

    // End with a blank line
    putLine();
}

/**
 * Get Line
 * Retrive the entire contents of a line: string from current position until CR or LF, whichever comes first, then increment the read position
 * until it's past the last CR or LF in the line
 *
 * @return Contents of the line in a string (without CR or LF)
 */
std::string HTTPMessage::getLine() {
    std::string ret = "";
    int32_t startPos = getReadPos();
    bool newLineReached = false;
    char c = 0;

    // Append characters to the return std::string until we hit the end of the buffer, a CR (13) or LF (10)
    for (uint32_t i = startPos; i < size(); i++) {
        // If the next byte is a \r or \n, we've reached the end of the line and should break out of the loop
        c = peek();
        if ((c == 13) || (c == 10)) {
            newLineReached = true;
            break;
        }

        // Otherwise, append the next character to the std::string
        ret += getChar();
    }

    // If a line termination was never reached, discard the result and conclude there are no more lines to parse
    if (!newLineReached) {
        setReadPos(startPos); // Reset the position to before the last line read that we are now discarding
        ret = "";
        return ret;
    }

    // Increment the read position until the end of a CR or LF chain, so the read position will then point to the next line
    // Also, only read a maximum of 2 characters so as to not skip a blank line that is only \r\n
    uint32_t k = 0;
    for (uint32_t i = getReadPos(); i < size(); i++) {
        if (k++ >= 2)
            break;
        c = getChar();
        if ((c != 13) && (c != 10)) {
            // Set the Read position back one because the retrived character wasn't a LF or CR
            setReadPos(getReadPos() - 1);
            break;
        }
    }

    return ret;
}

/**
 * getStrElement
 * Get a token from the current buffer, stopping at the delimiter. Returns the token as a string
 *
 * @param delim The delimiter to stop at when retriving the element. By default, it's a space
 * @return Token found in the buffer. Empty if delimiter wasn't reached
 */
std::string HTTPMessage::getStrElement(char delim) {
    int32_t startPos = getReadPos();
    if (startPos < 0)
        return "";

    int32_t endPos = find(delim, startPos);
    if (endPos < 0)
        return "";

    if (startPos > endPos)
        return "";

    // Calculate the size based on the found ending position
    uint32_t size = (endPos + 1) - startPos;
    if (size <= 0)
        return "";

    // Grab the std::string from the ByteBuffer up to the delimiter
    auto str = new char[size];
    memset(str, 0x00, size);
    getBytes((uint8_t*)str, size);
    str[size - 1] = 0x00; // NULL termination
    std::string ret = str;

    // Increment the read position PAST the delimiter
    setReadPos(endPos + 1);

    return ret;
}

/**
 * Parse Headers
 * When an HTTP message (request & response) has reached the point where headers are present, this method
 * should be called to parse and populate the internal map of headers.
 * Parse headers will move the read position past the blank line that signals the end of the headers
 */
void HTTPMessage::parseHeaders() {
    std::string hline = "";
    std::string app = "";

    // Get the first header
    hline = getLine();

    // Keep pulling headers until a blank line has been reached (signaling the end of headers)
    while (hline.size() > 0) {
        // Case where values are on multiple lines ending with a comma
        app = hline;
        while (app[app.size() - 1] == ',') {
            app = getLine();
            hline += app;
        }

        addHeader(hline);
        hline = getLine();
    }
}

/**
 * Parse Body
 * Parses everything after the headers section of an HTTP message. Handles chuncked responses/requests
 *
 * @return True if successful. False on error, parseErrorStr is set with a reason
 */
bool HTTPMessage::parseBody() {
    // Content-Length should exist (size of the Body data) if there is body data
    std::string hlenstr = "";
    uint32_t contentLen = 0;
    hlenstr = getHeaderValue("Content-Length");

    // No body data to read:
    if (hlenstr.empty())
        return true;

    contentLen = atoi(hlenstr.c_str());

    // contentLen should NOT exceed the remaining number of bytes in the buffer
    // Add 1 to bytesRemaining so it includes the byte at the current read position
    if (contentLen > bytesRemaining() + 1) {
        // If it exceeds, read only up to the number of bytes remaining
        // dataLen = bytesRemaining();

        // If it exceeds, there's a potential security issue and we can't reliably parse
        parseErrorStr = std::format("Content-Length ({}) is greater than remaining bytes ({})", hlenstr, bytesRemaining());
        return false;
    } else {
        // Otherwise, we ca probably trust Content-Length is valid and read the specificed number of bytes
        dataLen = contentLen;
    }

    // Create a big enough buffer to store the data
    uint32_t dIdx = 0;
    uint32_t s = size();
    data = new uint8_t[dataLen];

    // Grab all the bytes from the current position to the end
    for (uint32_t i = getReadPos(); i < s; i++) {
        data[dIdx] = get(i);
        dIdx++;
    }

    // We could handle chunked Request/Response parsing (with footers) here, but, we won't.

    return true;
}

/**
 * Add Header to the Map from string
 * Takes a formatted header string "Header: value", parse it, and put it into the std::map as a key,value pair.
 *
 * @param string containing formatted header: value
 */
void HTTPMessage::addHeader(std::string const& line) {
    size_t kpos = line.find(':');
    if (kpos == std::string::npos) {
        std::cout << "Could not addHeader: " << line << std::endl;
        return;
    }
    // We're choosing to reject HTTP Header keys longer than 32 characters
    if (kpos > 32)
        return;

    std::string key = line.substr(0, kpos);
    if (key.empty())
        return;

    int32_t value_len = line.size() - kpos - 1;
    if (value_len <= 0)
        return;

    // We're choosing to reject HTTP header values longer than 4kb
    if (value_len > 4096)
        return;

    std::string value = line.substr(kpos + 1, value_len);

    // Skip all leading spaces in the value
    int32_t i = 0;
    while (i < value.size() && value.at(i) == 0x20) {
        i++;
    }
    value = value.substr(i, value.size());
    if (value.empty())
        return;

    // Add header to the map
    addHeader(key, value);
}

/**
 * Add header key-value std::pair to the map
 *
 * @param key String representation of the Header Key
 * @param value String representation of the Header value
 */
void HTTPMessage::addHeader(std::string const& key, std::string const& value) {
    headers.try_emplace(key, value);
}

/**
 * Add header key-value std::pair to the map (Integer value)
 * Integer value is converted to a string
 *
 * @param key String representation of the Header Key
 * @param value Integer representation of the Header value
 */
void HTTPMessage::addHeader(std::string const& key, int32_t value) {
    headers.try_emplace(key, std::format("{}", value));
}

/**
 * Get Header Value
 * Given a header name (key), return the value associated with it in the headers map
 *
 * @param key Key to identify the header
 */
std::string HTTPMessage::getHeaderValue(std::string const& key) const {

    char c = 0;
    std::string key_lower = "";

    // Lookup in map
    auto it = headers.find(key);

    // Key wasn't found, try an all lowercase variant as some clients won't always use proper capitalization
    if (it == headers.end()) {

        for (uint32_t i = 0; i < key.length(); i++) {
            c = key.at(i);
            key_lower += tolower(c);
        }

        // Still not found, return empty string to indicate the Header value doesnt exist
        it = headers.find(key_lower);
        if (it == headers.end())
            return "";
    }

    // Otherwise, return the value
    return it->second;
}

/**
 * Get Header String
 * Get the full formatted header string "Header: value" from the headers map at position index
 *
 * @param index Position in the headers map to retrieve a formatted header string
 * @ret Formatted string with header name and value
 */
std::string HTTPMessage::getHeaderStr(int32_t index) const {
    int32_t i = 0;
    std::string ret = "";
    for (auto const &[key, value] : headers) {
        if (i == index) {
            ret = std::format("{}: {}", key, value);
            break;
        }

        i++;
    }
    return ret;
}

/**
 * Get Number of Headers
 * Return the number of headers in the headers map
 *
 * @return size of the map
 */
uint32_t HTTPMessage::getNumHeaders() const {
    return headers.size();
}

/**
 * Clear Headers
 * Removes all headers from the internal map
 */
void HTTPMessage::clearHeaders() {
    headers.clear();
}

