#include "COM_PORT_INIT.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
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
    LPCTSTR portName = L"COM1"; //!!!!!!!!!!
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
        std::cout << "Comm Port will be closed successfully\n";
    }
}

void Com_port::readData() {
    int counter = 0;
    bool flag = true;
    std::vector<char> buffer;
    int fullSize = 0;
    long int fullChecksum = 0;
    long int checksum = 1;
    long int trialChecksum = 0;
    const int READ_TIME = 100;
    OVERLAPPED sync = { 0 };
    int result = 0;
    unsigned long wait = 0, read = 0, state = 0;
    char dst[1024] = { 0 }; //unsigned
    unsigned long size = sizeof(dst);
    sync.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL); // Creating a synchronization object

    while (true) {
        if (SetCommMask(cPort, EV_RXCHAR)) { // Setting the mask for port events
            if (WaitCommEvent(cPort, &state, &sync)) { // Linking the port and the synchronization object
                wait = WaitForSingleObject(sync.hEvent, INFINITE);// Start waiting for data
                if (wait == WAIT_OBJECT_0) { // Data received
                    if (fullSize == 0) {
                        ReadFile(cPort, dst, size, &read, 0); //size of file
                        fullSize = std::atoi(dst);
                        //fullSize = std::stoi(std::string(buffer));
                        if (fullSize <= 0) {
                            std::cout << "Uncorrect size\n";
                        }
                        else
                            std::cout << "Accepted size\n";
                        memset(dst, 0, 1024);
                    }
                    else if (fullSize != 0 && fullChecksum == 0) {
                        ReadFile(cPort, dst, size, &read, &sync); //full file check sum  
                        fullChecksum = std::atoi(dst);
                        if (fullChecksum <= 0) {
                            std::cout << "Uncorrect checksum\n";
                        }
                        else
                            std::cout << "Accepted checksum\n";
                        memset(dst, 0, 1024);
                    }
                    else {
                        //give an answer about readiness to accept data
                        std::cout << "Ready to accept data\n";
                        if (trialChecksum > fullChecksum) {
                            //close
                            std::cout << "Too much fragments\n";
                            break;
                        }
                        else if (trialChecksum < fullChecksum) {
                            char temp[1024] = { 0 };
                            if (counter == MAX_ERROR) {
                                std::cout << "Too much tries for one fragment!\n";
                                //close
                                counter = 0;
                                break;
                            }
                            if (flag == true) {
                                ReadFile(cPort, temp, size, &read, &sync); //fragment 
                                flag = false;
                            }
                            else {
                                ReadFile(cPort, dst, size, &read, &sync); // checksum
                                checksum = std::atoi(dst);
                                if (calculateChecksumCRC16(temp, size) != checksum) {
                                    //uncorrect fragment
                                    std::cout << "The fragment was not accepted successfully! Try again\n";
                                    counter++;
                                    flag = true;
                                    continue;
                                }
                                else {
                                    std::cout << "The fragment was accepted successfully\n";
                                    for (int i : temp) {
                                        buffer.push_back(temp[i]);
                                    }
                                    int buffSize = buffer.size();
                                    char* array = new char[buffSize];
                                    for (int i = 0; i < buffSize; i++) {
                                        array[i] = buffer[i];
                                    }
                                    trialChecksum = calculateChecksumCRC16(array, buffSize);
                                    delete[] array;
                                    counter = 0;
                                }
                                flag = true;
                                //memset(dst, 0, 1024);
                            }
                            memset(dst, 0, 1024);
                        }
                        else if (trialChecksum == fullChecksum) {
                            //write file into the folder
                            std::cout << "The file was accepted successfully!\n";
                            //close
                        }
                        /*
                        if (wait == WAIT_OBJECT_0)
                            if (GetOverlappedResult(cPort, &sync, &read, FALSE)) {
                                result = read;
                                std::cout << "Size of fragment: " << result << '\n';
                        }
                        */
                    }
                }
            }
        }
        ResetEvent(sync.hEvent);
    }
    //std::cout << "Received bytes: " << result;
    //std::cout << "\nFile: " << dst;
    CloseHandle(sync.hEvent);
}


void Com_port::writeData(const std::string& file) {

    DWORD dwBytesWritten;
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    unsigned long file_size =  in.tellg();
    std::cout << "Rope Weight : " << file_size << " byte" << "\n";
    DWORD dwSize = sizeof(file_size);
    BOOL iRet = WriteFile(cPort, &file_size, dwSize, &dwBytesWritten, NULL);
    if (!iRet || dwBytesWritten != dwSize) {
        CloseHandle(cPort);
        cPort = INVALID_HANDLE_VALUE;
        std::cout << "Error writing file size to port\n";
    }

    const char file_name[10] = "hello.txt";//!!!!!!!!!!!!
    FILE* read_all_file;
    fopen_s(&read_all_file, file_name, "r");
    unsigned long all_file_size = file_size;
    unsigned char* mass_all_file = new unsigned char[all_file_size];
    if (read_all_file) {
        unsigned long count = fread(mass_all_file, sizeof mass_all_file[0], file_size, read_all_file);
        fclose(read_all_file);
        
        unsigned long check_sum = calculateChecksumCRC32(mass_all_file, count);
        std::cout << "CRC32 = " << check_sum << "\n\n";
        dwSize = sizeof(check_sum);
        iRet = WriteFile(cPort, &check_sum, dwSize, &dwBytesWritten, NULL);
        if (!iRet || dwBytesWritten != dwSize) {
            CloseHandle(cPort);
            cPort = INVALID_HANDLE_VALUE;
            std::cout << "Error writing check sum to port\n";
        }
    } else {
        std::cout << "File not found\n";
    }

    unsigned long buff = file_size;
    FILE* fp;
    fopen_s(&fp, file_name, "r");
    const int fragment_size = 10;
    char mass_fragment[fragment_size];
    if (fp) {
        while (buff > fragment_size) {
            size_t count = fread(mass_fragment, sizeof mass_fragment[0], fragment_size, fp);
            buff -= fragment_size;

            dwSize = sizeof(mass_fragment);
            iRet = WriteFile(cPort, &mass_fragment, dwSize, &dwBytesWritten, NULL);
            if (!iRet || dwBytesWritten != dwSize) {
                CloseHandle(cPort);
                cPort = INVALID_HANDLE_VALUE;
                std::cout << "Error writing to port\n";
            }
            unsigned short check_sum = calculateChecksumCRC16(mass_fragment, fragment_size);
            std::cout << "CRC16 = " << check_sum << "\n";
            dwSize = sizeof(check_sum);
            iRet = WriteFile(cPort, &check_sum, dwSize, &dwBytesWritten, NULL);
            if (!iRet || dwBytesWritten != dwSize) {
                CloseHandle(cPort);
                cPort = INVALID_HANDLE_VALUE;
                std::cout << "Error writing check sum fragment to port\n";
            }
        }
        unsigned long endPart_fragment_size = buff;
        char* endPart_mass_fragment = new char[endPart_fragment_size];
        if (buff > 0) {
            size_t count = fread(endPart_mass_fragment, sizeof endPart_mass_fragment[0], buff, fp);
            printf("read %zu elements out of %d\n", count, buff);
        }
        dwSize = sizeof(endPart_mass_fragment);
        iRet = WriteFile(cPort, &endPart_mass_fragment, dwSize, &dwBytesWritten, NULL);
        if (!iRet || dwBytesWritten != dwSize) {
            CloseHandle(cPort);
            cPort = INVALID_HANDLE_VALUE;
            std::cout << "Error writing to port\n";
        }
        unsigned short check_sum = calculateChecksumCRC16(endPart_mass_fragment, endPart_fragment_size);
        std::cout << "CRC16 = " << check_sum << "\n\n";
        dwSize = sizeof(check_sum);
        iRet = WriteFile(cPort, &check_sum, dwSize, &dwBytesWritten, NULL);
        if (!iRet || dwBytesWritten != dwSize) {
            CloseHandle(cPort);
            cPort = INVALID_HANDLE_VALUE;
            std::cout << "Error writing check sum fragment to port\n";
        }
        delete[] endPart_mass_fragment;
        delete[] mass_all_file;
        fclose(fp);
    }
    else {
        std::cout << "File not found\n";
    }
}

/*We describe the Crc32 calculation function
using a polynomial EDB88320UL=
x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11
+ x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1*/
unsigned long Com_port::calculateChecksumCRC32(unsigned char* mass, unsigned long count) {
    //initialize the Crc32 calculation table
    unsigned long crc_table[256];//mass 32 bit = 4 byte
    unsigned long crc;//variable 32 bit = 4 byte
    for (int i = 0; i < 256; i++)//initialize the array loop
    {
        crc = i;
        for (int j = 0; j < 8; j++)//polynomial iteration cycle
            crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
        crc_table[i] = crc;
    };
    crc = 0xFFFFFFFFUL;
    while (count--)// checking the continuation condition
        crc = crc_table[(crc ^ *mass++) & 0xFF] ^ (crc >> 8);
    return crc^ 0xFFFFFFFFUL;
}

/*We describe the Crc16 standard CCITT calculation function
using the polynomial 1021=x^16+x^12 +x^5+1*/
unsigned short Com_port::calculateChecksumCRC16(char* mass, unsigned long count) {
    unsigned short crc = 0xFFFF;//variable 16 bit = 2 byte
    unsigned char i; //variable 8 bit = 1 byte
    while (count--)// checking the continuation condition
    {
        crc ^= *mass++ << 8;
        for (i = 0; i < 8; i++)//polynomial iteration cycle
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }
    return crc;
}