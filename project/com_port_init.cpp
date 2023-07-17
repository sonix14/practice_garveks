#include "COM_PORT_INIT.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

Com_port::Com_port() {
    cPort = INVALID_HANDLE_VALUE;
}

Com_port::~Com_port() {
    closePort();
}

bool Com_port::openPort(const std::string& port, int baudrate) {

    closePort();
    wchar_t* wPort = convertToLPCTSTR(port);
    LPCTSTR portName = wPort;  // L"COM1";
    cPort = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL); // 0 instead of FILE_ATTRIBUTE_NORMAL means there will be synchronous transmission
    if (cPort == INVALID_HANDLE_VALUE) {
        std::cout << "Comm Port was not open successfully\n";
        DWORD dw = GetLastError();
        std::cout << std::to_string(dw) << "\n";
        CloseHandle(cPort);
        cPort = INVALID_HANDLE_VALUE;
        return false;
    }
    else {
        if (installPortSettings(baudrate) && installPortTimeouts()) {
            std::cout << "Comm Port will be opened successfully\n";
            return true;
        }
        else {
            CloseHandle(cPort);
            cPort = INVALID_HANDLE_VALUE;
            std::cout << "Comm Port was not open successfully\n";
            return false;
        }
    }
}

bool Com_port::installPortSettings(int baudrate) {
    DCB PortDCB = { 0 };
    PortDCB.DCBlength = sizeof(DCB); // getting the default information of the DCB structure
    if (!GetCommState(cPort, &PortDCB)) {
        std::cout << "GetCommState failed with error %d.\n " << std::to_string(GetLastError());
        return false;
    }
    // initialization of port parameters by setting values of DCB structure fields
    PortDCB.BaudRate = DWORD(baudrate); // data exchange rate 9600 baud/s  
    PortDCB.fBinary = TRUE;  // binary exchange mode
    PortDCB.ByteSize = 8;
    PortDCB.Parity = NOPARITY;  // 0-4 = no,odd,even,mark,space
    PortDCB.StopBits = ONESTOPBIT;

    if (!SetCommState(cPort, &PortDCB)) {
        std::cout << "Unable to configure the serial port\n";
        std::cout << "SetCommState failed with error %d.\n " << std::to_string(GetLastError());
        return false;
    }
    return true;
}

bool Com_port::installPortTimeouts() {
    COMMTIMEOUTS CommTimeouts;
    if (!GetCommTimeouts(cPort, &CommTimeouts)) {
        std::cout << "GetCommTimeouts failed with error %d.\n " << std::to_string(GetLastError());
        return false;
    }
    CommTimeouts.ReadIntervalTimeout = 0xFFFFFFFF; // maximum time between reading two characters
    CommTimeouts.ReadTotalTimeoutMultiplier = 0;
    CommTimeouts.ReadTotalTimeoutConstant = 0;
    CommTimeouts.WriteTotalTimeoutMultiplier = 0;
    CommTimeouts.WriteTotalTimeoutConstant = 1500; // no delay when reading

    if (!SetCommTimeouts(cPort, &CommTimeouts))
    {
        std::cout << "Unable to set the timeout parameters\n";
        std::cout << "SetCommTimeouts failed with error %d.\n " << std::to_string(GetLastError());
        return false;
    }
    return true;
}

void Com_port::closePort() {
    if (cPort != INVALID_HANDLE_VALUE) {
        CloseHandle(cPort);
        cPort = INVALID_HANDLE_VALUE;
        std::cout << "Comm Port will be closed successfully\n";
    }
}

void Com_port::writeData(const std::string& file) {

}

void Com_port::readData(char* dst) {
    const int READ_TIME = 100;
    OVERLAPPED sync = { 0 };
    int reuslt = 0;
    //char* dst[1024] = { 0 }; //unsigned
    //char* dst = new char(1024);
    unsigned long size = sizeof(dst);
    unsigned long wait = 0, read = 0, state = 0;
    sync.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // Creating a synchronization object

    if (SetCommMask(cPort, EV_RXCHAR)) {  // Setting the mask for port events  
        WaitCommEvent(cPort, &state, &sync);  // Linking the port and the synchronization object   
        wait = WaitForSingleObject(sync.hEvent, READ_TIME);  // Start waiting for data
        if (wait == WAIT_OBJECT_0) {  // Data received
            ReadFile(cPort, dst, size, &read, &sync);
            wait = WaitForSingleObject(sync.hEvent, READ_TIME);
            if (wait == WAIT_OBJECT_0)
                if (GetOverlappedResult(cPort, &sync, &read, FALSE))
                    reuslt = read;
        }
    }
    CloseHandle(sync.hEvent);
}

wchar_t* Com_port::convertToLPCTSTR(const std::string& str) {
    char* chars = new char[str.length() + 1];
    str.copy(chars, str.length());
    chars[str.length()] = '\0';
}