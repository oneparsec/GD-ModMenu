
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


template<typename T, typename U>
inline bool detour(const T src, const U dst, const int len) {
	if (len < 5) return false;
	char* _src = reinterpret_cast<char*>(src);
	char* _dst = reinterpret_cast<char*>(dst);
	int jmp = static_cast<int>(_dst - _src - 5);
	unsigned long old = 0;
	VirtualProtect(_src, len, PAGE_EXECUTE_READWRITE, &old);
	*_src = 0xE9;
	*reinterpret_cast<int*>(_src + 1) = jmp;
	return VirtualProtect(_src, len, old, &old);
}