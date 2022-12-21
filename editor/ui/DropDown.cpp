#include "DropDown.hpp"
#include "../Globals.hpp"


DropDown::DropDown(sf::Vector2f pos, std::vector<std::string>& str, sf::Font & font, unsigned size)
{
	this->backGround.setSize(sf::Vector2f(300.f, 50.f));
	this->backGround.setOrigin(this->backGround.getGlobalBounds().width / 2.f, this->backGround.getGlobalBounds().height / 2.f);
	this->backGround.setPosition(pos);
	this->backGround.setFillColor(sf::Color(100, 100, 100, 255));

	this->arrow.setRadius(30.f);
	this->arrow.setPointCount(3);
	this->arrow.setOrigin(this->arrow.getRadius(), this->arrow.getRadius());
	this->arrow.setPosition(sf::Vector2f(this->backGround.getPosition().x + 100.f, this->backGround.getPosition().y));
	this->arrow.setRotation(30.f);

	float shift = this->backGround.getGlobalBounds().height;
	for (size_t i = 0; i < str.size(); i++) {
		this->buttons.push_back(MenuButton(sf::Vector2f(pos.x ,pos.y + shift), str[i], font, size, (short)i));
		shift += this->buttons.front().GetText().getGlobalBounds().height + 20.f;	//Plus shift
	}
}

void DropDown::Update(sf::RenderWindow& window)
{
	if (this->arrow.getGlobalBounds().contains(sf::Vector2f(GetCorrectedMousePos(window)))) {
		state = !state;
		this->arrow.setRotation(30.f * (1 + state));
	}
}

void DropDown::Draw(sf::RenderWindow& window)
{
	window.draw(this->backGround);
	window.draw(this->arrow);

	UpdateHighlights(window);
}

void DropDown::UpdateHighlights(sf::RenderWindow& window)
{
	if (!state) return;

	for (MenuButton &button : this->buttons) {
		button.UpdateHighlight(window);

		button.Draw(window);
	}
}

int DropDown::IsAnyClicked(sf::RenderWindow& window)
{
	if (!state) return -1;

	int ID;
	for (MenuButton &button : this->buttons) {
		ID = button.IsClicked(window);
		if (ID != -1) return ID; 
	}

	return -1;
}

void DropDown::AddOption(std::string str, sf::Font &font)
{
	float shift = this->buttons.front().GetText().getGlobalBounds().height + 20.f;
	this->buttons.push_back(MenuButton(sf::Vector2f(this->buttons.back().GetPos() + sf::Vector2f(0.f, shift)), str, font, this->buttons.back().GetText().getCharacterSize(), this->buttons.back().GetID()+1));
}

DropDown::~DropDown()
{
}
