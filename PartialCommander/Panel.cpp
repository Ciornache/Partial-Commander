#include "Panel.h"


void Panel::init(sf::Vector2f pos, int width, int height, std::filesystem::path currentPath, std::vector<sf::Font>& fonts) {
	this->pos = pos;
	this->width = width;
	this->height = height;
	this->currentPath = currentPath;
	this->fonts = fonts;
	update(currentPath);
}

void Panel::drawFolders() {
	for (int index = firstToDisplay; index <= lastToDisplay; ++index) {
		mainWindow.draw(folders[index].folderText);
		mainWindow.draw(folders[index].sizeText);
		mainWindow.draw(folders[index].dateText);
	}
}
void Panel::drawBorders() {

	sf::RectangleShape rect;
	rect.setOutlineThickness(PANEL_LINE_WIDTH);
	rect.setPosition(pos);
	rect.setSize(sf::Vector2f(width, height));
	rect.setOutlineColor(sf::Color::White);
	rect.setFillColor(sf::Color::Transparent);

	sf::RectangleShape line1;
	line1.setOutlineThickness(PANEL_LINE_WIDTH / 2);
	line1.setPosition(pos + sf::Vector2f(FOLDER_SPACE + 160, 0));
	line1.setSize(sf::Vector2f(0, height - PANEL_BOTTOM_HEIGHT));
	line1.setOutlineColor(sf::Color::White);

	sf::RectangleShape line2;
	line2.setOutlineThickness(PANEL_LINE_WIDTH / 2);
	line2.setPosition(pos + sf::Vector2f(FOLDER_SPACE + SIZE_SPACE + 160, 0));
	line2.setSize(sf::Vector2f(0, height - PANEL_BOTTOM_HEIGHT));
	line2.setOutlineColor(sf::Color::White);

	sf::RectangleShape line3;
	line3.setOutlineThickness(PANEL_LINE_WIDTH / 2);
	line3.setPosition(pos + sf::Vector2f(0, PANEL_HEIGHT - PANEL_BOTTOM_HEIGHT));
	line3.setSize(sf::Vector2f(width, 0));
	line3.setOutlineColor(sf::Color::White);


	mainWindow.draw(rect);
	mainWindow.draw(line1);
	mainWindow.draw(line2);
	mainWindow.draw(line3);
}

void Panel::drawColumnTitles() {

	sf::Text folderName;
	folderName.setString("Name");
	folderName.setCharacterSize(CHARACTER_SIZE);
	folderName.setFillColor(titleColor);
	folderName.setFont(fonts[CustomFonts::Font::ROBOTO]);
	folderName.setPosition(pos + sf::Vector2f(FOLDER_SPACE / 2 + 80, 10));

	mainWindow.draw(folderName);

	sf::Text sizeName = folderName;
	sizeName.setString("Size");
	sizeName.setPosition(pos + sf::Vector2f(FOLDER_SPACE + SIZE_SPACE / 2 + 140, 10));
	mainWindow.draw(sizeName);

}

void Panel::drawSelectedFolderBackground() {

	sf::Vector2f topLeft = folders[selectedFolderIndex].position;
	sf::Vector2f botRight(width - 20, LINE_SPACING);

	sf::RectangleShape rect;
	rect.setSize(botRight);
	rect.setFillColor(secondaryColor);
	rect.setPosition(topLeft);

	mainWindow.draw(rect);
}

void Panel::draw() {
	if (isSelected)
		drawSelectedFolderBackground();
	drawFolders();
	drawBorders();
	drawColumnTitles();
	drawCurrentPath();
}

void Panel::update(std::filesystem::path path) {

	this->currentPath = path;
	updateDates();
	sf::Vector2f textPosition = pos;
	textPosition.x += 10;
	textPosition.y += 1.0 * height / LINE_SPACING;

	folders.clear();

	firstToDisplay = 0, lastToDisplay = 1, selectedFolderIndex = 0;
	folders.push_back(Folder("/..", textPosition, fonts, " "));
	folders[firstToDisplay].toggleIsSelected();
	for (auto const& entry : std::filesystem::directory_iterator(path)) {
		textPosition.y += 1.0 * height / LINE_SPACING;
		std::string name, pathName = entry.path().string();
		for (int i = pathName.size() - 1; i >= 0; --i) {
			if (pathName[i] == '/')
				break;
			name += pathName[i];
		}
		std::reverse(name.begin(), name.end());
		std::cout << name << '\n';
		folders.push_back(Folder(entry.path(), textPosition, fonts, dates[name]));
		if (textPosition.y <= height - PANEL_OFFSET - 20) 
			lastToDisplay++;
		if(folders.size() >= 300)
			break;
	}
	lastToDisplay--;
}

void Panel::updateDates() {
	dates.clear();
	freopen("modifytime.txt", "w", stdout);
	std::string command = "dir " + currentPath.string();
	for (int i = 0; i < command.size(); ++i) {
		if (command[i] == '/')
			command[i] = '\\';
	}
	system(command.c_str());
	char token[1005];
	FILE* f = fopen("modifytime.txt", "r");
	freopen("CON", "w", stdout);
	for (int i = 1; i <= 5; ++i)
		fgets(token, sizeof(token), f);
	while (true) {
		fgets(token, sizeof(token), f);
		if (token[0] == ' ')
			break;
		char nameToken[50];
		strcpy(nameToken, token + 39);
		token[10] = '\0';
		std::string date, name;
		for (int index = 0; index < strlen(token); ++index) 
			date += token[index];
		for (int index = 0; index < strlen(nameToken); ++index) 
			name += nameToken[index];
		name.pop_back();
		dates[name] = date;
	}
}

void Panel::updateSelectedFolder(sf::Keyboard::Scancode code) {

	if (isSelected) {
		switch (code)
		{
		case sf::Keyboard::Scancode::S:
		case sf::Keyboard::Scancode::Down:
		{
			if (selectedFolderIndex + 1 < folders.size()) {
				folders[selectedFolderIndex].toggleIsSelected();
				folders[selectedFolderIndex].updateText();
				selectedFolderIndex++;
				folders[selectedFolderIndex].toggleIsSelected();
				folders[selectedFolderIndex].updateText();
			}
			if (selectedFolderIndex == lastToDisplay) {
				int step = std::min((size_t)10, folders.size() - lastToDisplay - 1);
				firstToDisplay += step;
				lastToDisplay += step;
				updateFoldersPosition(sf::Vector2f(0, 1.0 * step * (-height) / LINE_SPACING));
			}
			break;
		}
		case sf::Keyboard::Scancode::W:
		case sf::Keyboard::Scancode::Up:
		{
			if (selectedFolderIndex != 0) {
				folders[selectedFolderIndex].toggleIsSelected();
				folders[selectedFolderIndex].updateText();
				selectedFolderIndex--;
				folders[selectedFolderIndex].toggleIsSelected();
				folders[selectedFolderIndex].updateText();
			}
			if (selectedFolderIndex == firstToDisplay) {
				int step = std::min(10, firstToDisplay);
				firstToDisplay -= step;
				lastToDisplay -= step;
				updateFoldersPosition(sf::Vector2f(0, 1.0 * step * height / LINE_SPACING));
			}
			break;
		}

		case sf::Keyboard::Scancode::F8:
		{
			int index = selectedFolderIndex;
			auto path = folders[index].path;

			if (std::filesystem::is_directory(path))
				std::cout << "Folder: " << path << " removed\n";
			else
				std::cout << "File: " << path << " removed\n";

			// remove from the filesystem
			// if it's a folder remove recursively 
			std::filesystem::remove_all(path);

			update(currentPath);
			//update the index to the old one
			selectedFolderIndex = std::min(index, (int)folders.size() - 1);
			break;
		}
		case sf::Keyboard::Scancode::F5: {

			// Todo: Choose the folder in which the file is coppied
			int index = selectedFolderIndex;
			auto path = folders[index].path;
			auto destPath = path;

			std::string extension = path.extension().string();
			for (int i = 1; std::filesystem::exists(destPath); i++) {
				destPath = path.parent_path() / (path.stem().string() + std::to_string(i) + extension);
			}

			try {
				std::filesystem::copy(path, destPath);
				std::cout << destPath.filename() << " coppied to " << destPath.parent_path() << '\n';
			}
			catch(const std::filesystem::filesystem_error& e){
				std::cerr << "Error copying file: " << e.what() << std::endl;
			}
			update(path.parent_path());

			// reset the index
			selectedFolderIndex = index;
			break;
		}
		default:
			break;
		}
	}
}

void Panel::updateFoldersPosition(sf::Vector2f move) {
	for (int index = 0; index < folders.size(); ++index) {
		sf::Vector2f folderPosition = folders[index].position;
		folderPosition += move;
		folders[index].position=folderPosition;
		folders[index].updateText();
	}
}

void Panel::toggleIsSelected() {
	isSelected = !isSelected;
	folders[selectedFolderIndex].toggleIsSelected();
	folders[selectedFolderIndex].updateText();
}

void Panel::changePath() {
	if (isSelected) {
		std::filesystem::path folderPath;
		if (selectedFolderIndex)
			folderPath = folders[selectedFolderIndex].path;
		else
			folderPath = currentPath.parent_path();
		std::cout << folderPath.string() << '\n';
		update(folderPath);	
	}
}

void Panel::drawCurrentPath()
{
	sf::Text pathText;
	pathText.setString(currentPath.string());
	pathText.setFillColor(textColor);
	pathText.setPosition(sf::Vector2f(pos.x + PANEL_MARGIN_X, PANEL_HEIGHT + BOTTOM_BUTTONS_HEIGHT / 3));
	pathText.setCharacterSize(CHARACTER_SIZE + 5);
	pathText.setFont(fonts[4]);

	mainWindow.draw(pathText);
}