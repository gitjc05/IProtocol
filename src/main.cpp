#include <iostream>
#include <thread>
#include <chrono>
#include "ComInterface.h"
#include "UdpInterface.h"

//*** To open project in CMD:   ***
//*** ...\modular_protocols> devenv .***

int main() {
    
    ComInterface comCLient;
    ComInterface comServer;
    UdpInterface UdpServer;

    std::string serialPort2 = "COM2";
    std::string serialPort4 = "COM4";
    std::string ipAddr = "127.0.0.1";
    uint16_t port = 8080;

    if(!comCLient.Connect(serialPort2)) {
        return -1;
    }
    if(!comServer.Connect(serialPort4)) {
        return -1;
    }
    if(!UdpServer.Connect(ipAddr, port, true)) {
        return -1;
    }

    bool running = true;
    int recvBufSize = 512;
    char* received = new char[recvBufSize] { 0 };
    char* receivedUdp = new char[recvBufSize] { 0 };

    while (running)
    {
        std::string input;
        std::cout << "Client> ";
        std::getline(std::cin, input);
        if (!comCLient.Write(input.c_str(), input.size()))
        {
            std::cerr << "Failed to write\n";
            return -1;
        }
        int count = comServer.Read(received, recvBufSize);
        if (count > 0)
        {
            UdpServer.Write(received, count);
            int readCount = UdpServer.Read(receivedUdp, recvBufSize);
            if (readCount > 0)
            {
                std::cout << "UDP Server: " << receivedUdp << std::endl;
            }
            std::cout << "COM Server: " << received << std::endl;
            if (received[0] == 'q' && count == 1) running = false;
            memset(received, 0x0, recvBufSize);
            memset(receivedUdp, 0x0, recvBufSize);
        }
    }

    return 0;
}
