#ifndef COM_PORT_INIT_H
#define COM_PORT_INIT_H

#include <windows.h>
#include <string>

struct Com_port {
    Com_port();
    virtual ~Com_port();

    bool openPort(const std::string& port, int baudrate);
    void closePort();

    virtual void writeData();
    virtual void readData();

private:
    bool installPortSettings(int baudrate);
    bool installPortTimeouts();
    void calculateChecksum(const std::string& file, const unsigned& size);

    HANDLE cPort;
};

#endif