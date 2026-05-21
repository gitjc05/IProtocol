#pragma once
#include <vector>
#include <string>
#include <asio.hpp>

#include <array>
#include <iostream>
#include <stdexcept>
#include <stdint.h>

#define UDPTYPE 0x1
#define TCPTYPE 0x2
#define COMTYPE 0x3

class ProtocolInterface
{
protected:
public:
	uint8_t Protocol;
	virtual bool Connect(std::string address, int port = 0, bool server = false) = 0;
	virtual int Read(char* data, int length) = 0;
	virtual bool Write(const char* data, int length) = 0;
	virtual bool SetTimeout(size_t timeout) = 0;
	virtual void Disconnect() = 0;
};