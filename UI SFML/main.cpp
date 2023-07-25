#include <iostream>
#include <SFML/Graphics.hpp>
#include "Textbox.h"
#include "Button.h"

#include "FTP.h"

void menu(sf::RenderWindow& window) {
	sf::Texture menuTexture1, menuTexture2, menuTexture3, aboutTexture, menuBackground, inputText;
	menuTexture1.loadFromFile("images/send.png");
	menuTexture2.loadFromFile("images/receive.png");
	menuTexture3.loadFromFile("images/exit.png");
	menuBackground.loadFromFile("images/menu.jpg");
	inputText.loadFromFile("images/text.png");
	sf::Sprite menu1(menuTexture1), menu2(menuTexture2), menu3(menuTexture3), about(aboutTexture), menuBg(menuBackground), input1(inputText), input2(inputText), input3(inputText);
	bool isMenu = 1;
	int menuNum = 0;
	menu1.setPosition(100, 150);
	menu2.setPosition(100, 210);
	menu3.setPosition(100, 270);
	menuBg.setPosition(345, 0);
	input1.setPosition(50, 110);
	input2.setPosition(50, 190);
	input3.setPosition(50, 270);

	sf::Font font;
	font.loadFromFile("ArialMT.ttf");

	sf::Text output;
	output.setFont(font);
	output.setCharacterSize(20);
	output.setFillColor(sf::Color::Black);
	output.setStyle(sf::Text::Bold | sf::Text::Underlined);
	//output.setPosition({ 40, 30 });

	sf::Text sendAction;
	sendAction.setFont(font);
	sendAction.setString("Sending a file via com port");
	sendAction.setCharacterSize(20);
	sendAction.setFillColor(sf::Color::Black);
	sendAction.setStyle(sf::Text::Bold | sf::Text::Underlined);
	sendAction.setPosition({ 40, 30 });

	sf::Text receiveAction;
	receiveAction.setFont(font);
	receiveAction.setString("Receiving a file via com port");
	receiveAction.setCharacterSize(20);
	receiveAction.setFillColor(sf::Color::Black);
	receiveAction.setStyle(sf::Text::Bold | sf::Text::Underlined);
	receiveAction.setPosition({ 40, 30 });
	
	sf::Text comPotrName;
	comPotrName.setFont(font);
	comPotrName.setString("Enter the name of the com port:");
	comPotrName.setCharacterSize(16);
	comPotrName.setFillColor(sf::Color::Black);
	comPotrName.setStyle(sf::Text::Bold);
	comPotrName.setPosition({ 50, 80 });
	
	Textbox boxComPotrName(14, sf::Color::Black, false);
	boxComPotrName.setLimit(true, 6);
	boxComPotrName.setFont(font);
	boxComPotrName.setPosition({ 60, 110 });
	
	sf::Text fullPathFolder;
	fullPathFolder.setFont(font);
	fullPathFolder.setString("Enter the full path to the folder where the file is located:");
	fullPathFolder.setCharacterSize(16);
	fullPathFolder.setFillColor(sf::Color::Black);
	fullPathFolder.setStyle(sf::Text::Bold);
	fullPathFolder.setPosition({ 50, 160 });
	
	Textbox boxFullPath(14, sf::Color::Black, false);
	boxFullPath.setLimit(false);
	boxFullPath.setFont(font);
	boxFullPath.setPosition({ 60, 190 });
	
	sf::Text nameFile;
	nameFile.setFont(font);
	nameFile.setString("Enter the file name:");
	nameFile.setCharacterSize(16);
	nameFile.setFillColor(sf::Color::Black);
	nameFile.setStyle(sf::Text::Bold);
	nameFile.setPosition({ 50, 240 });
	
	Textbox boxNameFile(14, sf::Color::Black, false);
	boxNameFile.setLimit(false);
	boxNameFile.setFont(font);
	boxNameFile.setPosition({ 60, 270 });
	
	Button btn("Start", { 100, 50 }, 20, sf::Color(15, 153, 153), sf::Color::Black);
	btn.setFont(font);
	btn.setPosition({ 200, 360 });

	sf::Text errorData;
    errorData.setFont(font);
    errorData.setString("The data was entered incorrectly, please click 'Enter' try again.");
    errorData.setCharacterSize(16);
    errorData.setFillColor(sf::Color::Black);
    errorData.setStyle(sf::Text::Bold);
    errorData.setPosition({ 50, 500 });

	sf::Text exitText;
	exitText.setFont(font);
	exitText.setString("Click 'Escape' to return to the main menu");
	exitText.setCharacterSize(16);
	exitText.setFillColor(sf::Color::Black);
	exitText.setStyle(sf::Text::Bold);
	exitText.setPosition({ 950, 500 });
	
	while (isMenu)
	{
		menu1.setColor(sf::Color(15, 153, 153));
		menu2.setColor(sf::Color(15, 153, 153));
		menu3.setColor(sf::Color(15, 153, 153));
		menuNum = 0;
		window.clear(sf::Color(255, 255, 255));

		if (sf::IntRect(100, 150, 300, 50).contains(sf::Mouse::getPosition(window))) { menu1.setColor(sf::Color::Magenta); menuNum = 1; }
		if (sf::IntRect(100, 210, 300, 50).contains(sf::Mouse::getPosition(window))) { menu2.setColor(sf::Color::Magenta); menuNum = 2; }
		if (sf::IntRect(100, 270, 300, 50).contains(sf::Mouse::getPosition(window))) { menu3.setColor(sf::Color::Magenta); menuNum = 3; }

		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			boxComPotrName.clearAll();
			boxFullPath.clearAll();
			boxNameFile.clearAll();

			if (menuNum == 1) {

				while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {

					input1.setColor(sf::Color(222, 146, 146));
					input2.setColor(sf::Color(222, 146, 146));
					input3.setColor(sf::Color(222, 146, 146));

					if (sf::IntRect(20, 80, 300, 60).contains(sf::Mouse::getPosition(window))) {
						input1.setColor(sf::Color(191, 191, 191));
						input2.setColor(sf::Color(222, 146, 146));
						input3.setColor(sf::Color(222, 146, 146));
						boxComPotrName.setSelected(true); 
						boxFullPath.setSelected(false);
						boxNameFile.setSelected(false);
					} else if (sf::IntRect(20, 161, 300, 60).contains(sf::Mouse::getPosition(window))) {
						input1.setColor(sf::Color(222, 146, 146));
						input2.setColor(sf::Color(191, 191, 191));
						input3.setColor(sf::Color(222, 146, 146));
						boxComPotrName.setSelected(false);
						boxFullPath.setSelected(true);
						boxNameFile.setSelected(false);
					} else if (sf::IntRect(20, 241, 300, 60).contains(sf::Mouse::getPosition(window))) {
						input1.setColor(sf::Color(222, 146, 146));
						input2.setColor(sf::Color(222, 146, 146));
						input3.setColor(sf::Color(191, 191, 191));
						boxComPotrName.setSelected(false);
						boxFullPath.setSelected(false);
						boxNameFile.setSelected(true);
					} else {
						boxComPotrName.setSelected(false);
						boxFullPath.setSelected(false);
						boxNameFile.setSelected(false);
					}

					sf::Event event;
					while (window.pollEvent(event)) {
						switch (event.type) {

						case sf::Event::Closed:
							window.close();
						case sf::Event::TextEntered:
							if (boxComPotrName.getSel())
								boxComPotrName.typedOn(event);
							else if (boxFullPath.getSel())
								boxFullPath.typedOn(event);
							else if (boxNameFile.getSel())
								boxNameFile.typedOn(event);
						case sf::Event::MouseMoved:
							if (btn.isMouseOver(window)) {
								btn.setBackColor(sf::Color(176, 43, 179));
							} else {
								btn.setBackColor(sf::Color(15, 153, 153));
							}
							break;
						case sf::Event::MouseButtonPressed:
							if (btn.isMouseOver(window)) {							
								std::string portName = boxComPotrName.getText();
								std::string fullPath = boxFullPath.getText();
								std::string fileName = boxNameFile.getText();
								if (!portName.empty() && !fullPath.empty() && !fileName.empty()) {
									std::string fullName = fullPath + "/" + fileName;
									std::cout << "yes\n";
									/*
									FTP protocol;
									Observer obs(&protocol, 1, &output);
									protocol.sendFile(portName, fileName);
									while (obs.flag) {
										window.draw(output);
										window.display();
									}
									*/
								} else {
									window.draw(errorData);
									window.display();
									while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Return));

								}
							}
						}
					}
					window.clear(sf::Color(255, 255, 255));

					window.draw(input1);
					window.draw(input2);
					window.draw(input3);
					window.draw(sendAction);
					window.draw(comPotrName);
					boxComPotrName.drawTo(window);
					window.draw(fullPathFolder);
					boxFullPath.drawTo(window);
					window.draw(nameFile);
					boxNameFile.drawTo(window);
					btn.drawTo(window);
					window.draw(exitText);

					window.display();
				}
			}
			if (menuNum == 2) { 

				while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {

					input1.setColor(sf::Color(222, 146, 146));
					input2.setColor(sf::Color(222, 146, 146));
					input3.setColor(sf::Color(222, 146, 146));

					if (sf::IntRect(20, 80, 300, 60).contains(sf::Mouse::getPosition(window))) {
						input1.setColor(sf::Color(191, 191, 191));
						input2.setColor(sf::Color(222, 146, 146));
						input3.setColor(sf::Color(222, 146, 146));
						boxComPotrName.setSelected(true);
						boxFullPath.setSelected(false);
						boxNameFile.setSelected(false);
					} else if (sf::IntRect(20, 161, 300, 60).contains(sf::Mouse::getPosition(window))) {
						input1.setColor(sf::Color(222, 146, 146));
						input2.setColor(sf::Color(191, 191, 191));
						input3.setColor(sf::Color(222, 146, 146));
						boxComPotrName.setSelected(false);
						boxFullPath.setSelected(true);
						boxNameFile.setSelected(false);
					} else if (sf::IntRect(20, 241, 300, 60).contains(sf::Mouse::getPosition(window))) {
						input1.setColor(sf::Color(222, 146, 146));
						input2.setColor(sf::Color(222, 146, 146));
						input3.setColor(sf::Color(191, 191, 191));
						boxComPotrName.setSelected(false);
						boxFullPath.setSelected(false);
						boxNameFile.setSelected(true);
					} else {
						boxComPotrName.setSelected(false);
						boxFullPath.setSelected(false);
						boxNameFile.setSelected(false);
					}

					sf::Event event;
					while (window.pollEvent(event)) {
						switch (event.type) {

						case sf::Event::Closed:
							window.close();
						case sf::Event::TextEntered:
							if (boxComPotrName.getSel())
								boxComPotrName.typedOn(event);
							else if (boxFullPath.getSel())
								boxFullPath.typedOn(event);
							else if (boxNameFile.getSel())
								boxNameFile.typedOn(event);
						case sf::Event::MouseMoved:
							if (btn.isMouseOver(window)) {
								btn.setBackColor(sf::Color(176, 43, 179));
							} else {
								btn.setBackColor(sf::Color(15, 153, 153));
							}
							break;
						case sf::Event::MouseButtonPressed:
							if (btn.isMouseOver(window)) {
								std::string portName = boxComPotrName.getText();
								std::string fullPath = boxFullPath.getText();
								std::string fileName = boxNameFile.getText();
								if (!portName.empty() && !fullPath.empty() && !fileName.empty()) {
									std::cout << "yes\n";
									/*
									FTP protocol;
									Observer obs(&protocol, 1, &output);
									protocol.receiveFile(portName, fullPath, fileName);
									while (obs.flag) {
										window.draw(output);
										window.display();
									}
									*/
								} else {
									window.draw(errorData);
									window.display();
									while (!sf::Keyboard::isKeyPressed(sf::Keyboard::Return));

								}
							}
						}
					}
					window.clear(sf::Color(255, 255, 255));

					window.draw(input1);
					window.draw(input2);
					window.draw(input3);
					window.draw(receiveAction);
					window.draw(comPotrName);
					boxComPotrName.drawTo(window);
					window.draw(fullPathFolder);
					boxFullPath.drawTo(window);
					window.draw(nameFile);
					boxNameFile.drawTo(window);
					btn.drawTo(window);
					window.draw(exitText);

					window.display();
				}
			}
			if (menuNum == 3) { window.close(); isMenu = false; }

		}

		window.draw(menuBg);
		window.draw(menu1);
		window.draw(menu2);
		window.draw(menu3);

		window.display();
	}
}

int main() {
	sf::RenderWindow window(sf::VideoMode(1376, 576), "Com port management");
	menu(window);
	//sf::RenderWindow window;
	//
	//sf::Vector2i centerWindow((sf::VideoMode::getDesktopMode().width / 2) - 445, (sf::VideoMode::getDesktopMode().height / 2) - 480);
	//
	//window.create(sf::VideoMode(900, 900), "Com port management", sf::Style::Titlebar | sf::Style::Close);
	//window.setPosition(centerWindow);
	//
	//window.setKeyRepeatEnabled(true);
}