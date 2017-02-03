// image.cpp : Defines the entry point for the console application.
//
//版本记录:
//Ver 1.0 原版
//Ver 1.1 增加了/?帮助选项
//Ver 1.2 增加了/d清空图片选项
//Ver 1.3 可以用各种光栅操作码，加了/s选项，效率更高 
//Ver 1.4 修复/d选项的BUG
//Ver 1.5 修复内存泄露
//Ver 1.6 增加了/TRANSPARENTBLT选项
//Ver 1.7 不区分大小写，增加了/l批量处理选项，修复/d选项的BUG
//Ver 1.8 /l支持双缓冲
//Ver 1.9 修复/l选项的BUG和内存泄露，修复/s选项的BUG

#include <fstream>
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

typedef HWND (WINAPI *PROCGETCONSOLEWINDOW)();
PROCGETCONSOLEWINDOW GetConsoleWindow;

void Draw(HDC hDC,HDC hDCMem,int imx,int imy,BITMAP bi,const char *dwRop);

int main(int argc, char *argv[])
{
	if (argc == 2 && _stricmp(argv[1], "/?") == 0 || argc == 1)
	{
		printf("控制台显示图片\n"
			"Ver 1.9 By Byaidu\n\n"
			"image [/d] [/s] [/l listfile] [bmpfile] [X] [Y] [Width] [Height] [/TRANSPARENTBLT] [/BLACKNESS] [/DSTINVERT] [/MERGECOPY] [/MERGEPAINT] [/NOTSRCCOPY] [/NOTSRCERASE] [/PATCOPY] [/PATPAINT] [/PATINVERT] [/SRCAND] [/SRCCOPY] [/SRCERASE] [/SRCINVERT] [/SRCPAINT] [/WHITENESS] \n\n"
			"\t/d\t清空当前窗口中显示的图片\n"
			"\t/s\t清空当前窗口指定大小中显示的图片\n"
			"\t/l listfile\t使用指定文件批量处理\n"
			"\tbmpfile\t指定要显示的图片\n"
			"\tX\t指定目标矩形区域左上角的X轴逻辑坐标\n"
			"\tY\t指定目标矩形区域左上角的Y轴逻辑坐标\n"
			"\tWidth\t为/s选项指定目标矩形区域的逻辑宽度。\n"
			"\tHeight\t为/s选项指定目标矩形区域的逻辑高度。\n"
			"\t/TRANSPARENTBLT\t表示把源位图中的RGB值为255,255,255当作透明颜色拷贝到目标矩形区域\n"
			"\t/BLACKNESS\t表示使用与物理调色板的索引0相关的色彩来填充目标矩形区域，（对缺省的物理调色板而言，该颜色为黑色）\n"
			"\t/DSTINVERT\t表示使目标矩形区域颜色取反\n"
			"\t/MERGECOPY\t表示使用布尔型的AND（与）操作符将源矩形区域的颜色与特定模式组合一起\n"
			"\t/MERGEPAINT\t通过使用布尔型的OR（或）操作符将反向的源矩形区域的颜色与目标矩形区域的颜色合并\n"
			"\t/NOTSRCCOPY\t将源矩形区域颜色取反，于拷贝到目标矩形区域\n"
			"\t/NOTSRCERASE\t使用布尔类型的OR（或）操作符组合源和目标矩形区域的颜色值，然后将合成的颜色取反\n"
			"\t/PATCOPY\t将特定的模式拷贝到目标位图上\n"
			"\t/PATPAINT\t通过使用布尔OR（或）操作符将源矩形区域取反后的颜色值与特定模式的颜色合并然后使用OR（或）操作符将该操作的结果与目标矩形区域内的颜色合并\n"
			"\t/PATINVERT\t通过使用XOR（异或）操作符将源和目标矩形区域内的颜色合并\n"
			"\t/SRCAND\t通过使用AND（与）操作符来将源和目标矩形区域内的颜色合并\n"
			"\t/SRCCOPY\t将源矩形区域直接拷贝到目标矩形区域\n"
			"\t/SRCERASE\t通过使用AND（与）操作符将目标矩形区域颜色取反后与源矩形区域的颜色值合并\n"
			"\t/SRCINVERT\t通过使用布尔型的XOR（异或）操作符将源和目标矩形区域的颜色合并"
			"\t/SRCPAINT\t通过使用布尔型的OR（或）操作符将源和目标矩形区域的颜色合并\n"
			"\t/WHITENESS\t使用与物理调色板中索引1有关的颜色填充目标矩形区域（对于缺省物理调色板来说，这个颜色就是白色）\n\n"
			"如直接使用image abc.bmp 10 50 不加后面的参数，默认使用/SRCCOPY\n"
			"注意，X,Y所构成的坐标由0起算\n"
			"例如：\n"
			"\timage abc.bmp 10 50\n"
			"\timage abc.bmp 10 50 /SRCAND\n"
			"\timage /d\n"
			"\timage /s 10 10 100 100\n");
		return 0;
	}
	HMODULE hKernel32 = GetModuleHandle("kernel32");
	GetConsoleWindow = (PROCGETCONSOLEWINDOW)GetProcAddress(hKernel32,"GetConsoleWindow");
	HWND hCMD = GetConsoleWindow();
	HDC hDC = GetDC(hCMD);
	if (argc == 2 && _stricmp(argv[1], "/d") == 0)
	{
		InvalidateRect(hCMD, NULL, true);
		ReleaseDC(hCMD, hDC);
		return 0;
	}
	if (argc == 6 && _stricmp(argv[1], "/s") == 0)
	{
		unsigned long colorlist[]={RGB(0,0,0),RGB(0,0,128),RGB(0,128,0),RGB(0,128,128),RGB(128,0,0),RGB(128,0,128),RGB(128,128,0),RGB(192,192,192),RGB(128,128,128),RGB(0,0,255),RGB(0,255,0),RGB(0,255,255),RGB(255,0,0),RGB(255,0,255),RGB(255,255,0),RGB(255,255,255)};
		HANDLE hStdout;
		CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
		hStdout = GetStdHandle(STD_OUTPUT_HANDLE); 
		GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
		int imx = atoi(argv[2]);
		int imy = atoi(argv[3]);
		int imw = atoi(argv[4]);
		int imh = atoi(argv[5]);
		tagRECT rect={imx, imy, imx+imw, imy+imh};
		HBRUSH brush=CreateSolidBrush(colorlist[csbiInfo.wAttributes>>4]);
		FillRect(hDC, &rect,brush);
		ReleaseDC(hCMD, hDC);
		return 0;
	}
	//////////////////////////////////////图像操作////////////////////////////////////////////
	HDC hDCMem = CreateCompatibleDC(hDC);//载入图像
	HBITMAP hBitmap;
	if (argc == 3 && _stricmp(argv[1], "/l") == 0)
	{
		RECT rc; 
		int width; 
		int height; 
		GetClientRect(hCMD, &rc); /* hwnd 为窗口句柄 */ 
		width = rc.right - rc.left; /* 高 */ 
		height = rc.bottom - rc.top; /* 宽 */ 
		HDC hDCBuffer = CreateCompatibleDC(hDC);//双缓冲
		HBITMAP hBitmapBuffer = CreateCompatibleBitmap(hDC,width,height);
		SelectObject(hDCBuffer, hBitmapBuffer);
		HBITMAP hBitmapX = CreateCompatibleBitmap(hDC,width,height);
		SelectObject(hDC, hBitmapX);
		ifstream in;
		string str;
		string sTmp[3];
		in.open(argv[2]);
		while(!in.eof())
		{
			int i=0;
			getline(in,str);
			istringstream istr(str);
			while(!istr.eof())
			{
				istr>>sTmp[i];
				i++;
			}
			hBitmap = (HBITMAP)LoadImageA(NULL, (char*)sTmp[0].data(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			SelectObject(hDCMem, hBitmap);
			BITMAP bi = {0};
			GetObject(hBitmap, sizeof(BITMAP), &bi);
			if (i == 3) Draw(hDCBuffer,hDCMem,atoi(sTmp[1].c_str()),atoi(sTmp[2].c_str()),bi,"/SRCCOPY");
			if (i == 4) Draw(hDCBuffer,hDCMem,atoi(sTmp[1].c_str()),atoi(sTmp[2].c_str()),bi,(char*)sTmp[3].data());
		}
		in.close();
		BitBlt(hDC, 0, 0, width, height, hDCBuffer, 0, 0, SRCCOPY);
		DeleteObject(hBitmap);
		DeleteObject(hBitmapBuffer);
		DeleteObject(hBitmapX);
		DeleteDC(hDCBuffer);
		DeleteDC(hDCMem);
		ReleaseDC(hCMD, hDC);
		return 0;
	}
	hBitmap = (HBITMAP)LoadImageA(NULL, argv[1], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	SelectObject(hDCMem, hBitmap);
	BITMAP bi = {0};
	GetObject(hBitmap, sizeof(BITMAP), &bi);
	int imx = atoi(argv[2]);
	int imy = atoi(argv[3]);
	if (argc == 4) Draw(hDC,hDCMem,imx,imy,bi,"/SRCCOPY");
	if (argc == 5) Draw(hDC,hDCMem,imx,imy,bi,argv[4]);
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
	ReleaseDC(hCMD, hDC);
	return 0;
}

void Draw(HDC hDC,HDC hDCMem,int imx,int imy,BITMAP bi,const char *dwRop)
{
	if (_stricmp(dwRop, "/TRANSPARENTBLT") == 0)
	{
		TransparentBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, bi.bmWidth, bi.bmHeight, RGB(255,255,255));
	}
	if (_stricmp(dwRop, "/BLACKNESS") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, BLACKNESS);
	}
	if (_stricmp(dwRop, "/DSTINVERT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, DSTINVERT);
	}
	if (_stricmp(dwRop, "/MERGECOPY") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, MERGECOPY);
	}
	if (_stricmp(dwRop, "/MERGEPAINT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, MERGEPAINT);
	}
	if (_stricmp(dwRop, "/NOTSRCCOPY") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, NOTSRCCOPY);
	}
	if (_stricmp(dwRop, "/NOTSRCERASE") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, NOTSRCERASE);
	}
	if (_stricmp(dwRop, "/PATCOPY") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, PATCOPY);
	}
	if (_stricmp(dwRop, "/PATPAINT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, PATPAINT);
	}
	if (_stricmp(dwRop, "/PATINVERT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, PATINVERT);
	}
	if (_stricmp(dwRop, "/SRCAND") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCAND);
	}
	if (_stricmp(dwRop, "/SRCCOPY") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCCOPY);
	}
	if (_stricmp(dwRop, "/SRCERASE") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCERASE);
	}
	if (_stricmp(dwRop, "/SRCINVERT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCINVERT);
	}
	if (_stricmp(dwRop, "/SRCPAINT") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCPAINT);
	}
	if (_stricmp(dwRop, "/WHITENESS") == 0)
	{
		BitBlt(hDC, imx, imy, bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, WHITENESS);
	}
}
