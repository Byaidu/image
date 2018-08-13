#ifndef _REGION_HPP_
#define _REGION_HPP_

#include <string>
#include <list>
#include <math.h>
#define toRange(x, min, max)	((x) < (min) ? (min) : (x) > (max) ? (max) : (x))
#define inRange(x, min, max)	((min) <= (x) && (x) < (max))
using namespace std;

void		colorregion(HDC hDC, int color, int x1, int y1, int x2, int y2);
void		colorborder(HDC hDC, int color, int x1, int y1, int x2, int y2);
int		HSL2RGB(double H, double S, double L);

class		region
{
public:
	int	x1, y1, x2, y2;
	wstring name;
	inline region(int ix1, int iy1, int ix2, int iy2, const wstring &iname) :
	x1(ix1),
	y1(iy1),
	x2(ix2),
	y2(iy2),
	name(iname)
	{
	}
};
class	regSet
{
	list<region>	regs;
	inline void region_add(int x1, int y1, int x2, int y2, const wstring &name)
	{
		if (!(x1 == x2 || y1 == y2)) regs.push_back(region(x1, y1, x2, y2, name));
	}

public:
	void add(const region &r)
	{
		for (list<region>::iterator ri = regs.begin(); ri != regs.end();)
		{
			int	x1 = toRange(r.x1, ri->x1, ri->x2);
			int	y1 = toRange(r.y1, ri->y1, ri->y2);
			int	x2 = toRange(r.x2, ri->x1, ri->x2);
			int	y2 = toRange(r.y2, ri->y1, ri->y2);

			if (!(x1 == x2 || y1 == y2))
			{
				region_add(x1, ri->y1, ri->x2, y1, ri->name);
				region_add(x2, y1, ri->x2, ri->y2, ri->name);
				region_add(ri->x1, y2, x2, ri->y2, ri->name);
				region_add(ri->x1, ri->y1, x1, y2, ri->name);

				regs.erase(ri++);
			}
			else
				++ri;
		}

		regs.push_back(r);
	}

	void add(const regSet &rS, int x0, int y0)
	{
		for (list<region>::const_iterator ri = rS.regs.begin(); ri != rS.regs.end(); ++ri)
			add(region(ri->x1 + x0, ri->y1 + y0, ri->x2 + x0, ri->y2 + y0, ri->name));
	}

	void clear()
	{
		for (list<region>::iterator ri = regs.begin(); ri != regs.end();) regs.erase(ri++);
	}

	const wstring query(int mx, int my) const
	{
		for (list<region>::const_iterator ri = regs.begin(); ri != regs.end(); ++ri)
			if (inRange(mx, ri->x1, ri->x2) && inRange(my, ri->y1, ri->y2)) return ri->name;

		return L"";
	}

	void debug() const
	{
		HDC	hdc = GetDC(GetConsoleWindow());
		for (list<region>::const_iterator ri = regs.begin(); ri != regs.end(); ++ri)
		{
			//wprintf(L"%d %d %d %d %s\n", ri->x1, ri->y1, ri->x2, ri->y2, ri->name.c_str());
			colorborder
			(
				hdc,
				HSL2RGB(rand() / (float)32768, 1, 0.5),
				ri->x1,
				ri->y1,
				ri->x2 - 1,
				ri->y2 - 1
			);
		}
	}
};

void colorregion(HDC hDC, int color, int x1, int y1, int x2, int y2)
{
	HPEN	gPen = CreatePen(PS_SOLID, 1, color);
	HBRUSH	gBrush = CreateSolidBrush(color);
	HPEN	oPen = (HPEN) SelectObject(hDC, gPen);
	HBRUSH	oBrush = (HBRUSH) SelectObject(hDC, gBrush);

	Rectangle(hDC, x1, y1, x2 + 1, y2 + 1);

	SelectObject(hDC, oPen);
	SelectObject(hDC, oBrush);
	DeleteObject(gPen);
	DeleteObject(gBrush);
};

void colorborder(HDC hDC, int color, int x1, int y1, int x2, int y2)
{
	HPEN	gPen = CreatePen(PS_SOLID, 1, color);
	HBRUSH	gBrush = (HBRUSH) GetStockObject(NULL_BRUSH);
	HPEN	oPen = (HPEN) SelectObject(hDC, gPen);
	HBRUSH	oBrush = (HBRUSH) SelectObject(hDC, gBrush);

	Rectangle(hDC, x1, y1, x2 + 1, y2 + 1);

	SelectObject(hDC, oPen);
	SelectObject(hDC, oBrush);
	DeleteObject(gPen);
	DeleteObject(gBrush);
};
int HSL2RGB(double H, double S, double L)
{
	double	R, G, B;
	if (S == 0)
	{
		R = G = B = L;
	}
	else
	{
		double	T[3];
		double	q, p;
		if (L < 0.5)
			q = L * (1.0 + S);
		else
			q = L + S - L * S;
		p = 2.0 * L - q;
		T[0] = H + 0.3333333f;
		T[1] = H;
		T[2] = H - 0.3333333f;
		for (int i = 0; i < 3; i++)
		{
			if (T[i] < 0) T[i] += 1.0f;
			if (T[i] > 1) T[i] -= 1.0f;
			if ((T[i] * 6) < 1)
			{
				T[i] = p + ((q - p) * 6.0f * T[i]);
			}
			else if ((T[i] * 2.0f) < 1)
			{
				T[i] = q;
			}
			else if ((T[i] * 3.0f) < 2)
			{
				T[i] = p + (q - p) * ((2.0f / 3.0f) - T[i]) * 6.0f;
			}
			else
				T[i] = p;
		}

		R = T[0];
		G = T[1];
		B = T[2];
	}

	R = ((R > 1) ? 1 : ((R < 0) ? 0 : R));	//取值范围(0,1)
	G = ((G > 1) ? 1 : ((G < 0) ? 0 : G));	//取值范围(0,1)
	B = ((B > 1) ? 1 : ((B < 0) ? 0 : B));	//取值范围(0,1)
	return RGB((int)(R * 255), (int)(G * 255), (int)(B * 255));
}
#endif // _REGION_HPP_
