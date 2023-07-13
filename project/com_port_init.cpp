#include "COM_PORT_INIT.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>

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

void Com_port::readData() {
   //DWORD iSize; //адресс переменной где хронится информация об количестве прочитанных байт (в случае успешного приема)
   //char sReceivedChar[2]; //адресс переменной куда нужно записать прочитанные данные
   //char recBuf[100];
   //std::string Symb;
   //sReceivedChar[1] = 0;
   //do {
   //    //ReadFile(cPort, &sReceivedChar, 1, &iSize, 0); 	// получаем 1 байт
   //    // если что-то принято, выводим
   //    if (iSize > 0) {
   //        strcat(recBuf, sReceivedChar);
   //    }
   //} while (iSize > 0);
    //DWORD iSize;
    //char sReceivedChar;
    //while (true)
    //{
    //    ReadFile(cPort, &sReceivedChar, 1, &iSize, 0);  // получаем 1 байт
    //    if (iSize > 0)   // если что-то принято, выводим
    //        std::cout << sReceivedChar;
    //}
}

void Com_port::writeData() {
    if (cPort == INVALID_HANDLE_VALUE) {
        //
    }
    
    std::string line;
    DWORD dwBytesWritten;
    std::ifstream in("hello.txt");
    calculateChecksum("hello.txt", 0);
    if (in.is_open())
    {
        while (!in.eof())
        {
            std::getline(in, line);
            const int size = line.length();
            char* char_array = new char[size + 1];
            char_array[size] = '\0';
            for (int i = 0; i < size; i++) {
                char_array[i] = line[i];
                std::cout << char_array[i];
            }
            //std::cout << char_array << "\n";
            ////strcpy(char_array, line.c_str());
            //
            ////const char* char_array = line.c_str();
            //
            ////in.getline(data, 50);
            DWORD dwSize = size + 1;// sizeof(char_array);   // размер этой строки
            BOOL iRet = WriteFile(cPort, &char_array, dwSize, &dwBytesWritten, NULL);
            if (dwBytesWritten != dwSize) {
                CloseHandle(cPort);
                cPort = INVALID_HANDLE_VALUE;
                std::cout << "Ошибка при записи в порт\n";
            }
            std::cout << dwSize << " Bytes in string. " << dwBytesWritten << " Bytes sended.\n";
            delete[] char_array;
        }
    } else {
        std::cout << "Файл не найден\n";
    }
    in.close();

    /*
    DWORD dwBytesWritten; 	// количество переданных байт
    char data[] = "Hello from C++";  // строка для передачи
    DWORD dwSize = sizeof(data);   // размер этой строки
    BOOL iRet = WriteFile(cPort, data, dwSize, &dwBytesWritten, NULL);
    if (dwBytesWritten != dwSize) {
        CloseHandle(cPort);
        cPort = INVALID_HANDLE_VALUE;
        std::cout << "Ошибка при записи в порт\n";
    }
    std::cout << dwSize << " Bytes in string. " << dwBytesWritten << " Bytes sended.\n";
    */
}

unsigned Com_port::calculateChecksum(char* mass, unsigned long count) {
    //int i;
    //unsigned crc = 0xFFFF;
    //
    //while (count--)
    //{
    //    crc ^= *mass++;
    //    for (i = 0; i < 8; ++i)
    //        if (crc & 0x01) { crc >>= 1; crc ^= 0xA001; }
    //        else crc >>= 1;
    //}
    //return crc;


    //инициализируем таблицу расчёта Crc32
    unsigned long crc_table[256];//массив 32 бита = 4 байтам
    unsigned long crc;//переменная 32 бита = 4 байтам
    for (int i = 0; i < 256; i++)//инициализируем цикл массива
    {
        crc = i;
        for (int j = 0; j < 8; j++)//цикл перебора полинома
            crc = crc & 1 ? (crc >> 1) ^ 0xEDB88320UL : crc >> 1;
        crc_table[i] = crc;
    };
    crc = 0xFFFFFFFFUL;
    while (count--)// проверка условия продолжения
        crc = crc_table[(crc ^ *mass++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFUL; //конец функции расчёта Crc32
}