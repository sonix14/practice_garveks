#include "COM_PORT_INIT.h"
#include <stdio.h>
#include <iostream>
#include "FTP.h"

int main() {
	FTP obj;
	const std::string portName = "COM1";
	const std::string folderPath = "";

	obj.recieveFile(portName, folderPath);
	/*
	Com_port port;
	if (port.openPort("COM1", 9600)) {
		port.writeData("hello.txt");
		//port.readData();
		port.closePort();
	} else {
		std::cout << "Opening error\n";
	}
	*/
}
