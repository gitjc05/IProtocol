#pragma once

#include "ProtocolInterface.h"

class UdpInterface : public ProtocolInterface
{
private:
	asio::io_context io;
	asio::ip::udp::endpoint UdpEndpoint;
	asio::ip::udp::socket UdpSock;
public:
	UdpInterface();
	bool Connect(std::string address, int port = 0, bool server = false) override;
	int Read(char* data, int length) override;
	bool Write(const char* data, int length) override;
	bool SetTimeout(size_t timeout) override;
	void Disconnect() override;
};