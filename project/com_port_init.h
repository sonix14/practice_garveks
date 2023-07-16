#ifndef COM_PORT_INIT_H
#define COM_PORT_INIT_H

#include <windows.h>
#include <string>

struct Com_port {
    HANDLE cPort;
    const int MAX_ERROR = 3;

    Com_port();
    virtual ~Com_port();

    bool openPort(const std::string& port, int baudrate);
    void closePort();

    virtual void writeData(const std::string& file);
    virtual void readData();

private:
    bool installPortSettings(int baudrate);
    bool installPortTimeouts();

    unsigned long calculateChecksumCRC32(unsigned char* mass, unsigned long count);
    unsigned short calculateChecksumCRC16( char* mass, unsigned long count);

    wchar_t* convertToLPCTSTR(const std::string& str);
    bool getAnswer();
};

#endif