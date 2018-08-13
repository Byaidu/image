/***********************************************************
image
����̨��ʾͼƬ Ver 3.6 by Byaidu
�������뼰��������:https://github.com/Byaidu/image
���ִ���ο�:https://github.com/YinTianliang/CAPIx
************************************************************/
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "GdiPlus.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "MSimg32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Winmm.lib")
#endif
#include <cstdio>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <wchar.h>
#include <process.h>
#include <windows.h>
#include <gdiplus.h>

#include "region.hpp"
#include "keydef.hpp"

using namespace std;
using namespace Gdiplus;

#define KEYDOWN(vk_code)	((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define DLL_EXPORT		__declspec(dllexport)
#define wtoi			_wtoi
#define wcsicmp			_wcsicmp
#define wcsnicmp		_wcsnicmp
#define match(x, y)		((x) < argc && !wcsnicmp(argv[x], (y), wcslen(argv[x])))
#define matchExt(x, y)		(!wcsicmp(&argv[x][wcslen(argv[x]) - 4], (y)))

enum imageType
{			//imageres������
	irDesktop,	//��������Ļ
	irCMD,		//��CMD
	irBuffer	//ֻ�ǻ�����
};

struct imageres;	//�����ṹ��
struct wndInfo		//������Ϣ�ṹ��
{
	wstring		tag;
	imageres	*hRes;
	HDC		hdc;
};

map<wstring, imageres>	resmap;			//����ӳ���
map<HWND, wndInfo>	wndmap;			//������Ϣӳ���
HWND			hCMD;			//����̨���ھ��
double			scale;			//У�����ű�
void			image(const wchar_t *);

GdiplusStartupInput	gdiplusStartupInput;
ULONG_PTR		gdiplusToken;

extern "C" DLL_EXPORT int WINAPI Init(void)	//��������������,�������
{
	return 0;
}

extern "C"
__declspec(dllexport)
void call(wchar_t *varName, wchar_t *varValue)
{
	//�жϱ������Ƿ�Ϊimage, �������image
	if (!wcsicmp(varName, L"image")) image(varValue);
	return;
}

/** �����ṹ��ʵ�� **/
struct imageres
{
	HWND		hwnd;
	HDC		hdc;
	HBITMAP		bmp, oldbmp;
	int		w, h;
	regSet		regTree;		//ͼԪ������
	enum imageType	type;

	inline imageres()
	{
		hwnd	= nullptr;
		hdc = nullptr;
		bmp = oldbmp = nullptr;
		w = h = 0;
		type = irBuffer;
	}

	inline imageres(const wchar_t *file)
	{
		hwnd = nullptr;
		type = irBuffer;
		if (!loadFile(file))
		{
			hdc = nullptr;
			bmp = oldbmp = nullptr;
			w = h = 0;
		}
	}

	imageres (int iw, int ih)		//���ݴ�С��ʼ��
	{
		hwnd = nullptr;
		hdc = CreateCompatibleDC(nullptr);
		bmp = CreateCompatibleBitmap(hTarget->hdc, iw, ih);
		oldbmp = (HBITMAP) SelectObject(hdc, bmp);
		w = iw;
		h = ih;
		type = irBuffer;
	}
	bool loadFile(const wchar_t *file)	//����ͼ�񵽻�����
	{
		this->free();

		//Support: BMP, GIF, EXIF, JPG, PNG and TIFF
		//bmp = (HBITMAP)LoadImageA(nullptr, file, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		Bitmap	*bm = new Bitmap(file);
		bm->GetHBITMAP(0, &bmp);

		//todo: ��������İ�ȫ��?
		delete bm;

		//Ϊ�˷�ֹ���hbmpͬʱ��һ��hdc������ͻ��������������е�hbmp������Ե�hdc
		hdc = CreateCompatibleDC(nullptr);
		oldbmp = (HBITMAP) SelectObject(hdc, bmp);

		BITMAP	bi;
		GetObject(bmp, sizeof(BITMAP), &bi);
		w = bi.bmWidth;
		h = bi.bmHeight;
		type = irBuffer;
		return true;
	}

	void resize(int iw, int ih)
	{
		HDC	nhdc = CreateCompatibleDC(nullptr);
		HBITMAP nbmp = CreateCompatibleBitmap(hTarget->hdc, iw, ih);

		DeleteObject(oldbmp);
		oldbmp = (HBITMAP) SelectObject(nhdc, nbmp);
		StretchBlt(nhdc, 0, 0, iw, ih, hdc, 0, 0, w, h, SRCCOPY);

		DeleteObject(hdc);
		DeleteObject(bmp);
		hdc = nhdc;
		bmp = nbmp;
		w = iw;
		h = ih;

		//����д���,��Ҫ�ı䴰�ڴ�С!
		if (hwnd != nullptr)
		{
			RECT	rc, rc2;
			GetClientRect(hwnd, &rc);
			GetWindowRect(hwnd, &rc2);

			int	w = (rc2.right - rc2.left) - (rc.right - rc.left) + iw;
			int	h = (rc2.bottom - rc2.top) - (rc.bottom - rc.top) + ih;

			MoveWindow(hwnd, rc2.left, rc2.top, w, h, 0);

			InvalidateRect(hwnd, nullptr, true);
		}
	}

	void free()	//�ͷ��ڴ�
	{
		switch (type)
		{
		case irDesktop:
			break;

		case irCMD:
			ReleaseDC(hwnd, hdc);
			break;

		case irBuffer:
			if (hwnd != nullptr) PostMessage(hwnd, WM_CLOSE, 0, 0);
			SelectObject(hdc, oldbmp);
			DeleteObject(bmp);
			DeleteDC(hdc);
		}
	}

	inline void regioninit(const wchar_t *tag)
	{
		regTree.clear();
		regTree.add(region(0, 0, w, h, tag));
	}
}
*hTarget, *hOldTarget;

/** ***** ������ ***** **/
int GetEncoderClsid(const WCHAR *format, CLSID *pClsid)
{
	UINT		num = 0;	//number of image encoders
	UINT		size = 0;	//size of the image encoder array in bytes
	ImageCodecInfo	*pImageCodecInfo = NULL;

	GetImageEncodersSize(&num, &size);
	if (size == 0) return -1;	// Failure
	pImageCodecInfo = (ImageCodecInfo *) (malloc(size));
	if (pImageCodecInfo == NULL) return -1; // Failure
	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;		// Success
		}
	}

	free(pImageCodecInfo);
	return -1;			// Failure
}

void Init_image()
{
	initkeydef();

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

	//��ȡcmd��С�Լ���ͼ���
	hCMD = GetConsoleWindow();

	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);

	int	ax = dm.dmPelsWidth;
	int	bx = GetSystemMetrics(SM_CXSCREEN);
	scale = ax / (double)bx;	//У�����ű�
	RECT	rc;
	GetClientRect(hCMD, &rc);

	imageres	res;
	res.hdc = GetDC(hCMD);
	res.hwnd = hCMD;
	res.w = int(scale * (rc.right - rc.left));
	res.h = int(scale * (rc.bottom - rc.top));
	res.type = irCMD;
	res.regioninit((wchar_t *)L"cmd");

	resmap[L"cmd"] = res;		//��cmd��ӵ�����ӳ�����
	hTarget = &resmap[L"cmd"];	//��ͼĬ��ָ��cmd
	hOldTarget = nullptr;

	//��ȡdesktop��С�Լ���ͼ���
	res.hdc = GetDC(nullptr);
	res.w = dm.dmPelsWidth;
	res.h = dm.dmPelsHeight;
	res.type = irDesktop;
	res.regioninit((wchar_t *)L"desktop");
	resmap[L"desktop"] = res;	//��desktop��ӵ�����ӳ�����

	//��һ��ʹ��TextOutA��Ч������Ǹ�bug
	TextOutA(hTarget->hdc, 0, 0, 0, 0);
	return;
}

void Stop_image()
{
	for (map<wstring, imageres>::iterator ir = resmap.begin(); ir != resmap.end();)
	{
		ir->second.free();
		resmap.erase(ir++);
	}

	GdiplusShutdown(gdiplusToken);
	return;
}

//���һ���
imageres *getres(const wchar_t *tag)
{
	if (!resmap.count(tag)) //����ڻ���ӳ������Ҳ��������ȼ���ͼԪ���������ٽ�������ӵ�����ӳ���
	{
		resmap[tag] = imageres(tag);
		resmap[tag].regioninit(tag);
	}

	return &resmap[tag];
}

//����ԭ���Ļ�������ֹ�ڴ�й©
void delres(const wchar_t *tag)
{
	if (resmap.count(tag))
	{
		enum imageType	type = resmap[tag].type;
		if (type != irDesktop && type != irCMD)
		{
			resmap[tag].free();
			resmap.erase(tag);
		}
	}
}

//������SelectObject��ȡcmd�����⻭����hbitmap������Ҫ����һ�ݳ�����ע��ʹ��֮��ҪDeleteObject
HBITMAP copyhbitmap(imageres *hSrc)
{
	HDC	hdc = CreateCompatibleDC(hSrc->hdc);
	HBITMAP bmp = CreateCompatibleBitmap(hSrc->hdc, hSrc->w, hSrc->h);
	HBITMAP oldbmp = (HBITMAP) SelectObject(hdc, bmp);

	BitBlt(hdc, 0, 0, hSrc->w, hSrc->h, hSrc->hdc, 0, 0, SRCCOPY);
	SelectObject(hdc, oldbmp);
	DeleteDC(hdc);
	return bmp;
}

void rotateres(wchar_t **argv)
{
	imageres	*hRes = getres(argv[1]);
	HBITMAP		hSrc = copyhbitmap(hRes);
	Rect		rect(0, 0, hRes->w, hRes->h);

	//���ڼ��ؾ�λͼ
	Bitmap		bitmap(hSrc, nullptr);
	BitmapData	bitmapData;
	bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData);

	byte		*pixels = (byte *)bitmapData.Scan0;

	//���ڼ�����λͼ
	Bitmap		bitmap2(hSrc, nullptr);
	BitmapData	bitmapData2;
	bitmap2.LockBits(&rect, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData2);

	byte	*pixels2 = (byte *)bitmapData2.Scan0;

	//��ת
	double	pi = 3.1415926;
	double	angle = -(double)wtoi(argv[2]) / 180 * pi;
	double	sina = sin(angle), cosa = cos(angle);
	int	cx = hRes->w / 2, cy = hRes->h / 2;
	for (int i = 0; i < hRes->w; i++)
	{
		for (int j = 0; j < hRes->h; j++)
		{
			int	x = (int)(cx + (i - cx) * cosa - (j - cy) * sina), y = (int)
				(cy + (i - cx) * sina + (j - cy) * cosa);	//ԭ����
			if (x >= 0 && x < hRes->w && y >= 0 && y < hRes->h)
			{
				for (int k = 0; k < 3; k++)
					pixels2[j * bitmapData2.Stride + 3 * i + k] = pixels[y * bitmapData.Stride + 3 * x + k];
			}
			else
			{
				for (int k = 0; k < 3; k++) pixels2[j * bitmapData2.Stride + 3 * i + k] = 0xFF;
			}
		}
	}

	bitmap.UnlockBits(&bitmapData);
	bitmap2.UnlockBits(&bitmapData2);

	//������ʱ������Ŀ�껭��
	HDC	hDCMem = CreateCompatibleDC(hRes->hdc);
	HBITMAP hBitmap;
	bitmap2.GetHBITMAP(0, &hBitmap);

	HBITMAP oldbmp = (HBITMAP) SelectObject(hDCMem, hBitmap);
	BitBlt(hRes->hdc, 0, 0, hRes->w, hRes->h, hDCMem, 0, 0, SRCCOPY);

	//������ʱ���ƵĻ���
	DeleteObject(hSrc);
	SelectObject(hDCMem, oldbmp);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
}

void alphares(wchar_t **argv)
{
	double		alpha = (double)wtoi(argv[5]) / 255;

	//���ڼ���Դλͼ
	imageres	*hRes = getres(argv[1]);
	HBITMAP		hSrc = copyhbitmap(hRes);
	Rect		rect(0, 0, hRes->w, hRes->h);
	Bitmap		bitmap(hSrc, nullptr);
	BitmapData	bitmapData;
	bitmap.LockBits(&rect, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData);

	byte		*pixels = (byte *)bitmapData.Scan0;

	//���ڼ���Ŀ��λͼ
	//����SelectObject��ȡcmd�����⻭����hbitmap������Ҫ����һ�ݳ�����ע��ʹ��֮��ҪDeleteObject
	HBITMAP		hSrc2 = copyhbitmap(hTarget);
	Rect		rect2(0, 0, hTarget->w, hTarget->h);
	Bitmap		bitmap2(hSrc2, nullptr);
	BitmapData	bitmapData2;
	bitmap2.LockBits(&rect2, ImageLockModeRead, PixelFormat24bppRGB, &bitmapData2);

	byte		*pixels2 = (byte *)bitmapData2.Scan0;

	//���ڼ�����λͼ
	Rect		rect3(0, 0, hTarget->w, hTarget->h);
	Bitmap		bitmap3(hSrc2, nullptr);
	BitmapData	bitmapData3;
	bitmap3.LockBits(&rect3, ImageLockModeWrite, PixelFormat24bppRGB, &bitmapData3);

	byte	*pixels3 = (byte *)bitmapData3.Scan0;

	//alpha���
	int	cx = wtoi(argv[2]), cy = wtoi(argv[3]);
	for (int i = 0; i < hTarget->w; i++)
	{
		for (int j = 0; j < hTarget->h; j++)
		{
			int	x = i - cx, y = j - cy; //Դ����
			if (x >= 0 && x < hRes->w && y >= 0 && y < hRes->h)
			{
				for (int k = 0; k < 3; k++)
				{
					pixels3[j * bitmapData3.Stride + 3 * i + k] = (byte)
						(
							(1 - alpha) *
							pixels2[j * bitmapData2.Stride + 3 * i + k] +
							alpha *
							pixels[y * bitmapData.Stride + 3 * x + k]
						);
				}
			}
			else
			{
				for (int k = 0; k < 3; k++)
					pixels3[j * bitmapData3.Stride + 3 * i + k] = pixels2[j * bitmapData2.Stride + 3 * i + k];
			}
		}
	}

	bitmap.UnlockBits(&bitmapData);
	bitmap2.UnlockBits(&bitmapData2);
	bitmap3.UnlockBits(&bitmapData3);

	//������ʱ������Ŀ�껭��
	HDC	hDCMem = CreateCompatibleDC(hTarget->hdc);
	HBITMAP hBitmap;
	bitmap3.GetHBITMAP(0, &hBitmap);

	HBITMAP oldbmp = (HBITMAP) SelectObject(hDCMem, hBitmap);
	BitBlt(hTarget->hdc, 0, 0, hTarget->w, hTarget->h, hDCMem, 0, 0, SRCCOPY);

	//������ʱ���ƵĻ���
	DeleteObject(hSrc);
	DeleteObject(hSrc2);
	SelectObject(hDCMem, oldbmp);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	wndInfo		&winfo = wndmap[hwnd];

	wstring		&windowtag = winfo.tag;
	imageres	*hRes = winfo.hRes;
	HDC		windowhdc = winfo.hdc;

	wchar_t		ret[10000];
	const wchar_t	*msgName;
	const wchar_t	*keyName;
	switch (uMsg)
	{
	case WM_KEYDOWN:
		msgName = L"KeyDown";
		goto KeyEvent;

	case WM_KEYUP:
		msgName = L"KeyUp";

		// goto KeyEvent;
KeyEvent:
		{
			if (('0' <= wParam && wParam <= '9') || ('A' <= wParam && wParam <= 'Z'))
			{
				wsprintfW(ret, L" %s.VK_%c.%d", msgName, wParam, wParam);
			}
			else
			{
				keyName = keymap.count(wParam) ? keymap[wParam] : L"0";
				wsprintfW(ret, L" %s.%s.%d", msgName, keyName, wParam);
			}
			break;
		}

	//����ܲ���goto���map��ö�
	case WM_LBUTTONDOWN:
		msgName = L"MouseDown.Left";
		goto MouseEvent;

	case WM_RBUTTONDOWN:
		msgName = L"MouseDown.Right";
		goto MouseEvent;

	case WM_MBUTTONDOWN:
		msgName = L"MouseDown.Middle";
		goto MouseEvent;

	case WM_LBUTTONUP:
		msgName = L"MouseUp.Left";
		goto MouseEvent;

	case WM_RBUTTONUP:
		msgName = L"MouseUp.Right";
		goto MouseEvent;

	case WM_MBUTTONUP:
		msgName = L"MouseUp.Middle";

		// goto MouseEvent;
		// case WM_LBUTTONDBLCLK: �ⲻ�ᱻ���ð�
		// û������ (WndClass.style |= CS_DBLCLKS)
		// case WM_RBUTTONDBLCLK:
		// case WM_MBUTTONDBLCLK:
MouseEvent:
		{
			//�ƺ�mkmap[LOWORD(wParam)]������
			wsprintfW
			(
				ret,
				L" %s.%d.%d.%s",
				msgName,
				LOWORD(lParam),
				HIWORD(lParam),
				resmap[windowtag].regTree.query(LOWORD(lParam), HIWORD(lParam)).c_str()
			);
			break;
		}

	case WM_MOUSEWHEEL:
		wsprintfW(ret, L" MouseWheel.%d.%d.%d", (short)HIWORD(wParam), LOWORD(lParam), HIWORD(lParam));
		break;

	case WM_MOUSEMOVE:
		wsprintfW
		(
			ret,
			L" MouseMove.%d.%d.%s",
			LOWORD(lParam),
			HIWORD(lParam),
			resmap[windowtag].regTree.query(LOWORD(lParam), HIWORD(lParam)).c_str()
		);
		break;

	case WM_PAINT:
		BitBlt(windowhdc, 0, 0, hRes->w, hRes->h, hRes->hdc, 0, 0, SRCCOPY);
		ValidateRect(hwnd, NULL);
		break;

	case WM_DESTROY:
		wcscpy(ret, L" Close");
		resmap[windowtag].type = irBuffer;
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	wstring varname(windowtag);
	varname += L".wm";

	wchar_t info[10000];
	GetEnvironmentVariableW(varname.c_str(), info, sizeof(info));
	wcscat(info, ret);

	if (wcslen(ret) > 8192)
		SetEnvironmentVariableW(varname.c_str(), &info[wcslen(ret) - 8192]);
	else
		SetEnvironmentVariableW(varname.c_str(), info);
	return 0;
}

//������Ϊʲô�� void *
//��image() -> L"show"

//ע��args������new wstring�õ�,��Ϊ��delete
unsigned int __stdcall makeWindow(void *args)
{
	wstring		&tag = *(wstring *)args;

	HINSTANCE	hInstance = GetModuleHandle(NULL);

	WNDCLASS	Draw;
	Draw.cbClsExtra = 0;
	Draw.cbWndExtra = 0;
	Draw.hCursor = LoadCursor(hInstance, IDC_ARROW);
	Draw.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	Draw.lpszMenuName = NULL;
	Draw.style = CS_HREDRAW | CS_VREDRAW;
	Draw.hbrBackground = (HBRUSH) COLOR_WINDOW;
	Draw.lpfnWndProc = WindowProc;
	Draw.lpszClassName = "imagewindow";
	Draw.hInstance = hInstance;
	RegisterClass(&Draw);

	imageres	*hRes = getres(tag.c_str());
	RECT		rc = { 0, 0, hRes->w, hRes->h };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, NULL);

	HWND	hwnd = CreateWindowW
		(
			L"imagewindow", //����ע���������Ҫ��ȫһ��
			tag.c_str(),	//���ڱ�������
			WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,	//���������ʽ
			CW_USEDEFAULT,			//��������ڸ�����X����
			CW_USEDEFAULT,			//��������ڸ�����Y����
			rc.right - rc.left,		//���ڵĿ��
			rc.bottom - rc.top,		//���ڵĸ߶�
			NULL,				//û�и����ڣ�ΪNULL
			NULL,				//û�в˵���ΪNULL
			hInstance,			//��ǰӦ�ó����ʵ�����
			NULL				//û�и������ݣ�ΪNULL
		);
	hRes->hwnd = hwnd;

	wndInfo winfo;
	winfo.tag = tag;
	winfo.hRes = hRes;
	winfo.hdc = GetDC(hwnd);
	wndmap[hwnd] = winfo;

	// ��ʾ����
	// wprintf(L"`%s' is opened.\n", tag.c_str());
	ShowWindow(hwnd, SW_SHOW);

	// ���´���
	UpdateWindow(hwnd);

	// ��Ϣѭ��
	MSG	msg;

	int	cnt = 0;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		cnt++;
		SetEnvironmentVariableW(L"cnt", to_wstring((long long)cnt).c_str());
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// wprintf(L"`%s' is closed.\n", tag.c_str());
	ReleaseDC(hwnd, winfo.hdc);
	wndmap.erase(hwnd);
	hRes->hwnd = nullptr;

	delete args;
	return 0;
}

void image(const wchar_t *CmdLine)
{
	//wcout << CmdLine << endl;
	int	argc;
	wchar_t **argv;
	argv = CommandLineToArgvW(CmdLine, &argc);

	if (0 < argc && argv[0][0] == L'[')
	{
		wstring arg0 = argv[0];

		hOldTarget = hTarget;
		hTarget = getres((wchar_t *)arg0.substr(1, arg0.length() - 2).c_str());

		argv = &argv[1];
		argc--;
	}

	if (match(0, L"help"))
	{
		printf("image\n����̨��ʾͼƬ Ver 3.6 by Byaidu\n");
	}

	//������
	if (match(0, L"newbuf") || match(0, L"buffer")) //�½�����
	{
		wchar_t		*tag = argv[1];

		int		w = argc > 2 ? wtoi(argv[2]) : hTarget->w;
		int		h = argc > 3 ? wtoi(argv[3]) : hTarget->h;
		imageres	res(w, h);

		int		color = argc > 6 ? RGB(wtoi(argv[4]), wtoi(argv[5]), wtoi(argv[6])) : RGB
			(
				255,
				255,
				255
			);
		colorregion(res.hdc, color, 0, 0, hTarget->w, hTarget->h);

		res.regioninit(tag);

		//����ԭ���Ļ�������ֹ�ڴ�й©
		if (resmap.count(tag)) delres(tag);

		//��buffer��ӵ��������ñ���
		resmap[tag] = res;
	}

	if (match(0, L"delbuf") || match(0, L"unload")) //ɾ������
	{
		delres(argv[1]);
	}

	if (match(0, L"load"))			//��ȡͼƬ������
	{
		wchar_t		*tag = argv[1]; //��������
		imageres	hRes(argc > 2 ? argv[2] : argv[1]);
		hRes.regioninit(tag);

		if (resmap.count(tag)) delres(tag);
		resmap[tag] = hRes;
	}

	if (match(0, L"save"))			//�ѻ�������ΪͼƬ
	{
		imageres	*hRes = getres(argv[1]);
		HBITMAP		hSrc = copyhbitmap(hRes);
		Rect		rect(0, 0, hRes->w, hRes->h);
		Bitmap		bitmap(hSrc, nullptr);

		//https://stackoverflow.com/questions/1584202/gdi-bitmap-save-problem
		CLSID		Clsid;
		if (matchExt(2, L".bmp")) GetEncoderClsid(L"image/bmp", &Clsid);
		if (matchExt(2, L".jpg")) GetEncoderClsid(L"image/jpeg", &Clsid);
		if (matchExt(2, L".png")) GetEncoderClsid(L"image/png", &Clsid);
		if (matchExt(2, L".gif")) GetEncoderClsid(L"image/gif", &Clsid);
		bitmap.Save(argv[2], &Clsid, nullptr);
		DeleteObject(hSrc);
	}

	if (match(0, L"target"))		//���Ļ�ͼĿ��
	{
		hTarget = getres(argv[1]);
	}

	//������С
	if (match(0, L"size"))			//��ô�С
	{
		imageres	*hRes = getres(argv[1]);
		wchar_t		info[30];
		swprintf(info, L"%d %d", hRes->w, hRes->h);
		SetEnvironmentVariableW(L"image", info);
	}

	if (match(0, L"resize"))		//����
	{
		imageres	*hRes = getres(argv[1]);
		if (match(1, L"cmd"))
		{
			//��ȡ��׼��������豸���
			HANDLE	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			HANDLE	hIn = GetStdHandle(STD_INPUT_HANDLE);
			DWORD	oldConMode;
			GetConsoleMode(hIn, &oldConMode);	// ����

			//��ֹ���ٱ༭����ˢ��ͼ��
			SetConsoleMode
			(
				hIn,
				(oldConMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT) &
					(~ENABLE_QUICK_EDIT_MODE)
			);

			//���ع��
			CONSOLE_CURSOR_INFO	cursor_info = { (DWORD) 25, FALSE };
			SetConsoleCursorInfo(hOut, &cursor_info);

			RECT	rc, rc2;
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			GetClientRect(hCMD, &rc);
			GetWindowRect(hCMD, &rc2);

			int	w = (rc2.right - rc2.left) - (rc.right - rc.left) + int((wtoi(argv[2])) / scale);
			int	h = (rc2.bottom - rc2.top) - (rc.bottom - rc.top) + int((wtoi(argv[3])) / scale);

			//printf("scale:%f\n", scale);
			//printf("C:%dx%d\n", rc.right - rc.left, rc.bottom - rc.top);
			//printf("W:%dx%d\n", rc2.right - rc2.left, rc2.bottom - rc2.top);
			MoveWindow(hCMD, rc2.left, rc2.top, w, h, 0);
			Sleep(10);
			SetScrollRange(hCMD, 0, 0, 0, 1);
			SetScrollRange(hCMD, 1, 0, 0, 1);
			Sleep(10);
			hRes->w = wtoi(argv[2]);
			hRes->h = wtoi(argv[3]);
			hRes->regioninit(argv[1]);
		}
		else
		{
			hRes->resize(wtoi(argv[2]), wtoi(argv[3]));
			hRes->regioninit(argv[1]);
		}
	}

	if (match(0, L"rotate"))
	{
		rotateres(argv);
	}

	//������
	if (match(0, L"show"))		//��ʾ/��������
	{
		imageres	*hRes = getres(argv[1]);
		if (hRes->type == irCMD) ShowWindow(hRes->hwnd, SW_SHOW);
		if (hRes->type == irBuffer && hRes->hwnd == nullptr)
		{
			wstring *arg = new wstring(argv[1]);

			//��Ϊvs2010,����thread,ֻ������_beginthreadex,ֻ�ܴ���ָ��
			_beginthreadex(NULL, 0, makeWindow, arg, 0, NULL);
		}
	}

	if (match(0, L"hide"))		//����/�رմ���
	{
		imageres	*hRes = &resmap[argv[1]];
		if (hRes->type == irCMD) ShowWindow(hRes->hwnd, SW_HIDE);
		if (hRes->type == irBuffer) PostMessage(hRes->hwnd, WM_CLOSE, 0, 0);
	}

	//���ز���
	if (match(0, L"getpix"))	//�������
	{
		wchar_t		info[60];
		COLORREF	color = GetPixel(hTarget->hdc, wtoi(argv[1]), wtoi(argv[2]));
		swprintf(info, L"%d %d %d", GetRValue(color), GetGValue(color), GetBValue(color));
		SetEnvironmentVariableW(L"image", info);
	}

	if (match(0, L"setpix"))	//��������
	{
		SetPixel(hTarget->hdc, wtoi(argv[1]), wtoi(argv[2]), RGB(wtoi(argv[3]), wtoi(argv[4]), wtoi(argv[5])));
	}

	//������
	if (match(0, L"draw"))
	{
		//ֱ����Ŀ���ϻ�ͼ
		imageres	*hRes = getres(argv[1]);

		hTarget->regTree.add(hRes->regTree, wtoi(argv[2]), wtoi(argv[3]));
		if (argc == 4)
		{
			BitBlt(hTarget->hdc, wtoi(argv[2]), wtoi(argv[3]), hRes->w, hRes->h, hRes->hdc, 0, 0, SRCCOPY);
		}
		else
		{
			if (match(4, L"trans"))
			{
				TransparentBlt
				(
					hTarget->hdc,
					wtoi(argv[2]),
					wtoi(argv[3]),
					hRes->w,
					hRes->h,
					hRes->hdc,
					0,
					0,
					hRes->w,
					hRes->h,
					argc == 8 ? RGB(wtoi(argv[5]), wtoi(argv[6]), wtoi(argv[7])) : RGB
						(
							255,
							255,
							255
						)
				);
			}

			if (match(4, L"alpha")) alphares(argv);
		}
	}

	//������
	if (match(0, L"font"))
	{
		SetBkMode(hTarget->hdc, TRANSPARENT);
		SetTextColor(hTarget->hdc, RGB(wtoi(argv[1]), wtoi(argv[2]), wtoi(argv[3])));
		if (argc > 4)
		{
			HFONT	hFont = CreateFontW
				(
					argc > 5 ? wtoi(argv[5]) : 0,
					argc > 4 ? wtoi(argv[4]) : 0,
					argc > 6 ? wtoi(argv[6]) : 0 /*���ù�*/,
					argc > 7 ? wtoi(argv[7]) : 0 /*���ù�*/,
					argc > 8 ? wtoi(argv[8]) : 400 /*һ�����ֵ��Ϊ400*/,
					argc > 9 ? wtoi(argv[9]) : 0 /*����б��*/,
					argc > 10 ? wtoi(argv[10]) : 0 /*�����»���*/,
					argc > 11 ? wtoi(argv[11]) : 0 /*����ɾ����*/,
					DEFAULT_CHARSET,	//��������ʹ��Ĭ���ַ��������������� _CHARSET ��β�ĳ�������
					OUT_CHARACTER_PRECIS,
					CLIP_CHARACTER_PRECIS,
					//���в������ù�
					DEFAULT_QUALITY,
					//Ĭ���������
					FF_DONTCARE,
					//��ָ��������*/
					argc > 12 ? argv[12] : L"������"	//������
				);
			DeleteObject((HFONT) SelectObject(hTarget->hdc, hFont));
		}
	}

	if (match(0, L"text"))
	{
		//��ʾ���βŻ�ˢ�³���������Ǹ�bug
		for (int i = 0; i < 2; i++)
			TextOutW(hTarget->hdc, wtoi(argv[2]), wtoi(argv[3]), argv[1], wcslen(argv[1]));
	}

	//��ͼ��
	if (match(0, L"pen"))
	{
		HPEN	gPen;
		if (match(1, L"-1"))
			gPen = (HPEN) GetStockObject(NULL_PEN);
		else
			gPen = CreatePen
			(
				PS_SOLID,
				argc > 4 ? wtoi(argv[4]) : 1,
				RGB(wtoi(argv[1]), wtoi(argv[2]), wtoi(argv[3]))
			);

		DeleteObject((HPEN) SelectObject(hTarget->hdc, gPen));
	}

	if (match(0, L"brush"))
	{
		HBRUSH	gBrush;
		if (match(1, L"-1"))
			gBrush = (HBRUSH) GetStockObject(NULL_BRUSH);
		else
			gBrush = CreateSolidBrush(RGB(wtoi(argv[1]), wtoi(argv[2]), wtoi(argv[3])));

		DeleteObject((HBRUSH) SelectObject(hTarget->hdc, gBrush));
	}

	if (match(0, L"line"))
	{
		MoveToEx(hTarget->hdc, wtoi(argv[1]), wtoi(argv[2]), nullptr);
		LineTo(hTarget->hdc, wtoi(argv[3]), wtoi(argv[4]));
	}

	if (match(0, L"ellipse"))
	{
		Ellipse(hTarget->hdc, wtoi(argv[1]), wtoi(argv[2]), wtoi(argv[3]), wtoi(argv[4]));
	}

	if (match(0, L"polygon"))
	{
		int	cnt = argc - 1;
		POINT	*polypt = new POINT[cnt];
		for (int i = 1; i < argc; i++) swscanf(argv[i], L"%d,%d", &polypt[i - 1].x, &polypt[i - 1].y);

		Polygon(hTarget->hdc, polypt, cnt);
		delete[] polypt;
	}

	//��������
	if (match(0, L"sleep"))
	{
		Sleep(wtoi(argv[1]));
	}

	if (match(0, L"cls"))
	{
		//����������cmdͼ���ͼԪ������
		resmap[L"cmd"].regioninit((wchar_t *)L"cmd");
		InvalidateRect(hCMD, nullptr, true);
	}

	if (match(0, L"setvar"))
	{
		HANDLE	hMap = OpenFileMappingW(FILE_MAP_ALL_ACCESS, 0, argv[1]);
		if (NULL == hMap)
			hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 10000, argv[1]);

		HANDLE	pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		wchar_t info[10000];
		GetEnvironmentVariableW(argv[1], info, sizeof(info));
		wcscpy((wchar_t *)pBuffer, info);
	}

	if (match(0, L"getvar"))
	{
		HANDLE	hMap = OpenFileMappingW(FILE_MAP_ALL_ACCESS, 0, argv[1]);
		if (NULL == hMap)
			hMap = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 10000, argv[1]);

		HANDLE	pBuffer = MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		SetEnvironmentVariableW(argv[1], (wchar_t *)pBuffer);
	}

	if (match(0, L"export"))
	{
		wchar_t info[20];
		swprintf(info, L"%d", (int)hCMD);
		SetEnvironmentVariableW(L"image", info);
	}

	if (match(0, L"import"))
	{
		wchar_t *tag = argv[1];

		//��ȡcmd��С�Լ���ͼ���
		DEVMODE dm;
		dm.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);

		int		ax = dm.dmPelsWidth;
		int		bx = GetSystemMetrics(SM_CXSCREEN);
		double		scale = (double)ax / bx;	//У�����ű�
		imageres	res;
		res.hwnd = (HWND) wtoi(argv[2]);
		res.hdc = GetDC(res.hwnd);

		RECT	rc;
		GetClientRect(res.hwnd, &rc);

		res.w = (int)ceil(scale * (rc.right - rc.left));
		res.h = (int)ceil(scale * (rc.bottom - rc.top));
		res.regioninit(tag);

		if (resmap.count(tag)) delres(tag);
		resmap[tag] = res;			//�ѻ�����ӵ����ñ���
	}

	if (match(0, L"list"))
	{
		bool	skip = false;
		if (argc > 2) skip = true;

		ifstream	in(argv[1]);
		string		str;
		wchar_t		wstr[100];
		while (!in.eof())
		{
			getline(in, str);
			MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wstr, sizeof(wstr));
			if (skip)
			{
				if (L":" + wstring(argv[2]) == wstring(wstr)) skip = false;
				continue;
			}

			if (wstring(L"exit") == wstring(wstr)) break;

			image(wstr);
		}

		in.close();
	}

	if (match(0, L"mouse"))
	{
		imageres	*hRes = getres(L"cmd");
		wchar_t		info[100];
		POINT		mosPos;
		int		x = -1, y = -1;
		int		timer = wtoi(argv[1]);

		// ��ȡ��׼��������豸���
		HANDLE		hIn = GetStdHandle(STD_INPUT_HANDLE);
		DWORD		oldConMode;
		GetConsoleMode(hIn, &oldConMode);	// ����
		SetConsoleMode
		(
			hIn,
			(oldConMode | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT) & (~ENABLE_QUICK_EDIT_MODE)
		);

		INPUT_RECORD	Rec;
		DWORD		res;
		DWORD		start = GetTickCount();

		while (timer <= 0 || GetTickCount() - start < timer)
		{
			DWORD	NCE;
			GetNumberOfConsoleInputEvents(hIn, &NCE);
			if (NCE > 0)
			{
				ReadConsoleInputW(hIn, &Rec, 1, &res);
				if (Rec.EventType == MOUSE_EVENT)
				{
					if (Rec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
					{
						GetCursorPos(&mosPos);
						ScreenToClient(hCMD, &mosPos);
						x = min(max((int)scale * mosPos.x, 0), hRes->w);
						y = min(max((int)scale * mosPos.y, 0), hRes->h);
						break;
					}
				}
			}

			Sleep(1);
		}

		if (argc >= 3)
		{
			//��ָ����region�б��в���
			int	ret = 0;
			for (int i = 2; i < argc; i++)
			{
				int	x1, y1, x2, y2;
				swscanf(argv[i], L"%d,%d,%d,%d", &x1, &y1, &x2, &y2);
				if (x >= x1 && x <= x2 && y >= y1 && y <= y2) ret = i - 1;
			}

			swprintf(info, L"%d %d %d", x, y, ret);
			SetEnvironmentVariableW(L"image", info);
			swprintf(info, L"%d", ret);
			SetEnvironmentVariableW(L"imagepic", info);
		}
		else
		{
			//��ͼԪ�������в���
			wstring ret;
			ret = resmap[L"cmd"].regTree.query(x, y);
			swprintf(info, L"%d %d %s", x, y, ret.c_str());
			SetEnvironmentVariableW(L"image", info);
			SetEnvironmentVariableW(L"imagepic", ret.c_str());
		}

		SetConsoleMode(hIn, oldConMode);
	}

	if (match(0, L"picquery"))
	{
		wstring ret = getres(argv[1])->regTree.query(wtoi(argv[2]), wtoi(argv[3]));
		SetEnvironmentVariableW(L"image", ret.c_str());
	}

	if (match(0, L"debug"))
	{
		hTarget->regTree.debug();
	}

	if (match(0, L"union"))
	{
		//�ϲ�ͼ���е�����ͼԪ��һ����ͼ��ͬ����ͼԪ��������ͼ���ͼԪ������
		resmap[argv[1]].regioninit(argv[1]);
	}

	if (match(0, L"cmd"))
	{
		_wsystem(argv[1]);
	}

	if (match(0, L"sound"))
	{
		if (match(1, L"nul"))
		{
			PlaySoundW(NULL, NULL, SND_FILENAME);
		}
		else
		{
			int	timer = wtoi(argv[2]);
			wcout << argv[1] << endl;
			if (timer < 0)
				PlaySoundW(argv[1], NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
			else
				for (int i = 1; i <= timer; i++) PlaySoundW(argv[1], NULL, SND_FILENAME | SND_ASYNC);
		}
	}

	if (hTarget->type == irBuffer && hTarget->hwnd != nullptr) InvalidateRect(hTarget->hwnd, nullptr, true);
	if (hOldTarget != nullptr)
	{
		hTarget = hOldTarget;
		hOldTarget = nullptr;
	}

	LocalFree(argv);
	return;
}

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
		Stop_image();
		break;
	}

	return true;
}
