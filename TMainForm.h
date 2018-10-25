//---------------------------------------------------------------------------

#ifndef TMainFormH
#define TMainFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Menus.hpp>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <ToolWin.hpp>
#include <ImgList.hpp>
#include <ActnList.hpp>
#include <OleCtrls.hpp>
#include "TCNCDesigner.h"
#include "cspin.h"
#include <Grids.hpp>
#include <ValEdit.hpp>

#include "TProject.h"
#include "TProgram.h"
#include <DockTabSet.hpp>
#include <Tabs.hpp>
#include "TCodeEditor.h"

//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *MainMenu;
	TMenuItem *Plik1;
	TMenuItem *Nowy1;
	TMenuItem *Otwrz1;
	TMenuItem *Zapisz1;
	TMenuItem *Zapiszjako1;
	TMenuItem *N1;
	TMenuItem *Drukuj1;
	TMenuItem *N2;
	TMenuItem *Zakocz1;
	TMenuItem *Edycja1;
	TMenuItem *Wytnij1;
	TMenuItem *Kopiuj1;
	TMenuItem *Wklej1;
	TMenuItem *Usu1;
	TMenuItem *N3;
	TMenuItem *Cofnij1;
	TMenuItem *N4;
	TMenuItem *Zaznaczwszystko1;
	TMenuItem *Widok1;
	TMenuItem *Siatka1;
	TMenuItem *Linijka1;
	TMenuItem *Numerypunktw1;
	TMenuItem *N5;
	TMenuItem *Powiksz1;
	TMenuItem *Pomniejsz1;
	TMenuItem *Dopasuj1;
	TMenuItem *Konfiguracja1;
	TMenuItem *Zmie1;
	TMenuItem *Pomoc1;
	TMenuItem *Oprogramie1;
	TOpenDialog *OpenDialog;
	TSaveDialog *SaveDialog;
	TPrintDialog *PrintDialog;
	TPanel *Panel1;
	TSplitter *Splitter1;
	TControlBar *ControlBar1;
	TToolBar *ToolBar1;
	TToolBar *ToolBar2;
	TToolButton *ToolButton1;
	TToolButton *ToolButton2;
	TToolButton *ToolButton3;
	TToolButton *ToolButton5;
	TImageList *ImageList1;
	TToolButton *ToolButton6;
	TToolButton *ToolButton7;
	TToolButton *ToolButton8;
	TToolButton *ToolButton9;
	TActionList *ActionList;
	TAction *FileNew;
	TAction *FileOpen;
	TAction *EditCut;
	TAction *EditCopy;
	TAction *EditPaste;
	TAction *EditDelete;
	TAction *FileSave;
	TAction *FilePrint;
	TAction *FileSaveAs;
	TAction *FileExit;
	TAction *EditUndo;
	TAction *EditSelectAll;
	TToolButton *ToolButton10;
	TToolBar *ToolBar3;
	TAction *ViewZoomIn;
	TAction *ViewZoomOut;
	TAction *ViewZoomAll;
	TAction *ViewGrid;
	TAction *ViewRuler;
	TAction *ViewNumbers;
	TToolButton *ToolButton11;
	TToolButton *ToolButton12;
	TToolButton *ToolButton13;
	TToolButton *ToolButton14;
	TToolButton *ToolButton15;
	TToolButton *ToolButton16;
	TPanel *Panel2;
	TSplitter *Splitter2;
	TPanel *Panel3;
	TValueListEditor *ObjectEditor;
	TPanel *ObjectPanel;
	TPanel *Panel5;
	TTreeView *ProjectTree;
	TToolBar *ToolBar4;
	TToolButton *ToolButton4;
	TToolButton *ToolButton17;
	TToolButton *ToolButton18;
	TToolButton *ToolButton19;
	TAction *ObjectDRV;
	TAction *ObjectLIN;
	TAction *ObjectCW;
	TAction *ObjectCCW;
	TMenuItem *Obiekt1;
	TMenuItem *DRV1;
	TMenuItem *LIN1;
	TMenuItem *CW1;
	TMenuItem *CCW1;
	TImageList *ProjectImageList;
	TCNCDesigner *CNCDesigner;
	TPanel *Panel4;
	TPanel *InfoPanel;
	TDockTabSet *DockTabSet;
	TCodeEditor *CodeEditor;
	TAction *FileProjectSaveAs;
	TMenuItem *Zapiszprojektjako1;
	TPopupMenu *ProjectPopupMenu;
	TAction *ProjectNewProgram;
	TAction *ProjectDeleteProgram;
	TMenuItem *Nowyprogram1;
	TMenuItem *Usuprogram1;
	TPopupMenu *RecentPopupMenu;
	TAction *EditMovePoint;
	TMenuItem *N6;
	TMenuItem *Przesupunkt1;
	TAction *EditRotate90;
	TAction *EditRotate180;
	TAction *EditRotate270;
	TMenuItem *Obr1;
	TMenuItem *Obr90st1;
	TMenuItem *Obr180st1;
	TMenuItem *Obr270st1;
	TAction *EditMirrorH;
	TAction *EditMirrorV;
	TMenuItem *Odbij1;
	TMenuItem *Odbijwpionie1;
	TMenuItem *Odbijwpoziomie1;
	TMenuItem *N7;
	TMenuItem *Kopiuj2;
	TMenuItem *Wklej2;
	TAction *ViewAntialiasing;
	TMenuItem *N8;
	TMenuItem *Antyaliasing1;
	TAction *FileExport;
	TMenuItem *Eksportuj1;
	TMenuItem *Inny1;
	TAction *EditRotateAny;
	TToolButton *ToolButton20;
	TAction *ViewDrvPath;
	TMenuItem *LinieDRV1;
	TAction *EditReverseStitch;
	TMenuItem *N9;
	TMenuItem *Rescale;
	TAction *EditMoveToZero;
	TMenuItem *Przesudopunktuzero1;
	TToolButton *ToolButton21;
	TAction *EditRescale;
	TToolButton *ToolButton22;
	void __fastcall FileNewExecute(TObject *Sender);
	void __fastcall FileOpenExecute(TObject *Sender);
	void __fastcall FileSaveExecute(TObject *Sender);
	void __fastcall FilePrintExecute(TObject *Sender);
	void __fastcall EditCutExecute(TObject *Sender);
	void __fastcall EditCopyExecute(TObject *Sender);
	void __fastcall EditPasteExecute(TObject *Sender);
	void __fastcall EditDeleteExecute(TObject *Sender);
	void __fastcall FileSaveAsExecute(TObject *Sender);
	void __fastcall FileExitExecute(TObject *Sender);
	void __fastcall EditSelectAllExecute(TObject *Sender);
	void __fastcall EditUndoExecute(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender);
	void __fastcall ViewZoomInExecute(TObject *Sender);
	void __fastcall ViewZoomOutExecute(TObject *Sender);
	void __fastcall ViewZoomAllExecute(TObject *Sender);
	void __fastcall ViewGridExecute(TObject *Sender);
	void __fastcall ViewRulerExecute(TObject *Sender);
	void __fastcall ViewNumbersExecute(TObject *Sender);
	void __fastcall ObjectDRVExecute(TObject *Sender);
	void __fastcall ObjectLINExecute(TObject *Sender);
	void __fastcall ObjectCWExecute(TObject *Sender);
	void __fastcall ObjectCCWExecute(TObject *Sender);
	void __fastcall ProjectTreeChange(TObject *Sender, TTreeNode *Node);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall DockTabSetChange(TObject *Sender, int NewTab,
          bool &AllowChange);
	void __fastcall Splitter1Moved(TObject *Sender);
	void __fastcall FormCloseQuery(TObject *Sender, bool &CanClose);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall Zmie1Click(TObject *Sender);
	void __fastcall ProjectNewProgramExecute(TObject *Sender);
	void __fastcall ProjectDeleteProgramExecute(TObject *Sender);
	void __fastcall ProjectPopupMenuPopup(TObject *Sender);
	void __fastcall ProjectTreeEdited(TObject *Sender, TTreeNode *Node,
          AnsiString &S);
	void __fastcall ProjectTreeEditing(TObject *Sender, TTreeNode *Node,
          bool &AllowEdit);
	void __fastcall ObjectEditorValidate(TObject *Sender, int ACol, int ARow,
          const AnsiString KeyName, const AnsiString KeyValue);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall EditMovePointExecute(TObject *Sender);
	void __fastcall EditRotate90Execute(TObject *Sender);
	void __fastcall EditRotate180Execute(TObject *Sender);
	void __fastcall EditRotate270Execute(TObject *Sender);
	void __fastcall EditMirrorVExecute(TObject *Sender);
	void __fastcall EditMirrorHExecute(TObject *Sender);
	void __fastcall ViewAntialiasingExecute(TObject *Sender);
	void __fastcall FileExportExecute(TObject *Sender);
	void __fastcall EditRotateAnyExecute(TObject *Sender);
	void __fastcall ViewDrvPathExecute(TObject *Sender);
	void __fastcall EditMoveToZeroExecute(TObject *Sender);
	void __fastcall EditRescaleExecute(TObject *Sender);
private:	// User declarations
	TProject *Project;
	TProgram *CurrProg;
	friend class TPrintForm;
public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);
	void __fastcall CNCDesignerDesignAction(int dAction);
	void __fastcall CNCDesignerModify(bool modified);
	void __fastcall CNCDesignerSetValues(TObject *Sender);
	void __fastcall LoadConfig();
	void __fastcall RecentItemClick(TObject *Sender);
	void __fastcall SaveConfig();
	void __fastcall SetDesigner(TProgram *Prog);
	void __fastcall UpdateCaption();
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
