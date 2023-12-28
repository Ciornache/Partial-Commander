#pragma once

#include "Panel.h"
#include "Button.h"

class App
{
private:
	int timer = 0;
	sf::RectangleShape background, bottomBackground;
	sf::RectangleShape upButton, downButton;
	sf::RenderWindow window;
	std::string name;
	Panel leftPanel{ window }, rightPanel{ window };
	std::vector<sf::Font> fonts = fontsHandler.getFonts();
	std::vector<Button> buttons;
	std::map<std::pair<int, int>, bool> mouseClicked;

	void initWindow(std::string name);
	void initPanels();
	void initButtons();
	void handleKeyboardEvents(sf::Event& event);
	void handleMouseScrollingEvents(sf::Event& event);
	void handleMousePressingEvents(sf::Event & event);
	void handleMouseMovedEvents();
	void handleKeyboardShortcuts(sf::Event event);

	void drawButtons();
	void drawScrollbarButtons();
	void initBackgrounds();

	void getCursor(sf::Cursor & cursor);

public:

	bool isMouseOnScrollbar = false;
	bool shortcutOn = false;

	void init();
	void run();
	App(std::string name) :name(name) {};

};