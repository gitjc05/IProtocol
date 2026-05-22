#include "UdpInterface.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#include <cerrno>
#include <cstring>
#endif
#include <format>

UdpInterface::UdpInterface() : io(), UdpSock(io)
{
	Protocol = UDPTYPE;
}

bool UdpInterface::Connect(std::string address, int port, bool server)
{
	std::error_code ec;

	UdpSock.open(asio::ip::udp::v4(), ec);
	if (ec)
	{
		std::cerr << std::format("UDP open failed: {}\n", ec.message());
		return false;
	}

	if (server)
	{
		asio::ip::udp::endpoint localEndpoint(asio::ip::udp::v4(), port);

		UdpSock.bind(localEndpoint, ec);
		if (ec)
		{
			std::cerr << std::format("UDP bind failed: {}\n", ec.message());
			return false;
		}
	}

	asio::ip::udp::resolver resolver(io);

	auto endpoints = resolver.resolve(
		asio::ip::udp::v4(),
		address,
		std::to_string(port),
		ec
	);

	if (ec)
	{
		std::cerr << std::format("UDP resolve failed: {}\n", ec.message());
		return false;
	}


	if (!server)
	{
		UdpEndpoint = *endpoints.begin();
		UdpSock.connect(UdpEndpoint, ec);
		if (ec)
		{
			std::cerr << std::format("UDP connect failed: {}\n", ec.message());
			return false;
		}
	}

	return true;
}

int UdpInterface::Read(char* data, int length)
{
	std::error_code ec;
	
	std::size_t readSize = UdpSock.receive(asio::buffer(data, length), 0, ec);

	if (ec)
	{
		std::cerr << std::format("Failed while reading: {}\n", ec.message());
		return 0;
	}
	return readSize;
}

bool UdpInterface::Write(const char* data, int length)
{
	std::error_code ec;
	UdpSock.send(asio::buffer(data, length),0,ec);

	if (ec)
	{
		std::cerr << std::format("Failed to write: {}\n", ec.message());
		return false;
	}
	return true;
}

bool UdpInterface::SetTimeout(size_t timeoutMs)
{
#ifdef _WIN32
	DWORD timeout = static_cast<DWORD>(timeoutMs);

	int result = setsockopt(
		UdpSock.native_handle(),
		SOL_SOCKET,
		SO_RCVTIMEO,
		reinterpret_cast<const char*>(&timeout),
		sizeof(timeout)
	);
	return result == 0;
#else
	timeval timeout {};
	timeout.tv_sec = static_cast<time_t>(timeoutMs / 1000);
	timeout.tv_usec = static_cast<suseconds_t>((timeoutMs % 1000) * 1000);

	int result = setsockopt(
		UdpSock.native_handle(),
		SOL_SOCKET,
		SO_RCVTIMEO,
		&timeout,
		sizeof(timeout)
	);

	if (result != 0)
	{
		std::cerr << std::format(
			"Failed to set UDP receive timeout: {}\n",
			std::strerror(errno)
		);
		return false;
	}

	return true;
#endif
}

void UdpInterface::Disconnect()
{
	std::error_code ec;
	UdpSock.close(ec);
	std::cerr << std::format("Failed in closing the UDP socket: {}\n", ec.message());
	return;
}
