/**
   ByteBuffer
   packets.cpp
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

 * The purpose of this example is to show that ByteBuffer can be used to easily create and parse custom packets
 * for a networking application. In this example, two packets (Login and Chat message) from a Chat application
 * are created and parsed by a server's packet parsing function. For simplicity, actual socket code has been omitted
 */

#include <memory>
#include <print>
#include <string>
#include "../../ByteBuffer.hpp"

using namespace std;

ByteBuffer* createLoginPacket(int32_t version, string username, string password);
ByteBuffer* createChatMsgPacket(string name, string msg);
void serverParser(ByteBuffer* pkt);
bool verifyLoginPacket(ByteBuffer* pkt, int32_t expVersion, const string& expUsername, const string& expPassword);
bool verifyChatMsgPacket(ByteBuffer* pkt, const string& expName, const string& expMsg);

/**
 * Opcodes for the fictional network protocol
 */
enum Opcode {
   LOGIN = 0x0001,
   DISCONNECT = 0x0002,
   MESSAGE = 0x0003,
   UNKNOWN = 0x0004
};

/**
 * Login packet
 * Create a login packet with the client's version, username, and password in the correct format
 *
 * @param version Client's version number to send to the server
 * @param username Username of client logging in
 * @param password Password of client logging in
 * @return A pointer to a byte array ready to be sent over the wire
 */
ByteBuffer* createLoginPacket(int32_t version, string username, string password) {
   ByteBuffer* pkt = new ByteBuffer(100);

   // Write the opcode as the first bytes of the packet (login)
   pkt->putShort(Opcode(LOGIN));

   // Version #
   pkt->putInt(version);

   // Size & Contents of null terminated username string
   pkt->putInt(username.size()+1);
   pkt->putBytes((uint8_t*)username.c_str(), username.size()+1);

   // Size & Contents of null terminated password string
   pkt->putInt(password.size()+1);
   pkt->putBytes((uint8_t*)password.c_str(), password.size()+1);

   return pkt;
}

/**
 * Chat Message packet
 * Create a chat message packet to send to the server
 *
 * @param name Name of user sending the chat message
 * @param msg String containing the content of the chat message
 * @return A pointer to a ByteBuffer ready to be sent over the wire
 */
ByteBuffer* createChatMsgPacket(string name, string msg) {
   ByteBuffer* pkt = new ByteBuffer();

   // Write the opcode as the first bytes of the packet (message)
   pkt->putShort(Opcode(MESSAGE));

   // Size & Contents of null terminated name string
   pkt->putInt(name.size()+1);
   pkt->putBytes((uint8_t*)name.c_str(), name.size()+1);

   // Size & Contents of null terminated message string
   pkt->putInt(msg.size()+1);
   pkt->putBytes((uint8_t*)msg.c_str(), msg.size()+1);

   return pkt;
}

/**
 * Packet Parser
 * This fictitious packet parser on the "server" reads the ByteBuffer'd packets and prints out
 * information about each packet it understands according to the networking protocol.
 *
 * @param pkt A pointer to a ByteBuffer containing the packet data
 */
void serverParser(ByteBuffer* pkt) {
   std::print("Parsing ByteBuffer'd packet of size: {}\n", pkt->size());

   // Read the first 2 bytes (short) of the packet to determine the opcode
   short opcode = 0;
   opcode = pkt->getShort();

   // Switch based off the opcode to handle the specific packet
   switch(opcode) {
      case Opcode(LOGIN): {
         std::print("Received a Login packet. Information: \n");

         int32_t version = 0;
         int32_t usize = 0, psize = 0;
         uint8_t *username;
         uint8_t *password;

         version = pkt->getInt();

         usize = pkt->getInt();
         username = new uint8_t[usize];
         pkt->getBytes(username, usize);

         psize = pkt->getInt();
         password = new uint8_t[psize];
         pkt->getBytes(password, psize);

         std::print("Client Version: {}, Username: {} Password: {}\n", version, (const char*)username, (const char*)password);

         delete [] username;
         delete [] password;
         }
         break;
      case Opcode(MESSAGE): {
         std::print("Received a Message packet. Information: \n");

         int32_t usize = 0, msize = 0;
         uint8_t *name;
         uint8_t *msg;

         usize = pkt->getInt();
         name = new uint8_t[usize];
         pkt->getBytes(name, usize);

         msize = pkt->getInt();
         msg = new uint8_t[msize];
         pkt->getBytes(msg, msize);

         std::print("Name: {} Msg: {}\n", (const char*)name, (const char*)msg);

         delete [] name;
         delete [] msg;
         }
         break;
      default:
         std::print("Unknown Opcode: 0x{:x}\n", opcode);
         break;
   }

   std::print("\n");
}

/**
 * Verify Login Packet
 * Re-parses a login packet from the start and confirms the encoded fields match the expected values.
 *
 * @return true if all fields match and the entire packet was consumed
 */
bool verifyLoginPacket(ByteBuffer* pkt, int32_t expVersion, const string& expUsername, const string& expPassword) {
   pkt->setReadPos(0);

   if (pkt->getShort() != Opcode(LOGIN))
      return false;

   int32_t version = pkt->getInt();
   if (version != expVersion)
      return false;

   int32_t usize = pkt->getInt();
   auto uBuf = make_unique<uint8_t[]>(usize);
   pkt->getBytes(uBuf.get(), usize);
   if (string((const char*)uBuf.get()) != expUsername)
      return false;

   int32_t psize = pkt->getInt();
   auto pBuf = make_unique<uint8_t[]>(psize);
   pkt->getBytes(pBuf.get(), psize);
   if (string((const char*)pBuf.get()) != expPassword)
      return false;

   return pkt->bytesRemaining() == 0;
}

/**
 * Verify Chat Message Packet
 * Re-parses a chat message packet from the start and confirms the encoded fields match the expected values.
 *
 * @return true if all fields match and the entire packet was consumed
 */
bool verifyChatMsgPacket(ByteBuffer* pkt, const string& expName, const string& expMsg) {
   pkt->setReadPos(0);

   if (pkt->getShort() != Opcode(MESSAGE))
      return false;

   int32_t usize = pkt->getInt();
   auto uBuf = make_unique<uint8_t[]>(usize);
   pkt->getBytes(uBuf.get(), usize);
   if (string((const char*)uBuf.get()) != expName)
      return false;

   int32_t msize = pkt->getInt();
   auto mBuf = make_unique<uint8_t[]>(msize);
   pkt->getBytes(mBuf.get(), msize);
   if (string((const char*)mBuf.get()) != expMsg)
      return false;

   return pkt->bytesRemaining() == 0;
}

int32_t main() {
   int failures = 0;
   auto check = [&failures](bool cond, string_view msg) {
      if (!cond) {
         std::print("  FAIL: {}\n", msg);
         ++failures;
      }
   };

   // --- Login packet ---
   std::print("== Login packet ==\n");
   {
      const int32_t version  = 1234;
      const string  username = "fubar";
      const string  password = "testpwd";
      // Expected wire size: 2 (opcode) + 4 (version) + 4 (usize) + 6 (username+null)
      //                   + 4 (psize)  + 8 (password+null) = 28 bytes
      const uint32_t expectedSize = 28;

      ByteBuffer* loginPkt = createLoginPacket(version, username, password);

      check(loginPkt->size() == expectedSize, "login packet: wire size is correct");

      serverParser(loginPkt); // display
      check(loginPkt->bytesRemaining() == 0, "login packet: all bytes consumed by parser");
      check(verifyLoginPacket(loginPkt, version, username, password),
            "login packet: opcode, version, username, password all verified");

      delete loginPkt;
   }

   // --- Chat message packet ---
   std::print("== Chat message packet ==\n");
   {
      const string name = "fubar";
      const string msg  = "message yay!";
      // Expected wire size: 2 (opcode) + 4 (nsize) + 6 (name+null)
      //                   + 4 (msize)  + 13 (msg+null) = 29 bytes
      const uint32_t expectedSize = 29;

      ByteBuffer* msgPkt = createChatMsgPacket(name, msg);

      check(msgPkt->size() == expectedSize, "chat packet: wire size is correct");

      serverParser(msgPkt); // display
      check(msgPkt->bytesRemaining() == 0, "chat packet: all bytes consumed by parser");
      check(verifyChatMsgPacket(msgPkt, name, msg),
            "chat packet: opcode, name, msg all verified");

      delete msgPkt;
   }

   // --- Unknown opcode ---
   std::print("== Unknown opcode ==\n");
   {
      ByteBuffer unknownPkt;
      unknownPkt.putShort(Opcode(UNKNOWN));
      unknownPkt.putInt(0xDEADBEEF);
      // serverParser should hit the default case and not crash
      serverParser(&unknownPkt);
      check(unknownPkt.bytesRemaining() == 4, // only the short was consumed
            "unknown opcode: only opcode bytes consumed by parser");
   }

   if (failures == 0) {
      std::print("All tests PASSED\n");
      return 0;
   }
   std::print("{} test(s) FAILED\n", failures);
   return 1;
}
