#ifndef FTP_H
#define FTP_H

#include "COM_PORT_INIT.h"
#include <vector>
//#include <SFML/Graphics.hpp>

const std::string END = "\r";

class IObserver
{
public:
	virtual ~IObserver();
	virtual void update() = 0;
};


class FTP //file transfer protocol
{
public:
	bool openConnection(const std::string& portName);
	void closeConnection();

	virtual void sendFile(const std::string& portName, const std::string& file);
	virtual bool receiveFile(const std::string& portName, const std::string& folderPath, const std::string& fileName);
	
	void attach(IObserver* obs);
	void setState(std::string str);
	std::string getState();
	void notify();
	
private:
	Com_port port;
	const int MAX_ERROR = 3;
	std::string state;
	std::vector<IObserver* > views;

	unsigned long calculateChecksumCRC32(unsigned char* mass, unsigned long count);
	unsigned short calculateChecksumCRC16(char* mass, unsigned long count);

	bool getAnswer(char* elem, unsigned long& read);
};

class Observer : public IObserver
{
public:
	FTP* model;
	//sf::Text* text;
	int denom;
	bool flag = true;

	Observer(FTP* mod, int div) { //sf::Text* txt
		model = mod;
		denom = div;
		//text = txt;
		model->attach(this);
	}
	void update() override {
		//std::string prev = text->getString();
		std::string str = getSubject()->getState();
		//text->setString(prev + str);
		if (str == END)
			flag = false;
	}
protected:
	FTP* getSubject() {
		return model;
	}
	int getDivisor() {
		return denom;
	}
};
#endif