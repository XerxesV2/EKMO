#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "DropDown.hpp"
#include "MenuButton.hpp"
#include "../SFML_adofai/KeyboardKey.hpp"
#include "Animation.hpp"
#include "json.hpp"

class CProgram
{
public:
	CProgram();
	~CProgram();

	CProgram(CProgram&) = delete;
	CProgram(CProgram&&) = delete;
	CProgram& operator=(CProgram&) = delete;

	void MainLoop();
private:
	void Update();
	void Draw();
	void HandleEvents();
	void HandleClick();
	void HandleReposition();

	void InitButtons();
	void InitDropDown();
	void UpdateDropDownOptions();

	void AddKey(int ID);
	void DeleteKey();
	void DragAndPlaceKey(int index = -1);
	void ApplyConfig();

private:
	std::string SFKeyToString(sf::Keyboard::Key keycode);

private:
	//init list
	sf::RenderWindow window;
	sf::View view;
	///////////////

	sf::Event event;
	sf::Font font;

	Animation* logoAnim;
	sf::Texture spriteTexture;
	sf::Sprite spriteSheet;
	sf::Music music;

private:
	std::vector<MenuButton> buttons;
	enum keyIDs : int {
		NOTHING = -1,
		ADDKEY = 1,
		ADDCURKEY,
		ADDTOTKEY,

		SAVE,
		APPLY
	};

	DropDown *configsDropDown;
	std::vector<std::string> configsDropDown_Options = { "Config_0", "Config_1" };
	int selectedConfig = 0;
	int applyedConfig = -1;

	std::vector<KeyboardKey> keys;
	std::vector<KeyboardKey> statKeys;
	std::vector<KeyboardKey> mouseKeys;
	std::map<int, bool> existingKeysLookup;
	std::map<int, bool> existingStatKeysLookup;
	std::map<int, bool> existingMouseButtonLookup;

	sf::Text pressAkeyText;

	enum keyTypes {
		KEY,
		STATKEY,
		MOUSEKEY
	};
	//FLAGS
	bool draggingKey = false;
	bool repositionKey = false; int repositionKeyIndex = -1;
	bool waitForPressAKey = false;
	keyTypes selectedKeyType = KEY;
	bool readyToPlaceStatKey = false;
	bool needAutoSave = false;
	//

private:
	//json
	nlohmann::json savej;

	void Save();
	void SaveKeysPos();
	void SaveStatKeysPos();
	void SaveMouseKeysPos();

	void LoadConfig();
	void LoadKeysPos();
	void LoadStatKeysPos();
	void LoadMouseKeysPos();

	sf::Vector2u GetFinalWindowSize();
	sf::Vector2f GetRelativeKeyPos(sf::Vector2f& pos);
	sf::Vector2f relativeZeroPoint;
};

