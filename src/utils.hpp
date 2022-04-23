
#include <cocos2d.h>
#include <GUI/CCControlExtension/CCScale9Sprite.h>
#include <MinHook.h>
#include <gd.h>

#pragma once

template <typename R, typename T>
inline R cast(T value) { return reinterpret_cast<R>(value); }

inline auto follow(uintptr_t addr) { return *cast<uintptr_t*>(addr); }
inline auto follow(void* addr) { return *cast<void**>(addr); }

// only use this when necessary
template <typename T, typename U>
T union_cast(U value) {
    union {
        T a;
        U b;
    } u;
    u.b = value;
    return u.a;
}



#pragma warning( pop )

template<typename T>
static T getChild(cocos2d::CCNode* x, int i) {
    return static_cast<T>(x->getChildren()->objectAtIndex(i));
}

template <typename T, typename R>
T as(R const v) { return reinterpret_cast<T>(v); }

inline std::string operator"" _s (const char* _txt, size_t) {
    return std::string(_txt);
}

template<typename T, typename U> constexpr size_t offsetOf(U T::*member) {
    return (char*)&((T*)nullptr->*member) - (char*)nullptr;
}

using unknown_t = uintptr_t;
using edx_t = uintptr_t;

typedef const char* nullstr_t;
static constexpr nullstr_t nullstr = "";