#include "Globals.hpp"

sf::Vector2f GetCorrectedMousePos(sf::RenderWindow& window) {
	sf::Vector2f mousePos = sf::Vector2f(sf::Mouse::getPosition(window));
	float adjustX = (float)window.getSize().x / 1920.f;
	float adjustY = (float)window.getSize().y / 1080.f;
	return { mousePos.x / adjustX, mousePos.y / adjustY };
}