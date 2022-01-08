#include "KeyboardKey.hpp"
#include <iostream>

KeyboardKey::KeyboardKey(sf::Vector2f pos, sf::Vector2f size, char key, sf::Font &font) : keyText(key, font, 40), hitsText("0", font, 30)
{
	this->background.setSize(size);
	this->background.setOrigin(this->background.getSize().x / 2.f, this->background.getSize().y / 2.f);
	this->background.setPosition(pos);
	this->background.setFillColor(sf::Color(0, 0, 0, 0));
	this->background.setOutlineThickness(5.f);
	this->background.setOutlineColor(sf::Color::White);

	this->helpdot.setSize(sf::Vector2f(10.f, 10.f));
	this->helpdot.setOrigin(this->helpdot.getSize().x / 2.f, this->helpdot.getSize().y / 2.f);
	this->helpdot.setFillColor(sf::Color::Yellow);

	this->keyText.setOrigin(this->keyText.getGlobalBounds().width / 2.f, 0);
	this->keyText.setPosition(this->background.getPosition().x, this->background.getPosition().y - this->background.getSize().y/2);
	
	this->hitsText.setOrigin(this->hitsText.getGlobalBounds().width / 2.f, this->hitsText.getGlobalBounds().height);
	this->hitsText.setPosition(this->background.getPosition().x, this->background.getGlobalBounds().top + this->background.getGlobalBounds().height - 20.f);
}

KeyboardKey::KeyboardKey(sf::Vector2f pos, sf::Vector2f size, std::string text, sf::Font & font) : keyText(text, font, 40), hitsText("0", font, 30)
{
	if (keyText.getGlobalBounds().width >= size.x - 10.f) {
		size.x = (std::round(keyText.getGlobalBounds().width / 20.f)+1.f) * 20.f;
		//std::cout << size.x << std::endl;
	}
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
	//window.draw(this->helpdot);
}

void KeyboardKey::SetStatKey(unsigned long long value)
{
	this->hits = value;
	this->hitsText.setString(std::to_string(value));

	this->hitsText.setOrigin(this->hitsText.getGlobalBounds().width / 2, this->hitsText.getGlobalBounds().height);
	this->hitsText.setPosition(this->background.getPosition().x, this->background.getGlobalBounds().top + this->background.getGlobalBounds().height - 20.f);

	if (this->hitsText.getGlobalBounds().width >= this->background.getGlobalBounds().width - 10.f) {
		this->hitsText.setScale(sf::Vector2f(this->hitsText.getScale() - sf::Vector2f(0.01f, 0.01f)));
	}
}

void KeyboardKey::SetHits(unsigned long long value)
{
	this->hits = value;
	this->hitsText.setString(std::to_string(value));

	this->hitsText.setOrigin(this->hitsText.getGlobalBounds().width / 2, this->hitsText.getGlobalBounds().height);
	this->hitsText.setPosition(this->background.getPosition().x, this->background.getGlobalBounds().top + this->background.getGlobalBounds().height - 20.f);

	if (this->hitsText.getGlobalBounds().width >= this->background.getGlobalBounds().width - 10.f) {
		this->hitsText.setScale(sf::Vector2f(this->hitsText.getScale() - sf::Vector2f(0.01f, 0.01f)));
	}
}

void KeyboardKey::Move(sf::Vector2f amout)
{
	this->background.move(amout);
	this->keyText.move(amout);
	this->hitsText.move(amout);
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
	//this->hits += 99999;
	this->hitsText.setString(std::to_string(this->hits));

	this->hitsText.setOrigin(this->hitsText.getGlobalBounds().width / 2.f, this->hitsText.getGlobalBounds().height);
	//this->hitsText.setPosition(this->background.getPosition().x, this->background.getGlobalBounds().top + this->background.getGlobalBounds().height - 20.f);
	

	if (this->hitsText.getGlobalBounds().width >= this->background.getGlobalBounds().width - 22.f) {
		this->hitsText.setScale(sf::Vector2f(this->hitsText.getScale() - sf::Vector2f(0.05f, 0.05f)));
		this->hitsText.setPosition(this->background.getGlobalBounds().left + this->hitsText.getGlobalBounds().width/2.f, this->hitsText.getPosition().y);
	}
	//this->helpdot.setPosition(this->hitsText.getGlobalBounds().left + this->hitsText.getGlobalBounds().width / 2.f, this->hitsText.getGlobalBounds().top);

	this->holding = true;
}
