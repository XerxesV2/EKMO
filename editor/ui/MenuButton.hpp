#pragma once
#include <iostream>
#include <vector>
#include <string>

#include <SFML/Graphics.hpp>

class MenuButton
{
public:
	MenuButton(sf::Vector2f, sf::String, sf::Font&, unsigned, short ID);

	void UpdateHighlight(sf::RenderWindow&);
	int IsClicked(sf::RenderWindow&);
	void Draw(sf::RenderWindow& window);

	inline sf::Text& GetText() { return this->playButtonText; }
	sf::Vector2f GetPos() { return playButtonText.getPosition(); }
	short GetID() { return ID; }

	~MenuButton();

private:
	sf::Text playButtonText;
	sf::RectangleShape background;
	short ID;

	///////for Highlights/////////
	bool needUpdate = false;
	bool isStateChanged = false;

public:
	static bool HasMouseInput();
};

