#include <iostream>
#include <SFML/Graphics.hpp>
#include "Textbox.h"
#include "Button.h"

int main() {
	sf::RenderWindow window;

	sf::Vector2i centerWindow((sf::VideoMode::getDesktopMode().width / 2) - 445, (sf::VideoMode::getDesktopMode().height / 2) - 480);

	window.create(sf::VideoMode(900, 900), "Com port management ", sf::Style::Titlebar | sf::Style::Close);
	window.setPosition(centerWindow);

	window.setKeyRepeatEnabled(true);

	sf::Font font;
	font.loadFromFile("ArialMT.ttf");

	sf::Text sendAction;
	sendAction.setFont(font);
	sendAction.setString("Sending a file via com port (click Enter)");
	sendAction.setCharacterSize(20);
	sendAction.setFillColor(sf::Color::White);
	sendAction.setStyle(sf::Text::Bold | sf::Text::Underlined);
	sendAction.setPosition({ 40, 30 });

	sf::Text comPotrName1;
	comPotrName1.setFont(font);
	comPotrName1.setString("Enter the name of the com port and click '1':");
	comPotrName1.setCharacterSize(16);
	comPotrName1.setFillColor(sf::Color::White);
	comPotrName1.setStyle(sf::Text::Bold);
	comPotrName1.setPosition({ 50, 80 });

	Textbox sendComPotrName(14, sf::Color::White, false);
	sendComPotrName.setLimit(true, 4);
	sendComPotrName.setFont(font);
	sendComPotrName.setPosition({ 60, 110 });

	sf::Text fullPathFolder1;
	fullPathFolder1.setFont(font);
	fullPathFolder1.setString("Enter the full path to the folder where the file is located and click '2':");
	fullPathFolder1.setCharacterSize(16);
	fullPathFolder1.setFillColor(sf::Color::White);
	fullPathFolder1.setStyle(sf::Text::Bold);
	fullPathFolder1.setPosition({ 50, 160 });

	Textbox sendFullPath(14, sf::Color::White, false);
	sendFullPath.setLimit(false);
	sendFullPath.setFont(font);
	sendFullPath.setPosition({ 60, 190 });

	sf::Text nameFile1;
	nameFile1.setFont(font);
	nameFile1.setString("Enter the file name and click '3':");
	nameFile1.setCharacterSize(16);
	nameFile1.setFillColor(sf::Color::White);
	nameFile1.setStyle(sf::Text::Bold);
	nameFile1.setPosition({ 50, 240 });

	Textbox sendNameFile(14, sf::Color::White, false);
	sendNameFile.setLimit(false);
	sendNameFile.setFont(font);
	sendNameFile.setPosition({ 60, 270 });

	Button btn1("Enter", { 100, 50 }, 20, sf::Color::Green, sf::Color::Black);
	btn1.setFont(font);
	btn1.setPosition({ 650, 150 });

	//////////////////////////////////////////////////////////////////////////////

	sf::Text receiveAction;
	receiveAction.setFont(font);
	receiveAction.setString("Receiving a file via com port (click Space)");
	receiveAction.setCharacterSize(20);
	receiveAction.setFillColor(sf::Color::White);
	receiveAction.setStyle(sf::Text::Bold | sf::Text::Underlined);
	receiveAction.setPosition({ 40, 370 });

	sf::Text comPotrName2;
	comPotrName2.setFont(font);
	comPotrName2.setString("Enter the name of the com port and click '4':");
	comPotrName2.setCharacterSize(16);
	comPotrName2.setFillColor(sf::Color::White);
	comPotrName2.setStyle(sf::Text::Bold);
	comPotrName2.setPosition({ 50, 420 });

	Textbox receiveComPotrName(14, sf::Color::White, false);
	receiveComPotrName.setLimit(true, 4);
	receiveComPotrName.setFont(font);
	receiveComPotrName.setPosition({ 60, 450 });

	sf::Text fullPathFolder2;
	fullPathFolder2.setFont(font);
	fullPathFolder2.setString("Enter the full path to the folder where the file is located and click '5':");
	fullPathFolder2.setCharacterSize(16);
	fullPathFolder2.setFillColor(sf::Color::White);
	fullPathFolder2.setStyle(sf::Text::Bold);
	fullPathFolder2.setPosition({ 50, 500 });

	Textbox receiveFullPath(14, sf::Color::White, false);
	receiveFullPath.setLimit(false);
	receiveFullPath.setFont(font);
	receiveFullPath.setPosition({ 60, 530 });

	sf::Text nameFile2;
	nameFile2.setFont(font);
	nameFile2.setString("Enter the file name and click '6':");
	nameFile2.setCharacterSize(16);
	nameFile2.setFillColor(sf::Color::White);
	nameFile2.setStyle(sf::Text::Bold);
	nameFile2.setPosition({ 50, 580 });

	Textbox receiveNameFile(14, sf::Color::White, false);
	receiveNameFile.setLimit(false);
	receiveNameFile.setFont(font);
	receiveNameFile.setPosition({ 60, 610 });

	Button btn2("Space", { 100, 50 }, 20, sf::Color::Blue, sf::Color::Black);
	btn2.setFont(font);
	btn2.setPosition({ 650, 490 });


	//Main Loop:
	while (window.isOpen()) {

		sf::Event event;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Return)) {
			sendComPotrName.setSelected(true);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) { //change active Keyboard (no number)
			sendComPotrName.setSelected(false);
			sendFullPath.setSelected(true);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
			sendFullPath.setSelected(false);
			sendNameFile.setSelected(true);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num3)) {
			sendNameFile.setSelected(false);
		}

		//////////////////////////////////////////

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
			receiveComPotrName.setSelected(true);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num4)) { //change active Keyboard (no number)
			receiveComPotrName.setSelected(false);
			receiveFullPath.setSelected(true);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num5)) {
			receiveFullPath.setSelected(false);
			receiveNameFile.setSelected(true);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num6)) {
			receiveNameFile.setSelected(false);
		}

		//Event Loop:
		while (window.pollEvent(event)) {
			switch (event.type) {

			case sf::Event::Closed:
				window.close();
			case sf::Event::TextEntered:
				if (sendComPotrName.getSel())
					sendComPotrName.typedOn(event);
				else if (sendFullPath.getSel())
					sendFullPath.typedOn(event);
				else 
					sendNameFile.typedOn(event);

				if (receiveComPotrName.getSel())
					receiveComPotrName.typedOn(event);
				else if (receiveFullPath.getSel())
					receiveFullPath.typedOn(event);
				else
					receiveNameFile.typedOn(event);
			case sf::Event::MouseMoved:
				if (btn1.isMouseOver(window)) {
					btn1.setBackColor(sf::Color::Magenta);
				}
				else {
					btn1.setBackColor(sf::Color::Green);
				}
				if (btn2.isMouseOver(window)) {
					btn2.setBackColor(sf::Color::Green);
				}
				else {
					btn2.setBackColor(sf::Color::Blue);
				}
				break;
			case sf::Event::MouseButtonPressed:
				if (btn1.isMouseOver(window)) {
					std::cout << "Hello " << sendComPotrName.getText() << "\n";
				}
			}
		}
		window.clear();

		window.draw(sendAction);
		window.draw(comPotrName1);
		sendComPotrName.drawTo(window);
		window.draw(fullPathFolder1);
		sendFullPath.drawTo(window);
		window.draw(nameFile1);
		sendNameFile.drawTo(window);
		btn1.drawTo(window);

		window.draw(receiveAction);
		window.draw(comPotrName2);
		receiveComPotrName.drawTo(window);
		window.draw(fullPathFolder2);
		receiveFullPath.drawTo(window);
		window.draw(nameFile2);
		receiveNameFile.drawTo(window);
		btn2.drawTo(window);

		window.display();
	}
}