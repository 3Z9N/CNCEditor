//---------------------------------------------------------------------------

#ifndef TMitDesignerH
#define TMitDesignerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <gl\gl.h>
#include <gl\glu.h>
class TGLFont2D;
//---------------------------------------------------------------------------
class TMitDesigner : public TFrame
{
__published:	// IDE-managed Components
	TPanel *PanelX;
	TPanel *Panel2;
	TLabel *LabelX;
	TLabel *LabelY;
	TLabel *LabelS;
	TPanel *Panel4;
	TScrollBar *ScrollX;
	TPanel *PanelY;
	TScrollBar *ScrollY;
	TPanel *GLPanel;
private:	// User declarations
	HDC hdc;
	HGLRC hrc;
	TGLFont2D *Font2D;
public:		// User declarations
	__fastcall TMitDesigner(TComponent* Owner);
	void __fastcall DrawText(AnsiString Text);
	void __fastcall DrawTextXY(double x, double y, AnsiString Text);
};
//---------------------------------------------------------------------------
extern PACKAGE TMitDesigner *MitDesigner;
//---------------------------------------------------------------------------
#endif
