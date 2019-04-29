//---------------------------------------------------------------------------


#pragma hdrstop

#include "TGCod.h"
#include <math.h>
#include <Math.hpp>
#include <stdio.h>
//---------------------------------------------------------------------------

#pragma package(smart_init)
//---------------------------------------------------------------------------
/// Constructor
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
/// Destructor
__fastcall TGCod::~TGCod()
{
}
//---------------------------------------------------------------------------
/// Constructor copying
__fastcall TGCod::TGCod(const TGCod &c)
{
  G = c.G;
  X = c.X;
  Y = c.Y;
  R = c.R;
  F = c.F;
  M = c.M;
  T = c.T;

  State = c.State;
  Prev = c.Prev;
  Next = c.Next;
  OX = c.OX;
  OY = c.OY;
  Return = c.Return;
  ID = c.ID;
}
//---------------------------------------------------------------------------
/// \brief Constructor Creates a TGCod with params.
/// \param g - type of cod
/// \param x - X coordinate
/// \param y - Y coordinate
/// \param r - radius
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
/// \brief Sets the params.
/// \param g - type of GCod
/// \param x - X coordinate
/// \param y - Y coordinate
/// \param r - radius
void __fastcall TGCod::Set(int g, float x, float y, float r)
{
  G = g;
  X = x;
  Y = y;
  R = r;
  UpdateOXY();
}
//---------------------------------------------------------------------------
/// \brief Sets the type of GCod
/// \param g - type of GCod
void __fastcall TGCod::SetG(int g)
{
  G = g;
  UpdateOXY();
}
//---------------------------------------------------------------------------
/// \brief Sets the F param
/// \param f - GCod F
void __fastcall TGCod::SetF(int f)
{
  F = f;
}
//---------------------------------------------------------------------------
/// \brief Sets the M param
/// \param m - GCod M
void __fastcall TGCod::SetM(int m)
{
  M = m;
}
//---------------------------------------------------------------------------
/// \brief Sets the T param
/// \param t - GCod T
void __fastcall TGCod::SetT(int t)
{
  T = t;
}
//---------------------------------------------------------------------------
/// \brief Sets the GCod radius
/// \param r - GCod R
void __fastcall TGCod::SetR(float r)
{
  if(G != G_CW && G != G_CCW) return;
  R = r;
  UpdateOXY();
  if(Next) Next->UpdateOXY();
}
//---------------------------------------------------------------------------
/// \brief Sets the X coordinate
/// \param x - GCod X
void __fastcall TGCod::SetX(float x)
{
  X = x;
  UpdateOXY();
  if(Next) Next->UpdateOXY();
}
//---------------------------------------------------------------------------
/// \brief Sets the Y coordinate
/// \param y - GCod Y
void __fastcall TGCod::SetY(float y)
{
  Y = y;
  UpdateOXY();
  if(Next) Next->UpdateOXY();
}
//---------------------------------------------------------------------------
/// \brief Sets the X coordinate
/// \param x - GCod X
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

  // the length of the section ab
  double ab = sqrt( (ax-bx)*(ax-bx) + (ay-by)*(ay-by) );
  double ab05 = ab/2;
  // if it turns out that the radius is too small
  if(r < ab05) {
	R = ab05;
    r  = fabs(R);
  }

  // the height of the triangle formed from the section ab
  // and radius r (segments oa and ob)
  double h = r > ab05 ? sqrt( (r*r) - (ab05*ab05) ) : 0; // 04.12.2014

  // alpha angle - angle inclination of the segment ab to the x axis
  double b = fabs( bx - ax );
  double cos_alpha = b / ab;

  // Beta angle - the angle of inclination of the radius R to the x axis is sin_alpha
  double beta = RadToDeg( ArcSin(cos_alpha) );

  // c - odleglosc w osi x srodka okregu O od punktu S (srodka odcinka ab)
  double c = h * cos( DegToRad(beta) );

  // d - distance in the center of the circle O from the point S (the center of the segment ab)
  double d = h * sin( DegToRad(beta) );

  // coordinate points O1 and O2 - the center of the circle
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
//---------------------------------------------------------------------------
