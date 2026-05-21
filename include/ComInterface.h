#include "ProtocolInterface.h"

class ComInterface : public ProtocolInterface
{
private:
public:
	asio::io_context io;
	asio::serial_port SerialPort;
	ComInterface();
	bool Connect(std::string address, int port = 0, bool server = false) override;
	int Read(char* data, int length) override;
	bool Write(const char* data, int length) override;
	bool SetTimeout(size_t timeout) override;
	void Disconnect() override;
};
