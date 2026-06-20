#include "../../include/core/packet_parsing.hpp"

static void printDebugBuffer(const PacketParsingData &data) {
    std::printf("Received data: ");
    std::printf("[");
    for(size_t i = 0; i < data.bufferLen; i++) {
        if(i == data.bufferLen - 1)
            std::printf("%02X", data.buffer[i]);
        else
            std::printf("%02X ", data.buffer[i]);
    }
    std::printf("]\n");
}
static void printDebugBufferChar(const PacketParsingData &data) {
    std::printf("Received data: ");
    std::printf("[");
    for(size_t i = 0; i < data.bufferLen; i++)
        if(data.buffer[i] == 0)
            std::printf(" ");
        else
            std::printf("%c", (char)data.buffer[i]);
    std::printf("]\n");
}

void initMessageParsing(PacketParsingData &data) {
    std::memset(data.buffer, 0, SIZE_PACKET_READ);
    data.bufferLen = 0;
    data.offset = 0;
    data.start = 0;
    data.messageSize = 0;
    data.state = PacketParsingState::NONE;
}
static void flushBuffer(PacketParsingData &data) {
    if(data.state == PacketParsingState::NONE) {
        std::memmove(data.buffer, data.buffer + data.offset, data.bufferLen - data.offset);
        data.bufferLen -= data.offset;
        data.offset = 0;
    } else {
        std::memmove(data.buffer, data.buffer + data.start, data.bufferLen - data.start);
        data.bufferLen -= data.start;
        data.start = 0;
    }
}
ssize_t recvInBufferFD(PacketParsingData &data, int fd) {
    ssize_t nbytes;

    nbytes = recv(fd, data.buffer + data.bufferLen, SIZE_PACKET_READ - data.bufferLen, MSG_DONTWAIT);
    if(nbytes > 0)
        data.bufferLen += nbytes;
    return  nbytes;
}
ssize_t recvInBufferSSL(PacketParsingData &data, SSL* ssl) {
    ssize_t nbytes;

    nbytes = SSL_read(ssl, data.buffer + data.bufferLen, SIZE_PACKET_READ - data.bufferLen);
    if(nbytes > 0)
        data.bufferLen += nbytes;
    return  nbytes;
}
ssize_t recvInBuffer(PacketParsingData &data, const Connection& connection) {
    if(connection.tls)
        return recvInBufferSSL(data, connection.ssl);
    else
        return recvInBufferFD(data, connection.fd);
}
NextMessageState nextMessage(PacketParsingData &data, uint8_t magicByte, uint8_t buffer[NM_MAX_SIZE]) {
    while(data.offset < data.bufferLen) {
        switch(data.state) {
        case PacketParsingState::NONE:
            if(data.buffer[data.offset] == magicByte) {
                data.state = PacketParsingState::MAGIC;
                data.start = data.offset;
            } else
                data.offset++;
            break;
        case PacketParsingState::MAGIC:
            data.state = PacketParsingState::TYPE;
            data.offset++;
            break;
        case PacketParsingState::TYPE:
            data.state = PacketParsingState::STATUS;
            data.offset++;
            break;
        case PacketParsingState::STATUS:
            data.state = PacketParsingState::ERROR;
            data.offset++;
            break;
        case PacketParsingState::ERROR:
            data.state = PacketParsingState::SIZE_1;
            data.offset++;
            break;
        case PacketParsingState::SIZE_1:
            data.messageSize = ((uint16_t)data.buffer[data.offset]) << 8;
            data.state = PacketParsingState::SIZE_2;
            data.offset++;
            break;
        case PacketParsingState::SIZE_2:
            data.messageSize |= (uint16_t)data.buffer[data.offset];
            if(data.messageSize < NM_HEADER_SIZE) {
                data.offset = data.start + 1;
                data.state = PacketParsingState::NONE;
            } else if(data.messageSize == NM_HEADER_SIZE)
                data.state = PacketParsingState::FULL;
            else {
                data.state = PacketParsingState::CONTENT;
                data.offset++;
            }
            break;
        case PacketParsingState::CONTENT:
            if(data.start + data.messageSize <= data.bufferLen) {
                data.state = PacketParsingState::FULL;
                data.offset = data.start + data.messageSize;
            } else
                data.offset = data.bufferLen;
            break;
        case PacketParsingState::FULL:
            std::memcpy(buffer, data.buffer + data.start + 1, data.messageSize);
            data.state = PacketParsingState::NONE;
            data.start = 0;
            data.messageSize = 0;
            flushBuffer(data);
            return NextMessageState::OK;
        }
    }
    if(data.bufferLen == SIZE_PACKET_READ)
        flushBuffer(data);
    return NextMessageState::NO_MESSAGE;
}
