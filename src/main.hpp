#include <Windows.h>
#include <MinHook.h>
#include <gd.h>
#include <cocos2d.h>
#include <Psapi.h>
#include <cocos-ext.h>

HWND hWnd;
DWORD procId;
HANDLE hProcess;
uint32_t base, alloc_base, alloc_offset;
size_t alloc_size;
std::map<std::string, std::pair<uint32_t, size_t>> alloc_map;

using namespace cocos2d;
using namespace cocos2d::extension;


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

	inline void(__thiscall* onQuit)(gd::PlayLayer* self);
	void __fastcall onQuitHook(gd::PlayLayer* self);

	void mem_init();

}

namespace LoadingLayer {
	
	static inline void (__thiscall* init_)(cocos2d::CCLayer*, char);
    static void __fastcall initHook(cocos2d::CCLayer*, void*, char);

	void mem_init();

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

namespace HttpRequests {
	class Callbacks {
	public:
		void doUpdateHttpRequest(CCObject*);
		void onUpdateHttpResponse(CCHttpClient* client, CCHttpResponse* response);
	};

	void init();
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

bool NewThread(uint32_t vaddress, void *param)
{
    return CreateRemoteThread(hProcess, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(vaddress), param, 0, 0);
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

bool Inject(const char *dll_path)
{
    uint32_t addr = Allocate(strlen(dll_path) + 1, 0);
    if (addr && WriteB(addr, dll_path, strlen(dll_path)))
        return NewThread(reinterpret_cast<uint32_t>(LoadLibraryA), reinterpret_cast<void*>(addr));
    return false;
}
auto director = cocos2d::CCDirector::sharedDirector();

static struct
{
	// speedhack
	bool speedhackEnabled;
	bool speedhackAudioEnabled;
	// player
	bool NoClipEnabled;
	bool NoSpikesEnabled;
	bool ForceBlockTypeEnabled;
	bool EverythingHurtsEnabled;
	bool FreezePlayerEnabled;
	bool JumpHackEnabled;
	bool TrailAlwaysOnEnabled;
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
	int NoClipAccPosition; // 0 - bottom left, 1 - top left, 2 - bottom right, 3 - top right
	int FPSCounterPosition; // 0 - bottom left, 1 - top left, 2 - bottom right, 3 - top right
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
	// mBot
	
} setting;

void WriteBytes(void* location, std::vector<BYTE> bytes) {
	DWORD old_prot;
	VirtualProtect(location, bytes.size(), PAGE_EXECUTE_READWRITE, &old_prot);

	memcpy(location, bytes.data(), bytes.size());

	VirtualProtect(location, bytes.size(), old_prot, &old_prot);
}

void checkHacks(){
	uint32_t cocosBase = GetModuleBase("libcocos2d.dll");

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
	if (setting.TrailAlwaysOnEnabled){
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
			director->getScheduler()->setTimeScale(setting.speed);
		} else {
			SpeedhackAudio::set(1);
			director->getScheduler()->setTimeScale(setting.speed);
		}
	} else {
		SpeedhackAudio::set(1);
		director->getScheduler()->setTimeScale(1);
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