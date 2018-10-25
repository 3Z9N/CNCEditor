//---------------------------------------------------------------------------

#ifndef TProjectH
#define TProjectH
//---------------------------------------------------------------------------
#include <vcl.h>


// typy maszyn
enum TMachineType{
  MAMMUT_20GM         = 0,
  MAMMUT_NextMove     = 1,
  MAMMUT_VMK_NextMove = 2
};

class TProgram;
//---------------------------------------------------------------------------
class TProject
{
public:
	TProject(TTreeView *view);
	~TProject();

	TTreeView *TreeView;
	TTreeNode *Root;
	AnsiString FileName;
	int Machine;
	bool Modified;
	TStringList *DeleteList;

	TTreeNode* __fastcall Add(TProgram* prog);
	void __fastcall Delete(TProgram* prog);
	void __fastcall Load();
	void __fastcall LoadFromFile(AnsiString fname);
	void __fastcall ImportMEF();
	void __fastcall Save();
	void __fastcall SaveToFile(AnsiString fname);
	const AnsiString __fastcall Name();

	void __fastcall SaveToFiles();
};
//---------------------------------------------------------------------------
#endif
