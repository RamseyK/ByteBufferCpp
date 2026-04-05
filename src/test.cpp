/**
 ByteBuffer
 test.cpp
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

#include <cmath>
#include <cstring>
#include <memory>
#include <print>
#include <string>

#include "ByteBuffer.hpp"

#ifdef BB_USE_NS
using namespace bb;
#endif

static int failures = 0;

static void check(bool cond, std::string_view msg) {
    if (!cond) {
        std::print("  FAIL: {}\n", msg);
        ++failures;
    }
}

int32_t main() {

    // --- Primitive round-trips ---
    std::print("== Primitive round-trips ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->put(0xABu);
        bb->putChar('Z');
        bb->putShort(0x1234u);
        bb->putInt(0xDEADBEEFu);
        bb->putLong(0xCAFEBABEDEAD1234ULL);
        bb->putFloat(3.14f);
        bb->putDouble(2.718281828);

        check(bb->get()   == 0xABu,                       "get uint8_t 0xAB");
        check(bb->getChar()   == 'Z',                     "getChar 'Z'");
        check(bb->getShort()  == 0x1234u,                 "getShort 0x1234");
        check(bb->getInt()    == 0xDEADBEEFu,             "getInt 0xDEADBEEF");
        check(bb->getLong()   == 0xCAFEBABEDEAD1234ULL,   "getLong 0xCAFEBABEDEAD1234");
        check(std::abs(bb->getFloat()  - 3.14f)        < 1e-5f, "getFloat 3.14");
        check(std::abs(bb->getDouble() - 2.718281828)  < 1e-9,  "getDouble 2.718281828");
        check(bb->bytesRemaining() == 0, "all bytes consumed after round-trips");
    }

    // --- bytesRemaining ---
    std::print("== bytesRemaining ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        check(bb->bytesRemaining() == 0, "empty buffer: 0 remaining");
        bb->putInt(0x12345678u);
        check(bb->bytesRemaining() == 4, "4 remaining after putInt");
        bb->getShort();
        check(bb->bytesRemaining() == 2, "2 remaining after getShort");
        bb->getShort();
        check(bb->bytesRemaining() == 0, "0 remaining after reading all");
        bb->get(); // read past end — should not crash
        check(bb->bytesRemaining() == 0, "0 remaining when reading past end");
    }

    // --- peek ---
    std::print("== peek ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->put(0x42u);
        bb->put(0x99u);
        check(bb->peek() == 0x42u, "peek returns first byte");
        uint32_t posBefore = bb->getReadPos();
        bb->peek();
        check(bb->getReadPos() == posBefore, "peek does not advance read position");
        bb->get();
        check(bb->peek() == 0x99u, "peek returns correct byte after advance");
    }

    // --- Absolute indexed put/get ---
    std::print("== Absolute indexed put/get ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->put(0x11u);
        bb->put(0x22u);
        bb->put(0x33u);
        check(bb->get(0) == 0x11u, "get(0) == 0x11");
        check(bb->get(1) == 0x22u, "get(1) == 0x22");
        check(bb->get(2) == 0x33u, "get(2) == 0x33");
        bb->put(0xFFu, 1); // overwrite index 1 in place
        check(bb->get(1) == 0xFFu, "absolute put at index 1, get(1) == 0xFF");
        check(bb->get(0) == 0x11u, "get(0) unchanged after indexed write");
        check(bb->get(2) == 0x33u, "get(2) unchanged after indexed write");
    }

    // --- putBytes / getBytes ---
    std::print("== putBytes / getBytes ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        const uint8_t src[] = {0x11, 0x22, 0x33, 0x44, 0x55};
        bb->putBytes(src, 5);
        check(bb->size() == 5, "size 5 after putBytes");
        uint8_t dst[5] = {};
        bb->getBytes(dst, 5);
        check(std::memcmp(dst, src, 5) == 0, "getBytes matches putBytes data");
        check(bb->bytesRemaining() == 0, "0 remaining after getBytes");

        // Absolute putBytes at index
        bb->setReadPos(0);
        const uint8_t patch[] = {0xAA, 0xBB};
        bb->putBytes(patch, 2, 1);
        check(bb->get(0) == 0x11u, "absolute putBytes: index 0 unchanged");
        check(bb->get(1) == 0xAAu, "absolute putBytes: index 1 == 0xAA");
        check(bb->get(2) == 0xBBu, "absolute putBytes: index 2 == 0xBB");
        check(bb->get(3) == 0x44u, "absolute putBytes: index 3 unchanged");
    }

    // --- find ---
    std::print("== find ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->put(0xDEu);
        bb->put(0xADu);
        bb->put(0xBAu);
        bb->put(0xBEu);
        // Buffer: [0xDE, 0xAD, 0xBA, 0xBE]
        check(bb->find<uint8_t>(0xDEu)    == 0,  "find 0xDE at index 0");
        check(bb->find<uint8_t>(0xBAu)    == 2,  "find 0xBA at index 2");
        check(bb->find<uint8_t>(0xFFu)    == -1, "find 0xFF: not found");
        check(bb->find<uint8_t>(0xDEu, 1) == -1, "find 0xDE from pos 1: not found");
        // On little-endian: bytes [0xBA, 0xBE] at index 2 = uint16_t 0xBEBA
        check(bb->find<uint16_t>(0xBEBAu) == 2,  "find uint16_t 0xBEBA at index 2");
    }

    // --- replace ---
    std::print("== replace ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->put(0xAAu);
        bb->put(0xBBu);
        bb->put(0xAAu);
        bb->put(0xCCu);
        bb->replace(0xAAu, 0xFFu); // replace all occurrences
        check(bb->get(0) == 0xFFu, "replace all: index 0 replaced");
        check(bb->get(2) == 0xFFu, "replace all: index 2 replaced");
        check(bb->get(1) == 0xBBu, "replace all: index 1 unchanged");
        check(bb->get(3) == 0xCCu, "replace all: index 3 unchanged");

        bb->replace(0xFFu, 0x11u, 0, true); // first occurrence only
        check(bb->get(0) == 0x11u, "firstOccurrenceOnly: index 0 replaced");
        check(bb->get(2) == 0xFFu, "firstOccurrenceOnly: index 2 untouched");
    }

    // --- clone and equals ---
    std::print("== clone and equals ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->put(0x01u);
        bb->putShort(0xBEEFu);
        bb->putInt(0xCAFEBABEu);
        auto cloned = bb->clone();
        check(bb->equals(cloned.get()), "clone equals original");
        cloned->put(0x00u, 0); // modify clone's first byte
        check(!bb->equals(cloned.get()), "modified clone not equal to original");

        // Empty buffer clone
        auto empty = std::make_unique<ByteBuffer>();
        auto emptyClone = empty->clone();
        check(empty->equals(emptyClone.get()), "clone of empty buffer equals original");
    }

    // --- resize ---
    std::print("== resize ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->putInt(0x12345678u);
        check(bb->size() == 4, "size 4 after putInt");
        bb->resize(8);
        check(bb->size() == 8, "size 8 after resize up");
        check(bb->getReadPos()  == 0, "rpos reset to 0 after resize");
        check(bb->getWritePos() == 0, "wpos reset to 0 after resize");
        bb->resize(2);
        check(bb->size() == 2, "size 2 after resize down");
    }

    // --- clear ---
    std::print("== clear ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->putInt(0xDEADBEEFu);
        bb->getShort();
        bb->clear();
        check(bb->size()           == 0, "size 0 after clear");
        check(bb->bytesRemaining() == 0, "bytesRemaining 0 after clear");
        check(bb->getReadPos()     == 0, "rpos 0 after clear");
        check(bb->getWritePos()    == 0, "wpos 0 after clear");
        bb->put(0xAAu); // can still write after clear
        check(bb->size() == 1 && bb->get() == 0xAAu, "writable and readable after clear");
    }

    // --- put(ByteBuffer*) ---
    std::print("== put(ByteBuffer*) ==\n");
    {
        auto src = std::make_unique<ByteBuffer>();
        src->put(0x01u);
        src->put(0x02u);
        src->put(0x03u);
        auto dst = std::make_unique<ByteBuffer>();
        dst->put(src.get());
        check(dst->size()  == 3,    "dst size 3 after put(src)");
        check(dst->get()   == 0x01u, "dst byte 0 == 0x01");
        check(dst->get()   == 0x02u, "dst byte 1 == 0x02");
        check(dst->get()   == 0x03u, "dst byte 2 == 0x03");
    }

    // --- Read/write position management ---
    std::print("== position management ==\n");
    {
        auto bb = std::make_unique<ByteBuffer>();
        bb->put(0xAAu);
        bb->put(0xBBu);
        bb->put(0xCCu);
        check(bb->getWritePos() == 3, "wpos 3 after writing 3 bytes");
        bb->get();
        bb->get();
        check(bb->getReadPos() == 2, "rpos 2 after two get() calls");
        bb->setReadPos(0);
        check(bb->get() == 0xAAu, "get() after setReadPos(0) returns first byte");
        bb->setWritePos(1);
        bb->put(0xFFu);
        check(bb->get(1) == 0xFFu, "put() after setWritePos(1) writes at index 1");
        check(bb->get(0) == 0xAAu, "byte at index 0 unaffected by setWritePos write");
    }

    // --- Dynamic buffer growth ---
    std::print("== dynamic buffer growth ==\n");
    {
        // ByteBuffer(n) reserves n bytes but size() starts at 0
        auto bb = std::make_unique<ByteBuffer>(4);
        check(bb->size() == 0, "initial size 0 (reserve != size)");
        // Absolute write well beyond reservation triggers internal resize
        bb->put(0xFFu, 16);
        check(bb->size()    == 17,   "size 17 after absolute write at index 16");
        check(bb->get(16)   == 0xFFu, "byte at index 16 == 0xFF");
    }

    if (failures == 0) {
        std::print("\nAll tests PASSED\n");
        return 0;
    }
    std::print("\n{} test(s) FAILED\n", failures);
    return 1;
}
