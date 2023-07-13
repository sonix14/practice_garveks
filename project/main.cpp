#include "COM_PORT_INIT.h"
#include <stdio.h>
#include <iostream>
//#pragma comment(lib, "libname.lib")

int main() {
	Com_port port;
	if (port.openPort("COM1", 9600)) {
		port.writeData("hello.txt");
		//port.readData();
		port.closePort();
	} else {
		std::cout << "Opening error\n";
	}
}
