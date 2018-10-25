//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TMainForm.h"
#include "TGCod.h"
#include "TProgram.h"
#include "TOptionsForm.h"
#include "TPrintForm.h"

#include <math.hpp>
#include <stdio.h>
#include <math.h>
#include <vector.h>
#include <fstream.h>
#include <string>
#include <strstrea.h>
#include <Clipbrd.hpp>
#include <Registry.hpp>
#include <XMLDoc.hpp>

int new_suffix = 1;
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "TCNCDesigner"
#pragma link "cspin"
#pragma link "TCodeEditor"
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileNewExecute(TObject *Sender)
{
  SaveDialog->Filter = "CNC Edytor Projekt (*.cne)|*.cne";
  SaveDialog->DefaultExt = ".cne";
  SaveDialog->Title = "CNC Edytor - Nowy projekt";
  if(!SaveDialog->Execute()) return;
  TProject *proj = new TProject(ProjectTree);
  proj->SaveToFile(SaveDialog->FileName);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileOpenExecute(TObject *Sender)
{
  OpenDialog->Filter = "CNC Editor Projekt (*.cne)|*.cne|MiteGL Projekt (*.mef)|*.mef";
  if(!OpenDialog->Execute()) return;
  SetDesigner(0);
  ProjectTree->Visible = false;
  Screen->Cursor = crHourGlass;
  if(Project) {
	delete Project->Root;
  }
  Project = new TProject(ProjectTree);
  Project->LoadFromFile(OpenDialog->FileName);
  FileSave->Enabled = false;
  Screen->Cursor = crDefault;
  ProjectTree->Visible = true;

  // update RecentPopupMenu->Items
  for(int i=0; i<RecentPopupMenu->Items->Count; i++) {
	TMenuItem *item = RecentPopupMenu->Items->Items[i];
	if(item->Caption == Project->FileName) {
	  RecentPopupMenu->Items->Delete(i);
	  break;
    }
  }
  TMenuItem *item = new TMenuItem(RecentPopupMenu);
  item->AutoHotkeys = maManual;
  item->AutoLineReduction = maManual;
  item->Caption = Project->FileName;
  item->OnClick = RecentItemClick;
  RecentPopupMenu->Items->Insert(0, item);

}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileSaveExecute(TObject *Sender)
{
  TTreeNode *Node = ProjectTree->Selected;
  if(!Node->Data) return;

  Screen->Cursor = crHourGlass;
  if(Project->FileName == "nowy")
	FileSaveAsExecute(this);
  else
	Project->Save();
  FileSave->Enabled = false;
  Screen->Cursor = crDefault;
  CNCDesigner->SetModified(false);
  //-----
  //Project->SaveToFiles();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileSaveAsExecute(TObject *Sender)
{
  TTreeNode *Node = ProjectTree->Selected;
  TProgram *Prog;
  TProject *Proj;
  if(!Node) return;
  if(Node->Level > 0) Proj = (TProject*)Node->Parent->Data;
  else Proj = (TProject*)Node->Data;
  SaveDialog->Filter = "CNC Edytor Projekt (*.cne)|*.cne";
  SaveDialog->DefaultExt = ".cne";
  SaveDialog->Title = "CNC Edytor - Zapisz projekt jako...";

  //---
	if(Proj->Root->Count > 0) {
		Prog = (TProgram*)Proj->Root->Item[0]->Data;
		SaveDialog->FileName = String(Prog->Number) + "-" + Prog->Name + ".cne";
	}
	else {
		SaveDialog->FileName = String(Proj->Name()) + ".cne";
	}
  //---

  if(!SaveDialog->Execute()) return;
  Proj->SaveToFile(SaveDialog->FileName);
  FileSave->Enabled = false;
  CNCDesigner->SetModified(false);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FilePrintExecute(TObject *Sender)
{
	//CNCDesigner->SetReadOnly(true);
	PrintForm->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileExitExecute(TObject *Sender)
{
	Close();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditUndoExecute(TObject *Sender)
{
  if(!CNCDesigner->Program) return;
  CNCDesigner->Program->LoadUndo();
  CNCDesigner->Render();
  //EditUndo->Enabled = 
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditCutExecute(TObject *Sender)
{
  CNCDesigner->Copy();
  CNCDesigner->Delete();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditCopyExecute(TObject *Sender)
{
  if(ProjectTree->Focused()) {
	if(!ProjectTree->Selected) return;
	TProgram *Prog = (TProgram*)ProjectTree->Selected->Data;
	CNCDesigner->CopyProgramToClipboard(Prog);
  }
  else {
	CNCDesigner->Copy();
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditPasteExecute(TObject *Sender)
{
  if(ProjectTree->Focused()) {
	TProgram *Prog = CNCDesigner->GetProgramFromClipboard();
	if(!Prog) return;

	for (int i=0; i<Project->Root->Count; i++) {
	  if(Prog->Name.AnsiCompareIC(Project->Root->Item[i]->Text) == 0) {
		Prog->Name += " (nowy)";
		//MessageBox(0, "Nazwa programu musi byæ ró¿na od pozosta³ych programów!",
		//		   "Zmiana nazwy programu", MB_OK | MB_ICONWARNING);
		break;
	  }
	}
	Project->Add(Prog);
	Project->Modified = true;
	FileSave->Enabled = true;
  }
  else {
	CNCDesigner->Paste();
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditDeleteExecute(TObject *Sender)
{
  CNCDesigner->Delete();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditSelectAllExecute(TObject *Sender)
{
  CNCDesigner->SelectAll();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditMovePointExecute(TObject *Sender)
{
  CNCDesigner->MovePoint();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditRotate90Execute(TObject *Sender)
{
  CNCDesigner->Rotate(RA_90);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditRotate180Execute(TObject *Sender)
{
  CNCDesigner->Rotate(RA_180);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditRotate270Execute(TObject *Sender)
{
  CNCDesigner->Rotate(RA_270);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditMirrorVExecute(TObject *Sender)
{
  CNCDesigner->Mirror(MD_VERTICAL);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditMirrorHExecute(TObject *Sender)
{
  CNCDesigner->Mirror(MD_HORIZONTAL);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormPaint(TObject *Sender)
{
  //CNCDesigner->Render();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewZoomInExecute(TObject *Sender)
{
  CNCDesigner->ZoomIn();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewZoomOutExecute(TObject *Sender)
{
  CNCDesigner->ZoomOut();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewZoomAllExecute(TObject *Sender)
{
  CNCDesigner->ZoomAll();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewGridExecute(TObject *Sender)
{
  CNCDesigner->ViewGrid(ViewGrid->Checked);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewRulerExecute(TObject *Sender)
{
  CNCDesigner->ViewRuler(ViewRuler->Checked);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ViewNumbersExecute(TObject *Sender)
{
  CNCDesigner->ViewNumbers(ViewNumbers->Checked);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewAntialiasingExecute(TObject *Sender)
{
  CNCDesigner->SetAntialiasing(ViewAntialiasing->Checked);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ObjectDRVExecute(TObject *Sender)
{
  CNCDesigner->SetDesignAction(DA_DRV);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ObjectLINExecute(TObject *Sender)
{
  CNCDesigner->SetDesignAction(DA_LIN);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ObjectCWExecute(TObject *Sender)
{
  CNCDesigner->SetDesignAction(DA_CW);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ObjectCCWExecute(TObject *Sender)
{
  CNCDesigner->SetDesignAction(DA_CCW);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProjectTreeChange(TObject *Sender, TTreeNode *Node)
{
  if(!Node) return;
  if(Node->Level == 0) {
	SetDesigner(0);
	return;
  }
  TProgram* Prog = (TProgram*)Node->Data;
  if(Prog) {
	//if(!Prog->Loaded) Prog->Load();
	CurrProg = Prog;
	SetDesigner(CurrProg);
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CNCDesignerDesignAction(int dAction)
{
  if(dAction == DA_NONE) {
	ObjectDRV->Checked = false;
	ObjectLIN->Checked = false;
	ObjectCW->Checked = false;
	ObjectCCW->Checked = false;
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CNCDesignerModify(bool modified)
{
  FileSave->Enabled = modified;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::CNCDesignerSetValues(TObject *Sender)
{
  TProgram *prog = CNCDesigner->Program;
  if(!prog) return;
  TGCod *cod = CNCDesigner->CurrCod;
  ObjectEditor->Strings->Clear();
  int id;

  ObjectEditor->Enabled = false;
  if(cod) { // set Cod values
	ObjectPanel->Caption = "G-Cod";
	// ---------- N ----------
	ObjectEditor->Strings->Add("Nr=" + IntToStr(prog->Codes->IndexOf(cod)));
	ObjectEditor->Refresh();
	ObjectEditor->ItemProps[ObjectEditor->Strings->IndexOfName("Nr")]->ReadOnly = true;
	// ---------- G ----------
	ObjectEditor->Strings->Add("G=");
	ObjectEditor->Refresh();
	id = ObjectEditor->Strings->IndexOfName("G");
	ObjectEditor->ItemProps[id]->EditStyle = esPickList;
	ObjectEditor->ItemProps[id]->PickList->Add("00 - DRV");
	ObjectEditor->ItemProps[id]->PickList->Add("01 - LIN");
	ObjectEditor->ItemProps[id]->PickList->Add("02 - CW");
	ObjectEditor->ItemProps[id]->PickList->Add("03 - CCW");
	ObjectEditor->Values["G"] = ObjectEditor->ItemProps[id]->PickList->Strings[cod->G];
	ObjectEditor->ItemProps[id]->ReadOnly = true;
	// ---------- X ----------
	ObjectEditor->Strings->Add("X=" + FloatToStr(cod->X));
	// ---------- Y ----------
	ObjectEditor->Strings->Add("Y=" + FloatToStr(cod->Y));
	//ObjectEditor->Refresh();
	// ---------- R, OX, OY ----------
	if(cod->G==G_CW || cod->G==G_CCW) {
	  ObjectEditor->Strings->Add("R=" + FloatToStr(cod->R));
	  ObjectEditor->Strings->Add("OX=" + FloatToStr(cod->OX));
	  ObjectEditor->Strings->Add("OY=" + FloatToStr(cod->OY));
	  ObjectEditor->Refresh();
	  ObjectEditor->ItemProps[ObjectEditor->Strings->IndexOfName("OX")]->ReadOnly = true;
	  ObjectEditor->ItemProps[ObjectEditor->Strings->IndexOfName("OY")]->ReadOnly = true;
	}
	// ---------- F ----------
	if(cod->F != M_NONE) ObjectEditor->Strings->Add("F=" + IntToStr(cod->F));
	else ObjectEditor->Strings->Add("F=");
	// ---------- M ----------
	ObjectEditor->Strings->Add("M=");
	ObjectEditor->Refresh();
	id = ObjectEditor->Strings->IndexOfName("M");
	ObjectEditor->ItemProps[id]->EditStyle = esPickList;
	ObjectEditor->ItemProps[id]->PickList->Add("00 - Program stop");
	ObjectEditor->ItemProps[id]->PickList->Add("01 - Optional stop");
	ObjectEditor->ItemProps[id]->PickList->Add("02 - End of program");
	ObjectEditor->ItemProps[id]->PickList->Add("03 - Spindle CW");
	ObjectEditor->ItemProps[id]->PickList->Add("04 - Spindle CCW");
	ObjectEditor->ItemProps[id]->PickList->Add("05 - Spindle OFF");
	ObjectEditor->ItemProps[id]->PickList->Add("06 - Tool change");
	ObjectEditor->ItemProps[id]->PickList->Add("07 - Coolant 1 ON");
	ObjectEditor->ItemProps[id]->PickList->Add("08 - Coolant 2 ON");
	ObjectEditor->ItemProps[id]->PickList->Add("09 - Coolant OFF");
	ObjectEditor->ItemProps[id]->PickList->Add("10 - Clamp");
	ObjectEditor->ItemProps[id]->PickList->Add("11 - Unclamp");
	ObjectEditor->ItemProps[id]->PickList->Add("30 - End of data");
	ObjectEditor->ItemProps[id]->PickList->Add("47 - Return to start");
	ObjectEditor->ItemProps[id]->PickList->Add("none");
	if(cod->M == M_NONE)
      ObjectEditor->Values["M"] = "none";
	else
	  ObjectEditor->Values["M"] = ObjectEditor->ItemProps[id]->PickList->Strings[cod->M];
    ObjectEditor->ItemProps[id]->ReadOnly = true;
	// ---------- T ----------
	if(cod->T != M_NONE) ObjectEditor->Strings->Add("T=" + IntToStr(cod->T));
	else ObjectEditor->Strings->Add("T=");
  }
  else {  // set Program values
	ObjectPanel->Caption = "Program";
	ObjectEditor->Strings->Add("Nazwa=" + prog->Name);
	ObjectEditor->Strings->Add("Nr=" + IntToStr(prog->Number));
	ObjectEditor->Strings->Add("Rama=" + IntToStr(prog->Frame));
	ObjectEditor->Strings->Add("Wymiary= ");
	//ObjectEditor->ItemProps[ObjectEditor->Strings->IndexOfName("Nazwa")]->ReadOnly = true;
  }
  ObjectEditor->Refresh();
  ObjectEditor->Enabled = true;

  UpdateCaption();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormResize(TObject *Sender)
{
  //InfoPanel->Width = Width - DockTabSet->MinClientRect().Width() - 50;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::DockTabSetChange(TObject *Sender, int NewTab,
	  bool &AllowChange)
{
  if(NewTab == 1 && CNCDesigner->Program) {
	//LoadMemo();
	CodeEditor->LoadProgram(CNCDesigner->Program);
	CNCDesigner->Visible = false;
	CodeEditor->Visible = true;
	DockTabSet->DestinationDockSite = CodeEditor;
  }
  else {
	CodeEditor->Visible = false;
	CNCDesigner->Visible = true;
  }
  AllowChange = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Splitter1Moved(TObject *Sender)
{
  CNCDesigner->Render();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormCloseQuery(TObject *Sender, bool &CanClose)
{
  if(FileSave->Enabled) {
	  int res =
		MessageBox(0, "Projekt zosta³ zmodyfikowany!\nCzy chcesz zapisaæ zmiany?",
				   "CNC Edytor", MB_YESNOCANCEL | MB_ICONQUESTION);
	switch(res) {
	  case IDYES :
		FileSaveExecute(this);
		break;

	  case IDCANCEL :
		CanClose = false;
		break;

	  default :
		CanClose = true;
	}
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SetDesigner(TProgram *Prog)
{
  if(!Prog) {
	CNCDesigner->Visible = false;
	CodeEditor->Visible  = false;
	DockTabSet->Visible  = false;
	return;
  }

  CNCDesigner->SetProgram(Prog);
  DockTabSet->Visible = true;
  if(DockTabSet->TabIndex == 1) {
	CodeEditor->LoadProgram(Prog);
	CNCDesigner->Visible = false;
	CodeEditor->Visible = true;
  }
  else {
	CodeEditor->Visible = false;
	CNCDesigner->Visible = true;
	CNCDesigner->Render();
  }

  UpdateCaption();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::UpdateCaption()
{
  char buf[128];//XXX
  if(!CNCDesigner->Program) {
	 sprintf(buf, "CNC Edytor");
  }
  else {
	sprintf(buf, "%s        X = %d        Y = %d", CNCDesigner->Program->Name.c_str(),
			(int)(CNCDesigner->max_x-CNCDesigner->min_x),
			(int)(CNCDesigner->max_y-CNCDesigner->min_y));//XXX
  }
  this->Caption = buf;//XXX


  if(!CNCDesigner->Program) return;
  TGCod *cod = CNCDesigner->CurrCod;
  if(!cod) {
	//sprintf(buf, "Wymiar=%d x %d",
	//		(int)RoundTo(CNCDesigner->max_x, 1) / 10,
	//		(int)RoundTo(CNCDesigner->max_y, 1) / 10);//XXX
	sprintf(buf, "%dx%d",   
			(int)RoundTo(CNCDesigner->max_x-CNCDesigner->min_x, 1) / 10,
			(int)RoundTo(CNCDesigner->max_y-CNCDesigner->min_y, 1) / 10);//XXX
	//ObjectEditor->Strings->Add(buf);
	ObjectEditor->Values["Wymiary"] = buf;
	ObjectEditor->Refresh();
  }
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  SaveConfig();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::Zmie1Click(TObject *Sender)
{
  OpenDialog->Filter = "MiteGL Projekt (*.mef)|*.mef";
  if(!OpenDialog->Execute()) return;

  TProgram *Prog = 0;
  std::ifstream ifs(OpenDialog->FileName.c_str());
  char buf[1024];
  char str[1024];
  AnsiString MiteName;
  AnsiString MiteVersion;
  memset(buf, 0, sizeof(buf));
  memset(str, 0, sizeof(buf));
  ifs >> buf;
  ifs >> str;
  MiteName = AnsiString(buf).LowerCase();
  MiteVersion = AnsiString(str).LowerCase();
  if(MiteName != "mite" && MiteName != "mitegl") return;
  AnsiString name = ExtractFileName(OpenDialog->FileName);
  name = name.SetLength(name.Length()-4);

  //----------------------------------------------
  TList *ProgList = new TList;
  //----------------------------------------------

  while(ifs) {
	memset(buf, 0, sizeof(buf));
	memset(str, 0, sizeof(buf));
	ifs.getline(buf, sizeof(buf));
	AnsiString line = buf;
	if(line.IsEmpty()) continue;
	line.SetLength(7);
	if(line == "Program") {
	  if(Prog) ProgList->Add(Prog); // poprzedni dodaj do projektu
	  int numer = 0, rama = 0, uzywany = 0;
	  if(MiteName == "mitegl")
		sscanf(buf, "Program|%d|%d|%d|%s", &numer, &rama, &uzywany, str);
	  else
		sscanf(buf, "Program|%d|%d|%s", &numer, &rama, str);
	  line = buf;
	  int pos = line.Pos(str);
	  Prog = new TProgram(buf+pos-1);
	  Prog->Number = numer;
	  Prog->Frame = rama;
	  continue;
	}
	if(!Prog) continue;
	int num[9];
	memset(num, 0, sizeof(num));
	sscanf(buf, "%d|%d|%d|%d|%d|%d|%d|%d|%d", &num[0], &num[1], &num[2],
				   &num[3], &num[4], &num[5], &num[6], &num[7], &num[8]);
	//---
	  //if(num[0] == 4) num[0] = 0;
	  //else if(num[0] == 3) num[0] = 1;
	  //else if(num[0] == 2) num[0] = 2;
	  //else if(num[0] == 1) num[0] = 3;
	  if(MiteName == "mite")
		num[0] = abs(4 - num[0]);
	//---
	TGCod *cod = new TGCod;
	//memcpy(cod, num, sizeof(num));
	cod->G = num[0];
	cod->X = num[1];
	cod->Y = num[2];
	cod->R = num[3];
	cod->M = num[7] > 0 ? num[7] : -1;
	if(Prog->Codes->Count) {
	  cod->Prev = (TGCod*)Prog->Codes->Last();
	  cod->Prev->Next = cod;
	}
	cod->UpdateOXY();
	Prog->Codes->Add(cod);
  }
  if(Prog) ProgList->Add(Prog); // ostatni dodaj do projektu

  ifs.close();



  //========================================================
  //               Zapis do pliku ".xml"
  //========================================================
  SaveDialog->Filter = "CNC Edytor Projekt (*.cne)|*.cne";
  SaveDialog->DefaultExt = ".cne";
  if(!SaveDialog->Execute()) return;

  AnsiString path = ExtractFilePath(SaveDialog->FileName);
  name = ExtractFileName(SaveDialog->FileName);
  name = name.SetLength(name.Length()-4);
  AnsiString fname = path + name + ".cne";
  remove(fname.c_str());

  TXMLDocument *XMLDoc = new TXMLDocument(this);
  XMLDoc->Active = true;
  XMLDoc->Encoding = "UTF-8";
  XMLDoc->Options = TXMLDocOptions(doNodeAutoCreate + doAutoSave + doNodeAutoIndent);

  IXMLNode *root = XMLDoc->AddChild("project");

  for(int i=0; i<ProgList->Count; i++) {
	TProgram *Prog = (TProgram*)ProgList->Items[i];
	IXMLNode *node = root->AddChild("program");
	node->SetAttribute("name", Prog->Name);
	node->SetAttribute("number", Prog->Number);
	node->SetAttribute("frame", Prog->Frame);
	IXMLNode *code = node->AddChild("code");
	code->Text = Prog->CodeTable();
  }

  XMLDoc->SaveToFile(fname);//"e:\\test.xml");
  XMLDoc->Active   = false;
  delete XMLDoc;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProjectNewProgramExecute(TObject *Sender)
{
  TTreeNode *Node = ProjectTree->Selected;
  if (!Node || !Node->Data) return;

  if (Node->Level == 0) {  // project node
	Project = (TProject*)Node->Data;
  }

  TProgram *Prog = new TProgram("nowy" + IntToStr(new_suffix++));
  TTreeNode*node = Project->Add(Prog);
  ProjectTree->Selected = node;
  FileSave->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProjectDeleteProgramExecute(TObject *Sender)
{
  TTreeNode *Node = ProjectTree->Selected;
  if (!Node || !Node->Data) return;
  if (Node->Level == 0) {  // project node
	return;
  }
  else {  // program node
	TProgram *Prog = (TProgram*)Node->Data;
	Project->Delete(Prog);
  }
  FileSave->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProjectPopupMenuPopup(TObject *Sender)
{
  TTreeNode *Node = ProjectTree->Selected;
  if (!Node) {
	ProjectNewProgram->Visible = false;
	ProjectDeleteProgram->Visible = false;
	return;
  }
  if (Node->Level == 0) {  // project node
	ProjectNewProgram->Visible = true;
	ProjectDeleteProgram->Visible = false;
  }
  else {  // program node
	ProjectNewProgram->Visible = true;
	ProjectDeleteProgram->Visible = true;
  }	
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProjectTreeEditing(TObject *Sender, TTreeNode *Node,
      bool &AllowEdit)
{
  if(Node->Level==0) AllowEdit = false;
  else AllowEdit = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ProjectTreeEdited(TObject *Sender, TTreeNode *Node,
	  AnsiString &S)
{
  if(S.IsEmpty()) {
	S = Node->Text;
	return;
  }

  for (int i=0; i<Project->Root->Count; i++) {
	if (S.AnsiCompareIC(Project->Root->Item[i]->Text) == 0) {
	  S = Node->Text;
	  MessageBox(0, "Nazwa programu musi byæ ró¿na od pozosta³ych programów!",
				 "Zmiana nazwy programu", MB_OK | MB_ICONWARNING);
	  return;
	}
  }

  Node->Text = S;
  TProgram *Prog = (TProgram*)Node->Data;
  Prog->Name = S;
  Project->Root->AlphaSort(true);
  FileSave->Enabled = true;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ObjectEditorValidate(TObject *Sender, int ACol,
	  int ARow, const AnsiString KeyName, const AnsiString KeyValue)
{
  if(!CNCDesigner->Program) return;
  TGCod *cod = CNCDesigner->CurrCod;
  if(!cod) { // program values
	if(KeyName == "Nazwa") {
	  if(KeyValue.IsEmpty()) {
		ObjectEditor->Values["Nazwa"] = CNCDesigner->Program->Name;
		return;
	  }

	  for (int i=0; i<Project->Root->Count; i++) {
		if (KeyValue.AnsiCompareIC(Project->Root->Item[i]->Text) == 0) {
		  ObjectEditor->Values["Nazwa"] = CNCDesigner->Program->Name;
		  MessageBox(0, "Nazwa programu musi byæ ró¿na od pozosta³ych programów!",
					 "Zmiana nazwy programu", MB_OK | MB_ICONWARNING);
		  return;
		}
	  }

	  ProjectTree->Selected->Text = KeyValue;
	  TProgram *Prog = (TProgram*)ProjectTree->Selected->Data;
	  Prog->Name = KeyValue;
	  Project->Root->AlphaSort(true);
      FileSave->Enabled = true;
	}
	else if(KeyName == "Nr") CNCDesigner->Program->Number = StrToInt(KeyValue);
	else if(KeyName == "Rama") CNCDesigner->Program->Frame = StrToInt(KeyValue);

	CNCDesigner->SetModified(true);
	return;
  }
  AnsiString s = KeyValue.SubString(1, 2); // 2 pierwsze znaki

  CNCDesigner->Program->SaveUndo();
  switch(KeyName[1]) {
  case 'G' :
	cod->SetG(StrToInt(s));
	break;
  case 'X' :
	cod->SetX(StrToFloat(KeyValue));
	break;
  case 'Y' :
	cod->SetY(StrToFloat(KeyValue));
	break;
  case 'R' :
	//cod->SetR(StrToFloat(KeyValue));
	{
		TList *Codes = CNCDesigner->Program->Codes;
		for(int i=0; i<Codes->Count; i++) {
			TGCod *C = (TGCod*)Codes->Items[i];
			if(C->G > G_LIN && C->State == csSelected) {
				C->SetR(StrToFloat(KeyValue));
            }
		}
	}
	break;
  case 'F' :
//	if(KeyValue.IsEmpty()) cod->SetF(-1);
//	else cod->SetF(StrToInt(KeyValue));
	{
		TList *Codes = CNCDesigner->Program->Codes;
		for(int i=0; i<Codes->Count; i++) {
			TGCod *C = (TGCod*)Codes->Items[i];
			if(C->G > G_DRV && C->State == csSelected) {
			//if(C->State == csSelected) {
				if(KeyValue.IsEmpty()) C->SetF(-1);
				else C->SetF(StrToInt(KeyValue));
            }
		}
	}
	break;
  case 'M' :
	if(KeyValue == "none") cod->SetM(-1);
	else cod->SetM(StrToInt(s));
	break;
  case 'T' :
	if(KeyValue.IsEmpty()) cod->SetT(-1);
	else cod->SetT(StrToInt(KeyValue));
	break;
  }
  CNCDesigner->SetModified(true);
  CNCDesigner->Render();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FormShow(TObject *Sender)
{
  CNCDesigner->InitOpenGL();
  OnMouseWheel = CNCDesigner->OnMouseWheel;
  CNCDesigner->OnSetValues = CNCDesignerSetValues;
  CNCDesigner->OnDesignAction = CNCDesignerDesignAction;
  CNCDesigner->OnModify = CNCDesignerModify;

  SetDesigner(0);
  FileSave->Enabled = false;
  ViewGrid->Checked = true;
  ViewNumbers->Checked = true;
  ViewRuler->Checked = true;
  ViewDrvPath->Checked = true;
  TProgram::UndoLimit = 32;

  LoadConfig();


	//ShowMessage(System::ParamStr(0));
	if(System::ParamCount() > 0) {
		SetDesigner(0);
		Screen->Cursor = crHourGlass;
		if(Project) delete Project;
		Project = new TProject(ProjectTree);
		Project->LoadFromFile(System::ParamStr(1));
		FileSave->Enabled = false;
		Screen->Cursor = crDefault;

		// update RecentPopupMenu->Items
		for(int i=0; i<RecentPopupMenu->Items->Count; i++) {
			TMenuItem *item = RecentPopupMenu->Items->Items[i];
			if(item->Caption == Project->FileName) {
				RecentPopupMenu->Items->Delete(i);
				break;
			}
		}
		TMenuItem *item = new TMenuItem(RecentPopupMenu);
		item->AutoHotkeys = maManual;
		item->AutoLineReduction = maManual;
		item->Caption = Project->FileName;
		item->OnClick = RecentItemClick;
  		RecentPopupMenu->Items->Insert(0, item);
	}
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::LoadConfig()
{
  AnsiString fname = ExtractFilePath(Application->ExeName) + "config.xml";
  if(!FileExists(fname)) return;

  TXMLDocument *XMLDoc = new TXMLDocument(this);
  XMLDoc->FileName = fname;
  XMLDoc->Active = true;

  IXMLNode *xml_root = XMLDoc->DocumentElement;
  IXMLNodeList *xml_root_nodes = xml_root->ChildNodes;
  IXMLNode *xml_node = xml_root_nodes->FindNode("MainForm");
  if(xml_node) {
	int x = xml_node->GetAttribute("WindowState");
	WindowState = (TWindowState)x;
	if(WindowState != wsMaximized) {
	  Left = xml_node->GetAttribute("Left");
	  Top = xml_node->GetAttribute("Top");
	  Width = xml_node->GetAttribute("Width");
	  Height = xml_node->GetAttribute("Height");
    }
  }


  xml_node = xml_root_nodes->FindNode("Panel1");
  if(xml_node) {
	Panel1->Width = xml_node->GetAttribute("Width");
  }

  xml_node = xml_root_nodes->FindNode("Panel2");
  if(xml_node) {
	Panel2->Height = xml_node->GetAttribute("Height");
  }

  IXMLNode *xml_recent = xml_root_nodes->FindNode("RecentFiles");
  IXMLNodeList *xml_recent_files = xml_recent->ChildNodes;
  xml_node = xml_recent_files->First();
  int count = 0;
  while(xml_node) {
  	if(count > 15) break;
	TMenuItem *item = new TMenuItem(RecentPopupMenu);
	item->AutoHotkeys = maManual;
	item->AutoLineReduction = maManual;
    RecentPopupMenu->Items->Add(item);
	item->Caption = xml_node->Text;
	item->OnClick = RecentItemClick;
	//RecentPopupMenu->Items->Add(item);
	xml_node = xml_node->NextSibling();
	count++;
  }

  delete XMLDoc;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::SaveConfig()
{
  TXMLDocument *XMLDoc = new TXMLDocument(this);
  XMLDoc->Active = true;
  XMLDoc->Encoding = "UTF-8";
  XMLDoc->Options = TXMLDocOptions(doNodeAutoCreate + doAutoSave + doNodeAutoIndent);

  IXMLNode *root = XMLDoc->AddChild("CNCEditor");

  // save MainForm info
  IXMLNode *node = root->AddChild("MainForm");
  node->SetAttribute("WindowState", WindowState);
  node->SetAttribute("Left", Left);
  node->SetAttribute("Top", Top);
  node->SetAttribute("Width", Width);
  node->SetAttribute("Height", Height);

  // save Panel1 info
  node = root->AddChild("Panel1");
  node->SetAttribute("Width", Panel1->Width);

  // save Panel2 info
  node = root->AddChild("Panel2");
  node->SetAttribute("Height", Panel2->Height);

  // save recent files info
  node = root->AddChild("RecentFiles");
  for(int i=0; i<RecentPopupMenu->Items->Count; i++) {
	TMenuItem *item = RecentPopupMenu->Items->Items[i];
	IXMLNode *fnode = node->AddChild("File");
	fnode->Text = item->Caption;
  }

  AnsiString fname = ExtractFilePath(Application->ExeName) + "config.xml";
  XMLDoc->SaveToFile(fname);//"e:\\test.xml");
  XMLDoc->Active = false;
  delete XMLDoc;
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::RecentItemClick(TObject *Sender)
{
  TMenuItem *mitem = (TMenuItem*)Sender;
  AnsiString fname = mitem->Caption;

  Screen->Cursor = crHourGlass;
  SetDesigner(0);
  if(Project) delete Project;
  Project = new TProject(ProjectTree);
  Project->LoadFromFile(fname);
  FileSave->Enabled = false;
  Screen->Cursor = crDefault;

  // update RecentPopupMenu->Items
  for(int i=0; i<RecentPopupMenu->Items->Count; i++) {
	TMenuItem *item = RecentPopupMenu->Items->Items[i];
	if(item->Caption == Project->FileName) {
	  RecentPopupMenu->Items->Delete(i);
	  break;
    }
  }
  TMenuItem *item = new TMenuItem(RecentPopupMenu);
  item->AutoHotkeys = maManual;
  item->AutoLineReduction = maManual;
  item->Caption = Project->FileName;
  item->OnClick = RecentItemClick;
  RecentPopupMenu->Items->Insert(0, item);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::FileExportExecute(TObject *Sender)
{
//
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditRotateAnyExecute(TObject *Sender)
{
  CNCDesigner->RotateAngle();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::ViewDrvPathExecute(TObject *Sender)
{
	CNCDesigner->ViewDrvPath(ViewDrvPath->Checked);
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditMoveToZeroExecute(TObject *Sender)
{
	CNCDesigner->MoveToZero();
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::EditRescaleExecute(TObject *Sender)
{
	 CNCDesigner->Rescale();
}
//---------------------------------------------------------------------------

