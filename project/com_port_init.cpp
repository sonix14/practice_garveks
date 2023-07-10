#include "COM_PORT_INIT.h"

#include <windows.h>
#include <iostream>
#include <string>

Com_port::Com_port() {
    cPort = INVALID_HANDLE_VALUE;
}

Com_port::~Com_port() {
    closePort();
}

bool Com_port::openPort(const std::string& port, int baudrate) {
    
    closePort();
    LPCTSTR portName = L"COM1";
    //(LPCTSTR)port.c_str() error code 123 Incorrect syntax of the file name, directory name, or volume label.
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
    }
}