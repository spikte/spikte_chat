#ifndef CORE_PACKET_PARSING_HPP
#define CORE_PACKET_PARSING_HPP

#include "api.hpp"
#include <array>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <queue>
#include <sys/socket.h>
#include <sys/types.h>

constexpr int SIZE_PACKET_READ = 65535;
enum class NextMessageState {
    NO_MESSAGE,
    OK
};
enum class PacketParsingState {
    NONE,
    MAGIC,
    TYPE,
    STATUS,
    ERROR,
    SIZE_1,
    SIZE_2,
    CONTENT,
    FULL
};
struct PacketParsingData {
    PacketParsingState state;
    uint8_t buffer[SIZE_PACKET_READ];
    size_t bufferLen; // Size of the buffer, i.e. when the real data stops
    size_t offset;    // Offset of parsed data
    size_t start;     // Message start offset
    NetworkMessageSize messageSize;
};
void initMessageParsing(PacketParsingData &data);

ssize_t recvInBufferFD(PacketParsingData &data, int fd);
ssize_t recvInBufferSSL(PacketParsingData &data, SSL *ssl);
ssize_t recvInBuffer(PacketParsingData &data, const Connection& connection);
NextMessageState nextMessage(PacketParsingData &data, uint8_t magicByte, uint8_t buffer[NM_MAX_SIZE]);

#endif
