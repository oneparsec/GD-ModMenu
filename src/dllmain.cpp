/*
Special thanks:
Absolute
HJFod
Matcool
Pixelsaft
Adaf
TobyAdd
fig
*/

#define DEVELOPER_MODE FALSE
#define VERSION "DEV_RELEASE"

#ifdef _MSC_VER
#pragma warning(disable: 4251) // disable warning 4251
#endif

#include <Windows.h>
#include <imgui_hook.h>
#include <imgui.h>
#include <MinHook.h>
#include <gd.h>
#include <iostream>
#include <Psapi.h>
#include <fstream>
#include "SimpleIni.h"
#include <cocos2d.h>
#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <shellapi.h>
#include <chrono>
#include <ctime>
#include <thread>
#include <commdlg.h>


static bool show = false;
static bool showDemoWindow = false;

static struct
{
	/* data */
} settings;

static bool speedhackEnabled;
static bool speedhackAudioEnabled;
// player
static bool NoClipEnabled;
static bool NoSpikesEnabled;
static bool NoHitboxEnabled;
static bool NoSolidsEnabled;
static bool ForceBlockTypeEnabled;
static bool EverythingHurtsEnabled;
static bool FreezePlayerEnabled;
static bool JumpHackEnabled;
static bool ForceTrailStateEnabled;
static bool InversedTrailEnabled;
static bool HideAttemptsEnabled;
static bool PracticeMusicHackEnabled;
static bool NoPulseEnabled;
static bool IgnoreESCEnabled;
static bool SuicideEnabled;
static bool AccuratePercentageEnabled;
static bool OnlyPercentageEnabled;
static bool NoParticlesEnabled;
static bool InstantCompleteEnabled;
// creator
static bool CopyHackEnabled;
static bool NoCMarkEnabled;
static bool LevelEditEnabled;
static bool ObjectBypassEnabled;
static bool CustomObjectBypassEnabled;
static bool ZoomBypassEnabled;
static bool ToolboxButtonBypassEnabled;
static bool VerifyHackEnabled;
static bool DefaultSongBypassEnabled;
static bool EditorExtensionEnabled;
static bool PlaceOverEnabled;
static bool TestmodeBypassEnabled;
static bool RotationHackEnabled;
static bool FreeScrollEnabled;
static bool HideUIEnabled;
static bool ZOrderBypassEnabled;
// fps bypass
static bool FPSBypassEnabled;
static float interval = 60.f;
// speedhack
static int currentPlace = 0;
static float speed = 1.f;
static float f1_offset = 0.5f;
static float f2_offset = 0.5f;
// bypass
static bool SongBypassEnabled;
static bool IconsEnabled;
static bool TextLengthEnabled;
static bool CharacterFilterEnabled;
static bool SliderLimitEnabled;
static bool MainLevelsEnabled;
static bool GuardVaultEnabled;
static bool KeymasterVaultEnabled;
static bool KeymasterBasementEnabled;
static bool BasementKeyBypassEnabled;
static bool ChallengeBypassEnabled;
static bool TreasureRoomEnabled;
static bool PotborShopEnabled;
static bool ScratchShopEnabled;
static bool FreeShopItemsEnabled;
static bool GatekeeperVaultEnabled;
static bool BackupStarsLimitEnabled;
static bool UnblockHackEnabled;
// utilities
static bool NoClipAccEnabled;
static bool FPSCounterEnabled;


static char license[1067]= "MIT License\nCopyright (c) 2022 Alexandr Simonov\nPermission is hereby granted, free of charge, to any person obtaining a copy\nof this software and associated documentation files (the ""Software""), to deal\nin the Software without restriction, including without limitation the rights\nto use, copy, modify, merge, publish, distribute, sublicense, and/or sell\ncopies of the Software, and to permit persons to whom the Software is\nfurnished to do so, subject to the following conditions:\n\nThe above copyright notice and this permission notice shall be included in all\ncopies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED ""AS IS"", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\nIMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\nFITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\nAUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\nLIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\nOUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\nSOFTWARE.";

int totalClicks = 0;
int midClickCount = 0, actualClickCount = 0;
int lastAsyncKeyStateValue = 1337;

static struct {
	bool wouldDie = false;
	int frames = 0;
	int deaths = 0;
	float totalDelta = 0;
	float prevX = 0;
	bool created = false;
	bool left = true;
	bool top = false;
} noclipacc;

static struct {
	bool created = false;
} fpsCounter;

static struct {
	bool inPractice;
	bool inTestmode;
	int smoothOut;
} startposFix;




typedef void*   (__cdecl *fSharedApplication)();
typedef void    (__thiscall *fSetAnimationInterval)(void *instance, double delay);
fSharedApplication sharedApplication;
fSetAnimationInterval setAnimInterval;

using namespace cocos2d;
using namespace std;

chrono::system_clock::time_point start = chrono::system_clock::now(), now;

chrono::duration<double> cycleTime;



char * filter = "Dynamic link library (*.dll)\0*.dll";

CSimpleIniA ini;

void WriteBytes(void* location, std::vector<BYTE> bytes) {
	DWORD old_prot;
	VirtualProtect(location, bytes.size(), PAGE_EXECUTE_READWRITE, &old_prot);

	memcpy(location, bytes.data(), bytes.size());

	VirtualProtect(location, bytes.size(), old_prot, &old_prot);
}


namespace SpeedhackAudio {
    void* channel;
    float speed;
    bool initialized = false;

    // setfrequency
    // setvolume

    void* (__stdcall* setVolume)(void* t_channel, float volume);
    void* (__stdcall* setFrequency)(void* t_channel, float frequency);

    void* __stdcall AhsjkabdjhadbjJHDSJ(void* t_channel, float volume) {
        channel = t_channel;

        if (speed != 1.f) {
            setFrequency(channel, speed);

        }
        return setVolume(channel, volume);
    }

    void init() {
        if (initialized)
            return;

        setFrequency = (decltype(setFrequency))GetProcAddress(GetModuleHandle(LPCSTR("fmod.dll")), "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");
        DWORD hkAddr = (DWORD)GetProcAddress(GetModuleHandle(LPCSTR("fmod.dll")), "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z");

        MH_CreateHook(
            (PVOID)hkAddr,
            AhsjkabdjhadbjJHDSJ,
            (PVOID*)&setVolume
        );

        speed = 1.f;
        initialized = true;
    }

    void set(float frequency) {
        if (!initialized)
            init();

        if (channel == nullptr)
            return;

        speed = frequency;
        setFrequency(channel, frequency);
    }
}


uint32_t GetModuleBase(const char *module)
{
    static const int size = 0x1000;
    DWORD out;
    HMODULE hmods[size];
    if (EnumProcessModulesEx(GetCurrentProcess(), hmods, 0x1000, &out, LIST_MODULES_ALL))
    {
        for (uint32_t i = 0; i < out / 4; ++i)
        {
            char path[MAX_PATH];
            if (GetModuleBaseNameA(GetCurrentProcess(), hmods[i], path, MAX_PATH))
            {
                if (!strcmp(path, module))
                    return reinterpret_cast<uint32_t>(hmods[i]);
            }
        }
    }
    return 0;
}

uint32_t cocosBase = GetModuleBase("libcocos2d.dll");

void saveHacks()
{



	ini.SetUnicode();
	ini.SetBoolValue("player","NoClipEnabled", NoClipEnabled);
	ini.SetBoolValue("player","NoSpikesEnabled", NoSpikesEnabled);
	ini.SetBoolValue("player","EverythingHurtsEnabled", EverythingHurtsEnabled);
	ini.SetBoolValue("player","FreezePlayerEnabled", FreezePlayerEnabled);
	ini.SetBoolValue("player","JumpHackEnabled", JumpHackEnabled);
	ini.SetBoolValue("player","ForceTrailStateEnabled", ForceTrailStateEnabled);
	ini.SetBoolValue("player","HideAttemptsEnabled", HideAttemptsEnabled);
	ini.SetBoolValue("player","PracticeMusicHackEnabled", PracticeMusicHackEnabled);
	ini.SetBoolValue("player","NoPulseEnabled", NoPulseEnabled);
	ini.SetBoolValue("player","IgnoreESCEnabled", IgnoreESCEnabled);
	ini.SetBoolValue("player","SuicideEnabled", SuicideEnabled);
	ini.SetBoolValue("player","AccuratePercentageEnabled", AccuratePercentageEnabled);
	ini.SetBoolValue("player","NoParticlesEnabled", NoParticlesEnabled);
	ini.SetBoolValue("player", "OnlyPercentageEnabled", OnlyPercentageEnabled);


	ini.SetBoolValue("creator","CopyHackEnabled", CopyHackEnabled);
	ini.SetBoolValue("creator","NoCMarkEnabled", NoCMarkEnabled);
	ini.SetBoolValue("creator","LevelEditEnabled", LevelEditEnabled);
	ini.SetBoolValue("creator","ObjectBypassEnabled", ObjectBypassEnabled);
	ini.SetBoolValue("creator","CustomObjectBypassEnabled", CustomObjectBypassEnabled);
	ini.SetBoolValue("creator","ZoomBypassEnabled", ZoomBypassEnabled);
	ini.SetBoolValue("creator","ToolboxButtonBypassEnabled", ToolboxButtonBypassEnabled);
	ini.SetBoolValue("creator","VerifyHackEnabled", VerifyHackEnabled);
	ini.SetBoolValue("creator","DefaultSongBypassEnabled", DefaultSongBypassEnabled);
	ini.SetBoolValue("creator","EditorExtensionEnabled", EditorExtensionEnabled);
	ini.SetBoolValue("creator","PlaceOverEnabled", PlaceOverEnabled);
	ini.SetBoolValue("creator","TestmodeBypassEnabled", TestmodeBypassEnabled);
	ini.SetBoolValue("creator","RotationHackEnabled", RotationHackEnabled);
	ini.SetBoolValue("creator","FreeScrollEnabled", FreeScrollEnabled);
	ini.SetBoolValue("creator","HideUIEnabled", HideUIEnabled);
	ini.SetBoolValue("creator","ZOrderBypassEnabled", ZOrderBypassEnabled);

	ini.SetBoolValue("fps-bypass","FPSBypassEnabled",FPSBypassEnabled);
	ini.SetDoubleValue("fps-bypass","TargetFPS",(double)interval);
	
	ini.SetBoolValue("bypass","IconsEnabled",IconsEnabled);
	ini.SetBoolValue("bypass", "TextLengthEnabled", TextLengthEnabled);
	ini.SetBoolValue("bypass", "CharacterFilterEnabled", CharacterFilterEnabled);
	ini.SetBoolValue("bypass", "SliderLimitEnabled", SliderLimitEnabled);
	ini.SetBoolValue("bypass", "MainLevelsEnabled", MainLevelsEnabled);
	ini.SetBoolValue("bypass", "GuardVaultEnabled", GuardVaultEnabled);
	ini.SetBoolValue("bypass", "KeymasterVaultEnabled", KeymasterVaultEnabled);
	ini.SetBoolValue("bypass", "KeymasterBasementEnabled", KeymasterBasementEnabled);
	ini.SetBoolValue("bypass", "BasementKeyBypassEnabled", BasementKeyBypassEnabled);
	ini.SetBoolValue("bypass", "ChallengeBypassEnabled", ChallengeBypassEnabled);
	ini.SetBoolValue("bypass", "TreasureRoomEnabled", TreasureRoomEnabled);
	ini.SetBoolValue("bypass", "PotborShopEnabled", PotborShopEnabled);
	ini.SetBoolValue("bypass", "ScratchShopEnabled", ScratchShopEnabled);
	ini.SetBoolValue("bypass", "FreeShopItemsEnabled", FreeShopItemsEnabled);
	ini.SetBoolValue("bypass", "GatekeeperVaultEnabled", GatekeeperVaultEnabled);
	ini.SetBoolValue("bypass", "BackupStarsLimitEnabled", BackupStarsLimitEnabled);
	ini.SetBoolValue("bypass", "UnblockHackEnabled", UnblockHackEnabled);
	ini.SaveFile("save.ini");
	
	
	/* 	ImGui::Checkbox("Icons", &IconsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks all icons.");
	ImGui::Checkbox("Text Length", &TextLengthEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for unlimited text length in text inputs.");
	ImGui::Checkbox("Character Filter", &CharacterFilterEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you input any character in all text inputs.");
	ImGui::Checkbox("Slider Limit", &SliderLimitEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets sliders be dragged beyond the visible limit.");
	ImGui::Checkbox("Main Levels", &MainLevelsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks locked demon levels.");
	ImGui::Checkbox("Guard Vault", &GuardVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the guard's vault.");
	ImGui::Checkbox("Keymaster Vault", &KeymasterVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the keymaster's vault.");
	ImGui::Checkbox("Keymaster Basement", &KeymasterBasementEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the keymaster's basement.");
	ImGui::Checkbox("Basement Key Bypass", &BasementKeyBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you unlock the locks in the basement.");
	ImGui::Checkbox("Challenge Bypass", &ChallengeBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks 'The Challenge' level.");
	ImGui::Checkbox("Treasure Room", &TreasureRoomEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the treasure room.");
	ImGui::Checkbox("Potbor Shop", &PotborShopEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks Potbor's shop in the treasure room.");
	ImGui::Checkbox("Scratch Shop", &ScratchShopEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks Scratch's shop in the treasure room.");
	ImGui::Checkbox("Free Shop Items", &FreeShopItemsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows all shop items to be bought for 0 mana orbs.");
	ImGui::Checkbox("Gatekeeper Vault", &GatekeeperVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the Gatekeeper's vault.");
	ImGui::Checkbox("Backup Stars Limit", &BackupStarsLimitEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you backup data, even with less than 10 stars.");
	ImGui::Checkbox("Unblock Hack", &UnblockHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you view profiles of users who have blocked you."); */




}

void loadHacks(){
	SI_Error rc = ini.LoadFile("save.ini");
	if (rc<0)
	{
		MessageBoxA(NULL, "Failed to load save file. It will be created.", "Error occured", MB_ICONERROR);
		ofstream of("save.ini",ios::app);
		of << endl;
	}
	NoClipEnabled                         = ini.GetBoolValue("player","NoClipEnabled");
	NoSpikesEnabled                     = ini.GetBoolValue("player","NoSpikesEnabled");
	EverythingHurtsEnabled       = ini.GetBoolValue("player","EverythingHurtsEnabled");
	FreezePlayerEnabled             = ini.GetBoolValue("player","FreezePlayerEnabled");
	JumpHackEnabled                     = ini.GetBoolValue("player","JumpHackEnabled");
	ForceTrailStateEnabled       = ini.GetBoolValue("player","ForceTrailStateEnabled");
	HideAttemptsEnabled             = ini.GetBoolValue("player","HideAttemptsEnabled");
	PracticeMusicHackEnabled   = ini.GetBoolValue("player","PracticeMusicHackEnabled");
	NoPulseEnabled                       = ini.GetBoolValue("player","NoPulseEnabled");
	IgnoreESCEnabled                   = ini.GetBoolValue("player","IgnoreESCEnabled");
	SuicideEnabled                       = ini.GetBoolValue("player","SuicideEnabled");
	AccuratePercentageEnabled = ini.GetBoolValue("player", "AccuratePercentageEnabled");
	NoParticlesEnabled               = ini.GetBoolValue("player","NoParticlesEnabled");
	OnlyPercentageEnabled = ini.GetBoolValue("player", "OnlyPercentageEnabled");
	
	CopyHackEnabled =                       ini.GetBoolValue("creator", "CopyHackEnabled");
	NoCMarkEnabled =                         ini.GetBoolValue("creator", "NoCMarkEnabled");
	LevelEditEnabled =                     ini.GetBoolValue("creator", "LevelEditEnabled");
	ObjectBypassEnabled =               ini.GetBoolValue("creator", "ObjectBypassEnabled");
	CustomObjectBypassEnabled =   ini.GetBoolValue("creator", "CustomObjectBypassEnabled");
	ZoomBypassEnabled =                   ini.GetBoolValue("creator", "ZoomBypassEnabled");
	ToolboxButtonBypassEnabled = ini.GetBoolValue("creator", "ToolboxButtonBypassEnabled");
	VerifyHackEnabled =                   ini.GetBoolValue("creator", "VerifyHackEnabled");
	DefaultSongBypassEnabled =     ini.GetBoolValue("creator", "DefaultSongBypassEnabled");
	EditorExtensionEnabled =         ini.GetBoolValue("creator", "EditorExtensionEnabled");
	PlaceOverEnabled =                     ini.GetBoolValue("creator", "PlaceOverEnabled");
	TestmodeBypassEnabled =           ini.GetBoolValue("creator", "TestmodeBypassEnabled");
	RotationHackEnabled =               ini.GetBoolValue("creator", "RotationHackEnabled");
	FreeScrollEnabled =                   ini.GetBoolValue("creator", "FreeScrollEnabled");
	HideUIEnabled =                           ini.GetBoolValue("creator", "HideUIEnabled");
	ZOrderBypassEnabled =               ini.GetBoolValue("creator", "ZOrderBypassEnabled");

	FPSBypassEnabled = ini.GetBoolValue("fps-bypass","FPSBypassEnabled");
	interval = (float)ini.GetDoubleValue("fps-bypass","TargetFPS");

	IconsEnabled  = ini.GetBoolValue("bypass", "IconsEnabled");
	TextLengthEnabled = ini.GetBoolValue("bypass", "TextLengthEnabled");
	CharacterFilterEnabled = ini.GetBoolValue("bypass", "CharacterFilterEnabled");
	SliderLimitEnabled = ini.GetBoolValue("bypass", "SliderLimitEnabled");
	MainLevelsEnabled = ini.GetBoolValue("bypass", "MainLevelsEnabled");
	GuardVaultEnabled = ini.GetBoolValue("bypass", "GuardVaultEnabled");
	KeymasterVaultEnabled = ini.GetBoolValue("bypass", "KeymasterVaultEnabled");
	KeymasterBasementEnabled = ini.GetBoolValue("bypass", "KeymasterBasementEnabled");
	BasementKeyBypassEnabled = ini.GetBoolValue("bypass", "BasementKeyBypassEnabled");
	ChallengeBypassEnabled = ini.GetBoolValue("bypass", "ChallengeBypassEnabled");
	TreasureRoomEnabled = ini.GetBoolValue("bypass", "TreasureRoomEnabled");
	PotborShopEnabled = ini.GetBoolValue("bypass", "PotborShopEnabled");
	ScratchShopEnabled = ini.GetBoolValue("bypass", "ScratchShopEnabled");
	FreeShopItemsEnabled = ini.GetBoolValue("bypass", "FreeShopItemsEnabled");
	GatekeeperVaultEnabled = ini.GetBoolValue("bypass", "GatekeeperVaultEnabled");
	BackupStarsLimitEnabled = ini.GetBoolValue("bypass", "BackupStarsLimitEnabled");
	UnblockHackEnabled = ini.GetBoolValue("bypass", "UnblockHackEnabled");
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
        return fileName;
}

std::string getAccuracyText() {
	if (noclipacc.frames == 0) return "Accuracy: 100.00%";
	float p = (float)(noclipacc.frames - noclipacc.deaths) / (float)noclipacc.frames;
	std::stringstream stream;
	stream << "Accuracy: " << std::fixed << std::setprecision(2) << p * 100.f << "%";
	return stream.str();
}

std::string getFramerateText(){
	std::stringstream stream2;
	stream2 << round(ImGui::GetIO().Framerate)<< " FPS";
	return stream2.str();
}

namespace PlayLayer {

	inline bool(__thiscall* pushButton)(void* self, int state, bool player);
	bool __fastcall pushButtonHook(void* self, uintptr_t, int state, bool player);

	inline bool(__thiscall* releaseButton)(void* self, int state, bool player);
	bool __fastcall releaseButtonHook(void* self, uintptr_t, int state, bool player);

	inline int(__thiscall* death)(void* self, void* go, void* powerrangers);
	int __fastcall deathHook(void* self, void*, void* go, void* powerrangers);

	inline bool(__thiscall* init)(CCLayer* self, void* GJGameLevel);
	bool __fastcall initHook(CCLayer* self, int edx, void* GJGameLevel);

	inline void(__thiscall* togglePractice)(void* self, bool practice);
	void __fastcall togglePracticeHook(void* self, int edx, bool practice);

	inline void(__thiscall* update)(cocos2d::CCLayer* self, float delta);
	void __fastcall updateHook(cocos2d::CCLayer* self, void*, float delta);

	inline int(__thiscall* resetLevel)(void* self);
	int __fastcall resetHook(void* self);

	void mem_init();

}

namespace LoadingLayer {
	
	static inline void (__thiscall* init_)(cocos2d::CCLayer*, char);
    static void __fastcall initHook(cocos2d::CCLayer*, void*, char);

	void mem_init();

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

    auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();

    label->setPosition(
        winSize.width / 2,
        20
    );

    _layer->addChild(label);

    return;
}

void LoadingLayer::mem_init() {
	MH_Initialize();

	size_t base = reinterpret_cast<size_t>(GetModuleHandle(0));
	MH_CreateHook(
		(PVOID)(base + 0x18c080),
		LoadingLayer::initHook,
		(LPVOID*)&LoadingLayer::init_
	);
	MH_EnableHook(MH_ALL_HOOKS);
}


bool __fastcall PlayLayer::initHook(CCLayer* self, int edx, void* GJGameLevel) {
	size_t base = (size_t)GetModuleHandle(0);
	totalClicks = 0;
	noclipacc.prevX = 0;
	noclipacc.created = true;
	startposFix.inPractice = false;
	startposFix.inTestmode = *(bool*)((uintptr_t)self + 0x494);
	startposFix.smoothOut = 0;

	//CCLabelBMFont* textObj = (CCLabelBMFont*)self->getChildByTag(4000);
	//CCLabelBMFont* textObj2 = (CCLabelBMFont*)self->getChildByTag(4001);

	CCLabelBMFont* textObj = CCLabelBMFont::create(getAccuracyText().c_str(), "goldFont.fnt");
	textObj->setZOrder(1000);
	textObj->setTag(4000);
	textObj->setScale(0.5);
	auto size = textObj->getScaledContentSize();
	textObj->setPosition({ size.width / 2 + 3, size.height / 2 + 3});
	self->addChild(textObj);

	CCLabelBMFont* textObj2 = CCLabelBMFont::create(getFramerateText().c_str(), "goldFont.fnt");
	textObj2->setZOrder(1000);
	textObj2->setTag(4001);
	textObj2->setScale(0.5);
	size = textObj2->getScaledContentSize();
	textObj2->setPosition({ size.width / 2 + 3, size.height / 2 + 3});
	self->addChild(textObj2);
	
	return init(self, GJGameLevel);
}

bool __fastcall PlayLayer::pushButtonHook(void* self, uintptr_t, int state, bool player) {
	totalClicks++;
	return PlayLayer::pushButton(self, state, player);
}
bool __fastcall PlayLayer::releaseButtonHook(void* self, uintptr_t, int state, bool player) {
	return PlayLayer::releaseButton(self, state, player);
}

void __fastcall PlayLayer::togglePracticeHook(void* self, int edx, bool practice) {
	size_t base = (size_t)GetModuleHandle(0);

	startposFix.inPractice = practice;

	return PlayLayer::togglePractice(self, practice);
}
int __fastcall PlayLayer::deathHook(void* self, void*, void* go, void* powerrangers) {
	if (!NoClipEnabled)
	{
		totalClicks = 0;
	}

	if (NoClipEnabled)
	{
		noclipacc.wouldDie = true;
	}
	return PlayLayer::death(self, go, powerrangers);
}
void __fastcall PlayLayer::updateHook(cocos2d::CCLayer* self, void* edx, float delta) {
	void* player1 = *(void**)((char*)self + 0x224);
	float x = *(float*)((size_t)player1 + 0x67c);
	float time = cocos2d::CCDirector::sharedDirector()->getAnimationInterval();
	if (startposFix.smoothOut != 0 && delta - time < 1) { // if close enough to normal speed
		startposFix.smoothOut --;
	}
	
	if (x != noclipacc.prevX) {
		noclipacc.frames += 1;
		noclipacc.totalDelta += delta;
	}

	size_t base = (size_t)GetModuleHandle(0);
	CCLabelBMFont* textObj = (CCLabelBMFont*)self->getChildByTag(4000);
	CCLabelBMFont* textObj2 = (CCLabelBMFont*)self->getChildByTag(4001);
	
	if (NoClipAccEnabled) {
		auto size = textObj->getScaledContentSize();
		textObj->setPosition({ size.width / 2 + 3, size.height / 2 + 3});
		textObj->setString(getAccuracyText().c_str());
		textObj->setVisible(true);
	} else {
		textObj->setVisible(false);
	}
	if (noclipacc.wouldDie) {
		noclipacc.wouldDie = false;
		if (noclipacc.totalDelta >= 0.1 && x != noclipacc.prevX) {
			noclipacc.deaths += 1;
		}
	}
	noclipacc.prevX = x;


	if (FPSCounterEnabled)
	{
		if (NoClipAccEnabled)
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

int __fastcall PlayLayer::resetHook(void* self) {
	void* player1 = *(void**)((char*)self + 0x224);
	noclipacc.prevX = *(float*)((size_t)player1 + 0x67c);
	noclipacc.frames = 0;
	noclipacc.totalDelta = 0;
	noclipacc.deaths = 0;
	noclipacc.wouldDie = false;


	if (startposFix.inTestmode || startposFix.inPractice) {
		startposFix.smoothOut = 2; // Account for 1 extra frame respawn
	}
	return resetLevel(self);
}

void PlayLayer::mem_init() {
	MH_Initialize();

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

	MH_EnableHook(MH_ALL_HOOKS);
}


void checkHacks(){
	if (NoClipEnabled){
	WriteBytes((void*)(gd::base + 0x20A23C), {0xE9, 0x79, 0x06, 0x00, 0x00});
	} else {
		WriteBytes((void*)(gd::base + 0x20A23C), {0x6A, 0x14, 0x8B, 0xCB, 0xFF});
	}
	if (NoSpikesEnabled){
		WriteBytes((void*)(gd::base + 0x205347), {0x75, });
	} else {
		WriteBytes((void*)(gd::base + 0x205347), {0x74, });
	}
	if (ForceBlockTypeEnabled){
		WriteBytes((void*)(gd::base + 0x20456D), {0x31, 0xC0, 0x83, 0x7B, 0x34, 0x00, 0xBA, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x44, 0xC2, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x20456D), {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00, 0x83, 0xF8, 0x07, 0x0F, 0x84, 0x7F, 0x0A, 0x00, 0x00, });
	}
	if (EverythingHurtsEnabled){
		WriteBytes((void*)(gd::base + 0x20456D), {0xB8, 0x02, 0x00, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x20456D), {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00, });
	}
	if (FreezePlayerEnabled){
		WriteBytes((void*)(gd::base + 0x203519), {0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x203519), {0x50, 0xFF, 0xD6, });
	}
	if (JumpHackEnabled){
		WriteBytes((void*)(gd::base + 0x1E9141), {0x01, });
		WriteBytes((void*)(gd::base + 0x1E9498), {0x01, });
	} else {
		WriteBytes((void*)(gd::base + 0x1E9141), {0x00, });
		WriteBytes((void*)(gd::base + 0x1E9498), {0x00, });
	}
	if (ForceTrailStateEnabled){
		WriteBytes((void*)(cocosBase + 0xAEDCC), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(cocosBase + 0xAEDCC), {0x0F, 0x84, 0x68, 0x02, 0x00, 0x00, });
	}
	if (HideAttemptsEnabled){
		WriteBytes((void*)(gd::base + 0x2D83B8), {0x00});
	} else {
		WriteBytes((void*)(gd::base + 0x2D83B8), {0x41});
	}
	if (PracticeMusicHackEnabled){
		WriteBytes((void*)(gd::base + 0x20C925), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
		WriteBytes((void*)(gd::base + 0x20D143), {0x90, 0x90});
		WriteBytes((void*)(gd::base + 0x20A563), {0x90, 0x90});
		WriteBytes((void*)(gd::base + 0x20A595), {0x90, 0x90});
	} else {
		WriteBytes((void*)(gd::base + 0x20C925), {0x0F, 0x85, 0xF7, 0x00, 0x00, 0x00});
		WriteBytes((void*)(gd::base + 0x20D143), {0x75, 0x41});
		WriteBytes((void*)(gd::base + 0x20A563), {0x75, 0x3E});
		WriteBytes((void*)(gd::base + 0x20A595), {0x75, 0x0C});
	}
	if (NoPulseEnabled){
		WriteBytes((void*)(gd::base + 0x2060D9), {0xEB, 0x4A, });
	} else {
		WriteBytes((void*)(gd::base + 0x2060D9), {0x74, 0x4A, });
	}
	if (IgnoreESCEnabled){
		WriteBytes((void*)(gd::base + 0x1E644C), {0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1E644C), {0xE8, 0xBF, 0x73, 0x02, 0x00, });
	}
	if (SuicideEnabled){
		WriteBytes((void*)(gd::base + 0x203DA2), {0xE9, 0x57, 0x02, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x20401A), {0xE9, 0x27, 0x02, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x203DA2), {0x0F, 0x86, 0x56, 0x02, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x20401A), {0x0F, 0x87, 0x26, 0x02, 0x00, 0x00, });
	}
	if (NoParticlesEnabled){
		WriteBytes((void*)(cocosBase + 0xB8ED6), {0x00});
	} else {
		WriteBytes((void*)(cocosBase + 0xB8ED6), {0x01});
	}
	if (CopyHackEnabled){
		WriteBytes((void*)(gd::base + 0x179B8E), {0x90, 0x90});
		WriteBytes((void*)(gd::base + 0x176F5C), {0x8B, 0xCA, 0x90});
		WriteBytes((void*)(gd::base + 0x176FE5), {0xB0, 0x01, 0x90});
	} else {
		WriteBytes((void*)(gd::base + 0x179B8E), {0x75, 0x0E});
		WriteBytes((void*)(gd::base + 0x176F5C), {0x0F, 0x44, 0xCA});
		WriteBytes((void*)(gd::base + 0x176FE5), {0x0F, 0x95, 0xC0});
	}
	if (NoCMarkEnabled){
		WriteBytes((void*)(gd::base + 0xA6B8B), {0x2B, 0x87, 0xCC, 0x02, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x70E87), {0xEB, 0x26});
	} else {
		WriteBytes((void*)(gd::base + 0xA6B8B), {0x2B, 0x87, 0xD0, 0x02, 0x00, 0x00});
		WriteBytes((void*)(gd::base + 0x70E87), {0x74, 0x26});
	}
	if (ObjectBypassEnabled){
		WriteBytes((void*)(gd::base + 0x73169), {0xFF, 0xFF, 0xFF, 0x7F, });
		WriteBytes((void*)(gd::base + 0x856A4), {0xFF, 0xFF, 0xFF, 0x7F, });
		WriteBytes((void*)(gd::base + 0x87B17), {0xFF, 0xFF, 0xFF, 0x7F, });
		WriteBytes((void*)(gd::base + 0x87BC7), {0xFF, 0xFF, 0xFF, 0x7F, });
		WriteBytes((void*)(gd::base + 0x87D95), {0xFF, 0xFF, 0xFF, 0x7F, });
		WriteBytes((void*)(gd::base + 0x880F4), {0xFF, 0xFF, 0xFF, 0x7F, });
		WriteBytes((void*)(gd::base + 0x160B06), {0xFF, 0xFF, 0xFF, 0x7F, });
	} else {
		WriteBytes((void*)(gd::base + 0x73169), {0x80, 0x38, 0x01, 0x00, });
		WriteBytes((void*)(gd::base + 0x856A4), {0x80, 0x38, 0x01, 0x00, });
		WriteBytes((void*)(gd::base + 0x87B17), {0x80, 0x38, 0x01, 0x00, });
		WriteBytes((void*)(gd::base + 0x87BC7), {0x80, 0x38, 0x01, 0x00, });
		WriteBytes((void*)(gd::base + 0x87D95), {0x80, 0x38, 0x01, 0x00, });
		WriteBytes((void*)(gd::base + 0x880F4), {0x80, 0x38, 0x01, 0x00, });
		WriteBytes((void*)(gd::base + 0x160B06), {0x80, 0x38, 0x01, 0x00, });
	}
	if (CustomObjectBypassEnabled){
		WriteBytes((void*)(gd::base + 0x7A100), {0xEB, });
		WriteBytes((void*)(gd::base + 0x7A022), {0xEB, });
		WriteBytes((void*)(gd::base + 0x7A203), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x7A100), {0x72, });
		WriteBytes((void*)(gd::base + 0x7A022), {0x76, });
		WriteBytes((void*)(gd::base + 0x7A203), {0x77, 0x3A, });
	}
	if (ZoomBypassEnabled){
		WriteBytes((void*)(gd::base + 0x87801), {0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x87806), {0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x87871), {0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x87876), {0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x87801), {0x0F, 0x2F, 0xC8, });
		WriteBytes((void*)(gd::base + 0x87806), {0x0F, 0x28, 0xC8, });
		WriteBytes((void*)(gd::base + 0x87871), {0x0F, 0x2F, 0xC8, });
		WriteBytes((void*)(gd::base + 0x87876), {0x0F, 0x28, 0xC8, });
	}
	if (ToolboxButtonBypassEnabled){
		WriteBytes((void*)(gd::base + 0x13A548), {0x83, 0xF9, 0x01, });
		WriteBytes((void*)(gd::base + 0x13A559), {0xB8, 0x01, 0x00, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x13A54D), {0x83, 0xF9, 0x7F, });
		WriteBytes((void*)(gd::base + 0x13A552), {0xB9, 0x7F, 0x00, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x13A5D8), {0x83, 0xF9, 0x01, });
		WriteBytes((void*)(gd::base + 0x13A5E9), {0xB8, 0x01, 0x00, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x13A5dd), {0x83, 0xF9, 0x7F, });
		WriteBytes((void*)(gd::base + 0x13A5E2), {0xB9, 0x7F, 0x00, 0x00, 0x00, });
	} else {
		WriteBytes((void*)(gd::base + 0x13A548), {0x83, 0xF9, 0x06, });
		WriteBytes((void*)(gd::base + 0x13A559), {0xB8, 0x06, 0x00, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x13A54D), {0x83, 0xF9, 0x0C, });
		WriteBytes((void*)(gd::base + 0x13A552), {0xB9, 0x0C, 0x00, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x13A5D8), {0x83, 0xF9, 0x02, });
		WriteBytes((void*)(gd::base + 0x13A5E9), {0xB8, 0x02, 0x00, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x13A5dd), {0x83, 0xF9, 0x03, });
		WriteBytes((void*)(gd::base + 0x13A5E2), {0xB9, 0x03, 0x00, 0x00, 0x00, });
	}
	if (VerifyHackEnabled){
		WriteBytes((void*)(gd::base + 0x71D48), {0xEB, });
	} else {
		WriteBytes((void*)(gd::base + 0x71D48), {0x74, });
	}
	if (DefaultSongBypassEnabled){
		WriteBytes((void*)(gd::base + 0x174407), {0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x174411), {0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x174456), {0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x174460), {0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x174407), {0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x174411), {0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x174456), {0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x174460), {0x90, 0x90, 0x90, });
	}
	if (LevelEditEnabled){
		WriteBytes((void*)(gd::base + 0x1E4A32), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1E4A32), {0x75, 0x6C, });
	}
	if (EditorExtensionEnabled){
		WriteBytes((void*)(gd::base + 0x2E67A4), {0x00, 0x60, 0xEA, 0x4B, });
		WriteBytes((void*)(gd::base + 0x8FA4D), {0x0F, 0x60, 0xEA, 0x4B, });
	} else {
		WriteBytes((void*)(gd::base + 0x2E67A4), {0x00, 0x60, 0x6A, 0x48, });
		WriteBytes((void*)(gd::base + 0x8FA4D), {0x80, 0x67, 0x6A, 0x48, });
	}
	if (PlaceOverEnabled){
		WriteBytes((void*)(gd::base + 0x160EE1), {0x8B, 0xC1, 0x90, });
		WriteBytes((void*)(gd::base + 0x160EF2), {0xE9, 0x23, 0x02, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x160EE1), {0x0F, 0x48, 0xC1, });
		WriteBytes((void*)(gd::base + 0x160EF2), {0x0F, 0x8F, 0x22, 0x02, 0x00, 0x00, });
	}
	if (TestmodeBypassEnabled){
		WriteBytes((void*)(gd::base + 0x1FD270), {0xE9, 0xB7, 0x00, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1FD270), {0x0F, 0x84, 0xB6, 0x00, 0x00, 0x00, });
	}
	if (RotationHackEnabled){
		WriteBytes((void*)(gd::base + 0x85CBC), {0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x8BDDD), {0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x8BE16), {0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0xECA3D), {0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0xEE5A9), {0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x20181E), {0xB8, 0x01, 0x00, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x85CBC), {0x8B, 0x80, 0x00, 0x03, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x8BDDD), {0x8B, 0x80, 0x00, 0x03, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x8BE16), {0x8B, 0x80, 0x00, 0x03, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0xECA3D), {0x8B, 0x87, 0x00, 0x03, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0xEE5A9), {0x8B, 0x86, 0x00, 0x03, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x20181E), {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00, });
	}
	if (FreeScrollEnabled){
		WriteBytes((void*)(gd::base + 0x8FAAC), {0xEB, });
		WriteBytes((void*)(gd::base + 0x8FA95), {0xEB, });
		WriteBytes((void*)(gd::base + 0x8FAC5), {0xEB, });
		WriteBytes((void*)(gd::base + 0x8FADC), {0xEB, });
	} else {
		WriteBytes((void*)(gd::base + 0x8FAAC), {0x77, });
		WriteBytes((void*)(gd::base + 0x8FA95), {0x77, });
		WriteBytes((void*)(gd::base + 0x8FAC5), {0x77, });
		WriteBytes((void*)(gd::base + 0x8FADC), {0x77, });
	}
	if (HideUIEnabled){
		WriteBytes((void*)(gd::base + 0x8720A), {0xB3, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x8720A), {0x0F, 0x44, 0xD9, });
	}
	if (ZOrderBypassEnabled){
		WriteBytes((void*)(gd::base + 0x22DEDE), {0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x22DEE8), {0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x22DEDE), {0x0F, 0x4C, 0xC1, });
		WriteBytes((void*)(gd::base + 0x22DEE8), {0x0F, 0x4F, 0xC1, });
	}
	
	if (speedhackEnabled)
	{
		if (speedhackAudioEnabled){
			SpeedhackAudio::set(speed);
			cocos2d::CCDirector::sharedDirector()->getScheduler()->setTimeScale(speed);
		} else {
			SpeedhackAudio::set(1);
			cocos2d::CCDirector::sharedDirector()->getScheduler()->setTimeScale(speed);
		}
	} else {
		SpeedhackAudio::set(1);
		cocos2d::CCDirector::sharedDirector()->getScheduler()->setTimeScale(1);
	}
	if (IconsEnabled){
		WriteBytes((void*)(gd::base + 0xC50A8), {0xB0, 0x01, 0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0xC54BA), {0xB0, 0x01, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0xC50A8), {0xE8, 0x7A, 0xCD, 0x19, 0x00, });
		WriteBytes((void*)(gd::base + 0xC54BA), {0xE8, 0x68, 0xC9, 0x19, 0x00, });
	}
	if (TextLengthEnabled){
		WriteBytes((void*)(gd::base + 0x21ACB), {0xEB, 0x04, });
	} else {
		WriteBytes((void*)(gd::base + 0x21ACB), {0x7C, 0x04, });
	}
	if (CharacterFilterEnabled){
		WriteBytes((void*)(gd::base + 0x21A99), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x21A99), {0x75, 0x04, });
	}
	if (SliderLimitEnabled){
		WriteBytes((void*)(gd::base + 0x2E5CA), {0xEB, });
		WriteBytes((void*)(gd::base + 0x2E5F8), {0xEB, });
	} else {
		WriteBytes((void*)(gd::base + 0x2E5CA), {0x76, });
		WriteBytes((void*)(gd::base + 0x2E5F8), {0x76, });
	}
	if (MainLevelsEnabled){
		WriteBytes((void*)(gd::base + 0x188CE1), {0xE9, 0x8A, 0x00, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x188CE1), {0x0F, 0x8E, 0x89, 0x00, 0x00, 0x00, });
	}
	if (GuardVaultEnabled){
		WriteBytes((void*)(gd::base + 0x1DE1DA), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1DE1DA), {0x7C, 0x4A, });
	}
	if (KeymasterVaultEnabled){
		WriteBytes((void*)(gd::base + 0x4F268), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x4F268), {0x74, 0x4A, });
	}
	if (BasementKeyBypassEnabled){
		WriteBytes((void*)(gd::base + 0x226E19), {0xE9, 0x59, 0x01, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x226FB8), {0xE9, 0x59, 0x01, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x227157), {0xE9, 0x30, 0x02, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x226E19), {0x0F, 0x85, 0x58, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x226FB8), {0x0F, 0x85, 0x58, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x227157), {0x0F, 0x85, 0x2F, 0x02, 0x00, 0x00, });
	}
	if (ChallengeBypassEnabled){
		WriteBytes((void*)(gd::base + 0x2214E0), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x2214E0), {0x0F, 0x84, 0x87, 0x00, 0x00, 0x00, });
	}
	if (TreasureRoomEnabled){
		WriteBytes((void*)(gd::base + 0x4F631), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x4F631), {0x74, 0x4A, });
	}
	if (PotborShopEnabled){
		WriteBytes((void*)(gd::base + 0x15706B), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x15706B), {0x0F, 0x8C, 0xB4, 0x01, 0x00, 0x00, 0x00, });
	}
	if (ScratchShopEnabled){
		WriteBytes((void*)(gd::base + 0x1562D3), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1562D3), {0x0F, 0x8C, 0xAF, 0x01, 0x00, 0x00, 0x00, });
	}
	if (FreeShopItemsEnabled){
		WriteBytes((void*)(gd::base + 0xF33BB), {0x8B, 0x93, 0x10, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x14B339), {0x2B, 0xB3, 0x10, 0x01, 0x00, 0x00, });
	} else {
		WriteBytes((void*)(gd::base + 0xF33BB), {0x8B, 0x93, 0x14, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x14B339), {0x2B, 0xB3, 0x14, 0x01, 0x00, 0x00, });
	}
	if (GatekeeperVaultEnabled){
		WriteBytes((void*)(gd::base + 0x188836), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x188836), {0x74, 0x61, });
	}
	if (BackupStarsLimitEnabled){
		WriteBytes((void*)(gd::base + 0x3928E), {0xEB, 0x3E, });
	} else {
		WriteBytes((void*)(gd::base + 0x3928E), {0x7D, 0x3E, });
	}
	if (UnblockHackEnabled){
		WriteBytes((void*)(gd::base + 0x29C0E8), {0x61, 0x48, 0x52, 0x30, 0x63, 0x48, 0x4D, 0x36, 0x4C, 0x79, 0x39, 0x68, 0x59, 0x6E, 0x4E, 0x76, 0x62, 0x47, 0x78, 0x73, 0x64, 0x58, 0x52, 0x6C, 0x4C, 0x6D, 0x4E, 0x76, 0x62, 0x53, 0x39, 0x68, 0x63, 0x47, 0x6B, 0x76, 0x5A, 0x32, 0x52, 0x66, 0x64, 0x58, 0x4E, 0x6C, 0x63, 0x6D, 0x6C, 0x75, 0x5A, 0x6D, 0x39, 0x66, 0x63, 0x33, 0x42, 0x76, 0x62, 0x32, 0x59, 0x3D, 0x00, });
	} else {
		WriteBytes((void*)(gd::base + 0x29C0E8), {0x61, 0x48, 0x52, 0x30, 0x63, 0x44, 0x6F, 0x76, 0x4C, 0x33, 0x64, 0x33, 0x64, 0x79, 0x35, 0x69, 0x62, 0x32, 0x39, 0x74, 0x62, 0x47, 0x6C, 0x75, 0x5A, 0x33, 0x4D, 0x75, 0x59, 0x32, 0x39, 0x74, 0x4C, 0x32, 0x52, 0x68, 0x64, 0x47, 0x46, 0x69, 0x59, 0x58, 0x4E, 0x6C, 0x4C, 0x32, 0x64, 0x6C, 0x64, 0x45, 0x64, 0x4B, 0x56, 0x58, 0x4E, 0x6C, 0x63, 0x6B, 0x6C, 0x75, 0x5A, 0x6D, 0x38, 0x79, 0x4D, 0x43, 0x35, 0x77, 0x61, 0x48, 0x41, 0x3D, 0x00, });
	}
	if (AccuratePercentageEnabled)
	{
		WriteBytes((void*)(gd::base + 0x2080FB), {0xFF, 0x50, 0x64, 0xF3, 0x0F, 0x10, 0x00, 0x8B, 0x87, 0xC0, 0x03, 0x00, 0x00, 0x83, 0xEC, 0x08, 0x42});
		WriteBytes((void*)(gd::base + 0x208114), { 0xF3, 0x0F, 0x5E, 0x87, 0xB4, 0x03, 0x00, 0x00, 0xC7, 0x02, 0x25, 0x2E, 0x32, 0x66, 0xC7, 0x42, 0x04, 0x25, 0x25, 0x00, 0x00, 0x8B, 0xB0, 0x04, 0x01, 0x00, 0x00, 0xF3, 0x0F, 0x5A, 0xC0, 0xF2, 0x0F, 0x11, 0x04, 0x24, 0x52});
		WriteBytes((void*)(gd::base + 0x20813F), {0x83, 0xC4, 0x0C});
	} else {
		WriteBytes((void*)(gd::base + 0x2080FB), {0xFF, 0x50, 0x64, 0xF3, 0x0F, 0x10, 0x00, 0x8B, 0x87, 0xC0, 0x03, 0x00, 0x00, 0x83, 0xEC, 0x08, 0x42});
		WriteBytes((void*)(gd::base + 0x208114), {0xF3, 0x0F, 0x5E, 0x87, 0xB4, 0x03, 0x00, 0x00, 0xC7, 0x02, 0x25, 0x2E, 0x30, 0x66, 0xC7, 0x42, 0x04, 0x25, 0x25, 0x00, 0x00, 0x8B, 0xB0, 0x04, 0x01, 0x00, 0x00, 0xF3, 0x0F, 0x5A, 0xC0, 0xF2, 0x0F, 0x11, 0x04, 0x24, 0x52});
		WriteBytes((void*)(gd::base + 0x20813F), {0x83, 0xC4, 0x0C});
	}
	if (OnlyPercentageEnabled)
	{
		WriteBytes((void*)(gd::base + 0x1FCE89), { 0x0F, 0x57, 0xC0, 0x90, 0x90, 0x90 });
		WriteBytes((void*)(gd::base + 0x1FCF38), { 0x0D });
		WriteBytes((void*)(gd::base + 0x1FCF6B), { 0x3F });
	} else {
		WriteBytes((void*)(gd::base + 0x1FCE89), { 0xF3, 0x0F, 0x10, 0x44, 0x24, 0x48 });
		WriteBytes((void*)(gd::base + 0x1FCF38), { 0x05 });
		WriteBytes((void*)(gd::base + 0x1FCF6B), { 0x00 });
	}
}

	


static void ShowPlayerHacks(){
	ImGui::Begin("Player", NULL, ImGuiWindowFlags_NoMove);
	
	ImGui::SetWindowSize(ImVec2(210, 380));
	
	ImGui::SetWindowPos(ImVec2(10, 10));


	ImGui::Checkbox("NoClip", &NoClipEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Makes the player invincible. (Safe patch.)");
	
	ImGui::Checkbox("NoSpikes", &NoSpikesEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Ignores spike objects.");

	ImGui::Checkbox("Force Block Type", &ForceBlockTypeEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Treats all objects as if they were blocks.");

	ImGui::Checkbox("Everything Hurts", &EverythingHurtsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Ouch.");

	ImGui::Checkbox("Freeze Player", &FreezePlayerEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Freezes player movement.");

	ImGui::Checkbox("Jump Hack", &JumpHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for jumping in mid-air.");

	ImGui::Checkbox("Force Trail State", &ForceTrailStateEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Sets the trail to always on.");

	ImGui::Checkbox("Hide Attempts", &HideAttemptsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Hides the attempt count in-game.");

	ImGui::Checkbox("Practice Music Hack", &PracticeMusicHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Plays the level's song in-sync with your position, instead of the standard practice song.");

	ImGui::Checkbox("No Pulse", &NoPulseEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Disables pulsing on objects.");

	ImGui::Checkbox("Ignore ESC", &IgnoreESCEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Stops the ESC key from exiting a level.");

	
	ImGui::Checkbox("Suicide", &SuicideEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Kills the player.");


	ImGui::Checkbox("Accurate Percentage", &AccuratePercentageEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for decimals in the level percentage.");

	ImGui::Checkbox("Only percentage", &OnlyPercentageEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Disables progress bar");




	/*
	ImGui::Checkbox("Instant Complete", &InstantCompleteEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Teleports the player to the end of a level, also know as the teleport hack.");
	if (InstantCompleteEnabled){
		
		WriteBytes((void*)(gd::base + 0x20350D), {0xC7, 0x81, 0x7C, 0x06, 0x00, 0x00, 0x20, 0xBC, 0xBE, 0x4C, 0x90, 0x90, 0x90, 0x90, 0x90});
	} else {
		WriteBytes((void*)(gd::base + 0x20350D), {0x8C, 0x03, 0x00, 0x00, 0x8B, 0x8E, 0x1C, 0x01, 0x00, 0x00, 0x8B, 0x01, 0xFF, 0x50, 0x64});
	} */
	ImGui::Checkbox("No Particles", &NoParticlesEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Disables resuming the particle system.");

	ImGui::End();

}
static void ShowCreatorHacks(){
	ImGui::Begin("Creator", NULL, ImGuiWindowFlags_NoMove);

	ImGui::SetWindowSize(ImVec2(210, 400));
	ImGui::SetWindowPos(ImVec2(230, 10));

	ImGui::Checkbox("Copy Hack", &CopyHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you copy any level, without a password.");

	ImGui::Checkbox("No (C) Mark", &NoCMarkEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removes the (C) mark when uploading copied levels.");

	ImGui::Checkbox("Level Edit", &LevelEditEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you edit any level, access the editor through the pause menu.");

	ImGui::Checkbox("Object Bypass", &ObjectBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Sets the object limit to around 2 billion.");

	ImGui::Checkbox("Custom Object Bypass", &CustomObjectBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removes the object limit for custom objects & lets you save over 50.");

	ImGui::Checkbox("Zoom Bypass", &ZoomBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you zoom an infinite amount in the editor. (NOTE: Can crash with an edited grid size.)");

	ImGui::Checkbox("Toolbox Button Bypass", &ToolboxButtonBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for more objects in the editor toolbox.");

	ImGui::Checkbox("Verify Hack", &VerifyHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you upload unverified levels.");

	ImGui::Checkbox("Default Song Bypass", &DefaultSongBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you use hidden default songs in the editor.");

	ImGui::Checkbox("Editor Extension", &EditorExtensionEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Increases the editor length by a factor of 128.");

	ImGui::Checkbox("Place Over", &PlaceOverEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you place the same object over itself in the editor.");

	ImGui::Checkbox("Testmode Bypass", &TestmodeBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Hides the 'Testmode' text when playing with a startpos.");

	ImGui::Checkbox("Rotation Hack", &RotationHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows you to rotate any object. Only works locally.");

	ImGui::Checkbox("Free Scroll", &FreeScrollEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows you to scroll out the editor.");

	ImGui::Checkbox("Hide UI", &HideUIEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Hides the Editor UI while building.");

	ImGui::Checkbox("Z Order Bypass", &ZOrderBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Removed the -100 to 100 Z order range limit.");
	ImGui::End();

}
static void ShowGlobalHacks(){
	if (!ImGui::CollapsingHeader("Global"))
		return;
}
static void ShowBypassHacks(){
	ImGui::Begin("Bypass", NULL, ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize(ImVec2(210, 430));
	ImGui::SetWindowPos(ImVec2(450, 10));
	ImGui::Checkbox("Icons", &IconsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks all icons.");
	ImGui::Checkbox("Text Length", &TextLengthEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for unlimited text length in text inputs.");
	ImGui::Checkbox("Character Filter", &CharacterFilterEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you input any character in all text inputs.");
	ImGui::Checkbox("Slider Limit", &SliderLimitEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets sliders be dragged beyond the visible limit.");
	ImGui::Checkbox("Main Levels", &MainLevelsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks locked demon levels.");
	ImGui::Checkbox("Guard Vault", &GuardVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the guard's vault.");
	ImGui::Checkbox("Keymaster Vault", &KeymasterVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the keymaster's vault.");
	ImGui::Checkbox("Keymaster Basement", &KeymasterBasementEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the keymaster's basement.");
	ImGui::Checkbox("Basement Key Bypass", &BasementKeyBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you unlock the locks in the basement.");
	ImGui::Checkbox("Challenge Bypass", &ChallengeBypassEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks 'The Challenge' level.");
	ImGui::Checkbox("Treasure Room", &TreasureRoomEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the treasure room.");
	ImGui::Checkbox("Potbor Shop", &PotborShopEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks Potbor's shop in the treasure room.");
	ImGui::Checkbox("Scratch Shop", &ScratchShopEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks Scratch's shop in the treasure room.");
	ImGui::Checkbox("Free Shop Items", &FreeShopItemsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows all shop items to be bought for 0 mana orbs.");
	ImGui::Checkbox("Gatekeeper Vault", &GatekeeperVaultEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Unlocks the Gatekeeper's vault.");
	ImGui::Checkbox("Backup Stars Limit", &BackupStarsLimitEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you backup data, even with less than 10 stars.");
	ImGui::Checkbox("Unblock Hack", &UnblockHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Lets you view profiles of users who have blocked you.");
	ImGui::End();
}
static void ShowSpeedhack(){
	ImGui::Begin("Speedhack", NULL, ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize(ImVec2(210, 110));
	ImGui::SetWindowPos(ImVec2(10, 400));
	
	if (GetAsyncKeyState(VK_F2) & 5)
	{
		speed += f2_offset;
	}
	if (GetAsyncKeyState(VK_F1) & 5)
	{
		speed -= f1_offset;
	}
	ImGui::SliderFloat("x", &speed, 0.0f, 20.0f, "%.1f");
	ImGui::Checkbox("Enabled", &speedhackEnabled);
	ImGui::Checkbox("Speedhack audio", &speedhackAudioEnabled);
	
	// ImGui::InputFloat2("F1 and F2 offsets", offsets, "%.1f");
	/*ImGui::InputFloat("F1 Offset", &f1_offset, 0.5, 1, "%.1f");
	if(ImGui::IsItemHovered()){
		ImGui::SetTooltip("Note: Use F1 to decrease speed and F2 to increase :)");
	}
	ImGui::InputFloat("F2 Offset", &f2_offset, 0.5, 1, "%.1f");
	if(ImGui::IsItemHovered()){
		ImGui::SetTooltip("Note: Use F1 to decrease speed and F2 to increase :)");
	} */
	ImGui::End();
}
static void ShowFPSBypass(){
	ImGui::Begin("FPS Bypass", NULL, ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize(ImVec2(210, 80));
	ImGui::SetWindowPos(ImVec2(230, 420));

	ImGui::InputFloat("FPS", &interval, 10.f, 20.f, "%.1f");
	ImGui::Checkbox("Enabled", &FPSBypassEnabled);

	ImGui::End();
}

static void ShowStatus(){
	ImGui::Begin("Status", NULL, ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize(ImVec2(210, 84));
	ImGui::SetWindowPos(ImVec2(670, 10));
	ImGui::Checkbox("Enable NoClip Accuracy", &NoClipAccEnabled);
	ImGui::Checkbox("Enable FPS Counter", &FPSCounterEnabled);
	/* if (ImGui::Button("Inject Dll")) {
    std::string stringpath = chooseDLL();
    const char* DllPath = stringpath.c_str();
    HWND hWnd = FindWindow(0, "Geometry Dash");
        DWORD proccess_ID;
        GetWindowThreadProcessId(hWnd, &proccess_ID);
        HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proccess_ID);
        LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(DllPath) + 1,
        MEM_COMMIT, PAGE_READWRITE);
        WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath,
        strlen(DllPath) + 1, 0);
        HANDLE hLoadThread = CreateRemoteThread(hProcess, 0, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA"), pDllPath, 0, 0);
        WaitForSingleObject(hLoadThread, INFINITE);     
		VirtualFreeEx(hProcess, pDllPath, strlen(DllPath) + 1, MEM_RELEASE);
	} */
	ImGui::End();
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
	ImGui::Checkbox("Enable FPS Counter", &FPSCounterEnabled);
	
}
static void ShowAboutWindow(){
	ImGui::Begin("About Mod Menu", NULL, ImGuiWindowFlags_NoMove);
	ImGui::SetWindowSize(ImVec2(210, 200));
	ImGui::SetWindowPos(ImVec2(670, 104));
    ImGui::Text("Mod Menu %s", VERSION);
    ImGui::Separator();
    ImGui::Text("By Alexandr Simonov");
    ImGui::Text("Mod Menu is licensed\nunder the MIT License.");
	ImGui::Separator();
	ImGui::Text("Used libraries: ");
	ImGui::Text("ImGui: %s", ImGui::GetVersion());
	ImGui::Text("MinHook: %s", "1.3.3");
	ImGui::Text("SimpleIni: %s", "4.19");

	if(ImGui::Button("View on GitHub"))
	{
		ShellExecute(0, 0, LPCSTR("https://github.com/OneParsec/GD-ModMenu"), 0, 0 , SW_SHOW );
	}
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
	ShowPlayerHacks();
	ShowCreatorHacks();
	ShowBypassHacks();
	ShowSpeedhack();
	ShowFPSBypass();
	ShowStatus();
	ShowAboutWindow();
	if (DEVELOPER_MODE)
	{
		ShowDeveloper();
		// ImGui::ShowDemoWindow();
	}
}


void MainThread() 
{
	const bool enable_touch = !ImGui::GetIO().WantCaptureMouse || !show;
	cocos2d::CCDirector::sharedDirector()->getTouchDispatcher()->setDispatchEvents(enable_touch);
	SpeedhackAudio::init();
	LoadingLayer::mem_init();
	PlayLayer::mem_init();
	checkHacks();
	disableAnticheat();
	DEVMODE dm;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	if (FPSBypassEnabled)
	{
		SetTargetFPS(interval);
	} else {
		SetTargetFPS(dm.dmDisplayFrequency);
	}
	if (GetAsyncKeyState(VK_INSERT) & 1) {
    	show = !show;
	}
	if (show) {
		saveHacks();
		MainWindow();
	}
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		loadHacks();
		ImGuiHook::Load(MainThread);
		break;
	case DLL_PROCESS_DETACH:
		saveHacks();
		ImGuiHook::Unload();
		break;
	}
	return TRUE;
}