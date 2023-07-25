#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include "FTP.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include <experimental/filesystem>

char APPROVAL[1] = { 'y' };
char REJECTION[1] = { 'n' };
namespace fs = std::experimental::filesystem;

void FTP::attach(IObserver* obs) {
    views.push_back(obs);
}

void FTP::setState(std::string str) {
    state = str;
    notify();
}

std::string FTP::getState() {
    return state;
}

void FTP::notify() {
    for (int i = 0; i < views.size(); i++)
        views[i]->update();
}

bool FTP::openConnection(const std::string& portName) {
    int baudrate = 9600;
    if (port.openPort(portName, &baudrate)) {
        setState("Port (" + portName + ") was opened\n");
        return true;
    }
    else {
        setState("Port (" + portName + ") was not opened\n");
        return false;
    }
}

void FTP::closeConnection() {
    if (port.closePort())
        setState("The connection is closed.\n");
    else
        setState("The connection couldn't be closed.\n");
}

void FTP::sendFile(const std::string& portName, const std::string& file) {

    if (!openConnection(portName)) {
        setState("Couldn't send file\n");
        return;
    }

    DWORD dwSize;
    char* elem = new char[1];
    unsigned long read = 0;
    std::ifstream in;
    in.open(file, std::ifstream::ate | std::ifstream::binary);
    unsigned long file_size = in.tellg();
    setState("Rope weight : " + std::to_string(file_size) + " byte\n");
    std::string s_file_size = std::to_string(file_size);
    char* ch_file_size = new char[s_file_size.length()];
    ch_file_size = const_cast<char*>(s_file_size.c_str());
    dwSize = s_file_size.length();
    port.writeData(ch_file_size, dwSize);

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
        setState("CRC32 = " + std::to_string(check_sum) + "\n");
        std::string s_check_sum = std::to_string(check_sum);
        char* ch_check_sum = new char[s_check_sum.length()];
        ch_check_sum = const_cast<char*>(s_check_sum.c_str());
        dwSize = s_check_sum.length();
        port.writeData(ch_check_sum, dwSize);
    }
    else {
        setState("File not found\n");
        closeConnection();
        delete[] mass_all_file;
        return;
    }
    delete[] mass_all_file;

    setState("Requesting permission to transfer data\n");
    if (!getAnswer(elem, read)) {
        setState("Permission was not received\n");
        closeConnection();
        return;
    }

    setState("Start sending data\n");
    unsigned long buff = file_size;
    FILE* fp;
    fopen_s(&fp, file_name, "r");
    const int fragment_size = 10;
    char mass_fragment[fragment_size];
    int count_error = 0;
    bool correct_fragment;
    if (!fp) {
        setState("File was not found\n");
        closeConnection();
        return;
    }
    while (buff > fragment_size && count_error != MAX_ERROR) {
        size_t count = fread(mass_fragment, sizeof mass_fragment[0], fragment_size, fp);
        buff -= fragment_size;

        correct_fragment = false;
        count_error = 0;
        while (!correct_fragment && count_error != MAX_ERROR) {
            count_error++;
            dwSize = sizeof(mass_fragment);
            port.writeData(mass_fragment, dwSize);
            unsigned short check_sum = calculateChecksumCRC16(mass_fragment, fragment_size);
            setState("CRC16 = " + std::to_string(check_sum) + "   ");
            std::string s_check_sum = std::to_string(check_sum);
            char* ch_check_sum = new char[s_check_sum.length()];
            ch_check_sum = const_cast<char*>(s_check_sum.c_str());
            dwSize = s_check_sum.length();
            port.writeData(ch_check_sum, dwSize);
            if (getAnswer(elem, read)) {
                correct_fragment = true;
                setState("The fragment was sent and received correctly\n");
            }
            else {
                correct_fragment = false;
                setState("The fragment was sent and received incorrectly, I try again\n");
            }
        }
    }
    if (count_error == MAX_ERROR) {
        setState("Error! The file was sent and received incorrectly\n");
        closeConnection();
        fclose(fp);
        return;
    }
    unsigned long endPart_fragment_size = buff;
    char* endPart_mass_fragment = new char[endPart_fragment_size];
    if (buff > 0) {
        size_t count = fread(endPart_mass_fragment, sizeof endPart_mass_fragment[0], buff, fp);
    }
    correct_fragment = false;
    count_error = 0;
    while (!correct_fragment && count_error != MAX_ERROR) {
        count_error++;
        dwSize = sizeof(endPart_mass_fragment);
        port.writeData(endPart_mass_fragment, dwSize);
        unsigned short check_sum = calculateChecksumCRC16(endPart_mass_fragment, endPart_fragment_size);
        setState("CRC16 = " + std::to_string(check_sum) + "   ");
        std::string s_check_sum = std::to_string(check_sum);
        char* ch_check_sum = new char[s_check_sum.length()];
        ch_check_sum = const_cast<char*>(s_check_sum.c_str());
        dwSize = s_check_sum.length();
        port.writeData(ch_check_sum, dwSize);
        if (getAnswer(elem, read)) {
            correct_fragment = true;
            setState("The fragment was sent and received correctly\n");
        }
        else {
            correct_fragment = false;
            setState("The fragment was sent and received incorrectly, try again\n");
        }
    }
    if (count_error != MAX_ERROR && getAnswer(elem, read)) {
        setState("The file was successfully sent and accepted by the second part\n");
    }
    else {
        setState("Error! The file was sent and received incorrectly\n");
    }
    delete[] endPart_mass_fragment;
    fclose(fp);
    closeConnection();
}

bool FTP::receiveFile(const std::string& portName, const std::string& folderPath, const std::string& fileName) {

    if (!openConnection(portName)) {
        setState("Couldn't receive a file\n");
        return false;;
    }

    char* dst = new char[1024];
    unsigned long read = 0, size = 0;
    std::size_t fullSize = 0;
    int counter = 0;
    unsigned long fullChecksum = 0, trialChecksum = 0, checksum = 0;
    std::vector<char> buffer;
    bool successFlag = true;
    DWORD dwSize = sizeof(APPROVAL);

    setState("Waiting for the size...\n");
    port.readData(dst, read);
    fullSize = std::atoi(dst);
    if (fullSize <= 0) {
        setState("Uncorrect size\n");
        delete[] dst;
        return false;;
    }
    else
        setState("Accepted size: " + std::to_string(fullSize) + "\n");

    setState("Waiting for the full checksum...\n");
    port.readData(dst, read);
    fullChecksum = std::atoll(dst);
    if (fullChecksum <= 0) {
        setState("Uncorrect checksum\n");
        delete[] dst;
        return false;;
    }
    else
        setState("Accepted checksum: " + std::to_string(fullChecksum) + "\n");


    port.writeData(APPROVAL, dwSize);
    setState("Ready to accept data.\n");

    char* dstSum = new char[1024];
    while (true) {
        if (counter == MAX_ERROR) {
            setState("Too much tries for one fragment!\n");
            closeConnection();
            successFlag = false;
            break;
        }
        port.readData(dst, read); //fragment
        size = read - 1;
        trialChecksum = calculateChecksumCRC16(dst, size);
        setState("Received fragment\n");

        port.readData(dstSum, read); // checksum
        checksum = std::atoll(dstSum);
        std::cout << "Accepted checksum\n";
        if (trialChecksum != checksum) {
            setState("The fragment was not accepted successfully! Try again\n");
            port.writeData(REJECTION, dwSize);
            counter++;
            continue;
        }
        else {
            setState("The fragment was accepted successfully\n");
            port.writeData(APPROVAL, dwSize);
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
        if (fullChecksum == trialChecksum) {
            setState("The file was accepted successfully!\n");
            port.writeData(APPROVAL, dwSize);
            successFlag = true;
            closeConnection();
            break;
        }
    }

    delete[] dst;
    delete[] dstSum;
    if (successFlag) {
        //write file into the folder
        std::error_code e;
        fs::path dir = folderPath;
        bool flag = fs::create_directories(dir, e);
        fs::current_path(dir);
        FILE* fp;
        if ((fopen_s(&fp, fileName.c_str(), "w")) == 0) {  //rb+
            for (int i = 0; i < buffer.size(); i++) {
                fputc(buffer[i], fp);
                if (feof(fp)) {
                    setState("Couldn't write to the new file\n");
                    break;
                }
            }
            if (!feof(fp))
                setState("File was written to the folder\n");
            fclose(fp);
        }
        else {
            setState("Couldn't write to the new file\n");
        }
        return true;
    }
    else {
        setState("The file was not accepted!\n");
        return false;
    }
}

/*We describe the Crc32 calculation function
 using a polynomial EDB88320UL=
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

bool FTP::getAnswer(char* elem, unsigned long& read) {
    port.readData(elem, read);
    if (elem[0] == APPROVAL[0])
        return true;
    else if (elem[0] == REJECTION[0])
        return false;
}