#ifndef __LEVELSETTINGSOBJECT_H__
#define __LEVELSETTINGSOBJECT_H__

#include <gd.h>

namespace gd {
    class GJEffectManager;
    class GJGameLevel;

    enum class Gamemode {
        Cube
    };

    enum class Speed {
        Normal,
        Half, // yes its out of order
        Double,
        Triple,
        Quad
    };

    class LevelSettingsObject : public cocos2d::CCNode {
    public:
        GJEffectManager* m_effectManager;
        Gamemode m_startGamemode; // 0xF0
        Speed m_startSpeed; // 0xF4
        bool m_startMini; // 0xF8
        bool m_startDual; // 0xF9
        bool m_twoPlayerMode; // 0xFA
        float m_songStartOffset; // 0xFC
        bool m_fadeIn; // 0x100
        bool m_fadeOut; // 0x101
        int m_selectedBackground; // 0x104 i need a bette rname for these
        int m_selectedGround;
        int m_selectedFont;
        PAD(4);
        GJGameLevel* m_level; // 0x114
        std::string unk118;
    };
}

#endif
