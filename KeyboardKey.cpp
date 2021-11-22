#include "KeyboardKey.hpp"

KeyboardKey::KeyboardKey(sf::Vector2f pos, sf::Vector2f size, char key, sf::Font &font) : keyText(key, font, 40), hitsText("0", font, 30)
{
	this->background.setSize(size);
	this->background.setOrigin(this->background.getSize().x / 2.f, this->background.getSize().y / 2.f);
	this->background.setPosition(pos);
	this->background.setFillColor(sf::Color(0, 0, 0, 0));
	this->background.setOutlineThickness(5.f);
	this->background.setOutlineColor(sf::Color::White);


	this->keyText.setOrigin(this->keyText.getGlobalBounds().width / 2.f, 0);
	this->keyText.setPosition(this->background.getPosition().x, this->background.getPosition().y - this->background.getSize().y/2);
	
	this->hitsText.setOrigin(this->hitsText.getGlobalBounds().width / 2.f, this->hitsText.getGlobalBounds().height);
	this->hitsText.setPosition(this->background.getPosition().x, this->background.getGlobalBounds().top + this->background.getGlobalBounds().height - 20.f);
}

KeyboardKey::KeyboardKey(sf::Vector2f pos, sf::Vector2f size, std::string text, sf::Font & font) : keyText(text, font, 40), hitsText("0", font, 30)
{
	this->background.setSize(size);
	this->background.setOrigin(this->background.getSize().x / 2.f, this->background.getSize().y / 2.f);
	this->background.setPosition(pos);
	this->background.setFillColor(sf::Color(0, 0, 0, 0));
	this->background.setOutlineThickness(5.f);
	this->background.setOutlineColor(sf::Color::White);


	this->keyText.setOrigin(this->keyText.getGlobalBounds().width / 2.f, 0);
	this->keyText.setPosition(this->background.getPosition().x, this->background.getPosition().y - this->background.getSize().y / 2);

	this->hitsText.setOrigin(this->hitsText.getGlobalBounds().width / 2.f, this->hitsText.getGlobalBounds().height);
	this->hitsText.setPosition(this->background.getPosition().x, this->background.getGlobalBounds().top + this->background.getGlobalBounds().height - 20.f);
}

KeyboardKey::~KeyboardKey()
{
}

void KeyboardKey::Update(sf::RenderWindow &window)
{
	this->background.setFillColor(sf::Color::White);
	this->keyText.setFillColor(sf::Color::Black);
	this->hitsText.setFillColor(sf::Color::Black);
	this->background.setOutlineColor(sf::Color::White);


	if (!this->holding)
		this->AddHit();

	this->Draw(window);
}

void KeyboardKey::Draw(sf::RenderWindow &window)
{
	window.draw(this->background);
	window.draw(this->keyText);
	window.draw(this->hitsText);
}

void KeyboardKey::SetStat(unsigned long long value)
{
	this->hitsText.setString(std::to_string(value));

	this->hitsText.setOrigin(this->hitsText.getGlobalBounds().width / 2, this->hitsText.getGlobalBounds().height);
	this->hitsText.setPosition(this->background.getPosition().x, this->background.getGlobalBounds().top + this->background.getGlobalBounds().height - 20.f);
}

void KeyboardKey::Released()
{
	this->holding = false;
	this->background.setFillColor(sf::Color(0, 0, 0, 0));
	this->background.setOutlineColor(sf::Color(0, 115, 230, 255));
	this->keyText.setFillColor(sf::Color(0, 115, 230, 255));
	this->hitsText.setFillColor(sf::Color(0, 115, 230, 255));

}

void KeyboardKey::AddHit()
{
	++this->hits;
	this->hitsText.setString(std::to_string(this->hits));

	this->hitsText.setOrigin(this->hitsText.getGlobalBounds().width / 2, this->hitsText.getGlobalBounds().height);
	this->hitsText.setPosition(this->background.getPosition().x, this->background.getGlobalBounds().top + this->background.getGlobalBounds().height - 20.f);

	this->holding = true;
}
