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

#include <iostream>
#include "../../ByteBuffer.hpp"

using namespace std;

ByteBuffer* createLoginPacket(int32_t version, string username, string password);
ByteBuffer* createChatMsgPacket(string name, string msg);
void serverParser(ByteBuffer* pkt);

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
 * This ficticious packet parser on the "server" will read the ByteBuffer'd packets and print32_t out information about each packet it understands
 * according to the networking protocol
 *
 * @param pkt A pointer to a ByteBuffer containing the packet data
 */
void serverParser(ByteBuffer* pkt) {
   printf("Parsing ByteBuffer'd packet of size: %i\n", pkt->size());

   // Read the first 2 bytes (short) of the packet to determine the opcode
   short opcode = 0;
   opcode = pkt->getShort();

   // Switch based off the opcode to handle the specific packet
   switch(opcode) {
      case Opcode(LOGIN): {
         printf("Received a Login packet. Information: \n");

         int32_t version = 0;
         int32_t usize = 0, psize = 0;
         uint8_t *username;
         uint8_t *password;

         // Parse the packet based off a known structure

         version = pkt->getInt();

         usize = pkt->getInt();
         username = new uint8_t[usize];
         pkt->getBytes(username, usize);
         
         psize = pkt->getInt();
         password = new uint8_t[psize];
         pkt->getBytes(password, psize);

         printf("Client Version: %i, Username: %s Password: %s\n", version, username, password);

         delete [] username;
         delete [] password;
         }
         break;
      case Opcode(MESSAGE): {
         printf("Received a Message packet. Information: \n");

         int32_t usize = 0, msize = 0;
         uint8_t *name;
         uint8_t *msg;

         // Parse the packet based off a known structure

         usize = pkt->getInt();
         name = new uint8_t[usize];
         pkt->getBytes(name, usize);
         
         msize = pkt->getInt();
         msg = new uint8_t[msize];
         pkt->getBytes(msg, msize);

         printf("Name: %s Msg: %s\n", name, msg);

         delete [] name;
         delete [] msg;
         }
         break;
      default:
         printf("Unknown Opcode: 0x%x\n", opcode);
         break;
   }

   printf("\n");
}

int32_t main() {
   // Create two packets that conform to the network protocol
   ByteBuffer *loginPkt = createLoginPacket(1234, "fubar", "testpwd");
   ByteBuffer *msg = createChatMsgPacket("fubar", "message yay!");

   // Have the server parse both packets
   serverParser(loginPkt);
   serverParser(msg);

   delete loginPkt;
   delete msg;

   return 0;
}
