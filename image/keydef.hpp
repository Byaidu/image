#ifndef _KEYDEF_HPP_
#define _KEYDEF_HPP_

#include <windows.h>
#include <string>
#include <map>
using namespace std;

//GNU C Complier support
#define WIDE(str)	L##str

#define KEYDEF(key)	(keymap[key] = WIDE(#key))
#define WMDEF(key)	(wmmap[key] = WIDE(#key))
#define MKDEF(key)	(mkmap[key] = WIDE(#key))

//不应该用wstring产生过多对象,用const wchar_t *
map<WPARAM, const wchar_t *>	keymap; //vk映射表

//不用这些了
//map<WPARAM, const wchar_t *>	mkmap;	//mk映射表

//map<UINT, const wchar_t *>	wmmap;	//wm映射表
void initkeydef()
{
	KEYDEF(VK_LBUTTON);
	KEYDEF(VK_RBUTTON);
	KEYDEF(VK_CANCEL);
	KEYDEF(VK_MBUTTON);
	KEYDEF(VK_XBUTTON1);
	KEYDEF(VK_XBUTTON2);
	KEYDEF(VK_BACK);
	KEYDEF(VK_TAB);
	KEYDEF(VK_CLEAR);
	KEYDEF(VK_RETURN);
	KEYDEF(VK_SHIFT);
	KEYDEF(VK_CONTROL);
	KEYDEF(VK_MENU);
	KEYDEF(VK_PAUSE);
	KEYDEF(VK_CAPITAL);
	KEYDEF(VK_KANA);
	KEYDEF(VK_HANGEUL);
	KEYDEF(VK_HANGUL);
	KEYDEF(VK_JUNJA);
	KEYDEF(VK_FINAL);
	KEYDEF(VK_HANJA);
	KEYDEF(VK_KANJI);
	KEYDEF(VK_ESCAPE);
	KEYDEF(VK_CONVERT);
	KEYDEF(VK_NONCONVERT);
	KEYDEF(VK_ACCEPT);
	KEYDEF(VK_MODECHANGE);
	KEYDEF(VK_SPACE);
	KEYDEF(VK_PRIOR);
	KEYDEF(VK_NEXT);
	KEYDEF(VK_END);
	KEYDEF(VK_HOME);
	KEYDEF(VK_LEFT);
	KEYDEF(VK_UP);
	KEYDEF(VK_RIGHT);
	KEYDEF(VK_DOWN);
	KEYDEF(VK_SELECT);
	KEYDEF(VK_PRINT);
	KEYDEF(VK_EXECUTE);
	KEYDEF(VK_SNAPSHOT);
	KEYDEF(VK_INSERT);
	KEYDEF(VK_DELETE);
	KEYDEF(VK_HELP);
	KEYDEF(VK_LWIN);
	KEYDEF(VK_RWIN);
	KEYDEF(VK_APPS);
	KEYDEF(VK_SLEEP);
	KEYDEF(VK_NUMPAD0);
	KEYDEF(VK_NUMPAD1);
	KEYDEF(VK_NUMPAD2);
	KEYDEF(VK_NUMPAD3);
	KEYDEF(VK_NUMPAD4);
	KEYDEF(VK_NUMPAD5);
	KEYDEF(VK_NUMPAD6);
	KEYDEF(VK_NUMPAD7);
	KEYDEF(VK_NUMPAD8);
	KEYDEF(VK_NUMPAD9);
	KEYDEF(VK_MULTIPLY);
	KEYDEF(VK_ADD);
	KEYDEF(VK_SEPARATOR);
	KEYDEF(VK_SUBTRACT);
	KEYDEF(VK_DECIMAL);
	KEYDEF(VK_DIVIDE);
	KEYDEF(VK_F1);
	KEYDEF(VK_F2);
	KEYDEF(VK_F3);
	KEYDEF(VK_F4);
	KEYDEF(VK_F5);
	KEYDEF(VK_F6);
	KEYDEF(VK_F7);
	KEYDEF(VK_F8);
	KEYDEF(VK_F9);
	KEYDEF(VK_F10);
	KEYDEF(VK_F11);
	KEYDEF(VK_F12);
	KEYDEF(VK_F13);
	KEYDEF(VK_F14);
	KEYDEF(VK_F15);
	KEYDEF(VK_F16);
	KEYDEF(VK_F17);
	KEYDEF(VK_F18);
	KEYDEF(VK_F19);
	KEYDEF(VK_F20);
	KEYDEF(VK_F21);
	KEYDEF(VK_F22);
	KEYDEF(VK_F23);
	KEYDEF(VK_F24);

	//无语...

	/*
	keymap['1'] = L"VK_1";
	keymap['2'] = L"VK_2";
	keymap['3'] = L"VK_3";
	keymap['4'] = L"VK_4";
	keymap['5'] = L"VK_5";
	keymap['6'] = L"VK_6";
	keymap['7'] = L"VK_7";
	keymap['8'] = L"VK_8";
	keymap['9'] = L"VK_9";
	keymap['0'] = L"VK_0";
	keymap['Q'] = L"VK_Q";
	keymap['W'] = L"VK_W";
	keymap['E'] = L"VK_E";
	keymap['R'] = L"VK_R";
	keymap['T'] = L"VK_T";
	keymap['Y'] = L"VK_Y";
	keymap['U'] = L"VK_U";
	keymap['I'] = L"VK_I";
	keymap['O'] = L"VK_O";
	keymap['P'] = L"VK_P";
	keymap['A'] = L"VK_A";
	keymap['S'] = L"VK_S";
	keymap['D'] = L"VK_D";
	keymap['F'] = L"VK_F";
	keymap['G'] = L"VK_G";
	keymap['H'] = L"VK_H";
	keymap['J'] = L"VK_J";
	keymap['K'] = L"VK_K";
	keymap['L'] = L"VK_L";
	keymap['Z'] = L"VK_Z";
	keymap['X'] = L"VK_X";
	keymap['C'] = L"VK_C";
	keymap['V'] = L"VK_V";
	keymap['B'] = L"VK_B";
	keymap['N'] = L"VK_N";
	keymap['M'] = L"VK_M";
	*/

	/*
	MKDEF(MK_CONTROL);
	MKDEF(MK_LBUTTON);
	MKDEF(MK_MBUTTON);
	MKDEF(MK_RBUTTON);
	MKDEF(MK_SHIFT);
	*/
}
#endif //_KEYDEF_HPP_
