//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TCodeEditor.h"
#include "TGCod.h"
#include "TProgram.h"

#include <strstrea.h>
#include <Clipbrd.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCodeEditor *CodeEditor;
//---------------------------------------------------------------------------
__fastcall TCodeEditor::TCodeEditor(TComponent* Owner)
	: TFrame(Owner)
{
	CurrProg = 0;
}
//---------------------------------------------------------------------------
void __fastcall TCodeEditor::LoadProgram(TProgram *Prog)
{
	CurrProg = Prog;
	Memo->Visible = false;
	Memo->Clear();
	if(!Prog) {
    	
		return;
	}

	if(Check20GM->Checked) {
		Generate20GMCode(Prog);
	}
	else {
		Panel2->Caption = "";
		AnsiString s = Prog->GCode();
		char buf[256];
		std::istrstream iss(s.c_str());
		if(!iss) return;

		while(!iss.eof()) {
			ZeroMemory(buf, sizeof(buf));
			iss.getline(buf, sizeof(buf));
			Memo->Lines->Append(buf);
		}
	}


	Memo->Visible = true;
}
//---------------------------------------------------------------------------
void __fastcall TCodeEditor::Generate20GMCode(TProgram *Prog)
{
	String line, buf;

	//==============================================================
	// Ustawienie znaczników m i F  ---  ManuUstawAutomClick(this);
	//==============================================================
	for(int i=0; i<Prog->Codes->Count; i++) {
		TGCod *cod = (TGCod*)Prog->Codes->Items[i];
		cod->F = 0;
		cod->M = 0;
		if(cod->G == G_DRV) continue;
		//Config->Values->FindRow("Szycie", cod->F);
		cod->F = 200;
		if(cod->Prev && cod->Prev->G == G_DRV) {
			//Config->Values->FindRow("Mocowanie", cod->F);
			cod->F = 202;
		}
		if(cod->Next && cod->Next->G == G_DRV) {
			//Config->Values->FindRow("Mocowanie", cod->F);
			cod->F = 202;
			cod->M = 11;
			continue;
		}
	}

	//==============================================================
	// start program
	//==============================================================
	int code_size = 3; // "Oxx"
	Memo->Lines->Add("DMOV(FNC12)  K" + IntToStr(Prog->Frame) + " DD100;");
	Memo->Lines->Add("CALL(FNC02) P200;");
	code_size += 13;

	//==============================================================
	//realizacja skoków
	//==============================================================
	int O = Prog->Number;
	int K = 1;
	int P = (O*10)+1;
	int x = -2000;
	int y = -2000;
	int f = 0;
	for(int i=0; i<Prog->Codes->Count; i++) {
		TGCod *cod = (TGCod*)Prog->Codes->Items[i];
		if(cod->G != G_DRV) continue;
		if(cod->G == G_DRV && (!cod->Prev || !cod->Next)) continue;
		if(cod->Prev->G == G_DRV || cod->Next->G == G_DRV) continue;
		if(cod->T < 1) continue;
		Memo->Lines->Add( "BRET(FNC05);" );
		Memo->Lines->Add( "DCMP(FNC10) DD102 K" + String(K) + " M102;" );
		Memo->Lines->Add( "LD M103;" );
		Memo->Lines->Add( "CJ(FNC00) P" + String(P) + ";" );
		Memo->Lines->Add( "BRET(FNC05);" );
		code_size += 21;
		P++;
		K++;
	}

	//==============================================================
	// w³asciwy kod programu
	//==============================================================
	P = (O*10)+1;
	for(int i=0; i<Prog->Codes->Count; i++) {
		line = "";
		TGCod *cod = (TGCod*)Prog->Codes->Items[i];

		switch(cod->G) {
			case G_DRV:
				buf = "cod00(DRV)";
				break;
			case G_LIN:
				buf = "cod01(LIN)";
				break;
			case G_CW:
				buf = "cod02(CW)";
				break;
			case G_CCW:
				buf = "cod03(CCW)";
				break;
			default:
				buf = "";
				break;
		}
		code_size += 2;
		if(!cod->M && (cod->Prev && (cod->G == cod->Prev->G))) {
			buf = "";
			code_size --;
		}

		//========================================================
		if(cod->G == G_DRV && cod->T > 0) {
			x = cod->X;
			y = cod->Y;
			buf += "  x" + IntToStr(x) + "  y" + IntToStr(y);
			code_size += 4;
		}
		else {
			if(cod->X != x) {
				x = cod->X;
				if(buf.Length()) buf += "  ";
				buf += "x" + IntToStr(x);
				code_size += 2;
			}
			if(cod->Y != y) {
				y = cod->Y;
				if(buf.Length()) buf += "  ";
				buf += "y" + IntToStr(y);
				code_size += 2;
			}
		}

		if((cod->G == G_CW || cod->G == G_CCW) && cod->R) {
			buf += "  r" + FloatToStr(cod->R);
			code_size += 2;
		}
		//========================================================


		if(i > 0 && cod->G != G_DRV && cod->Prev->G == G_DRV) {
			line = "CALL(FNC02) P201;";
			code_size += 4;
		}
		else if(i > 0 && cod->G == G_DRV && cod->Prev->G != G_DRV) {
			if(cod->T > 0 && cod->Next && cod->Next->G != G_DRV) {
				line = "CALL(FNC02) P202;";
				Memo->Lines->Add(line);
				line = "P" + String(P++) + ";";
				code_size += 6;
			}
			else {
				line = "CALL(FNC02) P203;";
				code_size += 4;
			}
		}
		if(!line.IsEmpty()) Memo->Lines->Add(line);


		line = buf;
		if(cod->F != f) {
			f = cod->F;
			if(f) {
				//line += "  f" + Config->Values->Cells[1][cod->F];
                line += "  fDD" + IntToStr(cod->F);
				code_size += 2;
			}
		}

		if(cod->M > 0) {
			line += "  m" + String(cod->M);
			code_size += 2;
		}
		line += ";";
		Memo->Lines->Add(line);
	}

	code_size += 2; // "m02(END)"

	Panel2->Caption = "Wielkosc kodu:  " + IntToStr(code_size) + "  kroków";
}
//---------------------------------------------------------------------------
void __fastcall TCodeEditor::Check20GMClick(TObject *Sender)
{
	LoadProgram(CurrProg);
}
//---------------------------------------------------------------------------

