#pragma once
#include "KeyboardKey.hpp"

#include <SFML/Graphics.hpp>

#include <iostream>
#include <map>
#include <vector>
#include <string>

class Program
{
public:
	Program();
	~Program();

	Program(Program&) = delete;
	Program(Program&&) = delete;
	Program& operator=(Program&) = delete;

	void MainLoop();
private:
	void Update();
	void Draw();
	void HandleEvents();
	void HandleKeyInputs();
	void HandleStats();

	void HideConsole();
	void MakeWindowOnTop(sf::RenderWindow& window);
	void MakeWindowTransparent();

	void AddKey(sf::Vector2f pos, char key);
	void AddStatKey(sf::Vector2f pos, std::string text);
	void RemoveKey(char key);
	int GetCorrectOffsetForKey(char key);

private:
	//init list
	sf::RenderWindow window;
	///////////////

	sf::Event event;
	sf::Font font;

	std::vector<bool> repeatProtection = { false };
	std::vector<char> keyList;
	std::map<char, KeyboardKey*> keys;

	//sf::Vector2f keySize = { 84.f, 135.f };
	sf::Vector2f keySize = { 76.f, 76.f };
	sf::Vector2f statKeySize = { 135.f, 76.f };
	sf::Vector2f offset;

	std::vector<KeyboardKey*> statKeys;

	enum STATKEY {
		CUR,
		TOT
	};

	unsigned long long totalHits = 0;
	unsigned long long avgHits = 0;
	sf::Clock avghitsTimer;
	sf::Time startTime;
	float tick = 0.f;

};

