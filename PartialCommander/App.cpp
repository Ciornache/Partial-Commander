﻿#include "App.h"

void App::initWindow(std::string name) {

	window.create(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), name);

	/// Calculating topLeft corner so that window is in the middle of the screen
	sf::VideoMode mode = sf::VideoMode::getDesktopMode();
	sf::Vector2i topLeft(mode.width / 10, mode.height / 10);

	window.setPosition(topLeft);
}

void App::init() {
	initWindow(name);
	initBackground(background, backgroundColor, sf::Vector2f(0, 0), sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
	initBackground(bottomBackground, bottomBackgroundColor, sf::Vector2f(0, WINDOW_HEIGHT - BOTTOM_BUTTONS_HEIGHT + 22),
		sf::Vector2f(WINDOW_WIDTH, BOTTOM_BUTTONS_HEIGHT));
	initPanels();
	initButtons();
	initScrollbarButtons();
	drawButtons();
}

void App::initPanels() {
	sf::Vector2f left{ PANEL_MARGIN_X,PANEL_MARGIN_TOP + TOP_BUTTONS_HEIGHT };
	sf::Vector2f right{ 2 * PANEL_MARGIN_X + PANEL_WIDTH - 3 * PANEL_LINE_WIDTH + left.x,left.y };

	leftPanel.init(left, PANEL_WIDTH, PANEL_HEIGHT, leftPanelDirectoryPath, fonts);
	rightPanel.init(right, PANEL_WIDTH, PANEL_HEIGHT, rightPanelDirectoryPath, fonts);

	leftPanel.isSelected = true;
	rightPanel.getFolders()[0].toggleIsSelected();
}

void App::initButtons() {

	sf::Vector2f topLeft(PANEL_MARGIN_X * 2, WINDOW_HEIGHT - PANEL_BOTTOM_HEIGHT / 1.25);
	int moveX = (WINDOW_WIDTH - PANEL_MARGIN_X * (buttonNames.size() + 2)) / buttonNames.size();

	for (unsigned int index = 0; index < buttonNames.size(); ++index) {
		Button button(buttonNames[index], BUTTON_HEIGHT, moveX, index + 1, topLeft, secondaryColor, window, fonts);
		buttons.push_back(button);
		topLeft += sf::Vector2f(moveX * 1.f + PANEL_MARGIN_X, 0);
	}
}

void App::initScrollbarButtons()
{
	auto initScrollBarButton = [](sf::RectangleShape& button, sf::Vector2f position)
		{
			button.setFillColor(scrollbarButtonColor);
			button.setSize(sf::Vector2f(SCROLLBAR_BUTTON_WIDTH, SCROLLBAR_BUTTON_HEIGHT));
			button.setPosition(position);
			button.setOutlineThickness(2);
		};
	auto initScrollBarButtonText = [&](sf::Text& buttonText, unsigned int characterSize, std::string text, sf::Vector2f position, float angle)
		{
			buttonText.setFillColor(scrollbarTextButtonColor);
			buttonText.setFont(fonts[CustomFonts::Font::UBUNTU]);
			buttonText.setString(text);
			buttonText.setCharacterSize(characterSize);

			sf::FloatRect rc = buttonText.getLocalBounds();
			buttonText.setOrigin(rc.width / 2, rc.height / 2);
			buttonText.setPosition(position);
			buttonText.setRotation(angle);
		};

	initScrollBarButton(upButton, sf::Vector2f(SCROLLBAR_X, SCROLLBAR_Y + 2));
	initScrollBarButton(downButton, sf::Vector2f(PANEL_WIDTH + PANEL_MARGIN_X + 2, PANEL_HEIGHT + TOP_BUTTONS_HEIGHT));

	initScrollBarButtonText(buttonText, 13, "V", sf::Vector2f(SCROLLBAR_X + SCROLLBAR_WIDTH / 2, SCROLLBAR_Y + SCROLLBAR_BUTTON_HEIGHT / 1.25f), 180.f);
	initScrollBarButtonText(buttonText2, 13, "V", sf::Vector2f(downButton.getPosition().x + SCROLLBAR_WIDTH / 2, downButton.getPosition().y + SCROLLBAR_BUTTON_HEIGHT / 3), 0);
}

void App::drawButtons() {
	for (unsigned int index = 0; index < buttons.size(); ++index)
		buttons[index].draw();
}

void App::run() {

	while (window.isOpen()) {

		timer++;
		if (timer % CLICK_ERROR == 0)
			mouseClicked.clear(), timer = 0;

		Panel& panel = rightPanel.isSelected ? rightPanel : leftPanel;
	
		/// Event handling
		sf::Event event;
		bool prevRename = renameShortcut;
		while (window.pollEvent(event)) {
			if(event.type == sf::Event::KeyPressed)
				panel.registerCharacter(event.key.scancode, pressed[sf::Keyboard::Scan::LShift], 1);
			switch (event.type)
			{
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed: case sf::Event::KeyReleased: case sf::Event::TextEntered:
				if (event.type == sf::Event::KeyPressed)
					sys->refresh(event.key.scancode, panel);
				handleKeyboardShortcuts(event, panel);
				handleKeyboardEvents(event);
				break;
			case sf::Event::MouseWheelScrolled:
				handleMouseScrollingEvents(event, panel);
				break;
			case sf::Event::MouseButtonPressed:
				handleMousePressingEvents(event, panel);
				break;
			case sf::Event::MouseButtonReleased:
				isMouseOnScrollbar = false;
				break;
			}
			if (prevRename && renameShortcut) 
				handleRenameShortcut(event, panel);
		}

		window.setActive();

		sf::Cursor cursor;
		getCursor(cursor, panel);
		window.setMouseCursor(cursor);

		/*  Drawing operations */

		/// Background colors
		window.draw(background);
		window.draw(bottomBackground);

		if (editor) {
			editor->draw();
		}
		else if (menu) {
			menu->draw();
		}
		else {
			handleMouseMovedEvents(panel);
			/// Panels
			leftPanel.draw();
			rightPanel.draw();

			window.draw(upButton), window.draw(downButton);
			window.draw(buttonText), window.draw(buttonText2);
		}

		/// Buttons
		drawButtons();

		window.display();
	}
}

void App::handleKeyboardEvents(sf::Event& event) {
	if (event.type == sf::Event::KeyReleased || renameShortcut)
		return;
	System* system = System::getInstance();

	if (editor) {
		if (event.key.scancode == sf::Keyboard::Scancode::F10)
		{
			delete editor;
			editor = NULL;
		}
		else editor->update(event);
		return;
	}
	if (menu) {
		if (event.key.scancode == sf::Keyboard::Scancode::F10)
		{
			delete menu;
			menu = NULL;
		}
		return;
	}


	Panel& panel = rightPanel.isSelected ? rightPanel : leftPanel;
	switch (event.key.scancode)
	{
	 case sf::Keyboard::Scan::Down: case sf::Keyboard::Scan::Up:
		if (!shortcutOn && !pathShortcut)  panel.updateSelectedFolder(event.key.scancode);
		break;
	case sf::Keyboard::Scan::Tab:
		leftPanel.toggleIsSelected();
		rightPanel.toggleIsSelected();
		pathShortcut = renameShortcut = shortcutOn = false;
		pressedKeys = 0;
		pressed.clear();
		break;
	case sf::Keyboard::Scan::Enter:
		if (panel.isSearchActive)
			break;
		if (pathShortcut)
			panel.changePath(2), pathShortcut = shortcutOn = false;
		else
			panel.changePath(1);
		break;
	case sf::Keyboard::Scancode::F8:
		panel.updateSelectedFolder(event.key.scancode);
		break;
	case sf::Keyboard::Scancode::F5:
		panel.updateSelectedFolder(event.key.scancode);
		break;
	case sf::Keyboard::Scancode::F10:
		window.close();
		break;
	case sf::Keyboard::Scancode::F3:
		editor = new Editor(window, Editor::Mode::VIEW);
		editor->init(panel.folders[panel.selectedFolderIndex].path);
		editor->update(event);
		break;
	case sf::Keyboard::Scancode::F4:
		editor = new Editor(window, Editor::Mode::EDIT);
		editor->init(panel.folders[panel.selectedFolderIndex].path);
		editor->update(event);
		break;
	case sf::Keyboard::Scancode::F1:
		menu = new Menu(window);
		menu->init();
	}
}

void App::handleRenameShortcut(sf::Event event, Panel& panel) {
	if (!renameShortcut)
		return;
	if (event.type == sf::Event::KeyPressed)
		panel.registerCharacter(event.key.scancode, pressed[sf::Keyboard::Scan::LShift], 2);
	if (event.key.scancode == sf::Keyboard::Scan::Enter || event.key.scancode == sf::Keyboard::Scan::Up || event.key.scancode == sf::Keyboard::Scan::Down) {
		panel.updateShortcutSelectedFolder(3, -1);
		renameShortcut = false;
		if (event.key.scancode == sf::Keyboard::Scan::Enter)
			sys->rename(renameString, panel);
		else panel.setSelectedFolder(renameString);
	}
	else  {
		if (event.type != sf::Event::KeyPressed && event.type != sf::Event::KeyReleased && event.type != sf::Event::TextEntered && event.type != sf::Event::MouseMoved && event.type != sf::Event::MouseWheelMoved) {
			panel.updateShortcutSelectedFolder(3, -1);
			renameShortcut = false;
		}
	}
}

void App::handleKeyboardShortcuts(sf::Event event, Panel& panel)
{
	if (event.type == sf::Event::KeyReleased) {
		if (pressed[event.key.scancode]) pressedKeys--;
		pressed[event.key.scancode] = false;
	}
	else if (event.type == sf::Event::KeyPressed) {
		if (!pressed[event.key.scancode]) pressedKeys++;
		count++;
		pressed[event.key.scancode] = count;
	}
	if (pathShortcut && event.type == sf::Event::KeyPressed) {
		panel.registerCharacter(event.key.scancode, pressed[sf::Keyboard::Scan::LShift], 3);
		return;
	}
	if (editor || renameShortcut && event.type != sf::Event::KeyReleased || event.type == sf::Event::TextEntered)
		return;
	
	if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::A] && pressedKeys == 2 && pressed[sf::Keyboard::Scan::LControl] < pressed[sf::Keyboard::Scan::A])
		shortcutOn = true, panel.updateShortcutSelectedFolder(1, 0);
	else if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::LShift] && pressed[sf::Keyboard::Scan::LControl] < pressed[sf::Keyboard::Scan::LShift] && pressedKeys == 3 && (pressed[sf::Keyboard::Scan::Up] || pressed[sf::Keyboard::Scan::Down])) {
		shortcutOn = true;
		if (pressed[sf::Keyboard::Scan::LShift] < std::max(pressed[sf::Keyboard::Scan::Up], pressed[sf::Keyboard::Scan::Down])) {
			int move = 1;
			if (pressed[sf::Keyboard::Scan::Up]) move = -move;
			panel.updateShortcutSelectedFolder(2, move);
			panel.updateByScrollbar(move, 1);
		}
	}
	else if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::C] && pressedKeys == 2 && pressed[sf::Keyboard::LControl] < pressed[sf::Keyboard::Scan::C])
		shortcutOn = true, clipboard->update(panel);
	else if ((pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::V] && pressedKeys == 2 && pressed[sf::Keyboard::Scan::LControl] < pressed[sf::Keyboard::Scan::V]) || pressed[sf::Keyboard::Scan::F6] && pressedKeys == 1)
		shortcutOn = true, clipboard->paste(panel);
	else if ((pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::R] && pressedKeys == 2 && pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::R]) || pressed[sf::Keyboard::Scan::F2] && pressedKeys == 1 || pressed[sf::Keyboard::Scan::F7] && pressedKeys == 1) {
		panel.updateShortcutSelectedFolder(3, -1);
		panel.registerCharacter(255, 0, 2);
		renameShortcut = true;
		renameString = panel.getSelectedFolder().folderText.getString();
	}
	else if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::T] && pressedKeys == 2 && pressed[sf::Keyboard::Scan::LControl] < pressed[sf::Keyboard::Scan::T]) {
		generateTheme();
		background.setFillColor(backgroundColor);
		bottomBackground.setFillColor(bottomBackgroundColor);
		for (int index = 0; index < buttons.size(); ++index) buttons[index].setColor(secondaryColor);
		leftPanel.updateColors();
		rightPanel.updateColors();
	}
	else if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::LShift] && pressed[sf::Keyboard::Scan::T] && pressedKeys >= 3 && pressed[sf::Keyboard::Scan::LControl] < pressed[sf::Keyboard::Scan::LShift] && pressed[sf::Keyboard::Scan::LShift] < pressed[sf::Keyboard::Scan::T]) {
		if (pressed[sf::Keyboard::Scan::Right] && pressedKeys == 4) {
			if (themes[(currentTheme + 1) % 10].size() > 0)
				currentTheme++;
		}
		else if (pressed[sf::Keyboard::Scan::Left] && pressedKeys == 4) {
			if (themes[(currentTheme + 9) % 10].size() > 0) 
				currentTheme--;
		}
		if (pressedKeys <= 4) {
			for (int index = 0; index < colors.size(); ++index) *colors[index] = themes[currentTheme][index];
			background.setFillColor(backgroundColor);
			bottomBackground.setFillColor(bottomBackgroundColor);
			for (int index = 0; index < buttons.size(); ++index) buttons[index].setColor(secondaryColor);
			leftPanel.updateColors();
			rightPanel.updateColors();
		}
	}
	else if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::S] && pressedKeys == 2 && pressed[sf::Keyboard::Scan::LControl] < pressed[sf::Keyboard::Scan::S]) {
		int loadTheme = -1;
		for (int index = 1; index < 10 && loadTheme == -1; ++index) {
			if (themes[index].size() == 0)
				loadTheme = index;
		}
		if(loadTheme == -1) 
			loadTheme = (currentTheme + 1) % 10;
		if (!loadTheme) loadTheme++;
		themes[loadTheme].clear();
		for (int index = 0; index < colors.size(); ++index) 
			themes[loadTheme].push_back(*colors[index]);
	}
	else if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::X] && pressedKeys == 2 && pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::X])
		shortcutOn = true, clipboard->move(panel);
	else if (pressed[sf::Keyboard::Scan::Semicolon]) {
		int code = -1;
		for (auto el : pressed) {
			if (el.second && el.first != sf::Keyboard::Scan::Semicolon)
				code = el.first;
		}
		if (pressedKeys == 2 && code >= 0 && code <= 25) {
			std::string path; path += (code + 'A'); path += ":\\";
			panel.changeDirectory(std::filesystem::path(path));
		}
	}
	else if (shortcutOn == true && !pressed[sf::Keyboard::Scan::LControl] && event.type == sf::Event::KeyPressed && !pressed[sf::Keyboard::Scan::LShift] && event.key.scancode != sf::Keyboard::Scan::Space) {
		if (event.key.scancode != sf::Keyboard::Scan::F5 && event.key.scancode != sf::Keyboard::Scan::F8) {
			panel.updateShortcutSelectedFolder(3, -1);
			shortcutOn = false;
		}
	}
	if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::F] && pressedKeys == 2 && pressed[sf::Keyboard::Scan::LControl] < pressed[sf::Keyboard::Scan::F]) {
		panel.activateSearch();
		renameShortcut = false;
	}
	else if (pressed[sf::Keyboard::Scan::LControl] && pressed[sf::Keyboard::Scan::LShift] && pressed[sf::Keyboard::Scan::M] && pressedKeys == 3 && pressed[sf::Keyboard::Scan::LControl] < pressed[sf::Keyboard::Scan::LShift] && pressed[sf::Keyboard::Scan::LShift] < pressed[sf::Keyboard::Scan::M])
		pathShortcut = true;
}

void App::handleMousePressingEvents(sf::Event& event, Panel& panel)
{
	if (pathShortcut) return;
	if (event.mouseButton.button == sf::Mouse::Left) {
		sf::Vector2f mouse{ (float)event.mouseButton.x , (float)event.mouseButton.y };
		if (pressed[sf::Keyboard::Scan::LControl]) {
			float height = PANEL_HEIGHT;
			panel.updateShortcutSelectedFolder(4, (int)((mouse.y - 58.261) / (height / LINE_SPACING)));
			shortcutOn = true;
		}
		if (!shortcutOn) {
			panel.checkTextLabels(mouse);
			panel.checkFolderLabels(mouse);
		}
		isMouseOnScrollbar = panel.checkScrollbarLabel(mouse);;
		auto checkScrollbarButton = [&](sf::RectangleShape& button)
			{
				sf::Vector2f buttonPosition = button.getPosition();
				sf::Vector2f buttonSize = button.getSize();
				return mouse.x >= buttonPosition.x && mouse.x <= buttonPosition.x + buttonSize.x && mouse.y >= buttonPosition.y && mouse.y <= buttonPosition.y + buttonSize.y;
			};
		if (checkScrollbarButton(upButton))
			panel.updateByScrollbar(-1, 2);
		else if (checkScrollbarButton(downButton))
			panel.updateByScrollbar(1, 2);
		if (mouseClicked[{mouse.x, mouse.y}] == true) 
			if (mouse.y >= PANEL_MARGIN_TOP + TOP_BUTTONS_HEIGHT + 38.f && mouse.y <= TOP_BUTTONS_HEIGHT + PANEL_HEIGHT + PANEL_MARGIN_TOP - BOTTOM_BUTTONS_HEIGHT) {
				panel.changePath(1);
			}
		mouseClicked.clear();
		mouseClicked[std::make_pair(mouse.x, mouse.y)] = true;
	}
}

void App::handleMouseMovedEvents(Panel& panel) {
	sf::Vector2i position = sf::Mouse::getPosition(window);
	int mouseX = position.x, mouseY = position.y;
	panel.activateLabel(mouseX, mouseY);
	if (isMouseOnScrollbar)
		panel.updateByScrollbar(sf::Vector2f((float)mouseX, (float)mouseY));
}

void App::handleMouseScrollingEvents(sf::Event& event, Panel& panel)
{
	panel.updateShortcutSelectedFolder(3, -1);
	renameShortcut = false;
	if (editor) {
		editor->update(event);
		return;
	}
	int delta = static_cast<int>(event.mouseWheelScroll.delta);
	if (delta < 0)
		panel.updateSelectedFolder(sf::Keyboard::Scan::S);
	else
		panel.updateSelectedFolder(sf::Keyboard::Scan::W);
	panel.updateShortcutSelectedFolder(3, -1);
	shortcutOn = false;
}

void App::getCursor(sf::Cursor& cursor, Panel& panel) {
	cursor.loadFromSystem(sf::Cursor::Hand);
	sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
	if (mousePosition.y >= 0 && mousePosition.y <= TOP_BUTTONS_HEIGHT + 15)
		cursor.loadFromSystem(sf::Cursor::Arrow);
	if (mousePosition.y >= TOP_BUTTONS_HEIGHT + PANEL_HEIGHT - BOTTOM_BUTTONS_HEIGHT + 15 && mousePosition.y <= window.getSize().y)
		cursor.loadFromSystem(sf::Cursor::Arrow);
	if (panel.checkMouseOnFolder(panel.getSelectedFolderIndex(), (float)mousePosition.x, (float)mousePosition.y))
		cursor.loadFromSystem(sf::Cursor::Arrow);
}