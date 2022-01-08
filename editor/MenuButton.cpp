#include "MenuButton.hpp"
#include "Globals.hpp"

#define WINDOWS_X 1920
#define WINDOWS_Y 1080

MenuButton::MenuButton(sf::Vector2f pos, sf::String str, sf::Font& font, unsigned size, short ID) :
			  playButtonText(str, font, size),
			  ID(ID)
{
	//this->playButtonText.setOrigin(this->playButtonText.getGlobalBounds().width / 2.f, this->playButtonText.getGlobalBounds().height / 2.f);
	this->playButtonText.setPosition(pos);
	this->playButtonText.setFillColor(sf::Color(255, 255, 255, 255));
	this->playButtonText.setOutlineColor(sf::Color(100, 100, 100, 200));


	this->background.setSize(sf::Vector2f(this->playButtonText.getGlobalBounds().width + 40.f,
		this->playButtonText.getGlobalBounds().height + 20.f));
	this->background.setPosition(this->playButtonText.getGlobalBounds().left - 20.f,
		this->playButtonText.getGlobalBounds().top - 10.f);
	//this->background.setOrigin(this->background.getSize().x / 2.f, this->background.getSize().y / 2.f);
	this->background.setFillColor(sf::Color(100, 100, 100, 0));

}

void MenuButton::UpdateHighlight(sf::RenderWindow& window)
{
	//system("cls");
	//std::cout << sf::Mouse::getPosition(window).x <<"   "<<sf::Mouse::getPosition(window).y << std::endl;
	//std::cout << "textPos: " << this->playButtonText.getGlobalBounds().left << std::endl;

	if (this->background.getGlobalBounds().contains(sf::Vector2f(GetCorrectedMousePos(window)))) {
		if (!isStateChanged) needUpdate = true;
		isStateChanged = true;
	}
	else { if (isStateChanged) needUpdate = true; isStateChanged = false; }

	if (!needUpdate) return;

	//this->playButtonText.setOutlineThickness(5.f * isStateChanged);
	this->background.setFillColor(sf::Color(100, 100, 100, 150 * isStateChanged));

	needUpdate = false;
}

int MenuButton::IsClicked(sf::RenderWindow& window)
{
	if (this->background.getGlobalBounds().contains(sf::Vector2f(GetCorrectedMousePos(window)))) {
		return ID;
	}

	return -1;
}

void MenuButton::Draw(sf::RenderWindow& window)
{
	window.draw(this->background);
	window.draw(this->playButtonText);
}

MenuButton::~MenuButton()
{
}

bool MenuButton::HasMouseInput()
{
	static bool isButtonAreadyPressed = false;
	if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) { isButtonAreadyPressed = false; return 0; }
	if (isButtonAreadyPressed) return 0;
	isButtonAreadyPressed = true;
	return 1;
}
