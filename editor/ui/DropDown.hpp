#pragma once
#include "MenuButton.hpp"

class DropDown
{
public:
	DropDown(sf::Vector2f pos, std::vector<std::string>& str, sf::Font& font, unsigned size);

	void Update(sf::RenderWindow& window);
	void Draw(sf::RenderWindow& window);

	void UpdateHighlights(sf::RenderWindow&);
	int IsAnyClicked(sf::RenderWindow&);	//return from 0 (0 air click)
	void AddOption(std::string str, sf::Font &font);

	~DropDown();

private:
	std::vector<MenuButton> buttons;

	sf::RectangleShape backGround;
	sf::CircleShape arrow;

	bool state = 0;
};

