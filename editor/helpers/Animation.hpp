#pragma once
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>

class Animation
{
public:
	Animation(sf::Texture*, sf::Vector2u imageCount, float switchTime);
	~Animation();
	void Update(int row, float deltaTime, bool faceRight);
	void Update(int row, int end, float deltaTime, bool faceRight);
	void Update(sf::Vector2u range, float deltaTime, bool faceRight);

public:
	sf::IntRect uvRect;
	sf::Vector2u imageCount;
	sf::Vector2u currentImage;
	bool done = false;

private:

	float totalTime;
	float switchTime;
};

