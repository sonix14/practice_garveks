#include "FTP.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

#include <experimental/filesystem>  //!!!!!!!!!

const char APPROVAL = 'y';
const char REJECTION = 'n';
namespace fs = std::experimental::filesystem;  //!!!!!!!!!!1

bool FTP::openConnection(const std::string& portName) {
    if (port.openPort(portName, 9600)) {
        std::cout << "Port (" << portName << ") was opened\n";
        return true;
    }
    else {
        std::cout << "Port (" << portName << ") was not opened\n";
        return false;
    }
}

void FTP::closeConnection() {
    port.closePort();
    std::cout << "The connection is closed.\n";
}

void FTP::sendFile(const std::string& file) {

    DWORD dwBytesWritten;
    std::ifstream in(file, std::ifstream::ate | std::ifstream::binary);
    unsigned long file_size = in.tellg();
    std::cout << "Rope Weight : " << file_size << " byte" << "\n";
    DWORD dwSize = sizeof(file_size);
    BOOL iRet = WriteFile(port.cPort, &file_size, dwSize, &dwBytesWritten, NULL);  //
    if (!iRet || dwBytesWritten != dwSize) {
        CloseHandle(port.cPort); //
        port.cPort = INVALID_HANDLE_VALUE;  //
        std::cout << "Error writing file size to port\n";
    }
    //const char file_name[10] = "hello.txt";//!!!!!!!!!!!!
    char* file_name = new char[file.length()];
    file_name = const_cast<char*>(file.c_str());

    FILE* read_all_file;
    fopen_s(&read_all_file, file_name, "r");
    unsigned long all_file_size = file_size;
    unsigned char* mass_all_file = new unsigned char[all_file_size];
    if (read_all_file) {
        unsigned long count = fread(mass_all_file, sizeof mass_all_file[0], file_size, read_all_file);
        fclose(read_all_file);

        unsigned long check_sum = calculateChecksumCRC32(mass_all_file, count);
        std::cout << "CRC32 = " << check_sum << "\n";
        dwSize = sizeof(check_sum);
        iRet = WriteFile(port.cPort, &check_sum, dwSize, &dwBytesWritten, NULL);  //
        if (!iRet || dwBytesWritten != dwSize) {
            CloseHandle(port.cPort);  //
            port.cPort = INVALID_HANDLE_VALUE;
            std::cout << "Error writing check sum to port\n";
        }
    }
    else {
        std::cout << "File not found\n";
    }
    delete[] mass_all_file;

    std::cout << "Requesting permission to transfer data\n";
    if (getAnswer()) {
        std::cout << "Start sent data\n";
        unsigned long buff = file_size;
        FILE* fp;
        fopen_s(&fp, file_name, "r");
        const int fragment_size = 10;
        char mass_fragment[fragment_size];
        int count_error = 0;
        bool correct_fragment;
        if (fp) {
            while (buff > fragment_size && count_error != MAX_ERROR) {
                size_t count = fread(mass_fragment, sizeof mass_fragment[0], fragment_size, fp);
                buff -= fragment_size;

                correct_fragment = false;
                count_error = 0;
                while (!correct_fragment && count_error != MAX_ERROR) {
                    count_error++;
                    dwSize = sizeof(mass_fragment);
                    iRet = WriteFile(port.cPort, &mass_fragment, dwSize, &dwBytesWritten, NULL);        //
                    if (!iRet || dwBytesWritten != dwSize) {
                        CloseHandle(port.cPort);       //
                        port.cPort = INVALID_HANDLE_VALUE;    //
                        std::cout << "Error writing to port\n";
                    }
                    unsigned short check_sum = calculateChecksumCRC16(mass_fragment, fragment_size);
                    std::cout << "CRC16 = " << check_sum << "   ";
                    dwSize = sizeof(check_sum);
                    iRet = WriteFile(port.cPort, &check_sum, dwSize, &dwBytesWritten, NULL);        //
                    if (!iRet || dwBytesWritten != dwSize) {
                        CloseHandle(port.cPort);         //
                        port.cPort = INVALID_HANDLE_VALUE;      //
                        std::cout << "Error writing check sum fragment to port\n";
                    }
                    if (getAnswer()) {
                        correct_fragment = true;
                        std::cout << "The fragment was sent and received correctly\n";
                    }
                    else {
                        correct_fragment = false;
                        std::cout << "The fragment was sent and received incorrectly, I try again\n";
                    }
                }
            }
            if (count_error != MAX_ERROR) {
                unsigned long endPart_fragment_size = buff;
                char* endPart_mass_fragment = new char[endPart_fragment_size];
                if (buff > 0) {
                    size_t count = fread(endPart_mass_fragment, sizeof endPart_mass_fragment[0], buff, fp);
                    //printf("read %zu elements out of %d\n", count, buff);
                }
                correct_fragment = false;
                count_error = 0;
                while (!correct_fragment && count_error != MAX_ERROR) {
                    count_error++;
                    dwSize = sizeof(endPart_mass_fragment);
                    iRet = WriteFile(port.cPort, &endPart_mass_fragment, dwSize, &dwBytesWritten, NULL);     //
                    if (!iRet || dwBytesWritten != dwSize) {
                        CloseHandle(port.cPort);     //
                        port.cPort = INVALID_HANDLE_VALUE;   //
                        std::cout << "Error writing to port\n";
                    }
                    unsigned short check_sum = calculateChecksumCRC16(endPart_mass_fragment, endPart_fragment_size);
                    std::cout << "CRC16 = " << check_sum << "   ";
                    dwSize = sizeof(check_sum);
                    iRet = WriteFile(port.cPort, &check_sum, dwSize, &dwBytesWritten, NULL);  //
                    if (!iRet || dwBytesWritten != dwSize) {
                        CloseHandle(port.cPort);   //
                        port.cPort = INVALID_HANDLE_VALUE;       //
                        std::cout << "Error writing check sum fragment to port\n";
                    }
                    if (getAnswer()) {
                        correct_fragment = true;
                        std::cout << "The fragment was sent and received correctly\n";
                    }
                    else {
                        correct_fragment = false;
                        std::cout << "The fragment was sent and received incorrectly, I try again\n";
                    }
                }
                if (count_error != MAX_ERROR && getAnswer()) {
                    std::cout << "The file was successfully accepted sent and accepted by the second party\n";
                }
                else {
                    std::cout << "Error! The file was sent and received incorrectly\n";
                }
                delete[] endPart_mass_fragment;
            }
            else {
                std::cout << "Error! The file was sent and received incorrectly\n";
            }
            fclose(fp);
        }
        else {
            std::cout << "File not found\n";
        }
    }
    else {
        std::cout << "Permission was not received\n";
    }
}

bool FTP::receiveFile(const std::string& portName, const std::string& folderPath, const std::string& fileName) {
    char* dst = new char(1024);
    char* dstSum = new char(1024);
    unsigned long read = 0, size = 0;
    int fullSize = 0;
    unsigned long trialChecksum = 0, checksum = 0;
    std::string fullChecksum;
    std::vector<char> buffer;
    int counter = 0;
    bool successFlag = true;

    if (!openConnection(portName)) {
        std::cout << "Couldn't receive a file\n";
        return false;;
    }

    port.readData(dst, read);
    fullSize = std::atoi(dst);
    if (fullSize <= 0) {
        std::cout << "Uncorrect size\n";
        return false;;
    }
    else
        std::cout << "Accepted size\n";
    memset(dst, 0, read);

    port.readData(dst, read);
    fullChecksum = dst;
    size_t end = fullChecksum.find_first_of('\r');
    fullChecksum = fullChecksum.substr(0, end);
    if (fullChecksum.size() == 0) {
        std::cout << "Uncorrect checksum\n";
        return false;;
    }
    else
        std::cout << "Accepted checksum\n";
    memset(dst, 0, read);

    port.writeData(APPROVAL); //give an answer about readiness to accept data
    std::cout << "Ready to accept data\n";

    while (true) {
        if (counter == MAX_ERROR) {
            std::cout << "Too much tries for one fragment!\n";
            closeConnection();
            successFlag = false;
            counter = 0;
            break;
        }
        port.readData(dst, read); //fragment
        size = read - 1;
        trialChecksum = calculateChecksumCRC16(dst, size);
        std::cout << "Accepted fragment\n";

        port.readData(dstSum, read); // checksum
        checksum = std::atoi(dstSum);
        std::cout << "Accepted checksum\n";
        memset(dstSum, 0, read);
        if (trialChecksum != checksum) {
            //uncorrect fragment
            std::cout << "The fragment was not accepted successfully! Try again\n";
            port.writeData(REJECTION); //give an answer
            counter++;
            continue;
        }
        else {
            std::cout << "The fragment was accepted successfully\n";
            port.writeData(APPROVAL);  //give an answer      
            for (int i = 0; i < size; i++) {
                buffer.push_back(dst[i]);
            }
            int buffSize = buffer.size();
            unsigned char* array = new unsigned char[buffSize];
            for (int i = 0; i < buffSize; i++) {
                array[i] = buffer[i];
            }
            trialChecksum = calculateChecksumCRC32(array, buffSize);
            delete[] array;
            counter = 0;
        }
        memset(dst, 0, read);
        std::string temp = std::to_string(trialChecksum);
        /*
        if (trialChecksum > fullChecksum) { 
            std::cout << "Too much fragments\n";
            port.writeData(REJECTION);  //give an answer
            successFlag = false;
            closeConnection();
            break;
        } else
        */
        if (fullChecksum.compare(temp) == 0) {
            std::cout << "The file was accepted successfully!\n";
            port.writeData(APPROVAL);  //give an answer about succsessful acceptance
            successFlag = true;
            closeConnection();
            break;
        }
    }

    if (successFlag) {
        //write file into the folder
        std::error_code e;
        fs::path dir = folderPath;
        bool flag = fs::create_directories(dir, e);
        fs::current_path(dir);
        FILE* fp;
        if ((fp = fopen(fileName.c_str(), "w")) != NULL) {  //rb+
            for (int i = 0; i < buffer.size(); i++) {
                fputc(buffer[i], fp);
                if (feof(fp))
                    break;
            }
            if (feof(fp))
                std::cout << "Couldn't write to the new file\n";
            else
                std::cout << "File was written to the folder\n";
            fclose(fp);
        }
        else {
            std::cout << "Couldn't write to the new file\n";
        }
        return true;
    }
    return false;
}

/*We describe the Crc32 calculation function
 *using a polynomial EDB88320UL=
 x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11
 + x^10 + x^8 + x^7 + x^5 + x^4 + x^2 + x + 1*/
unsigned long FTP::calculateChecksumCRC32(unsigned char* mass, unsigned long count) {
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
    return crc ^ 0xFFFFFFFFUL;
}

/*We describe the Crc16 standard CCITT calculation function
using the polynomial 1021=x^16+x^12 +x^5+1*/
unsigned short FTP::calculateChecksumCRC16(char* mass, unsigned long count) {
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

bool FTP::getAnswer() {
    const int READ_TIME = 100;
    OVERLAPPED sync = { 0 };
    int reuslt = 0;
    unsigned long wait = 0, read = 0, state = 0;
    char dst;
    unsigned long size = sizeof(dst);

    sync.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (sync.hEvent) {
        if (SetCommMask(port.cPort, EV_RXCHAR)) { //
            WaitCommEvent(port.cPort, &state, &sync); //
            wait = WaitForSingleObject(sync.hEvent, READ_TIME);
            if (wait == WAIT_OBJECT_0) {
                if (ReadFile(port.cPort, &dst, size, &read, &sync)) { //
                    wait = WaitForSingleObject(sync.hEvent, READ_TIME);
                    if (wait == WAIT_OBJECT_0) {
                        if (dst == 'y') {
                            CloseHandle(sync.hEvent);
                            return true;
                        }
                        else if (dst == 'n') {
                            CloseHandle(sync.hEvent);
                            return false;
                        }
                        else {
                            if (getAnswer()) {
                                return true;
                            }
                            else {
                                return false;
                            }
                        }
                    }
                }
            }
        }
        CloseHandle(sync.hEvent);
        return false;
    }
    else {
        return false;
    }
}