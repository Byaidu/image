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
//Ver 2.0 增强/l的功能

#include <fstream>
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

string help = "控制台显示图片\nVer 2.0 By Byaidu\n\nimage [/d] [/s] [/l listfile] [bmpfile] [X] [Y] [Width] [Height] [/TRANSPARENTBLT] [/SRCAND] [/SRCCOPY]\n\n\t/d\t清空当前窗口中显示的图片\n\t/s\t清空当前窗口中指定矩形区域中显示的图片\n\t/l listfile\t使用指定文件批量处理\n\tbmpfile\t指定要显示的图片\n\tX\t指定矩形区域左上角的X轴逻辑坐标\n\tY\t指定矩形区域左上角的Y轴逻辑坐标\n\tWidth\t为/s选项指定矩形区域的逻辑宽度。\n\tHeight\t为/s选项指定矩形区域的逻辑高度。\n\t/TRANSPARENTBLT\t表示把图片中的RGB值为255,255,255当作透明颜色拷贝到当前窗口\n\t/SRCAND\t通过使用与操作符来将图片和当前窗口中的颜色合并\n\t/SRCCOPY\t将图片直接拷贝到目标矩形区域\n\n若直接使用image abc.bmp 10 50 不加后面的参数，默认使用/SRCCOPY\n注意，X,Y所构成的坐标由0起算\n";
unsigned long colorlist[] = { RGB(0,0,0),RGB(0,0,128),RGB(0,128,0),RGB(0,128,128),RGB(128,0,0),RGB(128,0,128),RGB(128,128,0),RGB(192,192,192),RGB(128,128,128),RGB(0,0,255),RGB(0,255,0),RGB(0,255,255),RGB(255,0,0),RGB(255,0,255),RGB(255,255,0),RGB(255,255,255) };
string transparentlist[] = { "/SRCCOPY","/SRCPAINT","/SRCAND","/SRCINVERT","/SRCERASE","/NOTSRCCOPY","/NOTSRCERASE","/MERGECOPY","/MERGEPAINT","/PATCOPY","/PATPAINT","/PATINVERT","/DSTINVERT","/BLACKNESS","/WHITENESS" };
DWORD transparentlistsec[] = { SRCCOPY,SRCPAINT,SRCAND,SRCINVERT,SRCERASE,NOTSRCCOPY,NOTSRCERASE,MERGECOPY,MERGEPAINT,PATCOPY,PATPAINT,PATINVERT,DSTINVERT,BLACKNESS,WHITENESS };

HDC hDC, hDCMem, hDCBuffer;
string lastfile;
HWND hCMD;
RECT rc;
int winwidth, winheight;
HBITMAP hBitmap;
BITMAP bi;

void deleteimage(HDC dc,int x, int y, int w, int h)
{
	HANDLE hStdout;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo(hStdout, &csbiInfo);
	tagRECT rect = { x, y, x + w, y + h };
	HBRUSH brush = CreateSolidBrush(colorlist[csbiInfo.wAttributes >> 4]);
	FillRect(dc, &rect, brush);
}

void readlist(const char listfile[])
{
	ifstream in;
	string str, sTmp[5];
	in.open(listfile);
	while (!in.eof())
	{
		getline(in, str);
		istringstream istr(str);
		int i = 0;
		while (!istr.eof()) {
			istr >> sTmp[i];
			if (sTmp[i] != " ") i++;
		}
		if (_stricmp(sTmp[0].c_str(), "/d") == 0) { deleteimage(hDCBuffer, 0, 0, winwidth, winheight); continue; }
		if (_stricmp(sTmp[0].c_str(), "/s") == 0) { deleteimage(hDCBuffer, atoi(sTmp[1].c_str()), atoi(sTmp[2].c_str()), atoi(sTmp[3].c_str()), atoi(sTmp[4].c_str())); continue; }
		if (_stricmp(sTmp[0].c_str(), "/l") == 0) { readlist(sTmp[1].c_str()); continue; }
		//////////////////////////////////////图像操作////////////////////////////////////////////
		if (sTmp[0] != lastfile)
		{
			hBitmap = (HBITMAP)LoadImageA(NULL, (char*)sTmp[0].data(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
			lastfile = sTmp[0];
			GetObject(hBitmap, sizeof(BITMAP), &bi);
			SelectObject(hDCMem, hBitmap);
		}
		if (i == 3) BitBlt(hDCBuffer, atoi(sTmp[1].c_str()), atoi(sTmp[2].c_str()), bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCCOPY);
		if (i == 4)
		{
			if (_stricmp(sTmp[3].c_str(), "/TRANSPARENTBLT") == 0)
			{
				TransparentBlt(hDCBuffer, atoi(sTmp[1].c_str()), atoi(sTmp[2].c_str()), bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, bi.bmWidth, bi.bmHeight, RGB(255, 255, 255));
			}
			else
			{
				int j = 0;
				while (sTmp[3] != transparentlist[j]) j++;
				BitBlt(hDCBuffer, atoi(sTmp[1].c_str()), atoi(sTmp[2].c_str()), bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, transparentlistsec[j]);
			}
		}
	}
	in.close();
}

int main(int argc, char *argv[])
{
	if (argc == 1 || _stricmp(argv[1], "/?") == 0)
	{
		cout << help << endl;
		return 0;
	}
	hCMD = GetConsoleWindow();
	hDC = GetDC(hCMD);
	if (_stricmp(argv[1], "/d") == 0)
	{
		InvalidateRect(hCMD, NULL, true);
		ReleaseDC(hCMD, hDC);
		return 0;
	}
	if (_stricmp(argv[1], "/s") == 0)
	{
		deleteimage(hDC, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5]));
		ReleaseDC(hCMD, hDC);
		return 0;
	}
	//////////////////////////////////////图像操作////////////////////////////////////////////
	hDCMem = CreateCompatibleDC(hDC);//准备载入图像
	if (_stricmp(argv[1], "/l") == 0)
	{
		GetClientRect(hCMD, &rc);
		winwidth = rc.right - rc.left;
		winheight = rc.bottom - rc.top;
		hDCBuffer = CreateCompatibleDC(hDC);//双缓冲
		HBITMAP hBitmapBuffer = CreateCompatibleBitmap(hDC,winwidth,winheight);
		SelectObject(hDCBuffer, hBitmapBuffer);
		BitBlt(hDCBuffer, 0, 0, winwidth, winheight, hDC, 0, 0, SRCCOPY);
		readlist(argv[2]);
		BitBlt(hDC, 0, 0, winwidth, winheight, hDCBuffer, 0, 0, SRCCOPY);
		DeleteObject(hBitmap);
		DeleteObject(hBitmapBuffer);
		DeleteDC(hDCBuffer);
		DeleteDC(hDCMem);
		ReleaseDC(hCMD, hDC);
		return 0;
	}
	hBitmap = (HBITMAP)LoadImageA(NULL, argv[1], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	GetObject(hBitmap, sizeof(BITMAP), &bi);
	SelectObject(hDCMem, hBitmap);
	if (argc == 4) BitBlt(hDC, atoi(argv[2]), atoi(argv[3]), bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, SRCCOPY);
	if (argc == 5)
	{
		if (_stricmp(argv[4], "/TRANSPARENTBLT") == 0)
		{
			TransparentBlt(hDC, atoi(argv[2]), atoi(argv[3]), bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, bi.bmWidth, bi.bmHeight, RGB(255, 255, 255));
		}
		else
		{
			int j = 0;
			while (argv[4] != transparentlist[j]) j++;
			BitBlt(hDC, atoi(argv[2]), atoi(argv[3]), bi.bmWidth, bi.bmHeight, hDCMem, 0, 0, transparentlistsec[j]);
		}
	}
	DeleteObject(hBitmap);
	DeleteDC(hDCMem);
	ReleaseDC(hCMD, hDC);
	return 0;
}