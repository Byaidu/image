/***********************************************************
* Hook:http://blog.chinaunix.net/uid-660282-id-2414901.html
* Call:http://blog.csdn.net/yhz/article/details/1484073
* 核心代码:https://github.com/YinTianliang/CAPIx
************************************************************/

#include <windows.h>
#include <gdiplus.h>
#include <wchar.h>
#include <iostream>
#include <cstdio>
#include <map>
using namespace std;
using namespace Gdiplus;

#define DLL_EXPORT __declspec(dllexport)
#define wcsicmp _wcsicmp
#define stricmp _stricmp
#define match(x,y) if (!stricmp(argv[x],y))
#pragma comment(lib,"msimg32.lib")
#pragma comment(lib,"GdiPlus.lib")

struct imageres { //资源结构体
	HDC dc;
	HBITMAP oldbmp;
	int w, h;
	imageres() {};
	imageres(char *file) //初始化结构体，并加载资源
	{
		wchar_t wfile[100];
		size_t converted = 0;
		mbstowcs_s(&converted, wfile, file, strlen(file));

		BITMAP bi;
		//为了防止多个hbmp同时用一个hdc发生冲突，所以这里给所有的hbmp分配各自的hdc
		dc = CreateCompatibleDC(nullptr);
		//HBITMAP bmp = (HBITMAP)LoadImageA(nullptr, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		HBITMAP bmp;
		Bitmap *bm = new Bitmap(wfile);
		bm->GetHBITMAP(0, &bmp);
		delete bm;
		oldbmp = (HBITMAP)SelectObject(dc, bmp);
		GetObject(bmp, sizeof(BITMAP), &bi);
		w = bi.bmWidth;
		h = bi.bmHeight;
	}
}*hTarget;
map<string, imageres> resmap; //资源映射表
HWND hCMD;//控制台窗口句柄
char argv[10][100], info[50];

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
	double scale = (double)ax / bx;//校正缩放比
	RECT rc;
	GetClientRect(hCMD, &rc);
	hRes.dc = hDC;
	hRes.w = (int)ceil(scale*(rc.right - rc.left));
	hRes.h = (int)ceil(scale*(rc.bottom - rc.top));
	resmap["cmd"] = hRes; //把cmd作为资源添加到调用表中
	hTarget = &resmap["cmd"];//getres("cmd"); //绘图默认指向cmd
							 //获取desktop大小以及绘图句柄
	hDC = GetDC(nullptr);
	hRes.dc = hDC;
	hRes.w = dm.dmPelsWidth;
	hRes.h = dm.dmPelsHeight;
	resmap["desktop"] = hRes; //把desktop作为资源添加到调用表中
	return;
}
imageres * getres(char *tag) //在资源映射表中查找资源
{
	if (!resmap.count(tag)) //如果在资源映射表中找不到资源，则先加载图片到资源映射表
	{
		imageres hRes(tag);
		resmap[tag] = hRes;
	}
	return &resmap[tag];
}
void delres(char *tag) //销毁原来的资源，防止内存泄漏
{
	imageres * hRes = getres(tag);
	HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
	DeleteObject(bmp);
	DeleteDC(hRes->dc);
	resmap.erase(tag);
	return;
}

void image(wchar_t *CmdLine)
{
	int argc;
	wchar_t **argvw = CommandLineToArgvW(CmdLine, &argc);
	//wchar_t转char
	for (int i = 0; i < argc; i++)
	{
		size_t len = 2 * wcslen(argvw[i]) + 1;
		size_t converted = 0;
		wcstombs_s(&converted, argv[i], len, argvw[i], _TRUNCATE);
	}
	match(0, "help")
	{
		printf("image 3.0 by byaidu\n");
	}
	match(0, "load") //加载资源到资源映射表
	{
		char *tag; //资源描述符
		tag = (argc == 3) ? argv[2] : argv[1];
		//销毁原来的资源，防止内存泄漏
		if (resmap.count(tag)) delres(tag);
		imageres hRes(argv[1]);
		resmap[tag] = hRes;
	}
	match(0, "unload") //卸载资源
	{
		//销毁原来的资源，防止内存泄漏
		delres(argv[1]);
	}
	match(0, "save") //保存为图片
	{
		imageres * hRes = getres(argv[2]);
		HBITMAP hSrc = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
		SelectObject(hRes->dc, hSrc);
		Rect rect(0, 0, hRes->w, hRes->h);
		//用于加载旧位图
		Bitmap* bitmap = new Bitmap(hSrc, nullptr);
		//https://stackoverflow.com/questions/1584202/gdi-bitmap-save-problem
		CLSID Clsid;
		CLSIDFromString(L"{557cf400-1a04-11d3-9a73-0000f81ef32e}", &Clsid);
		bitmap->Save(argvw[1], &Clsid, nullptr);
		delete bitmap;
	}
	match(0, "target") //更改绘图目标
	{
		hTarget = getres(argv[1]);
	}
	match(0, "buffer") //新建一个buffer对象
	{
		char *tag = argv[1];
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
		resmap[tag] = hRes;
	}
	match(0, "stretch") //缩放
	{
		imageres * hRes = getres(argv[1]);
		HDC hDCMem = CreateCompatibleDC(hRes->dc);
		HBITMAP hBitmap = CreateCompatibleBitmap(hRes->dc, atoi(argv[2]), atoi(argv[3]));
		HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem, hBitmap);
		StretchBlt(hDCMem, 0, 0, atoi(argv[2]), atoi(argv[3]), hRes->dc, 0, 0, hRes->w, hRes->h, SRCCOPY);
		//销毁原来的资源，防止内存泄漏
		HBITMAP bmp = (HBITMAP)SelectObject(hRes->dc,hRes->oldbmp);
		DeleteObject(bmp);
		DeleteDC(hRes->dc);
		//替换原来的资源
		hRes->oldbmp = oldbmp;
		hRes->dc = hDCMem;
		hRes->w = atoi(argv[2]);
		hRes->h = atoi(argv[3]);
	}
	match(0, "cls") //清屏
	{
		InvalidateRect(hCMD, nullptr, true);
	}
	match(0, "rotate")
	{
		imageres * hRes = getres(argv[1]);
		HBITMAP hSrc = (HBITMAP)SelectObject(hRes->dc, hRes->oldbmp);
		Rect rect(0, 0, hRes->w, hRes->h);
		//用于加载旧位图
		Bitmap* bitmap = new Bitmap(hSrc, nullptr);
		BitmapData* bitmapData = new BitmapData;
		bitmap->LockBits(&rect,ImageLockModeRead,PixelFormat24bppRGB,bitmapData);
		byte* pixels = (byte*)bitmapData->Scan0;
		//用于加载新位图
		Bitmap* bitmap2 = new Bitmap(hSrc, nullptr);
		BitmapData* bitmapData2 = new BitmapData;
		bitmap2->LockBits(&rect,ImageLockModeWrite,PixelFormat24bppRGB,bitmapData2);
		byte* pixels2 = (byte*)bitmapData2->Scan0;
		//旋转
		double pi = 3.1415926;
		double angle = -strtod(argv[2], nullptr) / 180 * pi;
		double sina = sin(angle), cosa = cos(angle);
		int cx = hRes->w / 2, cy = hRes->h / 2;
		for (int i = 0; i<hRes->w; i++)
			for (int j = 0; j<hRes->h; j++)
			{
				int x = (int)(cx + (i - cx)*cosa - (j - cy)*sina), y = (int)(cy + (i - cx)*sina + (j - cy)*cosa);//原坐标
				if (x >= 0 && x < hRes->w&&y >= 0 && y < hRes->h)
				{
					for (int k = 0; k < 3; k++)
						pixels2[j*bitmapData2->Stride + 3*i + k] = pixels[y*bitmapData->Stride + 3*x + k];
				}
				else
				{
					for (int k = 0; k < 3; k++)
						pixels2[j*bitmapData2->Stride + 3 * i + k] = 0xFF;
				}
			}
		bitmap->UnlockBits(bitmapData);
		bitmap2->UnlockBits(bitmapData2);
		//存放新资源
		HDC hDCMem2 = CreateCompatibleDC(hRes->dc);
		HBITMAP hBitmap2;
		bitmap2->GetHBITMAP(0, &hBitmap2);
		HBITMAP oldbmp = (HBITMAP)SelectObject(hDCMem2, hBitmap2);
		//销毁原来的资源
		DeleteObject(hSrc);
		DeleteDC(hRes->dc);
		//替换旧资源
		hRes->oldbmp = oldbmp;
		hRes->dc = hDCMem2;

		delete bitmapData;
		delete bitmap;
		delete bitmapData2;
		delete bitmap2;
	}
	match(0, "draw")
	{
		//直接在目标上绘图
		imageres * hRes = getres(argv[1]);
		if (argc == 4)
		{
				BitBlt(hTarget->dc, atoi(argv[2]), atoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, SRCCOPY);
		}
		if (argc == 5)
		{
			match(4, "trans")
					TransparentBlt(hTarget->dc, atoi(argv[2]), atoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, hRes->w, hRes->h, RGB(255, 255, 255));
			match(4, "and")
					BitBlt(hTarget->dc, atoi(argv[2]), atoi(argv[3]), hRes->w, hRes->h, hRes->dc, 0, 0, SRCAND);
		}
	}
	match(0, "info")
	{
		imageres * hRes = getres(argv[1]);
		sprintf_s(info, sizeof(info), "%d %d", hRes->w, hRes->h);
		SetEnvironmentVariableA("image", info);
	}
	match(0, "export")
	{
		sprintf_s(info, sizeof(info), "%d", (int)hCMD);
		SetEnvironmentVariableA("image", info);
	}
	match(0, "import")
	{
		char *tag = argv[1];
		//销毁原来的资源，防止内存泄漏
		if (resmap.count(tag)) delres(tag);
		imageres hRes;
		//获取cmd大小以及绘图句柄
		HWND hCMD2 = (HWND)atoi(argv[2]);
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
		resmap[tag] = hRes; //把cmd作为资源添加到调用表中
	}
	match(0, "getpix")
	{
		imageres * hRes = getres(argv[1]);
		COLORREF color=GetPixel(hRes->dc, atoi(argv[2]), atoi(argv[3]));
		sprintf_s(info, sizeof(info), "%d,%d,%d", GetRValue(color), GetGValue(color), GetBValue(color));
		SetEnvironmentVariableA("image", info);
	}
	match(0, "setpix")
	{
		imageres * hRes = getres(argv[1]);
		SetPixel(hRes->dc, atoi(argv[2]), atoi(argv[3]), RGB(atoi(argv[4]), atoi(argv[5]), atoi(argv[6])));
	}
	LocalFree(argvw);
	return;
}