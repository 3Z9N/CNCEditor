//---------------------------------------------------------------------------

#ifndef TCodeEditorH
#define TCodeEditorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <ValEdit.hpp>
#include <Buttons.hpp>

class TProgram;

//---------------------------------------------------------------------------
class TCodeEditor : public TFrame
{
__published:	// IDE-managed Components
	TPanel *Panel1;
	TMemo *Memo;
	TPanel *Panel2;
	TCheckBox *Check20GM;
	TSpeedButton *SpeedButton1;
	void __fastcall Check20GMClick(TObject *Sender);
private:	// User declarations
	TProgram *CurrProg;
public:		// User declarations
	__fastcall TCodeEditor(TComponent* Owner);
	void __fastcall LoadProgram(TProgram *Prog);
	void __fastcall Generate20GMCode(TProgram *Prog);
};
//---------------------------------------------------------------------------
extern PACKAGE TCodeEditor *CodeEditor;
//---------------------------------------------------------------------------
#endif
