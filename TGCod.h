//---------------------------------------------------------------------------

#ifndef TGCodH
#define TGCodH

#include <vcl.h>
enum GCOD_TYPE { G_NONE=-1, G_DRV=0, G_LIN=1, G_CW=2, G_CCW=3,
                 G_ABS=90, G_INC=91 };
enum MCOD_TYPE { M_NONE=-1, M_END=2, M_SPINDLE_CW=3, M_SPINDLE_CCW=4,
				 M_SPINDLE_STOP=5, M_TOOL_CHANGE=6 };
enum GCOD_STATE { csNormal=0, csSelected=1, csDragged=2, csLocked=4 };


//---------------------------------------------------------------------------
 class TGCod
{
  public:
	// GCod data
	int   G;
	float X;
	float Y;
	float R;
	int   F;
	int   M;
	int   T;

	// other
	float OX;
	float OY;
	int Return;
	int State;
	TGCod *Prev;
	TGCod *Next;
	int ID;

	__fastcall TGCod();
	__fastcall TGCod(const TGCod &c);
	__fastcall TGCod(int g, float x, float y, float r);
    __fastcall ~TGCod();
	void __fastcall Set(int g, float x, float y, float r);
	void __fastcall SetG(int g);
	void __fastcall SetR(float r);
	void __fastcall SetX(float x);
	void __fastcall SetY(float y);
	void __fastcall SetF(int f);
	void __fastcall SetM(int m);
	void __fastcall SetT(int t);
	void __fastcall UpdateOXY();
};
//---------------------------------------------------------------------------
#endif
