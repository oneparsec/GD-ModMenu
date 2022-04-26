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
#define VERSION "1.0"
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
#include <imgui_hook.h>
#include <imgui.h>
#include <MinHook.h>
#include <gd.h>
#include <iostream>
#include <Psapi.h>
#include <fstream>
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
	bool speedhackEnabled;
	bool speedhackAudioEnabled;
	// player
	bool NoClipEnabled;
	bool NoSpikesEnabled;
	bool ForceBlockTypeEnabled;
	bool EverythingHurtsEnabled;
	bool FreezePlayerEnabled;
	bool JumpHackEnabled;
	bool ForceTrailStateEnabled;
	bool InversedTrailEnabled;
	bool HideAttemptsEnabled;
	bool PracticeMusicHackEnabled;
	bool NoPulseEnabled;
	bool IgnoreESCEnabled;
	bool SuicideEnabled;
	bool AccuratePercentageEnabled;
	bool OnlyPercentageEnabled;
	bool NoParticlesEnabled;
	bool InstantCompleteEnabled;
	// creator
	bool CopyHackEnabled;
	bool NoCMarkEnabled;
	bool LevelEditEnabled;
	bool ObjectBypassEnabled;
	bool CustomObjectBypassEnabled;
	bool ZoomBypassEnabled;
	bool ToolboxButtonBypassEnabled;
	bool VerifyHackEnabled;
	bool DefaultSongBypassEnabled;
	bool EditorExtensionEnabled;
	bool PlaceOverEnabled;
	bool TestmodeBypassEnabled;
	bool RotationHackEnabled;
	bool FreeScrollEnabled;
	bool HideUIEnabled;
	bool ZOrderBypassEnabled;
	// fps bypass
	bool FPSBypassEnabled;
	float interval = 60.f;
	// speedhack
	int currentPlace = 0;
	float speed = 1.f;
	float f1_offset = 0.5f;
	float f2_offset = 0.5f;
	// bypass
	bool SongBypassEnabled;
	bool IconsEnabled;
	bool TextLengthEnabled;
	bool CharacterFilterEnabled;
	bool SliderLimitEnabled;
	bool MainLevelsEnabled;
	bool GuardVaultEnabled;
	bool KeymasterVaultEnabled;
	bool KeymasterBasementEnabled;
	bool BasementKeyBypassEnabled;
	bool ChallengeBypassEnabled;
	bool TreasureRoomEnabled;
	bool PotborShopEnabled;
	bool ScratchShopEnabled;
	bool FreeShopItemsEnabled;
	bool GatekeeperVaultEnabled;
	bool BackupStarsLimitEnabled;
	bool UnblockHackEnabled;
	// status
	bool NoClipAccEnabled;
	bool FPSCounterEnabled;
	// other
	int currentTransition;
	bool MSAAEnabled;
	bool customBGEnabled = false;
	// universal
	bool ForceVisibilityEnabled;
	bool NoRotationEnabled;
	bool FreeWindowResizeEnabled;
	bool SafeModeEnabled;
	bool TransparentBGEnabled;
	bool TransparentListsEnabled;
	bool FastAltTabEnabled;
	bool AllowLowVolumeEnabled;
} setting;

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


using namespace cocos2d;
using namespace std;



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
    MH_EnableHook(MH_ALL_HOOKS);
    return true;
}

HWND hWnd;
DWORD procId;
HANDLE hProcess;
uint32_t base, alloc_base, alloc_offset;
size_t alloc_size;
std::map<std::string /*key*/, std::pair<uint32_t /*offset*/, size_t /*size*/>> alloc_map;
chrono::system_clock::time_point start = chrono::system_clock::now(), now;

chrono::duration<double> cycleTime;


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

bool Free(uint32_t vaddress, size_t size)
{
    return VirtualFreeEx(hProcess, reinterpret_cast<void*>(vaddress), size, MEM_RELEASE);
}

uint32_t Allocate(size_t size, uint32_t vaddress)
{
    return reinterpret_cast<uint32_t>(VirtualAllocEx(hProcess, reinterpret_cast<void*>(vaddress), size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
}

bool MemAlloc(size_t size)
{
    alloc_offset = 0;
    alloc_size = size;
    alloc_map.clear();
    return (alloc_base = Allocate(size, 0)) != 0;
}

bool AttemptAttach(const char *window, const char *process)
{
    if ((hWnd = FindWindowA(0, window)))
    {
        if (hProcess)
            Free(alloc_base, alloc_size);
        procId = 0;
        GetWindowThreadProcessId(hWnd, &procId);
        if ((hProcess = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION | PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION, FALSE, procId)))
        {
            base = GetModuleBase(process);
            MemAlloc(0x1000);
            return true;
        }
    }
    return false;
}
template<class T>
bool Write(uint32_t vaddress, const T& value) { return WriteProcessMemory(hProcess, reinterpret_cast<void*>(vaddress), &value, sizeof(T), NULL); }
bool WriteJump(uint32_t vaddress, uint32_t to) { return Write(vaddress, '\xE9') && Write(vaddress + 1, to - vaddress - 5); }

bool WriteB(uint32_t vaddress, const void *bytes, size_t size)
{
    return WriteProcessMemory(hProcess, reinterpret_cast<void*>(vaddress), bytes, size, NULL);
}

const char * filter = "Dynamic link library (*.dll)\0*.dll";
const char * filter2 = "JPEG image (*.jpg)\0*.jpg";


void WriteBytes(void* location, std::vector<BYTE> bytes) {
	DWORD old_prot;
	VirtualProtect(location, bytes.size(), PAGE_EXECUTE_READWRITE, &old_prot);

	memcpy(location, bytes.data(), bytes.size());

	VirtualProtect(location, bytes.size(), old_prot, &old_prot);
}

void setColors()
{
    // ImGui::GetIO().Fonts->AddFontFromFileTTF("../data/Fonts/Ruda-Bold.ttf", 15.0f, &config);
    ImGui::GetStyle().FrameRounding = 4.0f;
    ImGui::GetStyle().GrabRounding = 4.0f;
    
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
    colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

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


std::string choosePic() //dll
{
    OPENFILENAME ofn2;
    char fileName[MAX_PATH] = "";
    ZeroMemory(&ofn2, sizeof(ofn2));
	ofn2.lpstrFilter = filter2;
    ofn2.lStructSize = sizeof(OPENFILENAME);
    ofn2.lpstrFile = fileName;
    ofn2.nMaxFile = MAX_PATH;
    if (GetOpenFileName(&ofn2))
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

	inline bool(__thiscall* init)(gd::PlayLayer* self, void* GJGameLevel);
	bool __fastcall initHook(gd::PlayLayer* self, int edx, void* GJGameLevel);

	inline void(__thiscall* togglePractice)(void* self, bool practice);
	void __fastcall togglePracticeHook(void* self, int edx, bool practice);

	inline void(__thiscall* update)(cocos2d::CCLayer* self, float delta);
	void __fastcall updateHook(cocos2d::CCLayer* self, void*, float delta);

	inline int(__thiscall* resetLevel)(void* self);
	int __fastcall resetHook(gd::PlayLayer* self);

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
	totalClicks = 0;
	noclipacc.prevX = 0;
	noclipacc.created = true;
	startposFix.inPractice = false;
	startposFix.inTestmode = *(bool*)((uintptr_t)self + 0x494);
	startposFix.smoothOut = 0;
	
	const auto win_size = CCDirector::sharedDirector()->getWinSize();
	//void* player1 = *(void**)((char*)self + 0x224);
	//fromPercent.xPos = *(float*)((size_t)player1 + 0x67c);
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

	/*
	CCLabelBMFont* textObj3 = CCLabelBMFont::create("hi", "goldFont.fnt");
	textObj3->setZOrder(1000);
	textObj3->setTag(4002);
	textObj3->setScale(0.5);
	size = textObj3->getScaledContentSize();
	textObj3->setPosition({win_size.width / 2, win_size.height / 2});
	self->addChild(textObj3);
	*/
	// void* player1 = *(void**)((char*)self + 0x224);
	//fromPercent.xPos = *(float*)((size_t)player1 + 0x67c);
	// fromPercent.xPos = self->m_pPlayer1->m_position.x;
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
	
	if (setting.NoClipAccEnabled) {
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


	if (startposFix.inTestmode || startposFix.inPractice) {
		startposFix.smoothOut = 2; // Account for 1 extra frame respawn
	}
	return resetLevel(self);
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

	MH_EnableHook(MH_ALL_HOOKS);
}


void checkHacks(){


	if (setting.NoClipEnabled){
	WriteBytes((void*)(gd::base + 0x20A23C), {0xE9, 0x79, 0x06, 0x00, 0x00});
	} else {
		WriteBytes((void*)(gd::base + 0x20A23C), {0x6A, 0x14, 0x8B, 0xCB, 0xFF});
	}
	if (setting.NoSpikesEnabled){
		WriteBytes((void*)(gd::base + 0x205347), {0x75, });
	} else {
		WriteBytes((void*)(gd::base + 0x205347), {0x74, });
	}
	if (setting.ForceBlockTypeEnabled){
		WriteBytes((void*)(gd::base + 0x20456D), {0x31, 0xC0, 0x83, 0x7B, 0x34, 0x00, 0xBA, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x44, 0xC2, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x20456D), {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00, 0x83, 0xF8, 0x07, 0x0F, 0x84, 0x7F, 0x0A, 0x00, 0x00, });
	}
	if (setting.EverythingHurtsEnabled){
		WriteBytes((void*)(gd::base + 0x20456D), {0xB8, 0x02, 0x00, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x20456D), {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00, });
	}
	if (setting.FreezePlayerEnabled){
		WriteBytes((void*)(gd::base + 0x203519), {0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x203519), {0x50, 0xFF, 0xD6, });
	}
	if (setting.JumpHackEnabled){
		WriteBytes((void*)(gd::base + 0x1E9141), {0x01, });
		WriteBytes((void*)(gd::base + 0x1E9498), {0x01, });
	} else {
		WriteBytes((void*)(gd::base + 0x1E9141), {0x00, });
		WriteBytes((void*)(gd::base + 0x1E9498), {0x00, });
	}
	if (setting.ForceTrailStateEnabled){
		WriteBytes((void*)(cocosBase + 0xAEDCC), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(cocosBase + 0xAEDCC), {0x0F, 0x84, 0x68, 0x02, 0x00, 0x00, });
	}
	if (setting.HideAttemptsEnabled){
		WriteBytes((void*)(gd::base + 0x2D83B8), {0x00});
	} else {
		WriteBytes((void*)(gd::base + 0x2D83B8), {0x41});
	}
	if (setting.PracticeMusicHackEnabled){
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
	if (setting.NoPulseEnabled){
		WriteBytes((void*)(gd::base + 0x2060D9), {0xEB, 0x4A, });
	} else {
		WriteBytes((void*)(gd::base + 0x2060D9), {0x74, 0x4A, });
	}
	if (setting.IgnoreESCEnabled){
		WriteBytes((void*)(gd::base + 0x1E644C), {0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1E644C), {0xE8, 0xBF, 0x73, 0x02, 0x00, });
	}
	if (setting.SuicideEnabled){
		WriteBytes((void*)(gd::base + 0x203DA2), {0xE9, 0x57, 0x02, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x20401A), {0xE9, 0x27, 0x02, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x203DA2), {0x0F, 0x86, 0x56, 0x02, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x20401A), {0x0F, 0x87, 0x26, 0x02, 0x00, 0x00, });
	}
	if (setting.NoParticlesEnabled){
		WriteBytes((void*)(cocosBase + 0xB8ED6), {0x00});
	} else {
		WriteBytes((void*)(cocosBase + 0xB8ED6), {0x01});
	}
	if (setting.CopyHackEnabled){
		WriteBytes((void*)(gd::base + 0x179B8E), {0x90, 0x90});
		WriteBytes((void*)(gd::base + 0x176F5C), {0x8B, 0xCA, 0x90});
		WriteBytes((void*)(gd::base + 0x176FE5), {0xB0, 0x01, 0x90});
	} else {
		WriteBytes((void*)(gd::base + 0x179B8E), {0x75, 0x0E});
		WriteBytes((void*)(gd::base + 0x176F5C), {0x0F, 0x44, 0xCA});
		WriteBytes((void*)(gd::base + 0x176FE5), {0x0F, 0x95, 0xC0});
	}
	if (setting.NoCMarkEnabled){
		WriteBytes((void*)(gd::base + 0xA6B8B), {0x2B, 0x87, 0xCC, 0x02, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x70E87), {0xEB, 0x26});
	} else {
		WriteBytes((void*)(gd::base + 0xA6B8B), {0x2B, 0x87, 0xD0, 0x02, 0x00, 0x00});
		WriteBytes((void*)(gd::base + 0x70E87), {0x74, 0x26});
	}
	if (setting.ObjectBypassEnabled){
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
	if (setting.CustomObjectBypassEnabled){
		WriteBytes((void*)(gd::base + 0x7A100), {0xEB, });
		WriteBytes((void*)(gd::base + 0x7A022), {0xEB, });
		WriteBytes((void*)(gd::base + 0x7A203), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x7A100), {0x72, });
		WriteBytes((void*)(gd::base + 0x7A022), {0x76, });
		WriteBytes((void*)(gd::base + 0x7A203), {0x77, 0x3A, });
	}
	if (setting.ZoomBypassEnabled){
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
	if (setting.ToolboxButtonBypassEnabled){
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
	if (setting.VerifyHackEnabled){
		WriteBytes((void*)(gd::base + 0x71D48), {0xEB, });
	} else {
		WriteBytes((void*)(gd::base + 0x71D48), {0x74, });
	}
	if (setting.DefaultSongBypassEnabled){
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
	if (setting.LevelEditEnabled){
		WriteBytes((void*)(gd::base + 0x1E4A32), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1E4A32), {0x75, 0x6C, });
	}
	if (setting.EditorExtensionEnabled){
		WriteBytes((void*)(gd::base + 0x2E67A4), {0x00, 0x60, 0xEA, 0x4B, });
		WriteBytes((void*)(gd::base + 0x8FA4D), {0x0F, 0x60, 0xEA, 0x4B, });
	} else {
		WriteBytes((void*)(gd::base + 0x2E67A4), {0x00, 0x60, 0x6A, 0x48, });
		WriteBytes((void*)(gd::base + 0x8FA4D), {0x80, 0x67, 0x6A, 0x48, });
	}
	if (setting.PlaceOverEnabled){
		WriteBytes((void*)(gd::base + 0x160EE1), {0x8B, 0xC1, 0x90, });
		WriteBytes((void*)(gd::base + 0x160EF2), {0xE9, 0x23, 0x02, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x160EE1), {0x0F, 0x48, 0xC1, });
		WriteBytes((void*)(gd::base + 0x160EF2), {0x0F, 0x8F, 0x22, 0x02, 0x00, 0x00, });
	}
	if (setting.TestmodeBypassEnabled){
		WriteBytes((void*)(gd::base + 0x1FD270), {0xE9, 0xB7, 0x00, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1FD270), {0x0F, 0x84, 0xB6, 0x00, 0x00, 0x00, });
	}
	if (setting.RotationHackEnabled){
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
	if (setting.FreeScrollEnabled){
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
	if (setting.HideUIEnabled){
		WriteBytes((void*)(gd::base + 0x8720A), {0xB3, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x8720A), {0x0F, 0x44, 0xD9, });
	}
	if (setting.ZOrderBypassEnabled){
		WriteBytes((void*)(gd::base + 0x22DEDE), {0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0x22DEE8), {0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x22DEDE), {0x0F, 0x4C, 0xC1, });
		WriteBytes((void*)(gd::base + 0x22DEE8), {0x0F, 0x4F, 0xC1, });
	}
	
	if (setting.speedhackEnabled)
	{
		if (setting.speedhackAudioEnabled){
			SpeedhackAudio::set(setting.speed);
			cocos2d::CCDirector::sharedDirector()->getScheduler()->setTimeScale(setting.speed);
		} else {
			SpeedhackAudio::set(1);
			cocos2d::CCDirector::sharedDirector()->getScheduler()->setTimeScale(setting.speed);
		}
	} else {
		SpeedhackAudio::set(1);
		cocos2d::CCDirector::sharedDirector()->getScheduler()->setTimeScale(1);
	}
	if (setting.IconsEnabled){
		WriteBytes((void*)(gd::base + 0xC50A8), {0xB0, 0x01, 0x90, 0x90, 0x90, });
		WriteBytes((void*)(gd::base + 0xC54BA), {0xB0, 0x01, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0xC50A8), {0xE8, 0x7A, 0xCD, 0x19, 0x00, });
		WriteBytes((void*)(gd::base + 0xC54BA), {0xE8, 0x68, 0xC9, 0x19, 0x00, });
	}
	if (setting.TextLengthEnabled){
		WriteBytes((void*)(gd::base + 0x21ACB), {0xEB, 0x04, });
	} else {
		WriteBytes((void*)(gd::base + 0x21ACB), {0x7C, 0x04, });
	}
	if (setting.CharacterFilterEnabled){
		WriteBytes((void*)(gd::base + 0x21A99), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x21A99), {0x75, 0x04, });
	}
	if (setting.SliderLimitEnabled){
		WriteBytes((void*)(gd::base + 0x2E5CA), {0xEB, });
		WriteBytes((void*)(gd::base + 0x2E5F8), {0xEB, });
	} else {
		WriteBytes((void*)(gd::base + 0x2E5CA), {0x76, });
		WriteBytes((void*)(gd::base + 0x2E5F8), {0x76, });
	}
	if (setting.MainLevelsEnabled){
		WriteBytes((void*)(gd::base + 0x188CE1), {0xE9, 0x8A, 0x00, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x188CE1), {0x0F, 0x8E, 0x89, 0x00, 0x00, 0x00, });
	}
	if (setting.GuardVaultEnabled){
		WriteBytes((void*)(gd::base + 0x1DE1DA), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1DE1DA), {0x7C, 0x4A, });
	}
	if (setting.KeymasterVaultEnabled){
		WriteBytes((void*)(gd::base + 0x4F268), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x4F268), {0x74, 0x4A, });
	}
	if (setting.BasementKeyBypassEnabled){
		WriteBytes((void*)(gd::base + 0x226E19), {0xE9, 0x59, 0x01, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x226FB8), {0xE9, 0x59, 0x01, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x227157), {0xE9, 0x30, 0x02, 0x00, 0x00, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x226E19), {0x0F, 0x85, 0x58, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x226FB8), {0x0F, 0x85, 0x58, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x227157), {0x0F, 0x85, 0x2F, 0x02, 0x00, 0x00, });
	}
	if (setting.ChallengeBypassEnabled){
		WriteBytes((void*)(gd::base + 0x2214E0), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x2214E0), {0x0F, 0x84, 0x87, 0x00, 0x00, 0x00, });
	}
	if (setting.TreasureRoomEnabled){
		WriteBytes((void*)(gd::base + 0x4F631), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x4F631), {0x74, 0x4A, });
	}
	if (setting.PotborShopEnabled){
		WriteBytes((void*)(gd::base + 0x15706B), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x15706B), {0x0F, 0x8C, 0xB4, 0x01, 0x00, 0x00, 0x00, });
	}
	if (setting.ScratchShopEnabled){
		WriteBytes((void*)(gd::base + 0x1562D3), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x1562D3), {0x0F, 0x8C, 0xAF, 0x01, 0x00, 0x00, 0x00, });
	}
	if (setting.FreeShopItemsEnabled){
		WriteBytes((void*)(gd::base + 0xF33BB), {0x8B, 0x93, 0x10, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x14B339), {0x2B, 0xB3, 0x10, 0x01, 0x00, 0x00, });
	} else {
		WriteBytes((void*)(gd::base + 0xF33BB), {0x8B, 0x93, 0x14, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x14B339), {0x2B, 0xB3, 0x14, 0x01, 0x00, 0x00, });
	}
	if (setting.GatekeeperVaultEnabled){
		WriteBytes((void*)(gd::base + 0x188836), {0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x188836), {0x74, 0x61, });
	}
	if (setting.BackupStarsLimitEnabled){
		WriteBytes((void*)(gd::base + 0x3928E), {0xEB, 0x3E, });
	} else {
		WriteBytes((void*)(gd::base + 0x3928E), {0x7D, 0x3E, });
	}
	if (setting.UnblockHackEnabled){
		WriteBytes((void*)(gd::base + 0x29C0E8), {0x61, 0x48, 0x52, 0x30, 0x63, 0x48, 0x4D, 0x36, 0x4C, 0x79, 0x39, 0x68, 0x59, 0x6E, 0x4E, 0x76, 0x62, 0x47, 0x78, 0x73, 0x64, 0x58, 0x52, 0x6C, 0x4C, 0x6D, 0x4E, 0x76, 0x62, 0x53, 0x39, 0x68, 0x63, 0x47, 0x6B, 0x76, 0x5A, 0x32, 0x52, 0x66, 0x64, 0x58, 0x4E, 0x6C, 0x63, 0x6D, 0x6C, 0x75, 0x5A, 0x6D, 0x39, 0x66, 0x63, 0x33, 0x42, 0x76, 0x62, 0x32, 0x59, 0x3D, 0x00, });
	} else {
		WriteBytes((void*)(gd::base + 0x29C0E8), {0x61, 0x48, 0x52, 0x30, 0x63, 0x44, 0x6F, 0x76, 0x4C, 0x33, 0x64, 0x33, 0x64, 0x79, 0x35, 0x69, 0x62, 0x32, 0x39, 0x74, 0x62, 0x47, 0x6C, 0x75, 0x5A, 0x33, 0x4D, 0x75, 0x59, 0x32, 0x39, 0x74, 0x4C, 0x32, 0x52, 0x68, 0x64, 0x47, 0x46, 0x69, 0x59, 0x58, 0x4E, 0x6C, 0x4C, 0x32, 0x64, 0x6C, 0x64, 0x45, 0x64, 0x4B, 0x56, 0x58, 0x4E, 0x6C, 0x63, 0x6B, 0x6C, 0x75, 0x5A, 0x6D, 0x38, 0x79, 0x4D, 0x43, 0x35, 0x77, 0x61, 0x48, 0x41, 0x3D, 0x00, });
	}
	if (setting.AccuratePercentageEnabled)
	{
		WriteBytes((void*)(gd::base + 0x2080FB), {0xFF, 0x50, 0x64, 0xF3, 0x0F, 0x10, 0x00, 0x8B, 0x87, 0xC0, 0x03, 0x00, 0x00, 0x83, 0xEC, 0x08, 0x42});
		WriteBytes((void*)(gd::base + 0x208114), { 0xF3, 0x0F, 0x5E, 0x87, 0xB4, 0x03, 0x00, 0x00, 0xC7, 0x02, 0x25, 0x2E, 0x32, 0x66, 0xC7, 0x42, 0x04, 0x25, 0x25, 0x00, 0x00, 0x8B, 0xB0, 0x04, 0x01, 0x00, 0x00, 0xF3, 0x0F, 0x5A, 0xC0, 0xF2, 0x0F, 0x11, 0x04, 0x24, 0x52});
		WriteBytes((void*)(gd::base + 0x20813F), {0x83, 0xC4, 0x0C});
	} else {
		WriteBytes((void*)(gd::base + 0x2080FB), {0xFF, 0x50, 0x64, 0xF3, 0x0F, 0x10, 0x00, 0x8B, 0x87, 0xC0, 0x03, 0x00, 0x00, 0x83, 0xEC, 0x08, 0x42});
		WriteBytes((void*)(gd::base + 0x208114), {0xF3, 0x0F, 0x5E, 0x87, 0xB4, 0x03, 0x00, 0x00, 0xC7, 0x02, 0x25, 0x2E, 0x30, 0x66, 0xC7, 0x42, 0x04, 0x25, 0x25, 0x00, 0x00, 0x8B, 0xB0, 0x04, 0x01, 0x00, 0x00, 0xF3, 0x0F, 0x5A, 0xC0, 0xF2, 0x0F, 0x11, 0x04, 0x24, 0x52});
		WriteBytes((void*)(gd::base + 0x20813F), {0x83, 0xC4, 0x0C});
	}
	if (setting.OnlyPercentageEnabled)
	{
		WriteBytes((void*)(gd::base + 0x1FCE89), { 0x0F, 0x57, 0xC0, 0x90, 0x90, 0x90 });
		WriteBytes((void*)(gd::base + 0x1FCF38), { 0x0D });
		WriteBytes((void*)(gd::base + 0x1FCF6B), { 0x3F });
	} else {
		WriteBytes((void*)(gd::base + 0x1FCE89), { 0xF3, 0x0F, 0x10, 0x44, 0x24, 0x48 });
		WriteBytes((void*)(gd::base + 0x1FCF38), { 0x05 });
		WriteBytes((void*)(gd::base + 0x1FCF6B), { 0x00 });
	}
	if(setting.ForceVisibilityEnabled){
		WriteBytes((void*)(cocosBase + 0x60753), {0xB0, 0x01, 0x90, });
		WriteBytes((void*)(cocosBase + 0x60C5A), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(cocosBase + 0x60753), {0x8A, 0x45, 0x08, });
		WriteBytes((void*)(cocosBase + 0x60C5A), {0x0F, 0x84, 0xCB, 0x00, 0x00, 0x00, });
	}
	if(setting.NoRotationEnabled){
		WriteBytes((void*)(cocosBase + 0x60554), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(cocosBase + 0x60554), {0xF3, 0x0F, 0x11, 0x49, 0x24, 0xF3, 0x0F, 0x11, 0x49, 0x20, });
	}
	if(setting.FreeWindowResizeEnabled){
		WriteBytes((void*)(cocosBase + 0x11388B), {0x90, 0x90, 0x90, 0x90, 0x90, });
		WriteBytes((void*)(cocosBase + 0x11339D), {0xB9, 0xFF, 0xFF, 0xFF, 0x7F, 0x90, 0x90, });
		WriteBytes((void*)(cocosBase + 0x1133C0), {0x48, });
		WriteBytes((void*)(cocosBase + 0x1133C6), {0x48, });
		WriteBytes((void*)(cocosBase + 0x112536), {0xEB, 0x11, 0x90, });	
	} else {
		WriteBytes((void*)(cocosBase + 0x11388B), {0xE8, 0xB0, 0xF3, 0xFF, 0xFF, });
		WriteBytes((void*)(cocosBase + 0x11339D), {0xE8, 0xEE, 0xF6, 0xFF, 0xFF, 0x8B, 0xC8, });
		WriteBytes((void*)(cocosBase + 0x1133C0), {0x50, });
		WriteBytes((void*)(cocosBase + 0x1133C6), {0x50, });
		WriteBytes((void*)(cocosBase + 0x112536), {0x50, 0x6A, 0x00, });
	}
	if(setting.SafeModeEnabled){
		WriteBytes((void*)(gd::base + 0x20A3D1), {0xE9, 0x7B, 0x01, 0x00, 0x00, 0x90, });
		WriteBytes((void*)(gd::base + 0x1FF80B), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x20A3D1), {0x0F, 0x85, 0x7A, 0x01, 0x00, 0x00, });
		WriteBytes((void*)(gd::base + 0x1FF80B), {0x7D, 0x0C, 0xE9, 0xC2, 0xFB, 0xFF, 0xFF, });
	}
	if(setting.TransparentBGEnabled){
		WriteBytes((void*)(gd::base + 0x15A174), {0xFF, });
		WriteBytes((void*)(gd::base + 0x15A175), {0xFF, });
		WriteBytes((void*)(gd::base + 0x15A16F), {0xFF, });
		WriteBytes((void*)(gd::base + 0x15A16D), {0x90, 0xB1, });
		WriteBytes((void*)(gd::base + 0x15891D), {0xFF, });
		WriteBytes((void*)(gd::base + 0x15891E), {0xFF, });
		WriteBytes((void*)(gd::base + 0x158917), {0xFF, });
		WriteBytes((void*)(gd::base + 0x158915), {0x90, 0xB1, });
		WriteBytes((void*)(gd::base + 0x6F7FB), {0xFF, });
		WriteBytes((void*)(gd::base + 0x6F7FC), {0xFF, });
		WriteBytes((void*)(gd::base + 0x6F7F6), {0xFF, });
		WriteBytes((void*)(gd::base + 0x6F7F4), {0x90, 0xB1, });
		WriteBytes((void*)(gd::base + 0x1979AD), {0xFF, });
		WriteBytes((void*)(gd::base + 0x1979AE), {0xFF, });
		WriteBytes((void*)(gd::base + 0x1979A7), {0xFF, });
		WriteBytes((void*)(gd::base + 0x1979A5), {0x90, 0xB1, });
		WriteBytes((void*)(gd::base + 0x17DBC1), {0xFF, });
		WriteBytes((void*)(gd::base + 0x17DBC2), {0xFF, });
		WriteBytes((void*)(gd::base + 0x17DBBB), {0xFF, });
		WriteBytes((void*)(gd::base + 0x17DBB9), {0x90, 0xB1, });
		WriteBytes((void*)(gd::base + 0x176032), {0xFF, });
		WriteBytes((void*)(gd::base + 0x176033), {0xFF, });
		WriteBytes((void*)(gd::base + 0x176036), {0xFF, });
		WriteBytes((void*)(gd::base + 0x176034), {0x90, 0xB1, });
		WriteBytes((void*)(gd::base + 0x4DF7E), {0xFF, });
		WriteBytes((void*)(gd::base + 0x4DF7F), {0xFF, });
		WriteBytes((void*)(gd::base + 0x4DF78), {0xFF, });
		WriteBytes((void*)(gd::base + 0x4DF76), {0x90, 0xB1, });
	} else {
		WriteBytes((void*)(gd::base + 0x15A174), {0x00, });
		WriteBytes((void*)(gd::base + 0x15A175), {0x66, });
		WriteBytes((void*)(gd::base + 0x15A16F), {0xFF, });
		WriteBytes((void*)(gd::base + 0x15A16D), {0x80, 0xC9, });
		WriteBytes((void*)(gd::base + 0x15891D), {0x00, });
		WriteBytes((void*)(gd::base + 0x15891E), {0x66, });
		WriteBytes((void*)(gd::base + 0x158917), {0xFF, });
		WriteBytes((void*)(gd::base + 0x158915), {0x80, 0xC9, });
		WriteBytes((void*)(gd::base + 0x6F7FB), {0x00, });
		WriteBytes((void*)(gd::base + 0x6F7FC), {0x66, });
		WriteBytes((void*)(gd::base + 0x6F7F6), {0xFF, });
		WriteBytes((void*)(gd::base + 0x6F7F4), {0x80, 0xC9, });
		WriteBytes((void*)(gd::base + 0x1979AD), {0x00, });
		WriteBytes((void*)(gd::base + 0x1979AE), {0x66, });
		WriteBytes((void*)(gd::base + 0x1979A7), {0xFF, });
		WriteBytes((void*)(gd::base + 0x1979A5), {0x80, 0xC9, });
		WriteBytes((void*)(gd::base + 0x17DBC1), {0x00, });
		WriteBytes((void*)(gd::base + 0x17DBC2), {0x66, });
		WriteBytes((void*)(gd::base + 0x17DBBB), {0xFF, });
		WriteBytes((void*)(gd::base + 0x17DBB9), {0x80, 0xC9, });
		WriteBytes((void*)(gd::base + 0x176032), {0x00, });
		WriteBytes((void*)(gd::base + 0x176033), {0x66, });
		WriteBytes((void*)(gd::base + 0x176036), {0xFF, });
		WriteBytes((void*)(gd::base + 0x176034), {0x80, 0xC9, });
		WriteBytes((void*)(gd::base + 0x4DF7E), {0x00, });
		WriteBytes((void*)(gd::base + 0x4DF7F), {0x66, });
		WriteBytes((void*)(gd::base + 0x4DF78), {0xFF, });
		WriteBytes((void*)(gd::base + 0x4DF76), {0x80, 0xC9, });
	}
	if(setting.TransparentListsEnabled){
		WriteBytes((void*)(gd::base + 0x15C02C), {0x00, 0x00, 0x00, 0x40, });
		WriteBytes((void*)(gd::base + 0x5C70A), {0x60, });
		WriteBytes((void*)(gd::base + 0x5C6D9), {0x20, 0x20, });
		WriteBytes((void*)(gd::base + 0x5C6DC), {0x20, });
		WriteBytes((void*)(gd::base + 0x5C6CF), {0x40, 0x40, });
		WriteBytes((void*)(gd::base + 0x5C6D2), {0x40, });
	} else {
		WriteBytes((void*)(gd::base + 0x15C02C), {0xBF, 0x72, 0x3E, 0xFF, });
		WriteBytes((void*)(gd::base + 0x5C70A), {0xFF, });
		WriteBytes((void*)(gd::base + 0x5C6D9), {0xA1, 0x58, });
		WriteBytes((void*)(gd::base + 0x5C6DC), {0x2C, });
		WriteBytes((void*)(gd::base + 0x5C6CF), {0xC2, 0x72, });
		WriteBytes((void*)(gd::base + 0x5C6D2), {0x3E, });
	}
	if(setting.FastAltTabEnabled){
		WriteBytes((void*)(gd::base + 0x3D02E), {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
	} else {
		WriteBytes((void*)(gd::base + 0x3D02E), {0x8B, 0x03, 0x8B, 0xCB, 0xFF, 0x50, 0x18, });
	}
	if(setting.AllowLowVolumeEnabled){
		WriteBytes((void*)(gd::base + 0x1E5D7F), {0xEB, 0x08, });
		WriteBytes((void*)(gd::base + 0x1DDEC1), {0xEB, 0x08, });
	} else {
		WriteBytes((void*)(gd::base + 0x1E5D7F), {0x76, 0x08, });
		WriteBytes((void*)(gd::base + 0x1DDEC1), {0x76, 0x08, });
	}
};

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

	ImGui::Checkbox("Force Block Type", &setting.ForceBlockTypeEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Treats all objects as if they were blocks.");

	ImGui::Checkbox("Everything Hurts", &setting.EverythingHurtsEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Ouch.");

	ImGui::Checkbox("Freeze Player", &setting.FreezePlayerEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Freezes player movement.");

	ImGui::Checkbox("Jump Hack", &setting.JumpHackEnabled);
	if (ImGui::IsItemHovered())
	ImGui::SetTooltip("Allows for jumping in mid-air.");

	ImGui::Checkbox("Trail Always On", &setting.ForceTrailStateEnabled);
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
	ImGui::Checkbox("Enable FPS Counter", &setting.FPSCounterEnabled);
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

	/*
	if (ImGui::TreeNode("Background customiser")) {
		ImGui::PushItemWidth(176.000000);
		ImGui::Checkbox("Enabled", &setting.customBGEnabled);
		ImGui::Text("Put jpg image in the GD/Resources/background.jpg");
		ImGui::TreePop();
	} */

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
		if (stringpath != nofilename)
		{
			const char* DllPath = stringpath.c_str();
			LoadLibraryA(DllPath);
		}
	}

	if (ImGui::Button("Sort windows", {196.f,19.f})) {
		sortPlayer = true;
		sortAbout = true;
		sortBypass = true;
		sortCreator = true;
		sortFPSBypass = true;
		sortOther = true;
		sortSpeedhack = true;
		sortUniversal = true;
		sortStatus = true;
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
		ImGui::SetWindowPos(ImVec2(1110, 10));
		sortAbout = false;
	}

    ImGui::Text("Mod Menu %s", VERSION);
    ImGui::Separator();
    ImGui::Text("By Alexandr Simonov");
    ImGui::Text("Mod Menu is licensed\nunder the MIT License.");
	ImGui::Separator();
	ImGui::Text("Used libraries: ");
	ImGui::Text("ImGui: %s", ImGui::GetVersion());
	ImGui::Text("MinHook: %s", "1.3.3");

	if(ImGui::Button("View on GitHub", {196.f,19.f}))
	{
		ShellExecute(0, 0, LPCSTR("https://github.com/OneParsec/GD-ModMenu"), 0, 0 , SW_SHOW );
	}
	if(ImGui::Button("View license", {196.f,19.f}))
	{
		ShellExecute(0, 0, LPCSTR("https://github.com/OneParsec/GD-ModMenu/blob/main/LICENSE"), 0, 0 , SW_SHOW );
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
	setColors();
	disableAnticheat();
	DEVMODE dm;
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);
	if (setting.FPSBypassEnabled)
	{
		SetTargetFPS(setting.interval);
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
		MH_Initialize();
		loadHacks();
		if (setting.MSAAEnabled)
		{
			loadMSAA();
		}
		loadMods();
		ImGuiHook::Load(MainThread);
		break;
	case DLL_PROCESS_DETACH:
		saveHacks();
		ImGuiHook::Unload();
		break;
	}
	return TRUE;
}