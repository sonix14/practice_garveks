#ifndef COM_PORT_INIT_H
#define COM_PORT_INIT_H

#include <windows.h>
#include <string>

struct Com_port {
    HANDLE cPort;

    Com_port();
    virtual ~Com_port();

    bool openPort(const std::string& port, const int* baudrate);
    bool closePort();

    virtual bool writeData(const char* data, const DWORD& dwSize);
    virtual bool readData(char* dst, unsigned long& read);

private:
    bool installPortSettings(const int* baudrate);
    bool installPortTimeouts();

    wchar_t* convertToLPCTSTR(const std::string& str);
};

#endif