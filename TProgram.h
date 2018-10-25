//---------------------------------------------------------------------------

#ifndef TProgramH
#define TProgramH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <deque>

//---------------------------------------------------------------------------
class TProgram
{
  public:
	TProgram();
	TProgram(AnsiString nazwa);
	~TProgram();

	AnsiString Name;
	int Number;
	int Frame;
    TList *Codes;

	bool Loaded;
	bool Modified;
	bool New;
	std::deque<AnsiString> undo_deque;
	static unsigned int UndoLimit;

	void __fastcall ClearUndo();
	const AnsiString __fastcall CodeTable();
	const AnsiString __fastcall GCode();
	void __fastcall LoadCodeTable(AnsiString code);
	void __fastcall LoadGCode(const char *code);
	void __fastcall LoadUndo();
	void __fastcall SaveUndo();
	int __fastcall UndoCount();
};
//---------------------------------------------------------------------------
#endif
