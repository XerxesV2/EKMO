#include "CProgram.hpp"
#include "Globals.hpp"

#include <fstream>
#include <limits>
#include <mutex>
#include <chrono>

#include <Dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

#define SCREEN_X sf::VideoMode::getDesktopMode().width
#define SCREEN_Y sf::VideoMode::getDesktopMode().height

#define WINDOWS_X 1920
#define WINDOWS_Y 1080

static bool shouldListenForKey = false;

std::mutex callbackMut;
HHOOK keyboardHook;
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(!shouldListenForKey) return CallNextHookEx(keyboardHook, nCode, wParam, lParam);

	PKBDLLHOOKSTRUCT key = (PKBDLLHOOKSTRUCT)lParam;

	//a key was pressed
	if (wParam == WM_KEYDOWN && nCode == HC_ACTION)
	{
		//printf("\ndown: %d", key->vkCode);
		CProgram::Get().AddKeyFromCallback(key->vkCode);
	}

	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

CProgram::CProgram() : 
	window(sf::VideoMode(SCREEN_X / 2, SCREEN_Y / 2), "KadofaiKConfig", sf::Style::Close),
	view(sf::Vector2f(1920 / 2, 1080 / 2), sf::Vector2f(1920, 1080))
{
	this->window.setFramerateLimit(60);
	this->window.setView(this->view);
	this->font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");

	this->pressAkeyText.setFont(this->font);
	this->pressAkeyText.setString("Press A key!");

	this->InitButtons();
	this->InitDropDown();

	this->selectedConfig = 0;
	this->LoadConfig();

	this->spriteTexture.loadFromFile("planet.png");
	this->logoAnim = new Animation(&this->spriteTexture, sf::Vector2u(50, 1), 800.f);
	this->spriteSheet.setTexture(this->spriteTexture);
	this->spriteSheet.setScale(3.f, 3.f);
	this->spriteSheet.setPosition(1920.f - this->spriteSheet.getGlobalBounds().width / 50.f, 1080.f - this->spriteSheet.getGlobalBounds().height); //50 frame
	//std::cout << this->spriteSheet.getPosition().y << std::endl;

	this->music.openFromFile("AbsoluteUninstaller.wav");
	this->music.setVolume(10.f);
	this->music.play();

	this->StartKeyboardHookThread();
}

CProgram::~CProgram()
{
	this->Save();
	this->selectedConfig = this->configsDropDown_Options.size()-1;
	this->keys.clear();
	this->Save();
	this->th.detach();
}

void CProgram::MainLoop()
{
	while (this->window.isOpen())
	{
		this->HandleEvents();

		this->Update();
		this->Draw();
	}
}

void CProgram::Update()
{
	this->logoAnim->Update(0, 100.f, 1);

	if (this->keyExists) {	//the callback is faster so we need this workaround
		this->keys.back().SetKeyCode(this->codeToSet);
		this->keyExists = false;
	}

	if (this->draggingKey) {
		std::lock_guard<std::mutex> lock(callbackMut);
		this->DragAndPlaceKey();
		return;
	}

	if(this->repositionKey){
		std::lock_guard<std::mutex> lock(callbackMut);
		this->DragAndPlaceKey(this->repositionKeyIndex);
	}

	this->HandleClick();
	this->DeleteKey();
}

void CProgram::Draw()
{
	//planet animation
	this->window.clear();
	this->spriteSheet.setTextureRect(this->logoAnim->uvRect);
	this->window.draw(this->spriteSheet);
	//-------

	for (MenuButton &button : this->buttons) {
		button.UpdateHighlight(this->window);
		button.Draw(this->window);
	}
	this->configsDropDown->Draw(this->window);

	for (KeyboardKey &key : this->keys)
		key.Draw(this->window);
	for (KeyboardKey &key : this->statKeys)
		key.Draw(this->window);
	for (KeyboardKey &key : this->mouseKeys)
		key.Draw(this->window);

	if(this->waitForPressAKey)
		this->window.draw(this->pressAkeyText);

	this->window.display();
}

void CProgram::HandleEvents()
{
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
	}
}

void CProgram::HandleClick()
{
	if (!MenuButton::HasMouseInput()) return;

	if (!this->repositionKey) {

		for (MenuButton &button : this->buttons) {
			switch (button.IsClicked(this->window)) {
			case NOTHING: break;
			case ADDKEY: this->AddKey(ADDKEY); return;
			case ADDCURKEY: this->AddKey(ADDCURKEY); return;
			case ADDTOTKEY: this->AddKey(ADDTOTKEY); return;
			case APPLY: this->ApplyConfig(); return;

			case SAVE: this->Save(); return;
			default: std::cout << "Menu - HandleClickError" << std::endl; __debugbreak(); return;
			}
		}

		this->configsDropDown->Update(this->window);
		int ID = this->configsDropDown->IsAnyClicked(this->window);
		if (ID != -1)
		{
			if (this->needAutoSave)
				this->Save();     // ;)

			std::cout << "DropDown ID: " << ID << std::endl;
			this->selectedConfig = ID;
			this->LoadConfig();

			this->UpdateDropDownOptions();
		}
	}

	this->HandleReposition();
}

void CProgram::HandleReposition()
{
	int i = 0;
	for (KeyboardKey &key : this->keys) {
		if (key.GetBounds().contains(GetCorrectedMousePos(this->window))) {
			this->repositionKey = !this->repositionKey;
			this->repositionKeyIndex = i;
			this->selectedKeyType = KEY;
			return;
		}
		++i;
	}

	i = 0;
	for (KeyboardKey &key : this->statKeys) {
		if (key.GetBounds().contains(GetCorrectedMousePos(this->window))) {
			this->repositionKey = !this->repositionKey;
			this->repositionKeyIndex = i;
			this->selectedKeyType = STATKEY;
			return;
		}
		++i;
	}

	i = 0;
	for (KeyboardKey &key : this->mouseKeys) {
		if (key.GetBounds().contains(GetCorrectedMousePos(this->window))) {
			this->repositionKey = !this->repositionKey;
			this->repositionKeyIndex = i;
			this->selectedKeyType = MOUSEKEY;
			return;
		}
		++i;
	}
}

void CProgram::InitButtons()
{
	this->buttons.push_back(MenuButton(sf::Vector2f(0.f,0.f), "AddKey", this->font, 60, (short)ADDKEY));
	this->buttons.push_back(MenuButton(sf::Vector2f(0.f,100.f), "AddCURKey", this->font, 60, (short)ADDCURKEY));
	this->buttons.push_back(MenuButton(sf::Vector2f(0.f,200.f), "AddTOTKey", this->font, 60, (short)ADDTOTKEY));
	this->buttons.push_back(MenuButton(sf::Vector2f(1200.f, 0.f), "Apply", this->font, 60, (short)APPLY));

	this->buttons.push_back(MenuButton(sf::Vector2f(1500.f,0.f), "Save", this->font, 60, (short)SAVE));
}

void CProgram::InitDropDown()
{
	this->configsDropDown = new DropDown(sf::Vector2f(900.f, 50.f), this->configsDropDown_Options, this->font, 50);

	std::ifstream input_file("save.json");

	if (!input_file.good()) 
	{
		MessageBox(
			NULL,
			(LPCWSTR)L"Failed to load config file",
			(LPCWSTR)L"Error",
			MB_ICONERROR | MB_OK
		);
		abort();
	}
	input_file >> this->savej;

	int numOfConfigs = this->savej["configs"].get<int>();
	for (int i = 2; i < numOfConfigs; i++)
	{
		std::string str = "Config_" + std::to_string(i);
		this->configsDropDown_Options.push_back(str);
		this->configsDropDown->AddOption(str, this->font);
	}
	input_file.close();

}

void CProgram::UpdateDropDownOptions()
{
	if (this->selectedConfig == this->configsDropDown_Options.size() - 1) {
		this->configsDropDown_Options.push_back("Config_" + std::to_string(this->selectedConfig + 1));

		this->configsDropDown->AddOption(this->configsDropDown_Options.back(), this->font);
		//delete this->configsDropDown;
		//this->configsDropDown = new DropDown(sf::Vector2f(900.f, 50.f), this->configsDropDown_Options, this->font, 50);
		std::cout << "DropDown Options Updated!" << std::endl;
	}
}

void CProgram::StartKeyboardHookThread()
{
	this->th = std::thread([]() {
		printf("Hooking the keyboard\n");
		//Here we set the low level hook
		keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
		printf("%X\n", keyboardHook);
		printf("\nthread %d\n", std::this_thread::get_id());

		MSG msg{ 0 };

		while (GetMessage(&msg, NULL, 0, 0) != 0);
	});
}

void CProgram::AddKey(int ID)
{
	if (ID != 1) {
		if (!this->existingStatKeysLookup.count(ID)) {
			switch (ID)
			{
			case ADDCURKEY: this->statKeys.push_back(KeyboardKey({ 0.f, 0.f }, { 120.f, 80.f }, "CUR", this->font)); break;
			case ADDTOTKEY: this->statKeys.push_back(KeyboardKey({ 0.f, 0.f }, { 120.f, 80.f }, "TOT", this->font)); break;
			default: break;
			}
			
			this->statKeys.back().Released();
			this->selectedKeyType = STATKEY;
			this->draggingKey = true;
			this->existingStatKeysLookup[ID] = true;
			this->needAutoSave = true;
		}
		return;
		
	}else	this->ArmCallbackThread();

	this->waitForPressAKey = true;

	while (this->window.isOpen())
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::TextEntered) {	//and this secound
				this->ArmCallbackThread();	//we can take advantage of it and arm the callback again if key already exists
			}
			if (event.type == sf::Event::KeyPressed) { //<--this called first
				std::cout << "key: " << event.key.code << std::endl;
				
				if (!this->existingKeysLookup.count(event.key.code)) {

					this->keys.push_back(KeyboardKey({ 0.f, 0.f }, { 80.f, 80.f }, this->SFKeyToString(event.key.code), this->font)); //65 ASCII code for A
					
					/*this->keys.back().SetKeyCode(event.key.code);*/
					this->existingKeysLookup[event.key.code] = true;
					this->keys.back().SetSFMLKeyCode(event.key.code);
					this->keys.back().Released();

					this->draggingKey = true;
					this->waitForPressAKey = false;
					this->selectedKeyType = KEY;
					this->needAutoSave = true;

					std::lock_guard<std::mutex> lock(callbackMut);
					this->keyExists = true;

					return;
				}
			}
			if (event.type == sf::Event::MouseButtonPressed) {
				sf::Event &e = this->event;
				auto getMouseKeyString = [&e]() -> std::string {
					switch (e.mouseButton.button)
					{
					case sf::Mouse::Left: return "LButton";
					case sf::Mouse::Right: return "RButton";
					case sf::Mouse::Middle: return "MButton";
					default: return "Error";
					}
				};

				if (!this->existingMouseButtonLookup.count(event.mouseButton.button)) {
					this->mouseKeys.push_back(KeyboardKey({ 0.f, 0.f }, { 80.f, 80.f }, getMouseKeyString(), this->font)); //65 ASCII code for A
					this->mouseKeys.back().SetSFMLKeyCode(event.mouseButton.button);
					this->mouseKeys.back().Released();
					this->existingMouseButtonLookup[event.mouseButton.button] = true;

					this->draggingKey = true;
					this->waitForPressAKey = false;
					this->selectedKeyType = MOUSEKEY;
					this->needAutoSave = true;
					return;
				}
			}
		}

		this->pressAkeyText.setPosition(GetCorrectedMousePos(this->window));
		this->Draw();
		this->logoAnim->Update(0, 100.f, 1);
	}
}

void CProgram::AddKeyFromCallback(unsigned long code)
{
	std::lock_guard<std::mutex> lock(callbackMut);
	std::cout << "key from callback: " << code << std::endl;

	this->codeToSet = code;

	shouldListenForKey = false;
}

void CProgram::ArmCallbackThread()
{
	shouldListenForKey = true;
}

void CProgram::DeleteKey()
{
	if (this->repositionKey) return;

	std::lock_guard<std::mutex> lock(callbackMut);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Delete)) {
		int i = 0;
		for (KeyboardKey &key : this->keys) {
			if (key.GetBounds().contains(GetCorrectedMousePos(this->window))) {
				this->existingKeysLookup.erase(key.GetSFMLKeyCode());
				this->keys.erase(this->keys.begin() + i);

		std::cout << "Key deleted..." << std::endl;
			}
			++i;
		}

		i = 0;
		for (KeyboardKey &key : this->statKeys) {
			if (key.GetBounds().contains(GetCorrectedMousePos(this->window))) {

				switch (*key.GetString().begin())
				{
				case 'C': this->existingStatKeysLookup.erase(ADDCURKEY); break;
				case 'T': this->existingStatKeysLookup.erase(ADDTOTKEY);  break;
				default: break;
				}
				this->statKeys.erase(this->statKeys.begin() + i);

				std::cout << "StatKey deleted..." << std::endl;
			}
			++i;
		}

		i = 0;
		for (KeyboardKey &key : this->mouseKeys) {
			if (key.GetBounds().contains(GetCorrectedMousePos(this->window))) {
				this->existingMouseButtonLookup.erase(key.GetSFMLKeyCode());
				this->mouseKeys.erase(this->mouseKeys.begin() + i);

				std::cout << "MouseKey deleted..." << std::endl;
			}
			++i;
		}
	}
}

void CProgram::DragAndPlaceKey(int index)
{
	//if (this->draggingKey == false) return;

	if (this->selectedKeyType != KEY && this->readyToPlaceStatKey && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		this->draggingKey = false;
		this->readyToPlaceStatKey = false;
		return;
	}
	else if (this->selectedKeyType == KEY && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		this->draggingKey = false;
		this->readyToPlaceStatKey = false;
		return;
	}
	else if(!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
		this->readyToPlaceStatKey = true;
	}
	
	auto getCorrectKey = [&]() -> KeyboardKey& {
		if (this->selectedKeyType == KEY)
			return index == -1 ? this->keys.back() : this->keys.at(index);
		if (this->selectedKeyType == STATKEY)
			return index == -1 ? this->statKeys.back() : this->statKeys.at(index);
		if (this->selectedKeyType == MOUSEKEY)
			return index == -1 ? this->mouseKeys.back() : this->mouseKeys.at(index);
	};
	
	KeyboardKey &key = getCorrectKey();

	sf::Vector2f mousePos = GetCorrectedMousePos(this->window);
	sf::Vector2f keyPos = key.GetPos();
	sf::Vector2f dif = mousePos - keyPos;

	while (std::abs(dif.x) > 10.f || std::abs(dif.y) > 10.f) {
		if (dif.x >= 10.f)
			key.Move({ 10.f, 0.f });
		else if (dif.x <= -10.f)
			key.Move({ -10.f, 0.f });

		if (dif.y >= 10.f)
			key.Move({ 0.f, 10.f });
		else if (dif.y <= -10.f)
			key.Move({ 0.f, -10.f });

		mousePos = GetCorrectedMousePos(this->window);
		keyPos = key.GetPos();
		dif = mousePos - keyPos;
	}

	this->needAutoSave = true;
}

void CProgram::ApplyConfig()
{
	this->applyedConfig = this->selectedConfig;
}

std::string CProgram::SFKeyToString(sf::Keyboard::Key keycode)
{
	if (keycode >= 0 && keycode <= 25) {
		std::string key;
		key = char(keycode + 65);
		return key;
	}

	if (keycode >= 26 && keycode <= 35) {
		std::string key;
		key = char(keycode + 22);
		return key;
	}

	switch (keycode) {
	case sf::Keyboard::Escape: return "Escape";
	case sf::Keyboard::LControl: return "LControl";
	case sf::Keyboard::LShift: return "LShift";
	case sf::Keyboard::LAlt: return "LAlt";
	case sf::Keyboard::LSystem: return "LSystem";
	case sf::Keyboard::RControl: return "RControl";
	case sf::Keyboard::RShift: return "RShift";
	case sf::Keyboard::RAlt: return "RAlt";
	case sf::Keyboard::RSystem: return "RSystem";
	case sf::Keyboard::Menu: return "Menu";
	case sf::Keyboard::LBracket: return "LBracket";
	case sf::Keyboard::RBracket: return "RBracket";
	case sf::Keyboard::SemiColon: return ";";
	case sf::Keyboard::Comma: return ",";
	case sf::Keyboard::Period: return ".";
	case sf::Keyboard::Quote: return "\'";
	case sf::Keyboard::Slash: return "/";
	case sf::Keyboard::BackSlash: return "\\";
	case sf::Keyboard::Tilde: return "~";
	case sf::Keyboard::Equal: return "=";
	case sf::Keyboard::Dash: return "-";
	case sf::Keyboard::Space: return "Space";
	case sf::Keyboard::Return: return "Return";
	case sf::Keyboard::BackSpace: return "Back";
	case sf::Keyboard::Tab: return "Tab";
	case sf::Keyboard::PageUp: return "Page Up";
	case sf::Keyboard::PageDown: return "Page Down";
	case sf::Keyboard::End: return "End";
	case sf::Keyboard::Home: return "Home";
	case sf::Keyboard::Insert: return "Insert";
	case sf::Keyboard::Delete: return "Delete";
	case sf::Keyboard::Add: return "+";
	case sf::Keyboard::Subtract: return "-";
	case sf::Keyboard::Multiply: return "*";
	case sf::Keyboard::Divide: return "/";
	case sf::Keyboard::Left: return "Left";
	case sf::Keyboard::Right: return "Right";
	case sf::Keyboard::Up: return "UP";
	case sf::Keyboard::Down: return "Down";
	case sf::Keyboard::Numpad0: return "NP 0";
	case sf::Keyboard::Numpad1: return "NP 1";
	case sf::Keyboard::Numpad2: return "NP 2";
	case sf::Keyboard::Numpad3: return "NP 3";
	case sf::Keyboard::Numpad4: return "NP 4";
	case sf::Keyboard::Numpad5: return "NP 5";
	case sf::Keyboard::Numpad6: return "NP 6";
	case sf::Keyboard::Numpad7: return "NP 7";
	case sf::Keyboard::Numpad8: return "NP 8";
	case sf::Keyboard::Numpad9: return "NP 9";
	case sf::Keyboard::F1: return "F1";
	case sf::Keyboard::F2: return "F2";
	case sf::Keyboard::F3: return "F3";
	case sf::Keyboard::F4: return "F4";
	case sf::Keyboard::F5: return "F5";
	case sf::Keyboard::F6: return "F6";
	case sf::Keyboard::F7: return "F7";
	case sf::Keyboard::F8: return "F8";
	case sf::Keyboard::F9: return "F9";
	case sf::Keyboard::F10: return "F10";
	case sf::Keyboard::F11: return "F11";
	case sf::Keyboard::F12: return "F12";
	case sf::Keyboard::F13: return "F13";
	case sf::Keyboard::F14: return "F14";
	case sf::Keyboard::F15: return "F15";
	case sf::Keyboard::Pause: return "Paues";
	default: return "Error";
	}
}

void CProgram::Save()
{
	std::ofstream output_file("save.json");

	if (!output_file.good())
		abort();

	sf::Vector2u asd = this->GetFinalWindowSize();
	std::cout << "\nFinalWindowSize: " << asd.x << ",  " << asd.y << std::endl;
	std::cout << "relativeZeroPoint: " << this->relativeZeroPoint.x << ",  " << this->relativeZeroPoint.y << std::endl;

	this->SaveKeysPos();
	this->SaveStatKeysPos();
	this->SaveMouseKeysPos();

	output_file << std::setw(4) << this->savej << std::endl;
	output_file.close();
}

void CProgram::SaveKeysPos()
{
	/*if (this->keys.size() == 0)
		this->savej["keys"]["empty"] = true;
	else
		this->savej["keys"]["empty"] = false;*/

	this->savej["configs"] = this->configsDropDown_Options.size();	//num of configs

	std::string config = this->configsDropDown_Options[this->selectedConfig];
	if(this->applyedConfig != -1)
		this->savej["currConfig"] = this->configsDropDown_Options[this->applyedConfig];

	this->savej[config]["changed"] = true;

	sf::Vector2u wSize = this->GetFinalWindowSize();
	this->savej[config]["windowSize"] = { {"x", wSize.x}, { "y", wSize.y } };

	this->savej[config]["keylist"] = nullptr;
	for (auto &key : this->keys)
		this->savej[config]["keylist"] += key.GetString();

	this->savej[config]["keys"] = nullptr;
	for (auto &key : this->keys) {
		sf::Vector2f keypos = key.GetPos();
		sf::Vector2f rKeypos = this->GetRelativeKeyPos(keypos);
		this->savej[config]["keys"][key.GetString()] = { {"x", keypos.x}, {"y", keypos.y},
														 {"rx", rKeypos.x}, {"ry", rKeypos.y},
														 {"w", key.GetBounds().width},  {"h", key.GetBounds().height},
														 {"code", key.GetKeyCode() },
														 {"sfml_code", key.GetSFMLKeyCode() } };
	}
}

void CProgram::SaveStatKeysPos()
{
	std::string config = this->configsDropDown_Options[this->selectedConfig];

	this->savej[config]["statKeylist"] = nullptr;
	for (auto &key : this->statKeys)
		this->savej[config]["statKeylist"] += key.GetString();

	this->savej[config]["statKeys"] = nullptr;
	for (auto &key : this->statKeys) {
		sf::Vector2f keypos = key.GetPos();
		sf::Vector2f rKeypos = this->GetRelativeKeyPos(keypos);
		this->savej[config]["statKeys"][key.GetString()] = { {"x", key.GetPos().x}, {"y", key.GetPos().y},
															 {"rx", rKeypos.x}, {"ry", rKeypos.y},
															 {"w", key.GetBounds().width},  {"h", key.GetBounds().height} };
	}
}

void CProgram::SaveMouseKeysPos()
{
	std::string config = this->configsDropDown_Options[this->selectedConfig];

	this->savej[config]["mouseKeylist"] = nullptr;
	for (auto &key : this->mouseKeys)
		this->savej[config]["mouseKeylist"] += key.GetString();

	this->savej[config]["mouseKeys"] = nullptr;
	for (auto &key : this->mouseKeys) {
		sf::Vector2f keypos = key.GetPos();
		sf::Vector2f rKeypos = this->GetRelativeKeyPos(keypos);
		this->savej[config]["mouseKeys"][key.GetString()] = { {"x", key.GetPos().x}, {"y", key.GetPos().y},
															 {"rx", rKeypos.x}, {"ry", rKeypos.y},
															 {"w", key.GetBounds().width},  {"h", key.GetBounds().height},
															 {"code", key.GetSFMLKeyCode() } };
	}
}

void CProgram::LoadConfig()
{
	std::ifstream input_file("save.json");

	if (!input_file.good())
		abort();
	input_file >> this->savej;

	this->LoadKeysPos();
	this->LoadStatKeysPos();
	this->LoadMouseKeysPos();

	input_file.close();
	//this->savej.clear();

	this->needAutoSave = true;
}

void CProgram::LoadKeysPos()
{
	this->keys.clear();
	this->existingKeysLookup.clear();

	std::string config = this->configsDropDown_Options[this->selectedConfig];

	nlohmann::json arr = this->savej[config]["keylist"];

	for (size_t i = 0; i < arr.size(); i++)
	{
		std::string key = arr[i];
		float x = this->savej[config]["keys"][key]["x"].get<float>();
		float y = this->savej[config]["keys"][key]["y"].get<float>();

		this->keys.push_back(KeyboardKey({ x, y }, { 80.f, 80.f }, key, this->font));
		int keycode = this->savej[config]["keys"][key]["code"].get<int>();
		this->keys.back().SetKeyCode(keycode);
		int sfmlKeycode = this->savej[config]["keys"][key]["sfml_code"].get<int>();
		this->keys.back().SetSFMLKeyCode(sfmlKeycode);
		this->existingKeysLookup[sfmlKeycode] = true;
		this->keys.back().Released();
	}

	//std::string asd = this->savej[config]["keylist"][0].get<std::string>();

	//std::cout << "arr size: " << arr.size() << std::endl;
}

void CProgram::LoadStatKeysPos()
{
	this->statKeys.clear();
	this->existingStatKeysLookup.clear();
	std::string config = this->configsDropDown_Options[this->selectedConfig];

	nlohmann::json arr = this->savej[config]["statKeylist"];

	for (size_t i = 0; i < arr.size(); i++)
	{
		std::string name = arr[i];
		float x = this->savej[config]["statKeys"][name]["x"].get<float>();
		float y = this->savej[config]["statKeys"][name]["y"].get<float>();

		this->statKeys.push_back(KeyboardKey({ x, y }, { 120.f, 80.f }, name, this->font));
		this->statKeys.back().Released();

		switch (*name.begin())
		{
		case 'C': this->existingStatKeysLookup[ADDCURKEY] = true; break;
		case 'T': this->existingStatKeysLookup[ADDTOTKEY] = true;  break;
		default: break;
		}
	}
}

void CProgram::LoadMouseKeysPos()
{
	this->mouseKeys.clear();
	this->existingMouseButtonLookup.clear();

	std::string config = this->configsDropDown_Options[this->selectedConfig];

	nlohmann::json arr = this->savej[config]["mouseKeylist"];

	for (size_t i = 0; i < arr.size(); i++)
	{
		std::string key = arr[i];
		float x = this->savej[config]["mouseKeys"][key]["x"].get<float>();
		float y = this->savej[config]["mouseKeys"][key]["y"].get<float>();

		this->mouseKeys.push_back(KeyboardKey({ x, y }, { 80.f, 80.f }, key, this->font));
		int keycode = this->savej[config]["mouseKeys"][key]["code"].get<int>();
		this->mouseKeys.back().SetSFMLKeyCode(keycode);
		this->mouseKeys.back().Released();
		this->existingMouseButtonLookup[keycode] = true;
	}
}

sf::Vector2u CProgram::GetFinalWindowSize() //window size based on the keys position
{
	float windowTop = FLT_MAX;
	float windowLeft = FLT_MAX;
	
	float windowButtom = 0.f;
	float windowRight = 0.f;

	auto checkKeys = [&](std::vector<KeyboardKey>& keysToCheck) -> void {
		for (auto &key : keysToCheck) {
			sf::FloatRect bounds = key.GetBounds();
			if (windowTop > bounds.top)
				windowTop = bounds.top;

			if (windowLeft > bounds.left)
				windowLeft = bounds.left;

			if (windowButtom < bounds.top + bounds.height)
				windowButtom = bounds.top + bounds.height;

			if (windowRight < bounds.left + bounds.width)
				windowRight = bounds.left + bounds.width;
		}
	};

	checkKeys(this->keys);
	checkKeys(this->statKeys);
	checkKeys(this->mouseKeys);

	this->relativeZeroPoint = { windowLeft, windowTop };

	return sf::Vector2u(windowRight - windowLeft, windowButtom - windowTop);
}

sf::Vector2f CProgram::GetRelativeKeyPos(sf::Vector2f& pos)	//relative key pos to FinalWindowSize
{
	return sf::Vector2f(pos.x - this->relativeZeroPoint.x, pos.y - this->relativeZeroPoint.y);
}
