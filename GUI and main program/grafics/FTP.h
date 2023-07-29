#ifndef FTP_H
#define FTP_H
#define NOMINMAX

#include "ComPort.h"
#include <vector>
#include <SFML/Graphics.hpp>

class IObserver
{
public:
	virtual void update() = 0;
};


class FTP //file transfer protocol
{
public:
	bool openConnection(const std::string& portName);
	void closeConnection();

	virtual void sendFile(const std::string& portName, const std::string& file);
	virtual bool receiveFile(const std::string& portName, const std::string& folderPath, const std::string& fileName);

	/*methods for implementing the observer pattern*/
	void attach(IObserver* obs);
	void setState(std::string str);
	std::string getState();
	void notify();
	/*---------------------------------------------*/

private:
	ComPort port;
	const int MAX_ERROR = 3;
	std::string state;
	IObserver* viewer;

	unsigned long calculateChecksumCRC32(unsigned char* mass, unsigned long count);
	unsigned short calculateChecksumCRC16(char* mass, unsigned long count);

	bool getAnswer(char* elem, unsigned long& read);
};

class Observer : public IObserver
{
public:
	FTP* model;
	sf::Text* text;
	sf::RenderWindow* window;

	Observer(FTP* mod, sf::Text* txt, sf::RenderWindow* wind) {
		model = mod;
		text = txt;
		window = wind;
		model->attach(this);
	}

	void update() override {
		std::string prev = text->getString();
		std::string str = getSubject()->getState();
		text->setString(prev + str);
		window->draw(*text);
		window->display();
	}

protected:
	FTP* getSubject() {
		return model;
	}
};
#endif