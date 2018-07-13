#pragma   once
#include <windows.h>
#include <cstdio>
#include <iostream>
#include <algorithm>
#include <map>
#include <stack>
#include <ctime>
using namespace std;

struct P{
	int single;
	wstring src;
	int x1, y1, x2, y2, x0, y0;
	P *l, *r;
};
struct BUF{
	P *p;
	BUF(){};
	BUF(wstring src,int w,int h){
		p = new P{ 1,src,0,0,w,h,0,0 };
	}
};
map<wstring,BUF> bufmap;
/*
void Init_regionmgr() //初始化
{
	//srand(time(0));
	//获取cmd大小以及绘图句柄
	hCMD = GetConsoleWindow();
	HDC dc = GetDC(hCMD);
	DEVMODE dm;
	dm.dmSize = sizeof(DEVMODE);
	EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm);
	int ax = dm.dmPelsWidth;
	int bx = GetSystemMetrics(SM_CXSCREEN);
	double scale = (double)ax / bx;//校正缩放比
	RECT rc;
	GetClientRect(hCMD, &rc);
	int w = (int)ceil(scale*(rc.right - rc.left));
	int h = (int)ceil(scale*(rc.bottom - rc.top));
	srcmap[0] = dc;//0号资源为控制台dc
	srcmap[1] = 0;//1号资源为纯黑背景
	BUF buf(1, w-1, h-1);
	bufmap["cmd"] = buf; //把cmd作为资源添加到调用表中
	hTarget = &bufmap["cmd"];//绘图默认指向cmd
	return;
}
*/

//合并剪枝 
void singleunion(P *p)
{
	if (p->r->single == 1 && p->l->single == 1 && p->r->src == p->l->src&&
		p->r->x1 - p->l->x1 == p->r->x0 - p->l->x0&&
		p->r->y1 - p->l->y1 == p->r->y0 - p->l->y0)
	{
		p->x0 = p->l->x0;
		p->y0 = p->l->y0;
		p->src = p->l->src;
		p->single = 1;
		//todo:delete point
		return;
	}
	if (p->r->single &&p->r->x1== p->x1&&p->r->y1 == p->y1&&p->r->x2 == p->x2&&p->r->y2 == p->y2)
	{
		p->x0 = p->r->x0;
		p->y0 = p->r->y0;
		p->src = p->r->src;
		p->single = 1;
		return;
	}
	if (p->l->single &&p->l->x1 == p->x1&&p->l->y1 == p->y1&&p->l->x2 == p->x2&&p->l->y2 == p->y2)
	{
		p->x0 = p->l->x0;
		p->y0 = p->l->y0;
		p->src = p->l->src;
		p->single = 1;
		return;
	}
}
//x1y1x2y2为目标p坐标，x0y0为源v坐标 
void singleupdate(P *p,int x1,int y1,int x2,int y2,int x0,int y0, wstring v,int now=0)
{
	if (p->single==1&&p->x1==x1&&p->y1==y1&&p->x2==x2&&p->y2==y2){
		if (!(p->x0==x0&&p->y0==y0&&p->src==v)){
			p->x0=x0,p->y0=y0,p->src=v;
			/*
			if (hTarget==&bufmap["cmd"]){
				//printf("BITBLT:(%d,%d)(%d,%d):%d(%d,%d)\n",p->x1,p->y1,p->x2,p->y2,p->src,p->x0,p->y0);
				BitBlt(srcmap[0], p->x1, p->y1, p->x2 - p->x1 + 1, p->y2 - p->y1 + 1, srcmap[p->src], p->x0, p->y0, SRCCOPY);
			}
			*/
		}
		return;
	}
	if (p->single){
		if (now) {
			if (x1 != p->x1) {
				p->l = new P{ 1,p->src ,p->x1, p->y1, x1 - 1, p->y2, p->x0, p->y0 };
				p->r = new P{ 1,p->src ,x1,p->y1,p->x2,p->y2,p->x0 + x1 - p->x1,p->y0 };
				singleupdate(p->r, x1, y1, x2, y2, x0, y0, v, !now);
			}
			else {
				p->l = new P{ 1,p->src ,p->x1,p->y1,x2,p->y2,p->x0,p->y0 };
				p->r = new P{ 1,p->src ,x2 + 1,p->y1,p->x2,p->y2,p->x0 + x2 + 1 - p->x1,p->y0 };
				singleupdate(p->l, x1, y1, x2, y2, x0, y0, v, !now);
			}
		}
		else {
			if (y1 != p->y1) {
				p->l = new P{ 1,p->src ,p->x1, p->y1, p->x2, y1 - 1, p->x0, p->y0 };
				p->r = new P{ 1,p->src ,p->x1,y1,p->x2,p->y2,p->x0,p->y0 + y1 - p->y1 };
				singleupdate(p->r, x1, y1, x2, y2, x0, y0, v, !now);
			}
			else {
				p->l = new P{ 1,p->src ,p->x1,p->y1,p->x2,y2,p->x0,p->y0 };
				p->r = new P{ 1,p->src ,p->x1,y2 + 1,p->x2,p->y2,p->x0,p->y0 + y2 + 1 - p->y1 };
				singleupdate(p->l, x1, y1, x2, y2, x0, y0, v, !now);
			}
		}
		p->single=0;
	}else{
		if (now){
			if (x2<= p->l->x2) singleupdate(p->l,x1,y1,x2,y2,x0,y0,v,!now);
			else if (x1>= p->r->x1) singleupdate(p->r,x1,y1,x2,y2,x0,y0,v,!now);
			else singleupdate(p->l,x1,y1, p->l->x2,y2,x0,y0,v,!now),singleupdate(p->r,p->r->x1,y1,x2,y2,x0+p->r->x1-x1,y0,v,!now);
		}else{
			if (y2<= p->l->y2) singleupdate(p->l,x1,y1,x2,y2,x0,y0,v,!now);
			else if (y1>= p->r->y1) singleupdate(p->r,x1,y1,x2,y2,x0,y0,v,!now);
			else singleupdate(p->l,x1,y1,x2, p->l->y2,x0,y0,v,!now),singleupdate(p->r,x1,p->r->y1,x2,y2,x0,y0+p->r->y1-y1,v,!now);
		}
	}
	singleunion(p);
}
//x1y1x2y2为源p坐标，x0y0为目标v坐标 
void complexupdate(P *p,int x1,int y1,int x2,int y2,int x0,int y0,P *v,int now=0)
{
	if (p->single){//检索区域与节点x有交集，且节点x为单源图块 
		singleupdate(v,x0,y0,x0+x2-x1,y0+y2-y1,p->x0+x1-p->x1,p->y0+y1-p->y1,p->src);
		return;
	}
	if (now){
		if (x2<= p->l->x2) complexupdate(p->l,x1,y1,x2,y2,x0,y0,v,!now);
		else if (x1>= p->r->x1) complexupdate(p->r,x1,y1,x2,y2,x0,y0,v,!now);
		else complexupdate(p->l,x1,y1, p->l->x2,y2,x0,y0,v,!now),complexupdate(p->r, p->r->x1,y1,x2,y2,x0+p->r->x1-x1,y0,v,!now);
	}else{
		if (y2<= p->l->y2) complexupdate(p->l,x1,y1,x2,y2,x0,y0,v,!now);
		else if (y1>= p->r->y1) complexupdate(p->r,x1,y1,x2,y2,x0,y0,v,!now);
		else complexupdate(p->l,x1,y1,x2, p->l->y2,x0,y0,v,!now),complexupdate(p->r,x1, p->r->y1,x2,y2,x0,y0+p->r->y1-y1,v,!now);
	}
}

void colorregion(HDC hDC,int color,int x1,int y1,int x2,int y2)
{
	HPEN gPen = CreatePen(PS_SOLID, 1, color);
	HBRUSH gBrush = CreateSolidBrush(color);
	HPEN oPen = (HPEN)SelectObject(hDC, gPen);
	HBRUSH oBrush = (HBRUSH)SelectObject(hDC, gBrush);
	//Rectangle(srcmap[0], 500,500,505,505);
	Rectangle(hDC, x1, y1, x2 + 1, y2 + 1);
	//getchar();
	//printf("(%d,%d)(%d,%d):%d(%d,%d)\n",p->x1,p->y1,p->x2,p->y2,p->src,p->x0,p->y0);
	SelectObject(hDC, oPen);
	SelectObject(hDC, oBrush);
	DeleteObject(gPen);
	DeleteObject(gBrush);
}

//先序遍历
void show(P *p)
{
	HWND hCMD = GetConsoleWindow();
	HDC hDC = GetDC(hCMD);
	if (p->single){
		int color = rand() % 16777216;
		colorregion(hDC, color, p->x1, p->y1, p->x2, p->y2);
	} else {
		//printf("(%d,%d)(%d,%d)\n", p->x1, p->y1, p->x2, p->y2);
		show(p->l);
		show(p->r);
	}
	ReleaseDC(hCMD,hDC);
}
wstring query(P *p, int x, int y, int now = 0)
{
	if (p->single) return p->src;
	if (now) {
		if (x <= p->l->x2) return query(p->l, x, y, !now);
		else return query(p->r, x, y, !now);
	} else {
		if (y <= p->l->y2) return query(p->l, x, y, !now);
		else return query(p->r, x, y, !now);
	}
}
/*
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
	match(0, "load")
	{
		char *tag; //资源描述符
		tag = (argc == 3) ? argv[2] : argv[1];
		HBITMAP bmp=(HBITMAP)LoadImageA(NULL, argv[1], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
		HDC dc = CreateCompatibleDC(nullptr);
		SelectObject(dc, bmp);
		srcmap[++srccnt]=dc;
		BITMAP bi;
		GetObject(bmp, sizeof(BITMAP), &bi);
		BUF buf(srccnt,bi.bmWidth-1,bi.bmHeight-1);
		bufmap[tag]=buf;
	}
	match(0, "buffer")
	{
		BUF buf(1, hTarget->p->x2, hTarget->p->y2);
		bufmap[argv[1]] = buf;
	}
	match(0, "target")
	{
		hTarget = &bufmap[argv[1]];
	}
	match(0, "draw")
	{
		BUF *buf = &bufmap[argv[1]];
		complexupdate(buf->p, 0, 0, buf->p->x2, buf->p->y2, atoi(argv[2]), atoi(argv[3]), hTarget->p);
	}
	match(0, "show")
	{
		BUF *buf = &bufmap[argv[1]];
		show(buf->p);
	}
	LocalFree(argvw);
	return;
}
*/