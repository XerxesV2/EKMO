#pragma once
#include <SFML/Graphics.hpp>

class KeyboardKey
{
public:
	KeyboardKey(sf::Vector2f pos, sf::Vector2f size, char key, sf::Font &font);
	KeyboardKey(sf::Vector2f pos, sf::Vector2f size, std::string text, sf::Font &font);
	~KeyboardKey();

	void Update(sf::RenderWindow &window);
	void Draw(sf::RenderWindow &window);

	void SetStat(unsigned long long value);

	void Released();
	[[noreturn]]void AddHit();

private:
	//inits
	sf::Text keyText;
	sf::Text hitsText;
	////////////

	sf::RectangleShape background;

	long hits = 0;
	bool holding = false;
};

