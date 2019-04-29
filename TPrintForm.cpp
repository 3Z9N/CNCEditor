//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TPrintForm.h"
#include "TMainForm.h"
//#include "TCod.h"
#include <math.hpp>
#include <math.h>
#include <jpeg.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TPrintForm *PrintForm;
double maxx, maxy;
double offx, offy;
double skala;
int ilosc_igiel = 1;
int odleglosc_igiel = 300;
int rozstaw_rzedow = 225;
//---------------------------------------------------------------------------
/// Constructor
__fastcall TPrintForm::TPrintForm(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------
/// \brief Draw arc
/// \param Canvas - drawing VCL canvas
/// \param x1 - begin X coordinate
/// \param y1 - begin Y coordinate
/// \param x2 - end X coordinate
/// \param y2 - end Y coordinate
/// \param ox - center X arc coordinate
/// \param oy - center Y arc coordinate
/// \param r - radius
/// \param dir - direction (CW or CCW)
void __fastcall TPrintForm::DrawArc(TCanvas *Canvas, double x1, double y1, double x2, double y2,
                                    double ox, double oy, double r, int dir)
{
  double dx1 = x1 - ox;
  double dy1 = y1 - oy;
  double dx2 = x2 - ox;
  double dy2 = y2 - oy;
  double a1, a2, x, y;
  double r1 = fabs(r);

  if( dx1 == 0 )
	a1 = dy1 > 0 ? 90.0 : -90.0;
  else
	a1 = RadToDeg( ArcTan2(dy1, dx1) );

  if( dx2 == 0 )
    a2 = dy2 > 0 ? 90.0 : -90.0;
  else
    a2 = RadToDeg( ArcTan2(dy2, dx2) );

  if(dir == G_CW) {
    if(a1 < 0) a1 += 360.0;
    if(a2 < 0) a2 += 360.0;
    if(a1 < a2) a1 += 360.0;
  }
  else { // dir = G_CCW
    if(a1 < 0) a1 += 360.0;
    if(a2 < 0) a2 += 360.0;
    if(a1 > a2) a2 += 360.0;
  }

  int n = (fabs(a2-a1)/360 * 2 * M_PI * r1) / (4*skala);
  if(n == 0) n = 1;
  double step = fabs(a2-a1)/n;

  Canvas->MoveTo(x1, y1);

  if(dir == G_CW) {
    for(double angle=a1-step; angle>a2; angle-=step) {
      double rad = DegToRad(angle);
      x = r1 * cos(rad) + ox;
      y = r1 * sin(rad) + oy;
      Canvas->LineTo(x, y);
    }
  }
  else { // dir == G_CCW
    for(double angle=a1+step; angle<a2; angle+=step) {
      double rad = DegToRad(angle);
      x = r1 * cos(rad) + ox;
      y = r1 * sin(rad) + oy;
      Canvas->LineTo(x, y);
    }
  }

  Canvas->LineTo(x2, y2);
}
//---------------------------------------------------------------------------
/// ToolButton1 click evevt handler
void __fastcall TPrintForm::ToolButton1Click(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------
/// Form resize evevt handler
void __fastcall TPrintForm::FormResize(TObject *Sender)
{
  Panel2->Height = Panel1->Height - 20;
  Panel2->Top = 10;
  Panel2->Width = Panel2->Height / 1.41;
  Panel2->Left = (Panel1->Width - Panel2->Width) / 2;
}
//---------------------------------------------------------------------------
/// Export to JPG file
void __fastcall TPrintForm::ExportToJPEG()
{
  if(!MainForm->CurrProg) return;
  TList *Codes = MainForm->CurrProg->Codes;


  Graphics::TBitmap *Image = new Graphics::TBitmap;
  Image->Monochrome = true;
  double old_skala = skala;
  double margin = 50.0;

  String query = InputBox("CNCEditor", "Podaj skalê: 1/... ", "5");
  int tmp = StrToInt(query);

  maxx = -3000;
  maxy = -3000;

  for(int i=0; i<Codes->Count; i++) {
	TGCod *cod = (TGCod*)Codes->Items[i];
	if(cod->G == G_DRV) continue;
	if(maxx < cod->X) maxx = cod->X;
	if(maxy < cod->Y) maxy = cod->Y;
  }
  if(ilosc_igiel > 1) {
	  maxx = ilosc_igiel * odleglosc_igiel;
  }

  double skalax, skalay;
  skalax = ((maxx-margin)/(double)tmp) / maxx;
  skalay = ((maxy-margin)/(double)tmp) / maxy;

  if(skalay < skalax) skala = skalay;
  else skala = skalax;

  Image->Width = (maxx + margin) * skala;
  Image->Height = (maxy + margin) * skala;

  offx = (Image->Width - (maxx*skala)) / 2;
  if(ilosc_igiel > 1) {
	  TGCod *cod = (TGCod*)Codes->Items[0];
	  offx -= (cod->X - odleglosc_igiel/2) * skala;
  }
  offy = (Image->Height - (maxy*skala)) / 2;
  //===========================================

  double x1, y1, x2, y2, ox, oy, r;
  Image->Canvas->FillRect(TRect(0, 0, Image->Width, Image->Height));
  Image->Canvas->Pen->Width = 1;

  for(int k=0; k<ilosc_igiel; k++) {

  for(int i=0; i<Codes->Count; i++) {
	TGCod *cod = (TGCod*)Codes->Items[i];
	if(cod->Prev) {
	  x1 = cod->Prev->X * skala + offx;
	  y1 = Image->Height - (cod->Prev->Y * skala + offy);
	}
	x2 = (cod->X) * skala + offx;
	y2 = Image->Height - (cod->Y * skala + offy);

	ox = cod->OX * skala + offx;
	oy = Image->Height - (cod->OY * skala + offy);
	r = cod->R * skala;

	// row spacing
	if(k%2 == 0) {
		double fr = (double)rozstaw_rzedow * skala;
		y1 -= fr;
		y2 -= fr;
		oy -= fr;
	}

	switch(cod->G) {
	  case G_DRV:
		Image->Canvas->MoveTo(x2, y2);
		break;

	  case G_LIN:
		Image->Canvas->LineTo(x2, y2);
		break;

	  case G_CW:
		DrawArc(Image->Canvas, x1, y1, x2, y2, ox, oy, r, G_CCW);
		break;

	  case G_CCW:
		DrawArc(Image->Canvas, x1, y1, x2, y2, ox, oy, r, G_CW);
		break;
	}
  }

	offx += (double)odleglosc_igiel * skala;
  }  // for(k...)
  
	skala = old_skala;

	TJPEGImage *jp = new TJPEGImage();
	try
	{
		if(SaveDialog->Execute()) {

			jp->Assign(Image);
			jp->Smoothing = true;
			jp->SaveToFile(SaveDialog->FileName);
		}
	}
	__finally
	{
		delete jp;
	}
	delete Image;
}
//---------------------------------------------------------------------------
/// Box paint event handler
void __fastcall TPrintForm::BoxPaint(TObject *Sender)
{
  if(!MainForm->CurrProg) return;
  TList *Codes = MainForm->CurrProg->Codes;
  maxx = -3000;
  maxy = -3000;
  for(int i=0; i<Codes->Count; i++) {
	TGCod *cod = (TGCod*)Codes->Items[i];
	if(cod->G == G_DRV) continue;
	if(maxx < cod->X) maxx = cod->X;
	if(maxy < cod->Y) maxy = cod->Y;
  }
  if(ilosc_igiel > 1) {
	  maxx = ilosc_igiel * odleglosc_igiel;
  }

  double skalax, skalay;
  skalax = (Box->Width - 20) / maxx;
  skalay = (Box->Height - 20) / maxy;

  if(skalay < skalax) skala = skalay;
  else skala = skalax;

  offx = (Box->Width - (maxx*skala)) / 2;
  if(ilosc_igiel > 1) {
	  TGCod *cod = (TGCod*)Codes->Items[0];
	  offx -= (cod->X - odleglosc_igiel/2) * skala;
  }
  offy = (Box->Height - (maxy*skala)) / 2;
  //===========================================

  double x1, y1, x2, y2, ox, oy, r;
  Box->Canvas->FillRect(Box->ClientRect);
  Box->Canvas->Pen->Width = 2;


  for(int k=0; k<ilosc_igiel; k++) {


  for(int i=0; i<Codes->Count; i++) {
	TGCod *cod = (TGCod*)Codes->Items[i];
	if(cod->Prev) {
	  x1 = cod->Prev->X * skala + offx;
	  y1 = Box->Height - (cod->Prev->Y * skala + offy);
	}
	x2 = cod->X * skala + offx;
	y2 = Box->Height - (cod->Y * skala + offy);
	ox = cod->OX * skala + offx;
	oy = Box->Height - (cod->OY * skala + offy);
	r = cod->R * skala;

	// row spacing
	if(k%2 == 0) {
		double fr = (double)rozstaw_rzedow * skala;
		y1 -= fr;
		y2 -= fr;
		oy -= fr;
	}

	switch(cod->G) {
	  case G_DRV:
		Box->Canvas->MoveTo(x2, y2);
		break;

	  case G_LIN:
		Box->Canvas->LineTo(x2, y2);
		break;

	  case G_CW:
		DrawArc(Box->Canvas, x1, y1, x2, y2, ox, oy, r, G_CCW);
		break;

	  case G_CCW:
		DrawArc(Box->Canvas, x1, y1, x2, y2, ox, oy, r, G_CW);
		break;
	}
  }
	offx += (double)odleglosc_igiel * skala;
  }  // for(k...)
}
//---------------------------------------------------------------------------
/// ToolButton2 click event handler
void __fastcall TPrintForm::ToolButton2Click(TObject *Sender)
{
  if(!MainForm->PrintDialog->Execute()) return;


  TList *Codes = MainForm->CurrProg->Codes;
  maxx = -3000;
  maxy = -3000;
  for(int i=0; i<Codes->Count; i++) {
	TGCod *cod = (TGCod*)Codes->Items[i];
	if(cod->G == G_DRV) continue;
	if(maxx < cod->X) maxx = cod->X;
	if(maxy < cod->Y) maxy = cod->Y;
  }
  if(ilosc_igiel > 1) {
	  maxx = ilosc_igiel * odleglosc_igiel;
  }

  double margin = (Printer()->PageWidth*0.1);
  double skalax, skalay;
  skalax = (Printer()->PageWidth - margin*2) / maxx;
  skalay = (Printer()->PageHeight - margin*2) / maxy;

  if(skalay < skalax) skala = skalay;
  else skala = skalax;

  offx = (Printer()->PageWidth - (maxx*skala)) / 2;
  if(ilosc_igiel > 1) {
	  TGCod *cod = (TGCod*)Codes->Items[0];
	  offx -= (cod->X - odleglosc_igiel/2) * skala;
  }
  offy = (Printer()->PageHeight - (maxy*skala)) / 2;


  //===========================================

  double x1, y1, x2, y2, ox, oy, r;

  Printer()->BeginDoc();
  Printer()->Canvas->Pen->Width = 4;
  Printer()->Canvas->Font->Size = 11;

  for(int k=0; k<ilosc_igiel; k++) {
  for(int i=0; i<Codes->Count; i++) {
	TGCod *cod = (TGCod*)Codes->Items[i];
    if(cod->Prev) {
	  x1 = cod->Prev->X * skala + offx;
	  y1 = Printer()->PageHeight - (cod->Prev->Y * skala + offy);
	}
	x2 = cod->X * skala + offx;
	y2 = Printer()->PageHeight - (cod->Y * skala + offy);
	ox = cod->OX * skala + offx;
    oy = Printer()->PageHeight - (cod->OY * skala + offy);
	r = cod->R * skala;

	// row spacing
	if(k%2 == 0) {
		double fr = (double)rozstaw_rzedow * skala;
		y1 -= fr;
		y2 -= fr;
		oy -= fr;
	}

	switch(cod->G) {
	  case G_DRV:
        Printer()->Canvas->MoveTo(x2, y2);
		break;

	  case G_LIN:
		Printer()->Canvas->LineTo(x2, y2);
        break;

	  case G_CW:
		DrawArc(Printer()->Canvas, x1, y1, x2, y2, ox, oy, r, G_CCW);
		break;

	  case G_CCW:
        DrawArc(Printer()->Canvas, x1, y1, x2, y2, ox, oy, r, G_CW);
		break;
	}
  }

	offx += (double)odleglosc_igiel * skala;
  }  // for(k...)

  Printer()->EndDoc();
}
//---------------------------------------------------------------------------
/// ToolButton3 click event handler
void __fastcall TPrintForm::ToolButton3Click(TObject *Sender)
{
	ExportToJPEG();
}
//---------------------------------------------------------------------------
/// EIlosc change event handler
void __fastcall TPrintForm::EIloscChange(TObject *Sender)
{
	ilosc_igiel = StrToInt(EIlosc->Text);
	Box->Repaint();
}
//---------------------------------------------------------------------------
/// EOdleglosc change event handler
void __fastcall TPrintForm::EOdlegloscChange(TObject *Sender)
{
	odleglosc_igiel = StrToInt(EOdleglosc->Text);
	Box->Repaint();
}
//---------------------------------------------------------------------------
/// Form show event handler
void __fastcall TPrintForm::FormShow(TObject *Sender)
{
	ERozstaw->Text = IntToStr(rozstaw_rzedow);
}
//---------------------------------------------------------------------------
/// ERozstaw change event handler
void __fastcall TPrintForm::ERozstawChange(TObject *Sender)
{
	rozstaw_rzedow = StrToInt(ERozstaw->Text);
	Box->Repaint();
}
//---------------------------------------------------------------------------

