#include "Panel.h"


void Panel::init(sf::Vector2f pos,int width, int height, std::filesystem::path currentPath, std::vector<sf::Font> &fonts){
    this->pos = pos;
    this->width = width;
    this->height = height;
    this->currentPath = currentPath;
    this->fonts = fonts;
    update(currentPath);
}

void Panel::drawFolders() {
	for (int index = firstToDisplay; index <= lastToDisplay; ++index) {
		mainWindow.draw(folders[index].getFolderText());
		mainWindow.draw(folders[index].getSizeText());
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
	folderName.setFillColor(columnColor);
	folderName.setFont(fonts[3]);
	folderName.setPosition(pos + sf::Vector2f(FOLDER_SPACE / 2 + 80, 10));
	folderName.setStyle(sf::Text::Style::Bold);

	mainWindow.draw(folderName);

	sf::Text sizeName = folderName;
	sizeName.setString("Size");
	sizeName.setPosition(pos + sf::Vector2f(FOLDER_SPACE + SIZE_SPACE / 2 + 140, 10));
	mainWindow.draw(sizeName);

}

void Panel::drawSelectedFolderBackground() {

	sf::Vector2f topLeft = folders[selectedFolderIndex].getPosition();
	sf::Vector2f botRight(width - 20, LINE_SPACING);

	sf::RectangleShape rect;
	rect.setSize(botRight);
	rect.setFillColor(secondaryColor);
	rect.setPosition(topLeft);
	
	mainWindow.draw(rect);
}

void Panel::draw() {
	if(isSelected)
		drawSelectedFolderBackground();
	drawFolders();
	drawBorders();
	drawColumnTitles();
	
}

void Panel::update(std::filesystem::path path) {

	std::filesystem::path parentPath = currentPath;
    this->currentPath = path;
	sf::Vector2f textPosition = pos;
	textPosition.x += 10;
	textPosition.y += height / LINE_SPACING;

	folders.clear();

	firstToDisplay = 0, lastToDisplay = 1, selectedFolderIndex = 0;
	folders.push_back(Folder("/..", textPosition, parentPath, fonts));
	folders[firstToDisplay].toggleIsSelected();

    for (auto const& entry : std::filesystem::directory_iterator(currentPath)) {
		textPosition.y += height / LINE_SPACING;
        folders.push_back(Folder(entry.path(), textPosition, currentPath,fonts));
        if (textPosition.y <= height - PANEL_OFFSET - 20) {
            lastToDisplay++;
        }
    }
    lastToDisplay--;
}

void Panel::updateSelectedFolder(sf::Keyboard::Scancode code) {
	
	if (isSelected) {
		if (code == sf::Keyboard::Scancode::S || code == sf::Keyboard::Scancode::Down) {

		if (selectedFolderIndex + 1 < folders.size()) {
				folders[selectedFolderIndex].toggleIsSelected();
				folders[selectedFolderIndex].updateText();
				selectedFolderIndex++;
				folders[selectedFolderIndex].toggleIsSelected();
				folders[selectedFolderIndex].updateText();
			}
			if (lastToDisplay + 1 < folders.size()) {
				firstToDisplay++, lastToDisplay++;
				updateFoldersPosition(sf::Vector2f(0, -height / LINE_SPACING));
			}
			
		}
		else if (code == sf::Keyboard::Scancode::W || code == sf::Keyboard::Scancode::Up) {
			if (selectedFolderIndex != 0) {
				folders[selectedFolderIndex].toggleIsSelected();
				folders[selectedFolderIndex].updateText();
				selectedFolderIndex--;
				folders[selectedFolderIndex].toggleIsSelected();
				folders[selectedFolderIndex].updateText();
			}
			if (firstToDisplay != 0) {
				firstToDisplay--, lastToDisplay--;
				updateFoldersPosition(sf::Vector2f(0, height / LINE_SPACING));
			}
			
		}
		
	}
}

void Panel::updateFoldersPosition(sf::Vector2f move) {
	for (int index = 0;index < folders.size(); ++ index) {
		sf::Vector2f folderPosition = folders[index].getPosition();
		folderPosition += move;
		folders[index].setPosition(folderPosition);
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
		if(selectedFolderIndex)
			folderPath = folders[selectedFolderIndex].getFolderPath();
		else 
			folderPath = folders[selectedFolderIndex].getParentPath();
		std::cout << folderPath.string() << '\n';
		update(folderPath);
	}
}