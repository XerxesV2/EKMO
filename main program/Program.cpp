#include "Program.hpp"
#include <Windows.h>
#include <thread>
#include <math.h>

#include <Dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

#define SCREEN_X sf::VideoMode::getDesktopMode().width
#define SCREEN_Y sf::VideoMode::getDesktopMode().height

#define WINDOWS_X 1920
#define WINDOWS_Y 1080

using namespace std::chrono_literals;
constexpr std::chrono::nanoseconds timeStep(17ms);

Program::Program() : window(sf::VideoMode(SCREEN_X / 2, SCREEN_Y / 2), "KadofaiK", sf::Style::None),
view(sf::Vector2f(1920 / 2, 1080 / 2), sf::Vector2f(1920, 1080))
{
	//setlocale(LC_ALL, "");
	this->font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
	this->LoadConfig();
	this->InitWindow();
	
	this->MakeWindowTransparent();
	this->MakeWindowOnTop(this->window);

	this->LoadStat();

	


	/*offset.x = 80.f;
	AddKey(sf::Vector2f(0.f, 150.f), 'A');
	AddKey(sf::Vector2f(0.f, 150.f), 'S');
	AddKey(sf::Vector2f(0.f, 150.f), 'D');
	AddKey(sf::Vector2f(310.f, 250.f), 'V');
	AddKey(sf::Vector2f(430.f, 250.f), 'M');
	AddKey(sf::Vector2f(480.f, 150.f), 'L');
	AddKey(sf::Vector2f(0.f, 150.f), 'É');
	AddKey(sf::Vector2f(0.f, 150.f), 'Á');

	AddStatKey(sf::Vector2f(184.f, 250.f), "CUR");
	AddStatKey(sf::Vector2f(556.f, 250.f), "TOT");*/


	this->startTime = this->avghitsTimer.getElapsedTime();
	this->tick = this->avghitsTimer.getElapsedTime().asSeconds();
	currentTime = std::chrono::high_resolution_clock::now();
}

Program::~Program()
{
}

int GetFrameRate() {
	static sf::Clock clock;
	static int frames = 0;
	static int prevFPS = 0;

	if (clock.getElapsedTime().asSeconds() >= 1.f) {
		prevFPS = frames;
		frames = 0;
		clock.restart();
	}
	++frames;

	return prevFPS;
}

void Program::MainLoop()
{
	std::chrono::nanoseconds deltaTime(0ns);

	while (this->window.isOpen())
	{
		deltaTime += std::chrono::high_resolution_clock::now() - currentTime;
		currentTime = std::chrono::high_resolution_clock::now();

		while (deltaTime > timeStep) {
			this->HandleEvents();
			
			//this->window.clear();

			this->Update();

			if (needReDraw) this->drawAFrame = true;	//this is baaaad
			if (this->drawAFrame)
				this->window.clear(sf::Color(0, 0, 0, 0));
				//this->window.clear();

			this->HandleStats();
			this->Draw();

			//this->window.setTitle(std::to_string(GetFrameRate()));

			//std::this_thread::sleep_for(std::chrono::microseconds(1000));
			//std::cout << std::to_string(GetFrameRate()) << std::endl;
			deltaTime -= timeStep;
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1));
	}
}

void Program::Update()
{
	this->HandleKeyInputs();
	//this->HandleStats();
	this->ClickThrough();
	this->MoveWindow();
	this->ResizeWindow();
}

void Program::Draw()
{
	if (!this->drawAFrame) return;

	//this->window.clear(sf::Color(0,0,0,0));

	//this->window.clear();
	for (auto &k : this->keyList) {
		this->keys[k]->Draw(this->window);
	}

	for (auto &k : this->mouseKeyList) {
		this->mouseKeys[k]->Draw(this->window);
	}

	//std::cout << "draw" << std::endl;
	this->drawAFrame = false;
	this->needReDraw = false;

	this->window.display();
}

void Program::HandleEvents()
{
	while (window.pollEvent(event))
	{
		switch (event.type)
		{
		case sf::Event::Closed: this->SaveStat(); window.close(); break;
		//case sf::Event::MouseEntered: SetWindowPos(window.getSystemHandle(), HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); break;
		//case sf::Event::MouseLeft: SetWindowPos(window.getSystemHandle(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE); break;
		default: break;
		} 
	}
}

void Program::HandleKeyInputs()
{
	//-65
	int i = 0;
	for (auto &k : this->keyList) {
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(k))) {
			this->keys[k]->Update(this->window);
			if (this->repeatProtection[i]) {
				++this->totalHits;
				this->repeatProtection[i] = false;
				this->needReDraw = true;
			}
		}
		else if (!this->repeatProtection[i]) {
			this->keys[k]->Released();
			this->repeatProtection[i] = true;
			this->needReDraw = true;
		}
		++i;
	}

	for (auto &k : this->mouseKeyList) {

		if (sf::Mouse::isButtonPressed(sf::Mouse::Button(k))) {
			this->mouseKeys[k]->Update(this->window);
			if (this->repeatProtection[i]) {
				++this->totalHits;
				this->repeatProtection[i] = false;
				this->needReDraw = true;
			}
		}
		else if (!this->repeatProtection[i]) {
			this->mouseKeys[k]->Released();
			this->repeatProtection[i] = true;
			this->needReDraw = true;
		}
		++i;
	}
}

void Program::HandleStats()
{
	if (this->statKeys.size() == 0) return;

	static int prevTOT = this->totalHits;

	static constexpr int URATE = 10;
	static int parts[URATE] = {0};
	static auto sumArray = [](int *parts, int size) -> int
	{
		int sum = 0;
		for (size_t i = 0; i < size; i++)
		{
			sum += parts[i];
		}
		return sum;
	};
	static auto shiftArray = [](int *parts, int size)
	{
		for (size_t i = 0; i < size-1; i++)
		{
			parts[i] = parts[i + 1];
		}
	};

	if (this->avghitsTimer.getElapsedTime().asSeconds() > this->tick) {
		this->avgHits = (int)this->totalHits - prevTOT;

		this->tick += 0.1f;
		prevTOT = (int)this->totalHits;

		shiftArray(parts, URATE);
		parts[URATE - 1] = this->avgHits;
		this->needReDraw = true;
	}

	//auto temp = (unsigned long long)(this->avghitsTimer.getElapsedTime().asSeconds() - this->startTime.asSeconds());
	/*if (this->avghitsTimer.getElapsedTime().asSeconds() > this->tick) {
		this->avgHits = this->totalHits - prevTOT;

		this->tick += 1.f;
		prevTOT = this->totalHits;

	}*/

	this->statKeys[CUR].SetStatKey((unsigned long long)sumArray(parts, URATE));
	this->statKeys[TOT].SetStatKey(this->totalHits);

	this->statKeys[CUR].Draw(this->window);
	this->statKeys[TOT].Draw(this->window);
}

void Program::InitWindow()
{
	//this->window.setVerticalSyncEnabled(false);
	//this->window.setFramerateLimit(60);
	this->window.setPosition(sf::Vector2i(0, SCREEN_Y - window.getSize().y));
}

void Program::HideConsole()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
}

void Program::MakeWindowOnTop(sf::RenderWindow& window) {
	HWND hwnd = window.getSystemHandle();
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	
	/*LONG retv;
	retv = SetWindowLongPtr(hwnd, GWL_STYLE, WS_EX_LAYERED | WS_EX_TRANSPARENT);
	//retv = SetWindowLongPtr(hwnd, GWL_STYLE, WS_EX_COMPOSITED | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST);
	retv = SetLayeredWindowAttributes(hwnd, 0, char(128), LWA_ALPHA);
	std::cout << retv << std::endl;
	std::cout << GetLastError() << std::endl;*/


	//UpdateWindow(hwnd);
	//UpdateLayeredWindow(,);
	//SetCapture(hwnd);
	//ShowCursor(false);

	//SetLayeredWindowAttributes(hwnd, 0, 0, );
}

void Program::MakeWindowTransparent()
{
	MARGINS margins;
	DwmExtendFrameIntoClientArea(this->window.getSystemHandle(), &margins);
}

void Program::ClickThrough()
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) return;

	static HWND hwnd = window.getSystemHandle();
	sf::Vector2u WINDOWS = this->window.getSize();
	static bool repeatGuard = false;

	sf::IntRect windowRect(this->window.getPosition(), (sf::Vector2i)WINDOWS);

	if (windowRect.contains(sf::Mouse::getPosition())) {
		if (repeatGuard == false) {
			SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			repeatGuard = true;
		}
	}
	else {
		if (repeatGuard == true) {
			SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			repeatGuard = false;
		}
	}
}

void Program::MoveWindow()
{
	static bool dragging = false;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
		sf::IntRect wBounds = { this->window.getPosition(), sf::Vector2i(this->window.getSize()) };
		if (dragging || wBounds.contains(sf::Mouse::getPosition()) && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			this->window.setPosition(sf::Mouse::getPosition() - sf::Vector2i( this->window.getSize().x / 2, this->window.getSize().y / 2));
			dragging = true;
		}
		if (!sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
			dragging = false;
		}
	}
	else if (dragging == true) dragging = false;
}

void Program::ResizeWindow()
{
	static bool needRecreate = false;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {

		unsigned x = this->window.getSize().x;
		unsigned y = this->window.getSize().y;

		x /= 10;
		y /= 10;

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) 
			this->window.setSize(this->window.getSize() + sf::Vector2u(x, y));
		
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
			this->window.setSize(this->window.getSize() - sf::Vector2u(x, y));

		needRecreate = true;
	}
	//this->window.setView(this->view); 
	this->needReDraw = true;
}

void Program::AddKey(sf::Vector2f pos, char key)
{

	float temp = pos.x;
	if (pos.x == 0.f) 
		pos.x = offset.x;

	this->keyList.push_back(key);
	this->keys.insert({ key, new KeyboardKey(pos, this->keySize, key, this->font) });

	this->repeatProtection.push_back(false);

	if (temp == 0.f)
		offset.x += this->keySize.x + 20.f;
	else
		offset.x = pos.x + (this->keySize.x + 20.f);
}

void Program::AddStatKey(sf::Vector2f pos, std::string text)
{
	this->statKeys.push_back(KeyboardKey(pos, this->statKeySize, text, this->font));
	this->statKeys.back().Released();
}

void Program::RemoveKey(char key)
{
	delete this->keys[key];
	this->keys.erase(key);
}

int Program::GetCorrectOffsetForKey(char key)
{
	if (key < 0) {
		switch (key)
		{
		case 'É': return -103; //;
		case 'Á': return -114; //'
		default: return -1;
		}
	}
	else
		return 65;
}

void Program::SaveStat()
{
	std::ofstream ofile(this->loadedConfig + ".bin", std::ios::out | std::ios::binary);

	if (!ofile || !ofile.is_open()) {
		std::cout << "cannot open file" << std::endl;
		__debugbreak();
	}
	
	ofile.write((char *)&this->window.getSize(), sizeof(sf::Vector2u));
	ofile.write((char *)&this->window.getPosition(), sizeof(sf::Vector2i));
	//std::cout << this->window.getPosition().x << std::endl;

	//for (auto const&[keyChar, keyClass] : this->keys) {
	for(auto &key : this->keyList){
		//ofile.write(&keyChar, sizeof(char));
		ofile.write((char *) &this->keys[key]->GetHits(), sizeof(unsigned long long));
		ofile.write((char *) &key, sizeof(int));
	}

	for (auto &key : this->mouseKeyList) {
		ofile.write((char *) &this->mouseKeys[key]->GetHits(), sizeof(unsigned long long));
		ofile.write((char *) &key, sizeof(int));
	}

	ofile.write((char *) &this->totalHits, sizeof(unsigned long long));
	
	ofile.close();
	if (!ofile.good()) {
		std::cout << "file write error" << std::endl;
		__debugbreak();
	}
}

void Program::LoadStat()
{
	this->ifile.open(this->loadedConfig + ".bin", std::ios::in | std::ios::binary);
	if (!this->ifile || !this->ifile.is_open()) {
		std::cout << "cannot open file" << std::endl;
		//__debugbreak();
		return;
	}

	this->ifile.seekg(0, this->ifile.end);
	ifileLength = this->ifile.tellg();
	this->ifile.seekg(0, this->ifile.beg);

	if (this->ifileLength > 0) {
		sf::Vector2u size;
		sf::Vector2i pos;
		this->ifile.read((char *)&size, sizeof(sf::Vector2u));
		this->ifileLength -= sizeof(sf::Vector2u);
		this->ifile.read((char *)&pos, sizeof(sf::Vector2i));
		this->ifileLength -= sizeof(sf::Vector2i);
		if (!this->configChanged) 
			this->window.setSize(size);
		else 
			this->UpdateJsonFlags();
		
		this->window.setPosition(pos);
	}

	for (auto &key : this->keyList) {
		if (this->ifileLength > 12) {
			unsigned long long hits;
			this->ifile.read((char *)&hits, sizeof(unsigned long long));
			int code;
			this->ifile.read((char *)&code, sizeof(int));

			if (code == key) {
				this->keys[key]->SetHits(hits);
				this->ifileLength -= sizeof(unsigned long long);
				this->ifileLength -= sizeof(int);
			}
			else {
				//std::cout << "tellg: " << this->ifile.tellg() << std::endl;
				this->keys[key]->SetHits(0);
				//this->ifile.seekg(-(int)(sizeof(unsigned long long) + sizeof(int)), this->ifile.cur);
				this->ifile.seekg(-this->ifileLength, std::ios::end);
				std::cout << "No match seek back" << std::endl;
			}
		}
	}

	unsigned long long hitloss = 0;
	while (this->ifileLength > (this->mouseKeyList.size() * 12)+8) { 
		unsigned long long hits;
		this->ifile.read((char *)&hits, sizeof(unsigned long long));
		int code;
		this->ifile.read((char *)&code, sizeof(int));
		this->ifileLength -= sizeof(unsigned long long);
		this->ifileLength -= sizeof(int);
		hitloss += hits;
	}

	for (auto &key : this->mouseKeyList) {
		if (this->ifileLength > 12) {
			unsigned long long hits;
			this->ifile.read((char *)&hits, sizeof(unsigned long long));
			int code;
			this->ifile.read((char *)&code, sizeof(int));

			if (code == key) {
				this->mouseKeys[key]->SetHits(hits);
				this->ifileLength -= sizeof(unsigned long long);
				this->ifileLength -= sizeof(int);
			}
			else {
				this->mouseKeys[key]->SetHits(0);
				this->ifile.seekg(-this->ifileLength, std::ios::end);
				std::cout << "No match seek back" << std::endl;
			}
		}
	}

	while (this->ifileLength > 8) {
		unsigned long long hits;
		this->ifile.read((char *)&hits, sizeof(unsigned long long));
		int code;
		this->ifile.read((char *)&code, sizeof(int));
		hitloss += hits;

		this->ifileLength -= sizeof(unsigned long long);
		this->ifileLength -= sizeof(int);
	}
	this->ifile.read((char *)&this->totalHits, sizeof(unsigned long long));
	this->totalHits -= hitloss;
	this->ifileLength -= sizeof(unsigned long long);
	for (auto &key : this->statKeys) {
		if (key.GetString() == "TOT") {
			key.SetStatKey(this->totalHits);
		}
	}

	this->ifile.close();
	if (!this->ifile.good()) {
		std::cout << "file read error" << std::endl;
	}
}

void Program::LoadConfig()
{
	std::ifstream input_file("save.json");

	if (!input_file.good())
		abort();
	input_file >> this->loadj;

	std::string config = this->loadj["currConfig"].get<std::string>();
	this->loadedConfig = config;

	sf::Vector2u windowSize = { this->loadj[config]["windowSize"]["x"].get<unsigned>(),
								this->loadj[config]["windowSize"]["y"].get<unsigned>() };

	this->window.setSize(windowSize);
	this->view.setSize(sf::Vector2f(windowSize));
	this->view.setCenter(windowSize.x / 2, windowSize.y / 2);
	
	this->window.setView(this->view);


	/*if (this->ifileLength > 0) {
		unsigned long long hits;
		this->ifile.read((char *)&hits, sizeof(unsigned long long));
		this->keys[key]->SetHits(hits);
		this->ifileLength -= sizeof(unsigned long long);
	}*/

	// ------ keys ------- //
	nlohmann::json keyArr = this->loadj[config]["keylist"];

	//this->keys.insert({ key, new KeyboardKey(pos, this->keySize, key, this->font) });

	for (size_t i = 0; i < keyArr.size(); i++)
	{
		std::string key = keyArr[i];
		this->keyList.push_back(this->loadj[config]["keys"][key]["code"].get<int>());

		float x = this->loadj[config]["keys"][key]["rx"].get<float>();
		float y = this->loadj[config]["keys"][key]["ry"].get<float>();

		this->keys.insert({ this->keyList.back(), new KeyboardKey({ x, y }, {80.f, 80.f}, key, this->font) });

		//this->keys.back().Released();
	}

	// ------ stat keys -------//

	nlohmann::json statKeyArr = this->loadj[config]["statKeylist"];

	for (size_t i = 0; i < statKeyArr.size(); i++)
	{
		std::string name = statKeyArr[i];
		float x = this->loadj[config]["statKeys"][name]["rx"].get<float>();
		float y = this->loadj[config]["statKeys"][name]["ry"].get<float>();

		this->statKeys.push_back(KeyboardKey({ x, y }, { 120.f, 80.f }, name, this->font));
		this->statKeys.back().Released();
	}

	// ------ mouse keys -------//

	nlohmann::json mouseKeyArr = this->loadj[config]["mouseKeylist"];

	for (size_t i = 0; i < mouseKeyArr.size(); i++)
	{
		std::string key = mouseKeyArr[i];
		this->mouseKeyList.push_back(this->loadj[config]["mouseKeys"][key]["code"].get<int>());

		float x = this->loadj[config]["mouseKeys"][key]["rx"].get<float>();
		float y = this->loadj[config]["mouseKeys"][key]["ry"].get<float>();

		this->mouseKeys.insert({ this->mouseKeyList.back(), new KeyboardKey({ x, y }, {80.f, 80.f}, key, this->font) });
	}

	this->configChanged = this->loadj[config]["changed"].get<bool>();

	input_file.close();
}

void Program::UpdateJsonFlags()
{
	std::ofstream output_file("save.json");

	if (!output_file.good())
		abort();

	this->loadj[this->loadedConfig]["changed"] = false;
	this->configChanged = false;

	output_file << std::setw(4) << this->loadj << std::endl;
	output_file.close();
}
