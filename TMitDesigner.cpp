//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TMitDesigner.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMitDesigner *MitDesigner;

//---------------------------------------------------------------------------
class TGLFont2D
{
  public:
	TGLFont2D()    {}
	~TGLFont2D()   { glDeleteLists(ListBase,NumGylph); }
	int FirstGylph;
	int NumGylph;
	int ListBase;
};
//---------------------------------------------------------------------------
__fastcall TMitDesigner::TMitDesigner(TComponent* Owner)
	: TFrame(Owner)
{
}
//---------------------------------------------------------------------------
