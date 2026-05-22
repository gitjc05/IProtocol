#include "ComInterface.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#endif
#include <format>

ComInterface::ComInterface() : io(), SerialPort(io)
{
	Protocol = COMTYPE;
}

bool ComInterface::Connect(std::string address, int port, bool server)
{
	try
	{
		SerialPort.open(address);

		SerialPort.set_option(asio::serial_port_base::baud_rate(115200));
		SerialPort.set_option(asio::serial_port_base::parity(
			asio::serial_port_base::parity::odd
		));
		SerialPort.set_option(asio::serial_port_base::stop_bits(
			asio::serial_port_base::stop_bits::one
		));
		SerialPort.set_option(asio::serial_port_base::character_size(8));
		SerialPort.set_option(asio::serial_port_base::baud_rate(115200));
		SerialPort.set_option(asio::serial_port_base::flow_control(
			asio::serial_port_base::flow_control::none
		));
		return true;
	} catch (const std::exception& e)
	{
		std::cerr << std::format("Failed to connect to port {}\n", address);
		return false;
	}
}

int ComInterface::Read(char* data, int length)
{
	try
	{
		return SerialPort.read_some(asio::buffer(data, length));
	} catch (const std::exception& e)
	{
		std::cerr << "Failed while reading\n";
		return 0;
	}
}

bool ComInterface::Write(const char* data, int length)
{
	try
	{
		asio::write(SerialPort, asio::buffer(data, length));
		return true;
	} catch (const std::exception& e)
	{
		std::cerr << "Failed to write to port\n";
		return false;
	}
}

bool ComInterface::SetTimeout(size_t timeoutMs)
{
#ifdef _WIN32
	if (timeoutMs == 0)
	{
		COMMTIMEOUTS timeouts {};
		return SetCommTimeouts(SerialPort.native_handle(), &timeouts) != 0;
	}

	HANDLE handle = SerialPort.native_handle();

	COMMTIMEOUTS timeouts {};
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = timeoutMs;

	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;

	return SetCommTimeouts(handle, &timeouts) != 0;
#else
	int fd = SerialPort.native_handle();

	termios tty {};
	if (tcgetattr(fd, &tty) != 0)
	{
		std::cerr << std::format(
			"Failed to get serial timeout settings: {}\n",
			std::strerror(errno)
		);
		return false;
	}

	if (timeoutMs == 0)
	{
		tty.c_cc[VMIN] = 1;
		tty.c_cc[VTIME] = 0;
	} else
	{
		size_t deciseconds = (timeoutMs + 99) / 100;

		if (deciseconds == 0)
		{
			deciseconds = 1;
		} else if (deciseconds > 255)
		{
			deciseconds = 255;
		}

		tty.c_cc[VMIN] = 0;
		tty.c_cc[VTIME] = static_cast<cc_t>(deciseconds);
	}

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		std::cerr << std::format(
			"Failed to set serial timeout settings: {}\n",
			std::strerror(errno)
		);
		return false;
	}

	return true;
#endif
}

void ComInterface::Disconnect()
{
	try
	{
		SerialPort.close();
	} catch (const std::exception& e)
	{
		std::cerr << "Failed in closing the COM port\n";
		return;
	}
}
