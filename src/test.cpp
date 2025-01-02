/**
 ByteBuffer
 test.cpp
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

#include <cstdio>

#include <string>
#include <memory>

#include "ByteBuffer.hpp"

#ifdef BB_USE_NS
using namespace bb;
#endif

int32_t main() {
    auto bb1 = std::make_unique<ByteBuffer>(30);
    auto bb2 = std::make_unique<ByteBuffer>();

    bb1->printPosition();
    printf("bb1 bytes Remaining: %i\n", bb1->bytesRemaining());
    bb1->put(0xDE);
    bb1->put(0xAD);
    bb1->put(0xBA);
    bb1->put(0xBE);
    bb1->printPosition();
    printf("bb1 bytes Remaining: %i\n", bb1->bytesRemaining());

    bb1->printHex();
    bb1->printPosition();

    int32_t f0 = -1, f1 = -1, f2 = -1;
    f0 = bb1->find((uint16_t) 0xBEBA);
    f1 = bb1->find((uint8_t) 0xBA);

    uint8_t t1 = 0, t2 = 0, t3 = 0, t4 = 0, t5 = 1;
    t1 = bb1->get();
    t2 = bb1->get();
    t3 = bb1->get();
    t4 = bb1->get();
    t5 = bb1->get();

    f2 = bb1->find((uint8_t) 0xDE, bb1->getReadPos());

    std::printf("t1 = %x, t2 = %x, t3 = %x, t4 = %x, t5 = %x\n", t1, t2, t3, t4, t5);
    std::printf("f0 = %i, f1 = %i, f2 = %i\n", f0, f1, f2);
    bb1->printPosition();

    std::string test = "TesTtest2";
    bb2->putBytes((uint8_t*) test.c_str(), test.size());
    bb2->printHex();
    bb2->printAscii();

    auto bb3 = std::make_unique<ByteBuffer>(12);
    printf("bb3 size=%i\n", bb3->size());
    bb3->put('a', 16);
    bb3->printPosition();
    bb3->printHex();
    bb3->printAscii();
    printf("bb3 size=%i\n", bb3->size());

    return 0;
}
