/***********************************************************
* Hook:http://blog.chinaunix.net/uid-660282-id-2414901.html
* Call:http://blog.csdn.net/yhz/article/details/1484073
* 核心代码:https://github.com/YinTianliang/CAPIx
************************************************************/

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <windows.h>
#include <gdiplus.h>
#include <wchar.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <map>
#include "regionmgr.cpp"
using namespace std;
using namespace Gdiplus;

#define KEYDOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0) 
#define DLL_EXPORT __declspec(dllexport)
#define wtoi _wtoi
#define wcsicmp _wcsicmp
#define match(x,y) if (!wcsicmp(argv[x],y))
#define matchclsid(x) if (!wcsicmp(&argv[1][wcslen(argv[1]) - 3], x))
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"GdiPlus.lib")

struct imageres { //资源结构体
	HDC dc;
	HBITMAP oldbmp;
	int w, h;
	BUF region;
	imageres() {};
	imageres(wchar_t *file) //初始化结构体，并加载资源
	{
		BITMAP bi;
		//为了防止多个hbmp同时用一个hdc发生冲突，所以这里给所有的hbmp分配各自的hdc
		dc = CreateCompatibleDC(nullptr);
		//HBITMAP bmp = (HBITMAP)LoadImageA(nullptr, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		HBITMAP bmp;
		Bitmap *bm = new Bitmap(file);
		bm->GetHBITMAP(0, &bmp);
		delete bm;
		oldbmp = (HBITMAP)SelectObject(dc, bmp);
		GetObject(bmp, sizeof(BITMAP), &bi);
		w = bi.bmWidth;
		h = bi.bmHeight;
	}
	void regioninit(wchar_t *tag,int w,int h) {region = BUF(tag, w - 1, h - 1);}
}*hTarget;
map<wstring, imageres> resmap; //资源映射表
HWND hCMD;//控制台窗口句柄
double scale;//校正缩放比
wchar_t **argv;

void image(wchar_t *); //主函数
void Init_image(); //初始化

bool WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//HookAPI(SetEnvironmentVariableW, SetCall_image);
		DisableThreadLibraryCalls(hModule);
		Init_image();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return true;
}
extern "C" DLL_EXPORT int WINAPI Init(void)//随便给个导出函数,方便加载
{
	return 0;
}
extern "C" __declspec(dllexport) void call(wchar_t *varName, wchar_t *varValue)
{
	//判断变量名是否为image, 是则调用image
	if (!wcsicmp(varName, L"image")) image(varValue);
	return;
}

void Init_image() //初始化
{
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	imageres hRes;
	//获取cmd大小以及绘图句柄
	hCMD = GetConsoleWindow();
	HDC hDC = GetDC(hCMD);
	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
	int ax = dm.dmPelsWidth;
	int bx = GetSystemMetrics(SM_CXSCREEN);
	scale = (double)ax / bx;//校正缩放比
	RECT rc;
	GetClientRect(hCMD, &rc);
	hRes.dc = hDC;
	hRes.w = int(scale*(rc.right - rc.left));
	hRes.h = int(scale*(rc.bottom - rc.top));
	hRes.regioninit((wchar_t*)L"cmd", hRes.w, hRes.h);
	resmap[L"cmd"] = hRes; //把cmd作为资源添加到调用表中
	hTarget = &resmap[L"cmd"];//getres("cmd"); //绘图默认指向cmd
	//获取desktop大小以及绘图句柄
	hDC = GetDC(nullptr);
	hRes.dc = hDC;
	hRes.w = dm.dmPelsWidth;
	hRes.h = dm.dmPelsHeight;
	hRes.regioninit((wchar_t*)L"desktop", hRes.w, hRes.h);
	resmap[L"desktop"] = hRes; //把desktop作为资源添加到调用表中

	TextOutA(hTarget->dc, 0, 0, 0, 0);//第一次使用TextOutA无效，大概是个bug
	return;
}
imageres * getres(wchar_t *tag) //在资源映射表中查找资源
{
	if (!resmap.count(tag)) //如果在资源映射表中找不到资源，则先加载图片到资源映射表
	{
		imageres hRes(tag);
		hRes.regioninit(tag, hRes.w, hRes.h);
		resmap[tag] = hRes;
	}
	return &resmap[tag];
}
void delres(wchar_t *tag) //销毁原来的资源，防止内存泄漏
{
	imageres * hRes = getres(tag);
	HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
	DeleteObject(bmp);
	DeleteDC(hRes->dc);
	resmap.erase(tag);
	return;
}
//不能用SelectObject获取cmd等特殊资源的hbitmap，所以要复制一份出来，注意使用之后要DeleteObject
HBITMAP copyhbitmap(imageres *hSrc)
{
	imageres hRes;
	hRes.dc = CreateCompatibleDC(hSrc->dc);
	HBITMAP hBitmap = CreateCompatibleBitmap(hSrc->dc, hSrc->w, hSrc->h);
	hRes.oldbmp = (HBITMAP)SelectObject(hRes.dc, hBitmap);
	BitBlt(hRes.dc, 0, 0, hSrc->w, hSrc->h, hSrc->dc, 0, 0, SRCCOPY);
	SelectObject(hRes.dc, hRes.oldbmp);
	DeleteDC(hRes.dc);
	return hBitmap;
}
void rotateres()
{
	imageres * hRes = getres(argv[1]);
	HBITMAP hSrc = copyhbitmap(hRes);
	Rect rect(0, 0, hRes->w, hRes->h);
	//用于加载旧位图
	Bitmap bitmap(hSrc, nullptr);
	BitmapData bitmapData;
	bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData);
	byte* pixels = (byte*)bitmapData.Scan0;
	//用于加载新位图
	Bitmap bitmap2(hSrc, nullptr);
	BitmapData bitmapData2;
	bitmap2.LockBits(&rect, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData2);
	byte* pixels2 = (byte*)bitmapData2.Scan0;
	//旋转
	double pi = 3.1415926;
	double angle = -(double)wtoi(argv[2]) / 180 * pi;
	double sina = sin(angle), cosa = cos(angle);
	int cx = hRes->w / 2, cy = hRes->h / 2;
	for (int i = 0; i<hRes->w; i++)
		for (int j = 0; j<hRes->h; j++)
		{
			int x = (int)(cx + (i - cx)*cosa - (j - cy)*sina), y = (int)(cy + (i - cx)*sina + (j - cy)*cosa);//原坐标
			if (x >= 0 && x < hRes->w&&y >= 0 && y < hRes->h)
			{
				for (int k = 0; k < 3; k++)
					pixels2[j*bitmapData2.Stride + 3 * i + k] = pixels[y*bitmapData.Stride + 3 * x + k];
			}
			else
			{
				for (int k = 0; k < 3; k++)
					pixels2[j*bitmapData2.Stride + 3 * i + k] = 0xFF;
			}
		}
	bitmap.UnlockBits(&bitmapData);
	bitmap2.UnlockBits(&bitmapData2);
	//复制临时资源到目标资源
	HDC hDCMem = CreateCompatibleDC(hRes->dc);
	HBITMAP hBitmap;
	bitmap2.GetHBITMAP(0, &hBitmap);
	HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
	BitBlt(hRes->dc, 0, 0, hRes->w, hRes->h, hDCMem, 0, 0, SRCCOPY);
	//销毁临时复制的资源
	DeleteObject(hSrc);
	SelectObject(hDCMem, oldbmp);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
}
void alphares()
{
	double alpha = (double)wtoi(argv[5])/100;
	//用于加载源位图
	imageres * hRes = getres(argv[1]);
	HBITMAP hSrc = copyhbitmap(hRes);
	Rect rect(0, 0, hRes->w, hRes->h);
	Bitmap bitmap(hSrc, nullptr);
	BitmapData bitmapData;
	bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData);
	byte* pixels = (byte*)bitmapData.Scan0;
	//用于加载目标位图
	//不能SelectObject获取cmd等特殊资源的hbitmap，所以要复制一份出来，注意使用之后要DeleteObject
	HBITMAP hSrc2 = copyhbitmap(hTarget);
	Rect rect2(0, 0, hTarget->w, hTarget->h);
	Bitmap bitmap2(hSrc2, nullptr);
	BitmapData bitmapData2;
	bitmap2.LockBits(&rect2, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData2);
	byte* pixels2 = (byte*)bitmapData2.Scan0;
	//用于加载新位图
	Rect rect3(0, 0, hTarget->w, hTarget->h);
	Bitmap bitmap3(hSrc2, nullptr);
	BitmapData bitmapData3;
	bitmap3.LockBits(&rect3, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData3);
	byte* pixels3 = (byte*)bitmapData3.Scan0;
	//alpha混合
	int cx = wtoi(argv[2]), cy = wtoi(argv[3]);
	for (int i = 0; i<hTarget->w; i++)
		for (int j = 0; j<hTarget->h; j++)
		{
			int x = i - cx, y = j - cy;//源坐标
			if (x >= 0 && x < hRes->w&&y >= 0 && y < hRes->h)
			{
				for (int k = 0; k < 3; k++)
					pixels3[j*bitmapData3.Stride + 3 * i + k] =
					(byte)((1 - alpha) * pixels2[j*bitmapData2.Stride + 3 * i + k] +
						alpha * pixels[y*bitmapData.Stride + 3 * x + k]);
			}
			else
			{
				for (int k = 0; k < 3; k++)
					pixels3[j*bitmapData3.Stride + 3 * i + k] = pixels2[j*bitmapData2.Stride + 3 * i + k];
			}
		}
	bitmap.UnlockBits(&bitmapData);
	bitmap2.UnlockBits(&bitmapData2);
	bitmap3.UnlockBits(&bitmapData3);
	//复制临时资源到目标资源
	HDC hDCMem = CreateCompatibleDC(hTarget->dc);
	HBITMAP hBitmap;
	bitmap3.GetHBITMAP(0, &hBitmap);
	HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
	BitBlt(hTarget->dc, 0, 0, hTarget->w, hTarget->h, hDCMem, 0, 0, SRCCOPY);
	//销毁临时复制的资源
	DeleteObject(hSrc);
	DeleteObject(hSrc2);
	SelectObject(hDCMem, oldbmp);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
}

void image(wchar_t *CmdLine)
{
	//wcout << CmdLine << endl;
	int argc;
	argv = CommandLineToArgvW(CmdLine, &argc);
	match(0, L"help")
	{
		printf(
			"image\n"
			"控制台显示图片 Ver 3.1 by Byaidu\n"
		);
	}
	match(0, L"load") //加载资源到资源映射表
	{
		wchar_t *tag; //资源描述符
		tag = (argc == 3) ? argv[2] : argv[1];
		//销毁原来的资源，防止内存泄漏
		if (resmap.count(tag)) delres(tag);
		imageres hRes(argv[1]);
		hRes.regioninit(tag, hRes.w, hRes.h);
		resmap[tag] = hRes;
	}
	match(0, L"unload") //卸载资源
	{
		//销毁原来的资源，防止内存泄漏
		delres(argv[1]);
	}
	match(0, L"save") //保存为图片
	{
		imageres * hRes = getres(argv[2]);
		HBITMAP hSrc = copyhbitmap(hRes);
		Rect rect(0, 0, hRes->w, hRes->h);
		Bitmap bitmap(hSrc, nullptr);
		//https://stackoverflow.com/questions/1584202/gdi-bitmap-save-problem
		CLSID Clsid;
		matchclsid(L"bmp") CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		matchclsid(L"jpg") CLSIDFromString(L"{557cf401-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		matchclsid(L"png") CLSIDFromString(L"{557cf406-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		bitmap.Save(argv[1], &Clsid, nullptr);
		DeleteObject(hSrc);
	}
	match(0, L"target") //更改绘图目标
	{
		hTarget = getres(argv[1]);
	}
	match(0, L"buffer") //新建一个buffer对象
	{
		wchar_t *tag = argv[1];
		//销毁原来的资源，防止内存泄漏
		if (resmap.count(tag)) delres(tag);
		imageres hRes;
		hRes.dc = CreateCompatibleDC(hTarget->dc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hTarget->dc, hTarget->w, hTarget->h);
		hRes.oldbmp = (HBITMAP)SelectObject(hRes.dc, hBitmap);
		BitBlt(hRes.dc, 0, 0, hTarget->w, hTarget->h, nullptr, 0, 0, WHITENESS);
		hRes.w = hTarget->w;
		hRes.h = hTarget->h;
		//把buffer添加到资源调用表中
		hRes.regioninit(tag, hRes.w, hRes.h);
		resmap[tag] = hRes;
	}
	match(0, L"resize") //缩放
	{
		imageres * hRes = getres(argv[1]);
		match(1,L"cmd")
		{
			//防止快速编辑功能刷掉图像
			// 获取标准输入输出设备句柄  
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
			DWORD oldConMode;
			GetConsoleMode(hIn, &oldConMode); // 备份
			SetConsoleMode(hIn, (oldConMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)&(~ENABLE_QUICK_EDIT_MODE));

			RECT rc,rc2;
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			GetClientRect(hCMD, &rc);
			GetWindowRect(hCMD, &rc2);
			int w = (rc2.right - rc2.left) - (rc.right - rc.left) + int((wtoi(argv[2])) / scale);
			int h = (rc2.bottom - rc2.top) - (rc.bottom - rc.top) + int((wtoi(argv[3])) / scale);
			//printf("scale:%f\n", scale);
			//printf("C:%dx%d\n", rc.right - rc.left, rc.bottom - rc.top);
			//printf("W:%dx%d\n", rc2.right - rc2.left, rc2.bottom - rc2.top);
			MoveWindow(hCMD, rc2.left, rc2.top, w, h, 0);
			Sleep(10);
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			Sleep(10);
			hRes->w = (int)wtoi(argv[2]);
			hRes->h = (int)wtoi(argv[3]);
		}else{
			HDC hDCMem = CreateCompatibleDC(hRes->dc);
			HBITMAP hBitmap = CreateCompatibleBitmap(hRes->dc, wtoi(argv[2]), wtoi(argv[3]));
			HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
			StretchBlt(hDCMem, 0, 0, wtoi(argv[2]), wtoi(argv[3]), hRes->dc, 0, 0, hRes->w, hRes->h, SRCCOPY);
			//销毁原来的资源，防止内存泄漏
			HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
			DeleteObject(bmp);
			DeleteDC(hRes->dc);
			//替换原来的资源
			hRes->oldbmp = oldbmp;
			hRes->dc = hDCMem;
			hRes->w = wtoi(argv[2]);
			hRes->h = wtoi(argv[3]);
		}
		hRes->regioninit(argv[1], hRes->w, hRes->h);
	}
	match(0, L"cls") //清屏
	{
		imageres * hRes = getres((wchar_t*)L"cmd");
		hRes->regioninit((wchar_t*)L"cmd", hRes->w, hRes->h);
		InvalidateRect(hCMD, nullptr, true);
		Sleep(10);
	}
	match(0, L"rotate")
	{
		rotateres();
	}
	match(0, L"draw")
	{
		//直接在目标上绘图
		imageres * hRes = getres(argv[1]);
		complexupdate(hRes->region.p, 0, 0, hRes->region.p->x2, hRes->region.p->y2, wtoi(argv[2]), wtoi(argv[3]), hTarget->region.p);
		if (argc == 4)
		{
				BitBlt(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, SRCCOPY);
		}
		else
		{
			match(4, L"trans")
					TransparentBlt(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, hRes->w, hRes->h, RGB(255, 255, 255));
			match(4, L"alpha")
				alphares();
		}
	}
	match(0, L"text")
	{
		//显示两次才会刷新出来，大概是个bug
		for (int i = 0; i < 2;i++) TextOutW(hTarget->dc, wtoi(argv[2]), wtoi(argv[3]), argv[1], wcslen(argv[1]));
	}
	match(0, L"font")
	{
		SetBkMode(hTarget->dc, TRANSPARENT);
		SetTextColor(hTarget->dc, RGB(wtoi(argv[3]), wtoi(argv[4]), wtoi(argv[5])));
		HFONT hFont = CreateFontW(
			wtoi(argv[2]), wtoi(argv[1]), 0/*不用管*/, 0/*不用管*/, 400 /*一般这个值设为400*/,
			FALSE/*不带斜体*/, FALSE/*不带下划线*/, FALSE/*不带删除线*/,
			DEFAULT_CHARSET, //这里我们使用默认字符集，还有其他以 _CHARSET 结尾的常量可用
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, //这行参数不用管
			DEFAULT_QUALITY, //默认输出质量
			FF_DONTCARE, //不指定字体族*/
			L"新宋体" //字体名
		);
		SelectObject(hTarget->dc,hFont);
	}
	match(0, L"sleep")
	{
		Sleep(wtoi(argv[1]));
	}
	match(0, L"info")
	{
		wchar_t info[100];
		imageres * hRes = getres(argv[1]);
		swprintf(info, L"%d %d", hRes->w, hRes->h);
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"export")
	{
		wchar_t info[100];
		swprintf(info, L"%d", (int)hCMD);
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"import")
	{
		wchar_t *tag = argv[2];
		//销毁原来的资源，防止内存泄漏
		if (resmap.count(tag)) delres(tag);
		imageres hRes;
		//获取cmd大小以及绘图句柄
		HWND hCMD2 = (HWND)wtoi(argv[1]);
		HDC hDC = GetDC(hCMD2);
		DEVMODE dm;
		dm.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
		int ax = dm.dmPelsWidth;
		int bx = GetSystemMetrics(SM_CXSCREEN);
		double scale = (double)ax / bx;//校正缩放比
		RECT rc;
		GetClientRect(hCMD2, &rc);
		hRes.dc = hDC;
		hRes.w = (int)ceil(scale*(rc.right - rc.left));
		hRes.h = (int)ceil(scale*(rc.bottom - rc.top));
		hRes.regioninit(tag, hRes.w, hRes.h);
		resmap[tag] = hRes; //把cmd作为资源添加到调用表中
	}
	match(0, L"getpix")
	{
		wchar_t info[100];
		COLORREF color=GetPixel(hTarget->dc, wtoi(argv[1]), wtoi(argv[2]));
		swprintf(info, L"%d %d %d", GetRValue(color), GetGValue(color), GetBValue(color));
		SetEnvironmentVariableW(L"image", info);
	}
	match(0, L"setpix")
	{
		SetPixel(hTarget->dc, wtoi(argv[1]), wtoi(argv[2]), RGB(wtoi(argv[3]), wtoi(argv[4]), wtoi(argv[5])));
	}
	match(0, L"list")
	{
		ifstream in(argv[1]);
		string str;
		wchar_t wstr[100];
		while (!in.eof())
		{
			getline(in, str);
			MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, sizeof(wstr));
			image(wstr);
		}
		in.close();
	}
	match(0, L"mouse")
	{
		imageres *hRes = getres((wchar_t*)L"cmd");
		wchar_t info[100];
		POINT mosPos;
		int x, y;
		int timer = wtoi(argv[1]);
		//这里要重新设置一次，要不然ReadConsoleInput会卡住
		// 获取标准输入输出设备句柄  
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
		DWORD oldConMode;
		GetConsoleMode(hIn, &oldConMode); // 备份
		SetConsoleMode(hIn, (oldConMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)&(~ENABLE_QUICK_EDIT_MODE));
		if (timer < 0)
		{
			INPUT_RECORD	mouseRec;
			DWORD			res;
			while (1)
			{
				ReadConsoleInput(hIn, &mouseRec, 1, &res);
				if (mouseRec.EventType == MOUSE_EVENT)
				{
					if (mouseRec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
					{
						GetCursorPos(&mosPos);
						ScreenToClient(hCMD, &mosPos);
						x = min(max((int)scale*mosPos.x, 0), hRes->w);
						y = min(max((int)scale*mosPos.y, 0), hRes->h);
						break;
					}
				}
			}
		}
		if (timer >= 0)
		{
			DWORD tstart = GetTickCount();
			while (!(KEYDOWN(VK_LBUTTON) || int(GetTickCount() - tstart) >= timer));
			GetCursorPos(&mosPos);
			ScreenToClient(hCMD, &mosPos);
			x = min(max((int)scale*mosPos.x, 0), hRes->w);
			y = min(max((int)scale*mosPos.y, 0), hRes->h);
		}
		if (argc >= 3)
		{
			int ret = 0;
			for (int i = 2; i < argc; i++)
			{
				int x1, y1, x2, y2;
				swscanf(argv[i], L"%d,%d,%d,%d", &x1, &y1, &x2, &y2);
				if (x >= x1 && x <= x2 && y >= y1 && y <= y2) ret = i - 1;
			}
			swprintf(info, L"%d %d %d", x, y, ret);
			SetEnvironmentVariableW(L"image", info);
			swprintf(info, L"%d", ret);
			SetEnvironmentVariableW(L"errorlevel", info);
		}else{
			wstring ret = query(resmap[L"cmd"].region.p, x, y);
			swprintf(info, L"%d %d %s", x, y, ret.c_str());
			SetEnvironmentVariableW(L"image", info);
			swprintf(info, L"%s", ret.c_str());
			SetEnvironmentVariableW(L"errorlevel", info);
		}
		SetConsoleMode(hIn, oldConMode);
	}
	match(0, L"debug")
	{
		imageres *hRes = getres((wchar_t*)L"cmd");
		show(hRes->region.p);
	}
	LocalFree(argv);
	return;
}
