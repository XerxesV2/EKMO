#pragma once
#include "ui/KeyboardKey.hpp"
#include "helpers/json.hpp"
#include "helpers/Singleton.hpp"

#include <SFML/Graphics.hpp>

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <thread>
#include <Windows.h>

class Program : public Singleton<Program>
{
public:
	Program();
	~Program();

	//Program(Program&) = delete;
	//Program(Program&&) = delete;
	//Program& operator=(Program&) = delete;

	void MainLoop();
private:
	void Update();
	void Draw();
	void HandleEvents();
	void HandleKeyInputs();
	void HandleKeyQueues();
	void HandleStats();

	void InitWindow();
	void StartKeyboardHookThread();

	void HideConsole();
	void MakeWindowOnTop(sf::RenderWindow& window);
	void MakeWindowTransparent();
	void ClickThrough();
	void MoveWindow();
	void ResizeWindow();

	void AddKey(sf::Vector2f pos, char key);
	void AddStatKey(sf::Vector2f pos, std::string text);
	void RemoveKey(char key);
	int GetCorrectOffsetForKey(char key);

	void SaveStat();
	void LoadStat();
	void LoadConfig();
	void UpdateJsonFlags();

public:
	void HandleCallbackKeyDown(int code);
	void HandleCallbackKeyUp(int code);

private:
	//init list
	sf::RenderWindow window;
	sf::View view;
	///////////////

	sf::Event event;
	sf::Font font;

	std::vector<bool> repeatProtection = { false };
	std::vector<int> keyList;
	std::vector<int> mouseKeyList;
	std::unordered_map<int, KeyboardKey*> keys;
	std::unordered_map<int, KeyboardKey*> mouseKeys;

	//sf::Vector2f keySize = { 84.f, 135.f };
	sf::Vector2f keySize = { 76.f, 76.f };
	sf::Vector2f statKeySize = { 135.f, 76.f };
	sf::Vector2f offset;

	std::vector<KeyboardKey> statKeys;

	enum STATKEY {
		CUR,
		TOT
	};

	unsigned long long totalHits = 0;
	int avgHits = 0;
	sf::Clock avghitsTimer;
	sf::Time startTime;
	float tick = 0.f;

	bool needReDraw = true;
	bool drawAFrame = false;
	bool configChanged = false;

	std::ifstream ifile;
	int ifileLength = 0;

	nlohmann::json loadj;
	std::string loadedConfig;

	// SFML pls fix
	std::chrono::time_point<std::chrono::high_resolution_clock> currentTime;
	
	std::thread th;
	std::unordered_map<int, int> keyDownQueue;
	std::unordered_map<int, int> keyUpQueue;
};

