#include "ComPort.h"
#include <stdio.h>
#include <iostream>
#include "FTP.h"

int main() {
	FTP obj;
	const std::string portName = "COM1";
	const std::string folderPath = "";
	const std::string fileName = "";

	obj.receiveFile(portName, folderPath, fileName);
	//obj.sendFile(portName, fileName);
}
