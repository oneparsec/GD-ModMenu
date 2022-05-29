#include <Windows.h>
#include <MinHook.h>
#include <gd.h>
#include <cocos2d.h>

namespace mBot {
	void Update(gd::PlayLayer* self);
    void loadReplay();
    void saveReplay();
}