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
    int counter = 0;
    std::vector<char> buffer;
    int fullSize = 0;
    long int fullChecksum = 0;
    long int checksum = 0;
    long int trialChecksum = 0;
    const int READ_TIME = 100;
    OVERLAPPED sync = { 0 };
    int result = 0;
    unsigned long wait = 0, read = 0, state = 0;
    char dst[1024] = { 0 }; //unsigned
    unsigned long size = sizeof(dst);
    // Создаем объект синхронизации 
    sync.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    // Устанавливаем маску на события порта 
    if (SetCommMask(cPort, EV_RXCHAR)) {
        // Связываем порт и объект синхронизации 
        WaitCommEvent(cPort, &state, &sync);
        // Начинаем ожидание данных 
        wait = WaitForSingleObject(sync.hEvent, READ_TIME);
        // Данные получены  
        if (wait == WAIT_OBJECT_0) {
            ReadFile(cPort, dst, size, &read, &sync); //size of file
            fullSize = std::atoi(dst);
            //fullSize = std::stoi(std::string(buffer));
            if (fullSize <= 0) {
                std::cout << "Uncorrect size\n";
            }
            memset(dst, 0, 1024);
            wait = 0;
            sync.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            wait = WaitForSingleObject(sync.hEvent, READ_TIME);
            if (wait == WAIT_OBJECT_0) {
                ReadFile(cPort, dst, size, &read, &sync); //full file check sum
                fullChecksum = std::atoi(dst);
                if (fullChecksum <= 0) {
                    std::cout << "Uncorrect checksum\n";
                }
                memset(dst, 0, 1024);
                //дать ответ о готовности принять данные
                while (trialChecksum != fullChecksum) {
                    if (counter == MAX_ERROR) {
                        std::cout << "Too much tries for one fragment!\n";
                        //close
                        break;
                    }
                    sync.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                    wait = WaitForSingleObject(sync.hEvent, READ_TIME);
                    if (wait == WAIT_OBJECT_0) {
                        ReadFile(cPort, dst, size, &read, &sync); //fragment    
                        sync.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
                        wait = WaitForSingleObject(sync.hEvent, READ_TIME);
                        if (wait == WAIT_OBJECT_0) {
                            ReadFile(cPort, dst, size, &read, &sync); // checksum
                            checksum = std::atoi(dst);
                            //memset(dst, 0, 1024);
                        }
                        if (calculateChecksum(dst, size) != checksum) {
                            //uncorrect fragment
                            std::cout << "The fragment was not accepted successfully! Try again\n";
                            counter++;
                            continue;
                        }
                        else {
                            std::cout << "The fragment was accepted successfully\n";
                            for (int i : dst) {
                                buffer.push_back(dst[i]);
                            }
                            counter = 0;
                        }
                    }
                    memset(dst, 0, 1024);
                    int buffSize = buffer.size();
                    char* array = new char[buffSize];
                    for (int i = 0; i < buffSize; i++) {
                        array[i] = buffer[i];
                    }
                    trialChecksum = calculateChecksum(array, buffSize);
                    delete[] array;
                    if (trialChecksum > fullChecksum) {
                        //close
                        std::cout << "Too much fragments\n";
                        break;
                    }
                    /*
                    if (wait == WAIT_OBJECT_0)
                        if (GetOverlappedResult(cPort, &sync, &read, FALSE)) {
                            result = read;
                            std::cout << "Size of fragment: " << result << '\n';
                        }
                        */
                }
                if (trialChecksum == fullChecksum) {
                    //write file into the folder
                    std::cout << "The file was accepted successfully!\n";
                    //close
                }
            }
        }
    }
    //std::cout << "Received bytes: " << result;
    //std::cout << "\nFile: " << dst;
    CloseHandle(sync.hEvent);
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

void Com_port::calculateChecksum(const std::string& file, const unsigned& size) {
    std::ifstream in(file);
    unsigned s = 0;
    in.seekg(0, std::ios::end);
    s = in.tellg();
    std::cout << "Rope Weight : " << s << " byte" << "\n";
    in.close();
    /*
    char* mass = new char[size];
    std::ifstream fs(file, std::ios::in | std::ios::binary);
    if (!fs) {

    } else {
        for (int r = 0; r < size-1; r++)
        {
            fs.getline(mass[r], len - 1, ch); //Считываем строки в массив
            cout << "String " << r + 1 << " = " << mass[r] << endl; //Выводи строку из массива
        }
        fs.close();
    }
    */
}