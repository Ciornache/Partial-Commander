#pragma once

#include <filesystem>
#include <SFML/Graphics.hpp>
#include "Config.h"

class Folder
{
private:

	unsigned long long size;
	std::string modifyTime, date;
	sf::Font font;
	bool isSelected = false;

	std::string normalizePath(std::string path);
	std::string toString(std::uintmax_t size);


public:
	~Folder() {};
	Folder(std::filesystem::path path, sf::Vector2f position, std::vector<sf::Font>& fonts, std::string date);
	void initText(std::vector<sf::Font>& fonts);
	void toggleIsSelected();
	void updateText();
	unsigned int getSize()const;

	std::filesystem::path path;
	sf::Text folderText, sizeText, dateText;
	sf::Vector2f position;
	std::string date;
};