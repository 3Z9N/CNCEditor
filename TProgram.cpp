//---------------------------------------------------------------------------

#pragma hdrstop

#include "TProgram.h"
#include "TGCod.h"
#include "TProject.h"

//#include <vcl.h>
#include <strstream>
#include <stdio.h>
#include <string>
//---------------------------------------------------------------------------

#pragma package(smart_init)

unsigned int TProgram::UndoLimit = 10;
//---------------------------------------------------------------------------
TProgram::TProgram()
{
  Name = "nowy";
  Number = 0;
  Frame = 0;
  Codes = new TList;
  Loaded = false;
  Modified = false;
  New = true;
  undo_deque.clear();
}
//---------------------------------------------------------------------------
TProgram::TProgram(AnsiString name)
{
  Name = name;
  if(Name.IsEmpty()) Name = "nowy";
  Number = 0;
  Frame  = 0;
  Loaded = false;
  Modified = false;
  New = true;
  Codes = new TList;
  undo_deque.clear();
}
//---------------------------------------------------------------------------
TProgram::~TProgram()
{
  if(Codes) {
	for(int i=0; i<Codes->Count; i++) {
	  TGCod *cod = (TGCod*)Codes->Items[i];
	  delete cod;
	}
	Codes->Clear();
	delete Codes;
  }
}
//---------------------------------------------------------------------------
void __fastcall TProgram::ClearUndo()
{
  undo_deque.clear();
}
//---------------------------------------------------------------------------
const AnsiString __fastcall TProgram::CodeTable()
{
  AnsiString Code = "";
  for(int i=0; i<Codes->Count; i++) {
	TGCod *C = (TGCod*)Codes->Items[i];
	Code += String(C->G) + "|";
	Code += String(C->X) + "|";
	Code += String(C->Y) + "|";
	Code += String(C->R) + "|";
	Code += String(C->F) + "|";
	Code += String(C->M) + "|";
	Code += String(C->T) + ";";
  }
  if(Code.IsEmpty()) Code = "null";
  return Code;
}
//---------------------------------------------------------------------------
const AnsiString __fastcall TProgram::GCode()
{
  AnsiString Code = "";

  int _g = -1;
  int _x = -10000;
  int _y = -10000;

  Code += "%\n";
  Code += String(Number) + "\n";
  Code += "N0 T" + String(Frame) + "\n";

  for(int i=0; i<Codes->Count; i++) {
	TGCod *C = (TGCod*)Codes->Items[i];
	Code += "N" + String(i+1);
	if(C->G != _g) Code += " G0" + String(C->G);
	if(C->X != _x) Code += " X" + String(C->X);
	if(C->Y != _y) Code += " Y" + String(C->Y);
	if(C->G==G_CW || C->G==G_CCW) Code += " R" + String(C->R);
	if(C->G!=G_DRV && C->Prev && C->Prev->G==G_DRV) Code += " M03";
	//else if(C->M == 11) Line += " M05";
	else if(C->G>G_DRV && C->Next && C->Next->G==G_DRV) Code += " M05";
	Code += "\n";
	_g = C->G;
	_x = C->X;
	_y = C->Y;
  }

  Code += "N" + String(Codes->Count + 1) + " M02\n";
  return Code;
}
//---------------------------------------------------------------------------
void __fastcall TProgram::LoadCodeTable(AnsiString code)
{
  char buf[256];
  std::istrstream iss(code.c_str());
  if(!iss) return;
  Codes->Clear();
  
  while(!iss.eof())
  {
	ZeroMemory(buf, sizeof(buf));
	iss.getline(buf, sizeof(buf), ';');
	if(strlen(buf) < 13) continue;
	TGCod *cod = new TGCod;
	sscanf(buf, "%d|%f|%f|%f|%d|%d|%d",
		   &cod->G, &cod->X, &cod->Y, &cod->R, &cod->F, &cod->M, &cod->T);
    if(Codes->Count) {
	  TGCod *last = (TGCod*)Codes->Last();
	  last->Next = cod;
	  cod->Prev = last;
	}
	Codes->Add(cod);
	cod->UpdateOXY();
  }

  Loaded = true;
}
//---------------------------------------------------------------------------
void __fastcall TProgram::LoadGCode(const char *code)
{
  std::istrstream iss(code);
  if(!iss) return;

  std::string s;
  TGCod *cod = 0;
  int G=G_NONE, M=M_NONE, F=M_NONE, T=M_NONE;
  float X=0, Y=0, R=0;
  bool prog=false;

  while(!iss.eof())
  {
	s = "";
	iss >> s;
	if(s.empty()) continue;

	if(!prog) {
	  if(s == "%") {  // pocz¹tek programu
		prog = true;
		s = "";
		iss >> s;
		Number = StrToInt(s.c_str());
	  }
	  continue;
	}

	if(s[0] == 'N') { // nowy gcod
	  if(cod) {       // dodaj do listy ostatnio utworzony
		if(G == 1000 || G == G_NONE) {
		  if(M==6) Frame = T;
		  R=0, F=M_NONE, M=M_NONE, T=M_NONE;
		  continue;
		}
		cod->G = G;
		cod->X = X;
		cod->Y = Y;
		cod->R = R;
		cod->F = F;
		cod->M = M;
		cod->T = T;
		if(Codes->Count) {
		  TGCod *last = (TGCod*)Codes->Last();
		  last->Next = cod;
		  cod->Prev = last;
		}
		Codes->Add(cod);
		cod->UpdateOXY();
		if(M > 4) G = G_NONE;
	  }
	  R=0, F=M_NONE, M=M_NONE, T=M_NONE;
	  cod = new TGCod;
	  continue;
	}

	if(!cod) continue;

	switch(s[0]) {
	  case 'G' :
		G = StrToInt(String(s.c_str()+1));
		break;
	  case 'X' :
		X = StrToFloat(String(s.c_str()+1));
		break;
	  case 'Y' :
		Y = StrToFloat(String(s.c_str()+1));
		break;
	  case 'R' :
		R = StrToFloat(String(s.c_str()+1));
		break;
	  case 'F' :
		F = StrToInt(String(s.c_str()+1));
		break;
	  case 'M' :
		M = StrToInt(String(s.c_str()+1));
		break;
	  case 'T' :
		T = StrToInt(String(s.c_str()+1));
		break;
	}
  }
  Loaded = true;

  if(cod) {   // dodaj do listy ostatnio utworzony
	if(M == 2) {  // koniec programu
	  delete cod;
	  return;
	}
	cod->G = G;
	cod->X = X;
	cod->Y = Y;
	cod->R = R;
	cod->F = F;
	cod->M = M;
	cod->T = T;
	if(Codes->Count) {
	  TGCod *last = (TGCod*)Codes->Last();
	  last->Next = cod;
	  cod->Prev = last;
	}
	Codes->Add(cod);
	cod->UpdateOXY();
  }
}
//---------------------------------------------------------------------------
void __fastcall TProgram::LoadUndo()
{
  if(!undo_deque.size()) return;
  AnsiString s = undo_deque.front();
  LoadCodeTable(s.c_str());
  undo_deque.pop_front();
}
//---------------------------------------------------------------------------
void __fastcall TProgram::SaveUndo()
{
  undo_deque.push_front(CodeTable());
  if(undo_deque.size() > UndoLimit) undo_deque.pop_back();
}
//---------------------------------------------------------------------------
int __fastcall TProgram::UndoCount()
{
  return undo_deque.size();
}
//---------------------------------------------------------------------------

