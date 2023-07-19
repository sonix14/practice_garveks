#include "COM_PORT_INIT.h"
#include <stdio.h>
#include <iostream>
#include "FTP.h"

int main() {
	FTP obj;
	const std::string portName = "COM1";
	const std::string folderPath = "C:/folder";

	obj.receiveFile(portName, folderPath, "test.txt");
}
