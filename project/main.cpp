#include "COM_PORT_INIT.h"
#include <stdio.h>
#include <iostream>

int main() {
	Com_port port;
	if (port.openPort("COM1", 9600)) {
		std::cout << "yes\n";
		port.closePort();
	} else {
		std::cout << "no\n";
	}
}
