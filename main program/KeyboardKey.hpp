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

	void Released();
	[[noreturn]]void AddHit();

	void SetStatKey(unsigned long long value);
	void SetHits(unsigned long long value);
	void SetKeyCode(int keyCode) { SFKeyCode = keyCode; };
	void Move(sf::Vector2f amount);
	sf::Vector2f GetPos() { return this->background.getPosition(); }
	sf::String GetString() { return keyText.getString(); }
	sf::FloatRect GetBounds() { return background.getGlobalBounds(); }
	unsigned long long& GetHits() { return hits; }
	int GetKeyCode() { return SFKeyCode; }

private:
	//inits
	sf::Text keyText;
	sf::Text hitsText;
	////////////

	sf::RectangleShape background;
	sf::RectangleShape helpdot;

	unsigned long long hits = 0;
	bool holding = false;
	int SFKeyCode = 0;
};

