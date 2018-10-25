//---------------------------------------------------------------------------


#pragma hdrstop

#include "TProject.h"
#include "TProgram.h"

#include <fstream>
#include <string>
#include <vcl.h>
#include <stdio.h>
#include <XMLDoc.hpp>
//---------------------------------------------------------------------------

#pragma package(smart_init)

//---------------------------------------------------------------------------
TProject::TProject(TTreeView *view)
{
  FileName = "nowy";
  Modified = false;
  TreeView = view;
  if(!TreeView) return;
  Root = TreeView->Items->Add(NULL, "nowy");
  Root->Data = this;
  Root->ImageIndex = 0;
  Root->SelectedIndex = 0;
  Machine = MAMMUT_20GM;
}
//---------------------------------------------------------------------------
TProject::~TProject()
{
  delete DeleteList;
  if(Root) delete Root;
//  for(int i=0; i<Root->Count; i++) {
//	TProgram *Prog = (TProgram*)Root->Item[i]->Data;
//	delete Prog;
//  }
//  if(Root) TreeView->Items->Delete(Root);
//  TreeView->Items->Clear();
}
//---------------------------------------------------------------------------
TTreeNode* __fastcall TProject::Add(TProgram* prog)
{
  TTreeNode *Node = TreeView->Items->AddChild(Root, prog->Name);
  Node->ImageIndex = 2;
  Node->SelectedIndex = 2;
  Node->Data = prog;
  Root->AlphaSort(true);
  return Node;
}
//---------------------------------------------------------------------------
void __fastcall TProject::Delete(TProgram* prog)
{
  for(int i=0; i<Root->Count; i++) {
	TProgram *Prog = (TProgram*)Root->Item[i]->Data;
	if(Prog == prog) {
	  delete prog;
	  delete Root->Item[i];
	  Root->AlphaSort(true);
	  return;
    }
  }
}
//---------------------------------------------------------------------------
const AnsiString __fastcall TProject::Name()
{
  AnsiString name = ExtractFileName(FileName);
  AnsiString ext = ExtractFileExt(FileName);
  int pos = name.Pos(ext);
  if(pos > 0) name = AnsiLeftStr(name, pos-1);
  return name;
}
//---------------------------------------------------------------------------
void __fastcall TProject::Load()
{
  if(FileName == "nowy" || !TreeView) return;

  TXMLDocument *XMLDoc = new TXMLDocument(Application->MainForm);
  XMLDoc->FileName = FileName;
  XMLDoc->Active = true;

  //TreeView->Enabled = false;
  TreeView->Hide();
  Root->DeleteChildren();
  Root->Text = Name();
  IXMLNode *xml_root = XMLDoc->DocumentElement;
  IXMLNodeList *xml_nodes = xml_root->ChildNodes;

  IXMLNode *xml_prog = xml_nodes->First();
  while(xml_prog) {
	TProgram *Prog = new TProgram(xml_prog->GetAttribute("name"));
	Prog->Number = xml_prog->GetAttribute("number");
	Prog->Frame = xml_prog->GetAttribute("frame");
	Prog->LoadCodeTable(xml_prog->ChildValues["code"]);
	Prog->New = false;
	Add(Prog);
	xml_prog = xml_prog->NextSibling();
  }

  Root->Expand(true);
  Root->AlphaSort(true);
  
  //---
  TTreeNode* Node = Root->getFirstChild();
  if(Node) {
	TProgram* Prog = (TProgram*)Node->Data;
	Node->Selected = true;
  }
  //---

  TreeView->Show();
  
  delete XMLDoc;
}
//---------------------------------------------------------------------------
void __fastcall TProject::LoadFromFile(AnsiString fname)
{
  FileName = fname;
  if(ExtractFileExt(fname) == ".mef") {
	  ImportMEF();
	  return;
  }
  Load();
}
//---------------------------------------------------------------------------
void __fastcall TProject::Save()
{
  TXMLDocument *XMLDoc = new TXMLDocument(Application);
  XMLDoc->Active = true;
  XMLDoc->Encoding = "UTF-8";
  XMLDoc->Options = TXMLDocOptions(doNodeAutoCreate + doAutoSave + doNodeAutoIndent);

  IXMLNode *root = XMLDoc->AddChild("project");

  for(int i=0; i<Root->Count; i++) {
	TProgram *Prog = (TProgram*)Root->Item[i]->Data;
	IXMLNode *node = root->AddChild("program");
	node->SetAttribute("name", Prog->Name);
	node->SetAttribute("number", Prog->Number);
	node->SetAttribute("frame", Prog->Frame);
	IXMLNode *code = node->AddChild("code");
	code->Text = Prog->CodeTable();
  }

  XMLDoc->SaveToFile(FileName);//"e:\\test.xml");
  XMLDoc->Active   = false;
  delete XMLDoc;
  Modified = false;
}
//---------------------------------------------------------------------------
 void __fastcall TProject::SaveToFile(AnsiString fname)
{
  FileName = fname;
  Save();
  Root->Text = Name();
}
//---------------------------------------------------------------------------
void __fastcall TProject::SaveToFiles()
{
  for(int i=0; i<Root->Count; i++) {
    TXMLDocument *XMLDoc = new TXMLDocument(Application);
	XMLDoc->Active = true;
	XMLDoc->Encoding = "UTF-8";
	XMLDoc->Options = TXMLDocOptions(doNodeAutoCreate + doAutoSave + doNodeAutoIndent);

	IXMLNode *root = XMLDoc->AddChild("project");
	TProgram *Prog = (TProgram*)Root->Item[i]->Data;
	IXMLNode *node = root->AddChild("program");
	node->SetAttribute("name", Prog->Name);
	node->SetAttribute("number", Prog->Number);
	node->SetAttribute("frame", Prog->Frame);
	IXMLNode *code = node->AddChild("code");
	code->Text = Prog->CodeTable();

	AnsiString fname = "Programy\\" + Prog->Name + ".cne";
	XMLDoc->SaveToFile(fname);//"e:\\test.xml");
	XMLDoc->Active   = false;
    delete XMLDoc;
  }
}
//---------------------------------------------------------------------------
 void __fastcall TProject::ImportMEF()
{
	if(FileName == "nowy" || !TreeView) return;

	TreeView->Enabled = false;
	Root->DeleteChildren();
	Root->Text = Name();

	/////////////////////////////////
	/*
	while(xml_prog) {
		TProgram *Prog = new TProgram(xml_prog->GetAttribute("name"));
		Prog->Number = xml_prog->GetAttribute("number");
		Prog->Frame = xml_prog->GetAttribute("frame");
		Prog->LoadCodeTable(xml_prog->ChildValues["code"]);
		Prog->New = false;
		Add(Prog);
		xml_prog = xml_prog->NextSibling();
	}
	*/



	TProgram *Prog = 0;
	std::ifstream ifs(FileName.c_str());
	char buf[1024];
	char str[1024];
	AnsiString MiteName;
	AnsiString MiteVersion;
	AnsiString Code;
	memset(buf, 0, sizeof(buf));
	memset(str, 0, sizeof(buf));
	//ifs.getline(buf, sizeof(buf));
	ifs >> buf;
	ifs >> str;
	MiteName = AnsiString(buf).LowerCase();
	MiteVersion = AnsiString(str).LowerCase();
	if(MiteName != "mite" && MiteName != "mitegl") return;

	while(ifs) {
		memset(buf, 0, sizeof(buf));
		memset(str, 0, sizeof(buf));
		ifs.getline(buf, sizeof(buf));
		AnsiString line = buf;
		if(line.IsEmpty()) continue;
		line.SetLength(7);
		if(line == "Program") {
			if(Prog) {
				Prog->LoadCodeTable(Code);
				Prog->New = false;
				Add(Prog); // poprzedni dodaj do projektu
			}
			int numer = 0, rama = 0, uzywany = 0;
			if(MiteName == "mitegl")
				sscanf(buf, "Program|%d|%d|%d|%s", &numer, &rama, &uzywany, str);
			else
				sscanf(buf, "Program|%d|%d|%s", &numer, &rama, str);
			line = buf;
			int pos = line.Pos(str);
			//Prog = new TProgram(buf+pos-1, numer, rama, uzywany);
			Prog = new TProgram(buf+pos-1);
			Prog->Number = numer;
			Prog->Frame = rama;
			Code = "";
			continue;
		}
		if(!Prog) continue;
		int num[9];
		memset(num, 0, sizeof(num));
		sscanf(buf, "%d|%d|%d|%d|%d|%d|%d|%d|%d", &num[0], &num[1], &num[2],
				&num[3], &num[4], &num[5], &num[6], &num[7], &num[8]);
		//---
		if(MiteName == "mite")
			num[0] = abs(4 - num[0]);
		//---
		Code += String(num[0]) + "|";  // G
		Code += String(num[1]) + "|";  // X
		Code += String(num[2]) + "|";  // Y
		Code += String(num[3]) + "|";  // R
		Code += String(num[6]) + "|";  // F
		Code += String(num[7]) + "|";  // M
		Code += String(num[8]) + ";";  // return
	}
	if(Prog) {  // ostatni dodaj do projektu
		Prog->LoadCodeTable(Code);
		Prog->New = false;
		Add(Prog);
	}
	/////////////////////////////////
  
	Root->Expand(true);
	Root->AlphaSort(true);
	TreeView->Enabled = true;
}
//---------------------------------------------------------------------------

