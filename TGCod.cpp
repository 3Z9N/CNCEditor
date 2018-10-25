//---------------------------------------------------------------------------


#pragma hdrstop

#include "TGCod.h"
#include <math.h>
#include <Math.hpp>
#include <stdio.h>
//---------------------------------------------------------------------------

#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TGCod::TGCod()
{
  G = G_NONE;
  X = 0;
  Y = 0;
  R = 0;
  F = M_NONE;
  M = M_NONE;
  T = M_NONE;
  State = csNormal;
  Prev = 0;
  Next = 0;
  OX = 0.0;
  OY = 0.0;
  Return = false;
  ID = -1;
}
//---------------------------------------------------------------------------
__fastcall TGCod::~TGCod()
{
  //TCod *_prev = Prev;
  //TCod *_next = Next;
  //if(_next) _next->Prev = _prev;
  //if(_prev) _prev->Next = _next;
  //if(_next) _next->UpdateOXY();
}
//---------------------------------------------------------------------------
__fastcall TGCod::TGCod(const TGCod &c)
{
  G = c.G;
  X = c.X;
  Y = c.Y;
  R = c.R;
  F = c.F;
  M = c.M;
  T = c.T;

  State = c.State;//Selected = c.Selected;
  Prev = c.Prev;
  Next = c.Next;
  OX = c.OX;
  OY = c.OY;
  Return = c.Return;
  ID = c.ID;
}
//---------------------------------------------------------------------------
__fastcall TGCod::TGCod(int g, float x, float y, float r)
{
  G = g;
  X = x;
  Y = y;
  R = r;
  F = M_NONE;
  M = M_NONE;
  T = M_NONE;
  State = csNormal;
  Prev = 0;
  Next = 0;
  OX = 0.0;
  OY = 0.0;
  Return = false;
  ID = -1;
}
//---------------------------------------------------------------------------
void __fastcall TGCod::Set(int g, float x, float y, float r)
{
  G = g;
  X = x;
  Y = y;
  R = r;
  UpdateOXY();
}
//---------------------------------------------------------------------------
void __fastcall TGCod::SetG(int g)
{
  G = g;
  UpdateOXY();
}
//---------------------------------------------------------------------------
void __fastcall TGCod::SetF(int f)
{
  F = f;
}
//---------------------------------------------------------------------------
void __fastcall TGCod::SetM(int m)
{
  M = m;
}
//---------------------------------------------------------------------------
void __fastcall TGCod::SetT(int t)
{
  T = t;
}
//---------------------------------------------------------------------------
void __fastcall TGCod::SetR(float r)
{
  if(G != G_CW && G != G_CCW) return;
  R = r;
  UpdateOXY();
  if(Next) Next->UpdateOXY();
}
//---------------------------------------------------------------------------
void __fastcall TGCod::SetX(float x)
{
  X = x;
  UpdateOXY();
  if(Next) Next->UpdateOXY();

//	if(Prev) {
//		if(fabs(x - Prev->X) < 1 && Prev) {
//			if(Prev->X < x) Prev->X -= 1;
//			else Prev->X += 1;
//		}
//	}
//	X = x;
//	UpdateOXY();
//	if(Next) Next->UpdateOXY();
}
//---------------------------------------------------------------------------
void __fastcall TGCod::SetY(float y)
{
  Y = y;
  UpdateOXY();
  if(Next) Next->UpdateOXY();

//	if(Prev) {
//		if(fabs(y - Prev->Y) < 1) {
//			if(Prev->Y < y) Prev->Y -= 1;
//			else Prev->Y += 1;
//		}
//	}
//	Y = y;
//	UpdateOXY();
//	if(Next) Next->UpdateOXY();
}
//---------------------------------------------------------------------------
void __fastcall TGCod::UpdateOXY()
{
  if((G != G_CW && G != G_CCW) || R==0) return;
  if(!Prev) return;

  double ax = Prev->X;
  double ay = Prev->Y;
  double bx = X;
  double by = Y;
  double r  = fabs(R);
  double sx = bx>ax  ?  (ax+((bx-ax)/2))  :  (bx+((ax-bx)/2));
  double sy = by>ay  ?  (ay+((by-ay)/2))  :  (by+((ay-by)/2));

  //---------------------------------------------------------------
  // dlugosc odcinka ab
  //---------------------------------------------------------------
  double ab = sqrt( (ax-bx)*(ax-bx) + (ay-by)*(ay-by) );
  double ab05 = ab/2;
  //---- jesli okaze sie ze promien jest za maly
  if(r < ab05) {
	R = ab05;//+1;
    r  = fabs(R);
  }

  //---------------------------------------------------------------
  // wysokosc trojkata utworzonego z odcinka ab i promienia r (odcinki oa i ob)
  //---------------------------------------------------------------
	//double h = r > ab05 ? sqrt( (r*r) - (ab05*ab05) ) : 0;
	//double h = sqrt( (r*r) - (ab05*ab05) ); // 26.03.2014
	double h = r > ab05 ? sqrt( (r*r) - (ab05*ab05) ) : 0; // 04.12.2014

  //---------------------------------------------------------------
  // kat alpha - kata nachylenia odcinka ab do osi x
  //---------------------------------------------------------------
  double b = fabs( bx - ax );
  double cos_alpha = b / ab;

  //---------------------------------------------------------------
  // kat "beta" - kat nachylenia promienia R do osi x wynosi sin_alpha
  //---------------------------------------------------------------
  double beta = RadToDeg( ArcSin(cos_alpha) );

  //---------------------------------------------------------------
  // "c" - odleglosc w osi x srodka okregu O od punktu S (srodka odcinka ab)
  //---------------------------------------------------------------
  double c = h * cos( DegToRad(beta) );

  //---------------------------------------------------------------
  // "d" - odleglosc w osi y srodka okregu O od punktu S (srodka odcinka ab)
  //---------------------------------------------------------------
  double d = h * sin( DegToRad(beta) );

  //---------------------------------------------------------------
  // wspolrzedne punktu O1 i O2 - srodka okregu
  //---------------------------------------------------------------
  double ox1 =   ay > by  ?  sx - c  :  sx + c;
  double oy1 =   ax < bx  ?  sy - d  :  sy + d;
  double ox2 =  ox1 > sx  ?  sx - c  :  sx + c;
  double oy2 =  oy1 > sy  ?  sy - d  :  sy + d;

  if(G == G_CW) {
    OX = ox1;
    OY = oy1;
    if(R < 0) {
      OX = ox2;
      OY = oy2;
    }
  }
  else { // G == G_CCW
    OX = ox2;
    OY = oy2;
    if(R < 0) {
      OX = ox1;
      OY = oy1;
    }
  }
}
//---------------------------------------------------------------------------//---------------------------------------------------------------------------
