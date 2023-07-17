#ifndef COM_PORT_INIT_H
#define COM_PORT_INIT_H

#include <windows.h>
#include <string>

struct Com_port {
    HANDLE cPort;

    Com_port();
    virtual ~Com_port();

    bool openPort(const std::string& port, int baudrate);
    void closePort();

    virtual void writeData(const std::string& file);
    virtual void readData(char* dst);

private:
    bool installPortSettings(int baudrate);
    bool installPortTimeouts();

    wchar_t* convertToLPCTSTR(const std::string& str);
};

#endif