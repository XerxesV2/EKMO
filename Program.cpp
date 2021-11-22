#include "Program.hpp"
#include <Windows.h>

#include <Dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

Program::Program() : window(sf::VideoMode(800, 400), "KadofaiK", sf::Style::None)
{
	//setlocale(LC_ALL, "");
	window.setFramerateLimit(60);
	window.setPosition(sf::Vector2i(0, 1760));
	this->font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
	
	this->MakeWindowTransparent();
	this->MakeWindowOnTop(this->window);

	offset.x = 80.f;
	AddKey(sf::Vector2f(0.f, 150.f), 'A');
	AddKey(sf::Vector2f(0.f, 150.f), 'S');
	AddKey(sf::Vector2f(0.f, 150.f), 'D');
	AddKey(sf::Vector2f(310.f, 250.f), 'V');
	AddKey(sf::Vector2f(430.f, 250.f), 'M');
	AddKey(sf::Vector2f(480.f, 150.f), 'L');
	AddKey(sf::Vector2f(0.f, 150.f), 'É');
	AddKey(sf::Vector2f(0.f, 150.f), 'Á');

	AddStatKey(sf::Vector2f(184.f, 250.f), "CUR");
	AddStatKey(sf::Vector2f(556.f, 250.f), "TOT");

	this->startTime = this->avghitsTimer.getElapsedTime();
	this->tick = this->avghitsTimer.getElapsedTime().asSeconds();
}

Program::~Program()
{
}

void Program::MainLoop()
{
	while (this->window.isOpen())
	{
		this->HandleEvents();

		this->window.clear(sf::Color(0, 0, 0, 0));
		//this->window.clear();

		this->Update();
		this->Draw();

		window.display();
	}
}

void Program::Update()
{
	this->HandleKeyInputs();
	this->HandleStats();
}

void Program::Draw()
{
	//this->window.clear(sf::Color(0,0,0,0));

	//this->window.clear();
	int i = 0;
	for (auto &k : this->keyList) {
		if(this->repeatProtection[i])
			this->keys[k]->Draw(this->window);
		++i;
	}

	//this->window.display();
}

void Program::HandleEvents()
{
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			window.close();
	}
}

void Program::HandleKeyInputs()
{
	//-65
	int i = 0;
	for (auto &k : this->keyList) {
		
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key(k - GetCorrectOffsetForKey(k)))) {
			this->keys[k]->Update(this->window);
			if (this->repeatProtection[i])
				++this->totalHits;
			this->repeatProtection[i] = false;
		}
		else if (!this->repeatProtection[i]) {
			this->keys[k]->Released();
			this->repeatProtection[i] = true;
		}
		++i;
	}
}

void Program::HandleStats()
{
	static unsigned long long prevTOT = 0;

	//auto temp = (unsigned long long)(this->avghitsTimer.getElapsedTime().asSeconds() - this->startTime.asSeconds());
	if (this->avghitsTimer.getElapsedTime().asSeconds() > this->tick) {
		this->avgHits = this->totalHits - prevTOT;

		this->tick += 1.f;
		prevTOT = this->totalHits;

	}
	this->statKeys[CUR]->SetStat(this->avgHits);
	this->statKeys[TOT]->SetStat(this->totalHits);

	this->statKeys[CUR]->Draw(this->window);
	this->statKeys[TOT]->Draw(this->window);
}

void Program::HideConsole()
{
	ShowWindow(GetConsoleWindow(), SW_HIDE);
}

void Program::MakeWindowOnTop(sf::RenderWindow& window) {
	HWND hwnd = window.getSystemHandle();
	SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Program::MakeWindowTransparent()
{
	MARGINS margins;
	DwmExtendFrameIntoClientArea(this->window.getSystemHandle(), &margins);
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
	this->statKeys.push_back(new KeyboardKey(pos, this->statKeySize, text, this->font));
	this->statKeys.back()->Released();
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
