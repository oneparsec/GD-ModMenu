#define DEVELOPER_MODE TRUE
#define VERSION "1.1-alpha"
#define MBOT_VERSION "DEV_RELEASE"
#define _CRT_SECURE_NO_WARNINGS


#ifndef UNICODE
    #define LOADER_STR string
#else
    #define LOADER_STR wstring
#endif

#ifdef _MSC_VER
#pragma warning(disable: 4251) // disable warning 4251
#pragma warning(disable: 4244) // possible loss of data warning
#endif

#pragma comment(lib,"opengl32.lib")

#include <Windows.h>
#include "json.hpp"
#include "main.hpp"
#include "mBot.h"
#include <imgui-hook.hpp>
#include <imgui.h>
#include <MinHook.h>
#include <gd.h>
#include <iostream>
#include <Psapi.h>
#include <fstream>
#include <cocos2d.h>
#include <cocos-ext.h>
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <shellapi.h>
#include <chrono>
#include <ctime>
#include <thread>
#include <commdlg.h>
#include <deque>
#include <variant>
#include <string>
#include <cassert>
#include <iostream>


static ImFont* g_font = nullptr;

static bool show = false;
static bool showDemoWindow = false;



std::string nofilename = "false";

const char * transitions[]{"Fade", "CrossFade", "FadeBL", "FadeDown", "FadeTR", "FadeUp", "FlipAngular", "FlipX", "FlipY", "JumpZoom", "MoveInB", "MoveInL", "MoveInR", "MoveInT", "RotoZoom", "ShrinkGrow", "SlideInB", "SlideInL", "SlideInR", "SlideInT", "SplitCols", "SplitRows", "TurnOffTiles", "ZoomFlipAngular", "ZoomFlipX", "ZoomFlipY", "PageTurn", "ProgressHorizontal", "ProgressInOut", "ProgressOutIn", "ProgressRadialCCW", "ProgressRadialCW", "ProgressVertical"};
const std::vector<uint32_t> transitionaddr = { 0xA53D0, 0xA5320, 0xA54F0, 0xA55C0, 0xA5690, 0xA5760, 0xA5830, 0xA5950, 0xA5A70, 0xA5B90, 0xA5C40, 0xA5D10, 0xA5DE0, 0xA5EB0, 0xA5F80, 0xA6170, 0xA6240, 0xA6310, 0xA63E0, 0xA64B0, 0xA6580, 0xA6650, 0xA6720, 0xA67F0, 0xA6910, 0xA6A30, 0xA8D50, 0xA91D0, 0xA92A0, 0xA9370, 0xA9440, 0xA9510, 0xA95E0 };

int totalClicks = 0;
int midClickCount = 0, actualClickCount = 0;
int lastAsyncKeyStateValue = 1337;

bool sortPlayer = true;
bool sortBypass = true;
bool sortSpeedhack = true;
bool sortFPSBypass = true;
bool sortUniversal = true;
bool sortCreator = true;
bool sortStatus = true;
bool sortOther = true;
bool sortAbout = true;
bool sortMBot = true;


auto win_size = director->getWinSize();
bool inLevel = false;

bool inPractice;

// mbot stuff
static struct {
	int frame; // frame counter
	double macroFps;
	bool pushedMouse = false;
	bool releasedMouse = false;
	int mode; // 0 for disabled, 1 for record and 2 for playback
	std::map<std::string, std::variant<std::deque<bool>>> PlayerData = {
		{"Pushed", std::deque<bool>{}},
		{"Released", std::deque<bool>{}}
	};
	bool waitForFirstClick;
} mBotVars;

static struct {
	bool wouldDie = false;
	int frames = 0;
	int deaths = 0;
	float totalDelta = 0;
	float prevX = 0;
} noclipacc;

// startpos fix by fig
static struct {
	bool inPractice;
	bool inTestmode;
	int smoothOut;
} startposFix;


using namespace cocos2d;
using namespace nlohmann;
using namespace cocos2d::extension;


uint32_t cocosBase = GetModuleBase("libcocos2d.dll");

typedef void*   (__cdecl *fSharedApplication)();
typedef void    (__thiscall *fSetAnimationInterval)(void *instance, double delay);
fSharedApplication sharedApplication;
fSetAnimationInterval setAnimInterval;


HMODULE ccdll;

GLFWwindow* (__cdecl *host_glfwCreateWindow)(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) = nullptr;

GLFWwindow* __cdecl hook_glfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share) {
    // Set GLFW_SAMPLES Window Hint to 4 to enable MSAAx4
    *as<int*>((unsigned int)ccdll + 0x1a14e8) = 4;

    return host_glfwCreateWindow(width, height, title, monitor, share);
}

void (_fastcall *host_CCDirector_drawScene)(CCDirector* _this) = nullptr;

void _fastcall hook_CCDirector_drawScene(CCDirector* _this) {
    // Enable multisampling
    glEnable(GL_MULTISAMPLE);

    host_CCDirector_drawScene(_this);
}



bool loadMSAA() {
    ccdll = GetModuleHandleA("libcocos2d.dll");

    // Get the address for CCDirector::drawScene
    auto drawSceneAddr = GetProcAddress(ccdll, "?drawScene@CCDirector@cocos2d@@QAEXXZ");

    MH_CreateHook(
        drawSceneAddr,
        as<LPVOID>(hook_CCDirector_drawScene),
        as<LPVOID*>(&host_CCDirector_drawScene)
    );
    // Address to glfwCreateWindow in the cocos2d dll
    auto cwinaddr = as<LPVOID>((unsigned int)ccdll + 0x110f50);

    MH_CreateHook(
		cwinaddr,
        as<LPVOID>(hook_glfwCreateWindow),
        as<LPVOID*>(&host_glfwCreateWindow)
    );
    MH_EnableHook(drawSceneAddr);
    return true;
}

const char * filter = "Dynamic link library (*.dll)\0*.dll";

void saveHacks()
{
	auto file = fopen("mod-menu.dat", "wb");
	if (file) {
		fwrite(&setting, sizeof(setting), 1, file);
		fclose(file);
	}
}

void loadHacks(){

	auto file = fopen("mod-menu.dat", "rb");
	if (file) {
		fseek(file, 0, SEEK_END);
		auto size = ftell(file);

		if (size == sizeof(setting)) {
			fseek(file, 0, SEEK_SET);
			fread(&setting, sizeof(setting), 1, file);
			fclose(file);
		}
	}
}

void SetTargetFPS(float interval){
	interval = 1.0f / interval;

	HMODULE hMod = LoadLibrary(LPCSTR("libcocos2d.dll"));
	sharedApplication = (fSharedApplication)GetProcAddress(hMod, "?sharedApplication@CCApplication@cocos2d@@SAPAV12@XZ");
	setAnimInterval = (fSetAnimationInterval)GetProcAddress(hMod, "?setAnimationInterval@CCApplication@cocos2d@@UAEXN@Z");

	void *application = sharedApplication();
	setAnimInterval(application, interval);
}
std::string chooseDLL() //dll
{
    OPENFILENAME ofn;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn, sizeof(ofn));
	ofn.lpstrFilter = filter;
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    if (GetOpenFileName(&ofn))
	{
        return fileName;
	} else {
		return nofilename;
	}

}
std::string getAccuracyText() {
	if (noclipacc.frames == 0) return "Accuracy: 100.00%";
	float p = (float)(noclipacc.frames - noclipacc.deaths) / (float)noclipacc.frames;
	std::stringstream stream;
	stream << "Accuracy: " << std::fixed << std::setprecision(2) << p * 100.f << "%";
	return stream.str();
}

std::string getFramerateText(){
	std::stringstream stream;
	stream << round(ImGui::GetIO().Framerate)<< " FPS";
	return stream.str();
}



void __fastcall LoadingLayer::initHook(cocos2d::CCLayer* _layer, void*, char _bool) {
    init_(_layer, _bool);

    std::stringstream infoText;

    infoText << "ModMenu " << VERSION;
    auto label = cocos2d::CCLabelBMFont::create(
        infoText.str().c_str(),
        "goldFont.fnt"
    );

    label->setScale(.4f);

    auto winSize = director->getWinSize();

    label->setPosition(
        winSize.width / 2,
        20
    );

    _layer->addChild(label);

    return;
}

void LoadingLayer::mem_init() {
	

	size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));
	MH_CreateHook(
		(PVOID)(base + 0x18c080),
		LoadingLayer::initHook,
		(LPVOID*)&LoadingLayer::init_
	);
	MH_EnableHook(MH_ALL_HOOKS);
}
bool __fastcall PlayLayer::initHook(gd::PlayLayer* self, int edx, void* GJGameLevel) {
	
	size_t base = (size_t)GetModuleHandle(0);
	inLevel = true;
	totalClicks = 0;
	noclipacc.prevX = 0;
	startposFix.inPractice = false;
	startposFix.inTestmode = *(bool*)((uintptr_t)self + 0x494);
	startposFix.smoothOut = 0;
	const CCSize win_size = director->getWinSize();
	//void* player1 = *(void**)((char*)self + 0x224);
	//fromPercent.XPositionition = *(float*)((size_t)player1 + 0x67c);
	//CCLabelBMFont* textObj = (CCLabelBMFont*)self->getChildByTag(4000);
	//CCLabelBMFont* textObj2 = (CCLabelBMFont*)self->getChildByTag(4001);

	CCLabelBMFont* textObj = CCLabelBMFont::create(getAccuracyText().c_str(), "goldFont.fnt");
	textObj->setZOrder(1000);
	textObj->setTag(8000);
	textObj->setScale(0.5);
	CCSize size = textObj->getScaledContentSize();
	textObj->setPosition({ size.width / 2 + 3, size.height / 2 + 3});
	self->addChild(textObj);

	CCLabelBMFont* textObj2 = CCLabelBMFont::create(getFramerateText().c_str(), "goldFont.fnt");
	textObj2->setZOrder(1000);
	textObj2->setTag(8001);
	textObj2->setScale(0.5);
	size = textObj2->getScaledContentSize();
	textObj2->setPosition({ size.width / 2 + 3, size.height / 2 + 3});
	self->addChild(textObj2);
	std::cout << "PlayLayer initialized" << std::endl;
	return init(self, GJGameLevel);
}

bool __fastcall PlayLayer::pushButtonHook(void* self, uintptr_t, int state, bool player) {
	totalClicks++;
	if (mBotVars.mode != 0)
	{
		mBotVars.pushedMouse = true;
		mBotVars.releasedMouse = false;
		std::cout << "[Real] Pushed at frame: " << mBotVars.frame << std::endl;
	}
	return PlayLayer::pushButton(self, state, player);
}
bool __fastcall PlayLayer::releaseButtonHook(void* self, uintptr_t, int state, bool player) {
	if (mBotVars.mode != 0)
	{
		mBotVars.pushedMouse = false;
		mBotVars.releasedMouse = true;
		std::cout << "[Real] Released at frame: " << mBotVars.frame << std::endl;
	}
	return PlayLayer::releaseButton(self, state, player);
}

void __fastcall PlayLayer::togglePracticeHook(void* self, int edx, bool practice) {
	size_t base = (size_t)GetModuleHandle(0);

	startposFix.inPractice = practice;

	return PlayLayer::togglePractice(self, practice);
}
int __fastcall PlayLayer::deathHook(void* self, void*, void* go, void* powerrangers) {
	if (!setting.NoClipEnabled)
	{
		totalClicks = 0;
	}
	if (setting.NoClipEnabled)
	{
		noclipacc.wouldDie = true;
	}

	return PlayLayer::death(self, go, powerrangers);
}
void __fastcall PlayLayer::updateHook(CCLayer* self, void* edx, float delta) {
	void* player1 = *(void**)((char*)self + 0x224);
	float nx = *(float*)((size_t)player1 + 0x67c);
	float time = director->getAnimationInterval();
	// auto winSize = director->getWinSize();

	if (startposFix.smoothOut != 0 && delta - time < 1) { // if close enough to normal speed
		startposFix.smoothOut --;
	}
	
	if (nx != noclipacc.prevX) {
		noclipacc.frames += 1;
		noclipacc.totalDelta += delta;
	}

	size_t base = (size_t)GetModuleHandle(0);
	CCLabelBMFont* textObj = (CCLabelBMFont*)self->getChildByTag(8000);
	CCLabelBMFont* textObj2 = (CCLabelBMFont*)self->getChildByTag(8001);

	// mBot::Update(gd::GameManager::sharedState()->getPlayLayer());
	
	if (setting.NoClipAccEnabled) {
		auto size = textObj->getScaledContentSize();
		textObj->setPosition({size.width / 2 + 3, size.height / 2 + 3});
		textObj->setString(getAccuracyText().c_str());
		textObj->setVisible(true);
	} else {
		textObj->setVisible(false);
	}
	if (noclipacc.wouldDie) {
		noclipacc.wouldDie = false;
		if (noclipacc.totalDelta >= 0.1 && nx != noclipacc.prevX) {
			noclipacc.deaths += 1;
		}
	}
	noclipacc.prevX = nx;


	if (setting.FPSCounterEnabled)
	{
		if (setting.NoClipAccEnabled)
		{
			auto size = textObj2->getScaledContentSize();
			textObj2->setPosition({ size.width / 2 + 3, size.height / 2 + 16});
		} else {
			auto size = textObj2->getScaledContentSize();
			textObj2->setPosition({ size.width / 2 + 3, size.height / 2 + 3});
		}
		
		textObj2->setString(getFramerateText().c_str());
		textObj2->setVisible(true);
	} else {
		textObj2->setVisible(false);
	}
	
	if (!startposFix.smoothOut) {
		return update(self, delta);
	}
	return update(self, time);
}

int __fastcall PlayLayer::resetHook(gd::PlayLayer* self) {
	void* player1 = *(void**)((char*)self + 0x224);
	noclipacc.prevX = *(float*)((size_t)player1 + 0x67c);
	noclipacc.frames = 0;
	noclipacc.totalDelta = 0;
	noclipacc.deaths = 0;
	noclipacc.wouldDie = false;

	if (inPractice)
	{
		while (mBotVars.frame < (int)(std::get<std::deque<bool>>(mBotVars.PlayerData["Pushed"]).size())) { if (std::get<std::deque<bool>>(mBotVars.PlayerData["Pushed"]).size() != 0) { std::get<std::deque<bool>>(mBotVars.PlayerData["Pushed"]).pop_back(); } else { break; } };
		while (mBotVars.frame < (int)(std::get<std::deque<bool>>(mBotVars.PlayerData["Released"]).size())) { if (std::get<std::deque<bool>>(mBotVars.PlayerData["Released"]).size() != 0) { std::get<std::deque<bool>>(mBotVars.PlayerData["Released"]).pop_back(); } else { break; } };
	}
	
	
	if (startposFix.inTestmode || startposFix.inPractice) {
		startposFix.smoothOut = 2; // Account for 1 extra frame respawn
	}
	return resetLevel(self);
}

void __fastcall PlayLayer::onQuitHook(gd::PlayLayer* self) {

	inLevel = false;
	system("cls");
	return PlayLayer::onQuit(self);
}

void PlayLayer::mem_init() {
	

	size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));
	MH_CreateHook(
		(PVOID)(base + 0x111500),
		PlayLayer::pushButtonHook,
		(LPVOID*)&PlayLayer::pushButton
	);

	MH_CreateHook(
		(PVOID)(base + 0x111660),
		PlayLayer::releaseButtonHook,
		(LPVOID*)&PlayLayer::releaseButton
	);

	MH_CreateHook(
		(PVOID)(base + 0x20A1A0),
		PlayLayer::deathHook,
		(LPVOID*)&PlayLayer::death
	);
	MH_CreateHook(
		(PVOID)(base + 0x01FB780),
		PlayLayer::initHook,
		(LPVOID*)&PlayLayer::init);
	
	MH_CreateHook(
		(PVOID)(base + 0x20BF00),
		PlayLayer::resetHook,
		(LPVOID*)&PlayLayer::resetLevel);

	MH_CreateHook(
		(PVOID)(base + 0x2029C0),
		PlayLayer::updateHook,
		(LPVOID*)&PlayLayer::update);
	
	MH_CreateHook(
		(PVOID)(base + 0x20D0D0),
		PlayLayer::togglePracticeHook,
		(LPVOID*)&togglePractice);
	
	MH_CreateHook(
		(PVOID)(base + 0x20D810),
		PlayLayer::onQuitHook,
		(LPVOID*)&onQuit);

	MH_EnableHook(MH_ALL_HOOKS);
}

std::filesystem::path get_executable_path()
{
    char szFilePath[MAX_PATH + 1] = { 0 };
    GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
    return szFilePath;
}

std::filesystem::path get_executable_directory()
{
    return get_executable_path().parent_path();
}

static void loadMods()
{
	const auto base_path = get_executable_directory() / TEXT("addons");
	if (!std::filesystem::is_directory(base_path) || !std::filesystem::exists(base_path))
	{
		std::filesystem::create_directory(base_path);
	}
	if (!std::filesystem::is_directory(base_path) || !std::filesystem::exists(base_path))
	{
		std::filesystem::create_directory(base_path);
	}
	for (const auto& file : std::filesystem::directory_iterator(base_path))
	{
		LoadLibrary(file.path().LOADER_STR().c_str());
	}
}


static void ShowPlayerHacks(){
	ImGui::Begin("Player", NULL, ImGuiWindowFlags_NoResize);
	
	if (sortPlayer)
	{
		ImGui::SetWindowSize(ImVec2(210, 380));
		ImGui::SetWindowPos(ImVec2(10, 10));
		sortPlayer = false;
	}
	ImGui::Checkbox("NoClip", &setting.NoClipEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Makes the player invincible. (Safe patch.)");
	
	ImGui::Checkbox("NoSpikes", &setting.NoSpikesEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Ignores spike objects.");

	ImGui::Checkbox("Everything Hurts", &setting.EverythingHurtsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Ouch.");

	ImGui::Checkbox("Freeze Player", &setting.FreezePlayerEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Freezes player movement.");

	ImGui::Checkbox("Jump Hack", &setting.JumpHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for jumping in mid-air.");

	ImGui::Checkbox("Trail Always On", &setting.TrailAlwaysOnEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Sets the trail to always on.");

	ImGui::Checkbox("Hide Attempts", &setting.HideAttemptsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Hides the attempt count in-game.");

	ImGui::Checkbox("Practice Music Hack", &setting.PracticeMusicHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Plays the level's song in-sync with your position, instead of the standard practice song.");

	ImGui::Checkbox("No Pulse", &setting.NoPulseEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Disables pulsing on objects.");

	ImGui::Checkbox("Ignore ESC", &setting.IgnoreESCEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Stops the ESC key from exiting a level.");

	
	ImGui::Checkbox("Suicide", &setting.SuicideEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Kills the player.");


	ImGui::Checkbox("Accurate Percentage", &setting.AccuratePercentageEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for decimals in the level percentage.");

	ImGui::Checkbox("Only percentage", &setting.OnlyPercentageEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Disables progress bar.");




	/*
	ImGui::Checkbox("Instant Complete", &InstantCompleteEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Teleports the player to the end of a level, also know as the teleport hack.");
	if (InstantCompleteEnabled){
		
		WriteBytes((void*)(gd::base + 0x20350D), {0xC7, 0x81, 0x7C, 0x06, 0x00, 0x00, 0x20, 0xBC, 0xBE, 0x4C, 0x90, 0x90, 0x90, 0x90, 0x90});
	} else {
		WriteBytes((void*)(gd::base + 0x20350D), {0x8C, 0x03, 0x00, 0x00, 0x8B, 0x8E, 0x1C, 0x01, 0x00, 0x00, 0x8B, 0x01, 0xFF, 0x50, 0x64});
	} */
	ImGui::Checkbox("No Particles", &setting.NoParticlesEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Disables resuming the particle system.");

	ImGui::End();

}
static void ShowCreatorHacks(){
	ImGui::Begin("Creator", NULL, ImGuiWindowFlags_NoResize);
	if (sortCreator)
	{
		ImGui::SetWindowSize(ImVec2(210, 400));
		ImGui::SetWindowPos(ImVec2(230, 10));
		sortCreator = false;
	}

	ImGui::Checkbox("Copy Hack", &setting.CopyHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you copy any level, without a password.");

	ImGui::Checkbox("No (C) Mark", &setting.NoCMarkEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removes the (C) mark when uploading copied levels.");

	ImGui::Checkbox("Level Edit", &setting.LevelEditEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you edit any level, access the editor through the pause menu.");

	ImGui::Checkbox("Object Bypass", &setting.ObjectBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Sets the object limit to around 2 billion.");

	ImGui::Checkbox("Custom Object Bypass", &setting.CustomObjectBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removes the object limit for custom objects & lets you save over 50.");

	ImGui::Checkbox("Zoom Bypass", &setting.ZoomBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you zoom an infinite amount in the editor. (NOTE: Can crash with an edited grid size.)");

	ImGui::Checkbox("Toolbox Button Bypass", &setting.ToolboxButtonBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for more objects in the editor toolbox.");

	ImGui::Checkbox("Verify Hack", &setting.VerifyHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you upload unverified levels.");

	ImGui::Checkbox("Default Song Bypass", &setting.DefaultSongBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you use hidden default songs in the editor.");

	ImGui::Checkbox("Editor Extension", &setting.EditorExtensionEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Increases the editor length by a factor of 128.");

	ImGui::Checkbox("Place Over", &setting.PlaceOverEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you place the same object over itself in the editor.");

	ImGui::Checkbox("Testmode Bypass", &setting.TestmodeBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Hides the 'Testmode' text when playing with a startpos.");

	ImGui::Checkbox("Rotation Hack", &setting.RotationHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows you to rotate any object. Only works locally.");

	ImGui::Checkbox("Free Scroll", &setting.FreeScrollEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows you to scroll out the editor.");

	ImGui::Checkbox("Hide UI", &setting.HideUIEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Hides the Editor UI while building.");

	ImGui::Checkbox("Z Order Bypass", &setting.ZOrderBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removed the -100 to 100 Z order range limit.");
	ImGui::End();

}
static void ShowGlobalHacks(){
	if (!ImGui::CollapsingHeader("Global"))
		return;
}
static void ShowBypassHacks(){
	ImGui::Begin("Bypass", NULL, ImGuiWindowFlags_NoResize);


	if (sortBypass)
	{
		ImGui::SetWindowSize(ImVec2(210, 430));
		ImGui::SetWindowPos(ImVec2(450, 10));
		sortBypass = false;
	}
	
	ImGui::Checkbox("Icons", &setting.IconsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks all icons.");
	ImGui::Checkbox("Text Length", &setting.TextLengthEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for unlimited text length in text inputs.");
	ImGui::Checkbox("Character Filter", &setting.CharacterFilterEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you input any character in all text inputs.");
	ImGui::Checkbox("Slider Limit", &setting.SliderLimitEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets sliders be dragged beyond the visible limit.");
	ImGui::Checkbox("Main Levels", &setting.MainLevelsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks locked demon levels.");
	ImGui::Checkbox("Guard Vault", &setting.GuardVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the guard's vault.");
	ImGui::Checkbox("Keymaster Vault", &setting.KeymasterVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the keymaster's vault.");
	ImGui::Checkbox("Keymaster Basement", &setting.KeymasterBasementEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the keymaster's basement.");
	ImGui::Checkbox("Basement Key Bypass", &setting.BasementKeyBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you unlock the locks in the basement.");
	ImGui::Checkbox("Challenge Bypass", &setting.ChallengeBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks 'The Challenge' level.");
	ImGui::Checkbox("Treasure Room", &setting.TreasureRoomEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the treasure room.");
	ImGui::Checkbox("Potbor Shop", &setting.PotborShopEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks Potbor's shop in the treasure room.");
	ImGui::Checkbox("Scratch Shop", &setting.ScratchShopEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks Scratch's shop in the treasure room.");
	ImGui::Checkbox("Free Shop Items", &setting.FreeShopItemsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows all shop items to be bought for 0 mana orbs.");
	ImGui::Checkbox("Gatekeeper Vault", &setting.GatekeeperVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the Gatekeeper's vault.");
	ImGui::Checkbox("Backup Stars Limit", &setting.BackupStarsLimitEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you backup data, even with less than 10 stars.");
	ImGui::Checkbox("Unblock Hack", &setting.UnblockHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you view profiles of users who have blocked you.");
	ImGui::End();
}
static void ShowSpeedhack(){
	ImGui::Begin("Speedhack", NULL, ImGuiWindowFlags_NoResize);

	if (sortSpeedhack)
	{
		ImGui::SetWindowSize(ImVec2(210, 110));
		ImGui::SetWindowPos(ImVec2(10, 400));
		sortSpeedhack = false;
	}

	
	if (GetAsyncKeyState(VK_F2) & 5)
	{
		setting.speed += setting.f2_offset;
	}
	if (GetAsyncKeyState(VK_F1) & 5)
	{
		setting.speed -= setting.f1_offset;
	}
	ImGui::SliderFloat("x", &setting.speed, 0.0f, 20.0f, "%.1f");
	ImGui::Checkbox("Enabled", &setting.speedhackEnabled);
	ImGui::Checkbox("Speedhack audio", &setting.speedhackAudioEnabled);
	
	ImGui::End();
}
static void ShowFPSBypass(){
	ImGui::Begin("FPS Bypass", NULL, ImGuiWindowFlags_NoResize);

	if (sortFPSBypass)
	{
		ImGui::SetWindowSize(ImVec2(210, 80));
		ImGui::SetWindowPos(ImVec2(230, 420));
		sortFPSBypass = false;
	}
	ImGui::InputFloat("FPS", &setting.interval, 10.f, 20.f, "%.1f");
	ImGui::Checkbox("Enabled", &setting.FPSBypassEnabled);

	ImGui::End();
}

static void ShowStatus(){
	ImGui::Begin("Status", NULL, ImGuiWindowFlags_NoResize);

	if (sortStatus)
	{
		ImGui::SetWindowSize(ImVec2(210, 114));
		ImGui::SetWindowPos(ImVec2(890, 10));
		sortStatus = false;
	}

	ImGui::Checkbox("Enable NoClip Accuracy", &setting.NoClipAccEnabled);
	if (ImGui::BeginPopupContextItem())
    {
        ImGui::RadioButton("Bottom left", &setting.NoClipAccPosition, 0);
		ImGui::RadioButton("Top left", &setting.NoClipAccPosition, 1);
		ImGui::RadioButton("Bottom right", &setting.NoClipAccPosition, 2);
		ImGui::RadioButton("Top right", &setting.NoClipAccPosition, 3);
        ImGui::EndPopup();
    }
	ImGui::Checkbox("Enable FPS Counter", &setting.FPSCounterEnabled);
	if (ImGui::BeginPopupContextItem())
    {
        ImGui::RadioButton("Bottom left", &setting.FPSCounterPosition, 0);
		ImGui::RadioButton("Top left", &setting.FPSCounterPosition, 1);
		ImGui::RadioButton("Bottom right", &setting.FPSCounterPosition, 2);
		ImGui::RadioButton("Top right", &setting.FPSCounterPosition, 3);
        ImGui::EndPopup();
    }
	ImGui::End();
}

static void ShowUniversal()
{
	ImGui::Begin("Universal", NULL, ImGuiWindowFlags_NoResize);

	if (sortUniversal)
	{
		ImGui::SetWindowSize(ImVec2(210, 280));
		ImGui::SetWindowPos(ImVec2(670, 10));
		sortUniversal = false;
	}


	ImGui::Checkbox("Force Visibility", &setting.ForceVisibilityEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Sets all nodes to be visible.");
	ImGui::Checkbox("No Rotation", &setting.NoRotationEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Locks all rotation at 0 degrees.");
	ImGui::Checkbox("Free Window Resize", &setting.FreeWindowResizeEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removes limits in place for window resizing.");
	ImGui::Checkbox("Safe Mode", &setting.SafeModeEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Disables progress, completion & verification of levels.");
	ImGui::Checkbox("Transparent BG", &setting.TransparentBGEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removes the blue filter from menu's backgrounds. By WEGFan");
	ImGui::Checkbox("Transparent Lists", &setting.TransparentListsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Makes the menu lists transparent. By WEGFan");
	ImGui::Checkbox("Fast Alt-Tab", &setting.FastAltTabEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Disables savefile saving on minimize. By PoweredByPie");
	ImGui::Checkbox("Allow Low Volume", &setting.AllowLowVolumeEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removed snapping to 0.00 setting volume to 0.03");
}

static void ShowOther()
{
	ImGui::Begin("Other", NULL, ImGuiWindowFlags_NoResize);

	if (sortOther)
	{
		ImGui::SetWindowSize(ImVec2(210, 140));
		ImGui::SetWindowPos(ImVec2(890, 134));
		sortOther = false;
	}

	

	if (ImGui::TreeNode("Transition customiser")) {
		ImGui::PushItemWidth(176.000000);
		ImGui::Combo("t", &setting.currentTransition, transitions, IM_ARRAYSIZE(transitions));

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Anti-aliasing")) 
	{
		ImGui::Checkbox("Enable MSAAx4", &setting.MSAAEnabled);
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Requires game restart.");
		}
		ImGui::TreePop();
	}

	ImGui::Separator();

	if (ImGui::Button("Inject Dll", {196.f,19.f})) {
		std::string stringpath = chooseDLL();
		const char* DllPath = stringpath.c_str();
		if (stringpath != nofilename)
		{
			if(Inject(DllPath))
			{
				ImGui::OpenPopup("DLL successfully injected");
			} else {
				ImGui::OpenPopup("DLL injection failed");
			}
		}
	}
	if (ImGui::Button("Sort windows", {196.f,19.f})) {
		sortPlayer    = true;
		sortAbout     = true;
		sortBypass    = true;
		sortCreator   = true;
		sortFPSBypass = true;
		sortOther     = true;
		sortSpeedhack = true;
		sortUniversal = true;
		sortStatus    = true;
		sortMBot      = true;
	}

	if (ImGui::BeginPopupModal("DLL successfully injected", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("DLL injected");
		if(ImGui::Button("OK", {196.f,19.f}))
		{
			ImGui::CloseCurrentPopup();
		}
	}
	if (ImGui::BeginPopupModal("DLL injection failed", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Failed to inject DLL");
		if(ImGui::Button("OK", {196.f,19.f}))
		{
			ImGui::CloseCurrentPopup();
		}
	}
	
}

static void ShowDeveloper(){
	ImGui::Begin("Developer options");

	ImGui::Checkbox("Show Demo Window", &showDemoWindow);
	if (showDemoWindow)
	{
		ImGui::ShowDemoWindow();
	}
	if(ImGui::Button("Save hacks")){
		saveHacks();
	}
	if(ImGui::Button("Load hacks")){
		loadHacks();
	}
	ImGui::Separator();
	
	
}
static void ShowAboutWindow(){
	ImGui::Begin("About Mod Menu", NULL, ImGuiWindowFlags_NoResize);

	if (sortAbout)
	{
		ImGui::SetWindowSize(ImVec2(210, 200));
		ImGui::SetWindowPos(ImVec2(1330, 10));
		sortAbout = false;
	}

    ImGui::Text("Mod Menu %s", VERSION);
    ImGui::Separator();
    ImGui::Text("By Alexander Simonov");
    ImGui::Text("Mod Menu is licensed\nunder the MIT License.");
	ImGui::Separator();
	ImGui::Text("3rd party libraries: ");
	ImGui::Text("ImGui: %s", ImGui::GetVersion());
	ImGui::Text("MinHook: %s", "1.3.3");

	if(ImGui::Button("View on GitHub", {196.f,19.f}))
	{
		ShellExecute(0, 0, LPCSTR("https://github.com/oneparsec/GD-ModMenu"), 0, 0 , SW_SHOW );
	}
	if(ImGui::Button("View license", {196.f,19.f}))
	{
		ShellExecute(0, 0, LPCSTR("https://github.com/oneparsec/GD-ModMenu/blob/main/LICENSE"), 0, 0 , SW_SHOW );
	}
}

void mBot::Update(gd::PlayLayer* self)
{
	if (self->m_pPlayer1->getPositionX() == 0) { mBotVars.frame = 0; }
	else { mBotVars.frame++; }
	std::cout << "Frame: " << mBotVars.frame << "	Pushed: " << mBotVars.pushedMouse << "	Released: " << mBotVars.releasedMouse << "	XPos: " << self->m_pPlayer1->getPositionX() << std::endl;
	if (mBotVars.mode == 1)
	{
		if(gd::GameManager::sharedState()->getPlayLayer()->m_isPracticeMode)
		{
			inPractice = true;
		} else {
			inPractice = false;
		}
		if (mBotVars.frame > 0)
		{
			std::get<std::deque<bool>>(mBotVars.PlayerData["Pushed"]).insert(std::get<std::deque<bool>>(mBotVars.PlayerData["Pushed"]).end(), mBotVars.pushedMouse);
			mBotVars.pushedMouse = false;
			std::get<std::deque<bool>>(mBotVars.PlayerData["Released"]).insert(std::get<std::deque<bool>>(mBotVars.PlayerData["Released"]).end(), mBotVars.releasedMouse);
			mBotVars.releasedMouse = false;
		}
	}
	else if (mBotVars.mode == 2)
	{
		if (mBotVars.frame != 0)
		{
			if(std::get<std::deque<bool>>(mBotVars.PlayerData["Pushed"])[mBotVars.frame])
			{
				std::cout << "[mBot] Pushed at frame: " << mBotVars.frame << std::endl;
				PlayLayer::pushButton(self, 0, true);
			}
			if(std::get<std::deque<bool>>(mBotVars.PlayerData["Released"])[mBotVars.frame])
			{
				std::cout << "[mBot] Released at frame: " << mBotVars.frame << std::endl;
				PlayLayer::releaseButton(self, 0, true);
			}
		}
	}
}

void mBot::loadReplay()
{

}

void mBot::saveReplay()
{

}

static void ShowMBot()
{
	ImGui::Begin("mBot " MBOT_VERSION);

	if (sortMBot)
	{
		ImGui::SetWindowSize(ImVec2(210, 200));
		ImGui::SetWindowPos(ImVec2(1110, 10));
		sortMBot = false;
	}

	
    ImGui::RadioButton("Disabled", &mBotVars.mode, 0);
	ImGui::RadioButton("Record",   &mBotVars.mode, 1);
    ImGui::RadioButton("Playback", &mBotVars.mode, 2);
	

	if (mBotVars.mode == 1)
	{
		ImGui::Separator();
		if(ImGui::Button("Save replay", {196.f,19.f}))
		{
			mBot::saveReplay();
		}
		if(ImGui::Button("Load replay", {196.f,19.f}))
		{
			mBot::loadReplay();
		}
		if(ImGui::Button("Clear replay", {196.f,19.f}))
		{
			std::get<std::deque<bool>>(mBotVars.PlayerData["Pushed"]).clear();
			std::get<std::deque<bool>>(mBotVars.PlayerData["Released"]).clear();
		}
	}
	ImGui::End();
}

static void disableAnticheat()
{
	WriteBytes((void*)(gd::base + 0x202AAA), { 0xEB, 0x2E }),
	WriteBytes((void*)(gd::base + 0x15FC2E), { 0xEB }),
	WriteBytes((void*)(gd::base + 0x20D3B3), { 0x90, 0x90, 0x90, 0x90, 0x90 }),
	WriteBytes((void*)(gd::base + 0x1FF7A2), { 0x90, 0x90 }),
	WriteBytes((void*)(gd::base + 0x18B2B4), { 0xB0, 0x01 }),
	WriteBytes((void*)(gd::base + 0x20C4E6), { 0xE9, 0xD7, 0x00, 0x00, 0x00, 0x90 }),
	WriteBytes((void*)(gd::base + 0x1FD557), { 0xEB, 0x0C }),
	WriteBytes((void*)(gd::base + 0x1FD742), { 0xC7, 0x87, 0xE0, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xC7, 0x87, 0xE4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }),
	WriteBytes((void*)(gd::base + 0x1FD756), { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }),
	WriteBytes((void*)(gd::base + 0x1FD79A), { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 }),
	WriteBytes((void*)(gd::base + 0x1FD7AF), { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });
}

void MainWindow()
{
	AttemptAttach("Geometry Dash", "GeometryDash.exe");
	if (setting.currentTransition == 0)
	{
		WriteBytes((void*)(cocosBase + transitionaddr[0]), {0x55,0x8B,0xEC,0x6A,0xFF});
	} else {
		WriteJump(cocosBase + transitionaddr[0], cocosBase + transitionaddr[setting.currentTransition]);
	}
	ShowPlayerHacks();
	ShowCreatorHacks();
	ShowBypassHacks();
	ShowSpeedhack();
	ShowFPSBypass();
	ShowStatus();
	ShowOther();
	ShowUniversal();
	ShowAboutWindow();
	ShowMBot();
	if (DEVELOPER_MODE)
	{
		ShowDeveloper();
		// ImGui::ShowDemoWindow();
	}
}


void RenderUI() 
{
	
	const bool enable_touch = !ImGui::GetIO().WantCaptureMouse || !show;
	director->getTouchDispatcher()->setDispatchEvents(enable_touch);


	checkHacks();
	DEVMODE dm;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	if (setting.FPSBypassEnabled)
	{
		SetTargetFPS(setting.interval);
	} else {
		SetTargetFPS(dm.dmDisplayFrequency);
	}
	if (show) {
		saveHacks();
		MainWindow();
	}
}

void InitUI()
{
	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_PopupBg]                = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
	colors[ImGuiCol_Border]                 = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
	colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_MenuBarBg]              = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
	colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
	colors[ImGuiCol_Button]                 = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_Header]                 = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
	colors[ImGuiCol_Separator]              = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
	colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabHovered]             = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_TabActive]              = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
	colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
	colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogram]          = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
	colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
	colors[ImGuiCol_TableBorderLight]       = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
	colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
	colors[ImGuiCol_DragDropTarget]         = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding                     = ImVec2(8.00f, 8.00f);
	style.FramePadding                      = ImVec2(5.00f, 2.00f);
	style.CellPadding                       = ImVec2(6.00f, 6.00f);
	style.ItemSpacing                       = ImVec2(6.00f, 6.00f);
	style.ItemInnerSpacing                  = ImVec2(6.00f, 6.00f);
	style.TouchExtraPadding                 = ImVec2(0.00f, 0.00f);
	style.IndentSpacing                     = 25;
	style.ScrollbarSize                     = 15;
	style.GrabMinSize                       = 10;
	style.WindowBorderSize                  = 1;
	style.ChildBorderSize                   = 1;
	style.PopupBorderSize                   = 1;
	style.FrameBorderSize                   = 1;
	style.TabBorderSize                     = 1;
	style.WindowRounding                    = 7;
	style.ChildRounding                     = 4;
	style.FrameRounding                     = 3;
	style.PopupRounding                     = 4;
	style.ScrollbarRounding                 = 9;
	style.GrabRounding                      = 3;
	style.LogSliderDeadzone                 = 4;
	style.TabRounding                       = 4;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		MH_Initialize();
		loadHacks();
		disableAnticheat();
		SpeedhackAudio::init();
		LoadingLayer::mem_init();
		PlayLayer::mem_init();
		if (setting.MSAAEnabled)
		{
			loadMSAA();
		}
		if (DEVELOPER_MODE)
		{
			AllocConsole();
			SetConsoleTitleA("GDMM Debug console");
			freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
			std::cout << "Mod Menu successfully loaded!" << std::endl;
			std::cout << "Running Mod Menu version: " VERSION << std::endl;
			std::cout << "Running mBot version: " MBOT_VERSION << std::endl;
		}
		
		loadMods();
		ImGuiHook::setToggleKey(VK_DELETE);
		ImGuiHook::setRenderFunction(RenderUI);
		ImGuiHook::setToggleCallback([]() {
			show = !show;
		});
		ImGuiHook::setInitFunction(InitUI);
		ImGuiHook::setupHooks([](void* target, void* hook, void** trampoline) {
			MH_CreateHook(target, hook, trampoline);
		});
		MH_EnableHook(MH_ALL_HOOKS);
		
		break;
	case DLL_PROCESS_DETACH:
		saveHacks();
		break;
	}
	return TRUE;
}