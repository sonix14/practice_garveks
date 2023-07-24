#ifndef FTP_H
#define FTP_H

#include "COM_PORT_INIT.h"
//#include "Observer.h"

class FTP //file transfer protocol
{
public:
	bool openConnection(const std::string& portName);
	void closeConnection();

	virtual void sendFile(const std::string& file, const std::string& portName);
	virtual bool receiveFile(const std::string& portName, const std::string& folderPath, const std::string& fileName);

	void attach(Observer* obs) {
		views.push_back(obs);
	}
	void setState(std::string str) {
		state = str;
		notify();
	}
	std::string getState() {
		return state;
	}
	void notify() {
		for (int i = 0; i < views.size(); i++)
			views[i]->update();
	}

private:
	Com_port port;
	const int MAX_ERROR = 3;
	std::string state;
	std::vector<class Observer*> views;

	unsigned long calculateChecksumCRC32(unsigned char* mass, unsigned long count);
	unsigned short calculateChecksumCRC16(char* mass, unsigned long count);

	bool getAnswer(char* elem, unsigned long& read);
};


class Observer
{
public:
	FTP* model;
	int denom;
public:
	Observer(FTP* mod, int div) {
		model = mod;
		denom = div;
		// 4. Наблюдатели регистрируются у субъекта
		model->attach(this);
	}
	virtual void update() = 0;
protected:
	FTP* getSubject() {
		return model;
	}
	int getDivisor() {
		return denom;
	}
};
#endif