#include "ComInterface.h"
#include <Windows.h>
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
