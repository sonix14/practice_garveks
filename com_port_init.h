#ifndef COM_PORT_INIT_H
#define COM_PORT_INIT_H

#include <windows.h>
#include <string>

struct Com_port {
    Com_port();
    virtual ~Com_port();

    bool openPort(const std::string& port, int baudrate);
    void closePort();

private:
    bool installPortSettings(int baudrate);
    bool installPortTimeouts();

    HANDLE cPort;
};

#endif