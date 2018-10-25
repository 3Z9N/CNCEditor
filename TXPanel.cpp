//---------------------------------------------------------------------------

#include <vcl.h>

#pragma hdrstop

#include "TXPanel.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(TXPanel *)
{
	new TXPanel(NULL);
}
//---------------------------------------------------------------------------
__fastcall TXPanel::TXPanel(TComponent* Owner)
	: TPanel(Owner)
{

}
//---------------------------------------------------------------------------
namespace Txpanel
{
	void __fastcall PACKAGE Register()
	{
		TComponentClass classes[1] = {__classid(TXPanel)};
		RegisterComponents("KP_Components", classes, 0);
	}
}
//---------------------------------------------------------------------------
void __fastcall TXPanel::Paint()
{
  if(FOnPaint) FOnPaint(this);
}
//---------------------------------------------------------------------------
