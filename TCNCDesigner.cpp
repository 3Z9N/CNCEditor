//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "TCNCDesigner.h"
#include <math.hpp>
#include <stdio.h>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <strstream>
#include <Clipbrd.hpp>
#include <Registry.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCNCDesigner *CNCDesigner;

//-------------------------------
// static variables
//-------------------------------
bool TCNCDesigner::grid = true;
bool TCNCDesigner::ruler = true;
bool TCNCDesigner::numbers = true;
bool TCNCDesigner::drv_path = true;


#define Round(x) RoundTo((x),0)

std::vector<TGCod*> copy_list;
std::vector<TGCod*> drag_list;

//-------------------------------
// standard colors for OpenGL
//-------------------------------
const float CL_RED[3]    = {1.0, 0.0, 0.0};
const float CL_GREEN[3]  = {0.0, 1.0, 0.0};
const float CL_BLUE[3]   = {0.0, 0.0, 1.0};
const float CL_YELLOW[3] = {1.0, 1.0, 0.0};
const float CL_CYAN[3]   = {0.0, 1.0, 1.0};
const float CL_WHITE[3]  = {1.0, 1.0, 1.0};
const float CL_BLACK[3]  = {0.0, 0.0, 0.0};

typedef std::vector<std::string> array;

//---------------------------------------------------------------------------
/// splits std c string to std::vector<std::string> with separator
array explode(const char* s, char separator)
{
  array v;
  if(!s) return v;
  const char* s1 = s;
  int len = 0;
  
  while(*s) {
	if(*s == '#') break;
    if(*s == separator) {
      if(! (s+1)) {
		v.push_back("");
		break;
	  }
	  v.push_back(std::string(s1, len));
	  s++;
	  s1 = s;
	  len = 0;
	  continue;
	}
	s++;
	len++;
  }
  v.push_back(std::string(s1, len));
  return v;
}
//---------------------------------------------------------------------------
// Constructor
__fastcall TCNCDesigner::TCNCDesigner(TComponent* Owner)
	: TFrame(Owner)
{
  zoom = 1.0;
  prev_zoom = 1.0;
  mx = 0;
  my = 0;
  prev_mx = 0;
  prev_my = 0;
  wx = 0;
  wy = 0;
  curs_x = 0;
  curs_y = 0;
  offsetx = 0.0;
  offsety = 0.0;
  start_drag = false;
  insert_r = 0;
  grid_size = 10;
  Program = 0;//new TProgram;
  DesignAction = DA_NONE;
  CurrCod = 0;
  DragCod = 0;
  DragMultCod = 0;
  modified = false;
  read_only = false;
  antialiasing = false;
  max_x = 0.0;//XXX
  max_y = 0.0;//XXX

  //--- create XPanel ---
  XPanel = new TXPanel(this);
  XPanel->Parent = this;
  XPanel->Align = alClient;
  XPanel->BevelInner = bvNone;
  XPanel->BevelOuter = bvNone;
  XPanel->Color = clBlack;
  XPanel->Cursor = crCross;
  XPanel->OnDblClick = this->XPanelDblClick;
  XPanel->OnMouseDown = this->XPanelMouseDown;
  XPanel->OnMouseMove = this->XPanelMouseMove;
  XPanel->OnStartDrag = this->XPanelStartDrag;
  XPanel->OnPaint = this->XPanelPaint;
}
//---------------------------------------------------------------------------
/// Move program to X=0, Y=0 coordinates
void __fastcall TCNCDesigner::MoveToZero()
{
	if(!Program) return;
	Program->SaveUndo();
	for(int i=0; i<Program->Codes->Count; i++) {
		TGCod *cod = (TGCod*)Program->Codes->Items[i];
		cod->X = (cod->X - min_x);
		cod->Y = (cod->Y - min_y);
		if(cod->X < 0) cod->X = 0;
		if(cod->Y < 0) cod->Y = 0;
	}
	for(int i=0; i<Program->Codes->Count; i++) {
		TGCod *cod = (TGCod*)Program->Codes->Items[i];
		cod->UpdateOXY();
	}
	SetModified(true);
  	Render();
}
//---------------------------------------------------------------------------
/// Rescal program
void __fastcall TCNCDesigner::Rescale()
{
	if(!Program) return;
	String s = InputBox("Przeskaluj",
						"WprowadŸ wspó³czynnik skalowania.\nNp 1,2 lub 120%",
						"1,0");
	bool percent = false;
	float scale = 1.0;
	char buf[32];
	strcpy(buf, s.c_str());
	buf[s.Length()] = 0;
	for(int i=0; i<s.Length(); i++) {
		if(buf[i] == ',') buf[i] = '.';
		if(buf[i] == '%') {
			percent = true;
			buf[i] = 0;
		}
		if(buf[i] == ' ') buf[i] = 0;
	}

	scale = atof(buf);//StrToFloat(buf);
	if(percent) scale /= 100.0f;

	Program->SaveUndo();
	for(int i=0; i<Program->Codes->Count; i++) {
		TGCod *cod = (TGCod*)Program->Codes->Items[i];
		if(cod->State != csSelected) continue;
		cod->X = RoundTo(cod->X * scale, 0);
		cod->Y = RoundTo(cod->Y * scale, 0);
		if(cod->G == G_CW || cod->G == G_CCW)
			cod->R = RoundTo(cod->R * scale, 0);
	}
	for(int i=0; i<Program->Codes->Count; i++) {
		TGCod *cod = (TGCod*)Program->Codes->Items[i];
		cod->UpdateOXY();
	}
	SetModified(true);
	Render();
}
//---------------------------------------------------------------------------
/// Add new cod
void __fastcall TCNCDesigner::AddCod(TGCod *cod)
{
  if(!cod) return;
  if(!Program->Codes->Count && cod->G != G_DRV) {
    delete cod;
    return;
  }
  if(Program->Codes->Count) {
	cod->Prev = (TGCod*)Program->Codes->Last();
    cod->Prev->Next = cod;
  }
  Program->Codes->Add(cod);
  cod->UpdateOXY();
}
//---------------------------------------------------------------------------
/// Copy selected codes to clipboard
void __fastcall TCNCDesigner::Copy()
{
  if(!Program) return;
  char buf[65536];
  sprintf(buf, "CNCEditor|%s|%d|%d;", Program->Name.c_str(),
		  Program->Number, Program->Frame);
  AnsiString Line = buf;

  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *C = (TGCod*)Program->Codes->Items[i];
	if(C->State != csSelected) continue;
	sprintf(buf, "%d|%f|%f|%f|%d|%d|%d;",
			C->G, C->X, C->Y, C->R, C->F, C->M, C->T);
    Line += buf;
  }
  Clipboard()->Open();
  Clipboard()->SetTextBuf(Line.c_str());
  Clipboard()->Close();
}
//---------------------------------------------------------------------------
/// Copy program to clipboard
void __fastcall TCNCDesigner::CopyProgramToClipboard(TProgram *Prog)
{
  if(!Prog) return;
  char buf[65536];
  sprintf(buf, "CNCEditor|%s|%d|%d;", Prog->Name.c_str(),
		  Prog->Number, Prog->Frame);
  AnsiString Line = buf;

  for(int i=0; i<Prog->Codes->Count; i++) {
	TGCod *C = (TGCod*)Prog->Codes->Items[i];
	sprintf(buf, "%d|%f|%f|%f|%d|%d|%d;",
			C->G, C->X, C->Y, C->R, C->F, C->M, C->T);
    Line += buf;
  }
  Clipboard()->Open();
  Clipboard()->SetTextBuf(Line.c_str());
  Clipboard()->Close();
}
//---------------------------------------------------------------------------
/// Creates 2D font for OpenGL
TGLFont2D * __fastcall TCNCDesigner::CreateFont2D(TFont *Font,
												  int FirstGylph,
												  int NumGylph)
{
  HFONT         HFont;
  LOGFONT       LogFont;
  GLuint        Base;

  if (Font==NULL) return(NULL);
  if ((Base=glGenLists(NumGylph))==0) return(NULL);
  TGLFont2D *Font2D=new TGLFont2D();
  Font2D->FirstGylph=FirstGylph;
  Font2D->NumGylph=NumGylph;
  Font2D->ListBase=Base;
  GetObject(Font->Handle,sizeof(LOGFONT),&LogFont);
  HFont = CreateFontIndirect(&LogFont);
  SelectObject (hdc, HFont);
  wglUseFontBitmaps(hdc, FirstGylph, NumGylph, Base);
  DeleteObject(HFont);
  return(Font2D);
}
//---------------------------------------------------------------------------
/// Delete selected codes
void __fastcall TCNCDesigner::Delete()
{
  if(!Program) return;
  Program->SaveUndo();
  for(int i=Program->Codes->Count-1; i>=0; i--) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
    if(cod->State == csSelected) {
      DeleteCod(cod);
    }
  }
  CurrCod = 0;
  SetValues();
  SetModified(true);
  Render();
}
//---------------------------------------------------------------------------
/// Delete cod
void __fastcall TCNCDesigner::DeleteCod(TGCod *cod)
{
  if(!cod) return;
  TGCod *prev = cod->Prev;
  TGCod *next = cod->Next;
  if(next) next->Prev = prev;
  if(prev) prev->Next = next;
  Program->Codes->Remove(cod);
  delete cod;
  if(next) next->UpdateOXY();
}
//---------------------------------------------------------------------------
/// Deselect all codes
void __fastcall TCNCDesigner::DeselectAll()
{
  if(!Program) return;
  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
	cod->State = csNormal;
  }
  Render();
}
//---------------------------------------------------------------------------
/// Select cod at x,y coordinates
GLint __fastcall TCNCDesigner::DoSelect(int x, int y)
{
  GLint hits;
  glSelectBuffer(MAXSELECT, SelectBuf);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(~0);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glGetIntegerv(GL_VIEWPORT, VP);
  //gluPickMatrix(x, y, 4, 4, VP);
  gluPickMatrix(x, y, 5, 5, VP);
  glOrtho(-(GLdouble)XPanel->Width/2,  (GLdouble)XPanel->Width/2,
		  -(GLdouble)XPanel->Height/2, (GLdouble)XPanel->Height/2,
          -100.0f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  DrawDesignerScene(GL_SELECT);

  hits = glRenderMode(GL_RENDER);
  if (hits <= 0) {
    return -1;
  }

  return SelectBuf[(hits-1)*4+3];
}
//---------------------------------------------------------------------------
/// TCNCDesigner - drag drop event handler
void __fastcall TCNCDesigner::DragDropCod()
{
  if(DragCod) {
	Program->SaveUndo();
    DragCod->X = drop_mx;
    DragCod->Y = drop_my;
    DragCod->UpdateOXY();
	if(DragCod->Next) DragCod->Next->UpdateOXY();
	SetModified(true);
  }
  else if(DragMultCod) {
    Program->SaveUndo();
    for(unsigned int i=0; i<drag_list.size(); i++) {
	  TGCod *cod = drag_list[i];
      if(cod->ID > -1) {
		TGCod* old_cod = (TGCod*)Program->Codes->Items[cod->ID];
        old_cod->X = cod->X;
        old_cod->Y = cod->Y;
        old_cod->UpdateOXY();
      }
	}
	SetModified(true);
  }
}
//---------------------------------------------------------------------------
/// Draw text
void __fastcall TCNCDesigner::Draw2DText(AnsiString Text)
{
  unsigned char *c=(unsigned char *)Text.c_str();
  int            Index,Length=strlen((char *)c);

  if ((GLFont2D==NULL)||(Length==0)) return;
  for (int i=0;i<Length;i++) {
	Index=c[i]- GLFont2D->FirstGylph;
	if ((Index<0)||(Index>GLFont2D->NumGylph-1)) return;
  }
  glPushAttrib(GL_LIST_BIT);
  glListBase(GLFont2D->ListBase-GLFont2D->FirstGylph);
  glCallLists (Length, GL_UNSIGNED_BYTE,c);
  glPopAttrib();
}
//---------------------------------------------------------------------------
/// Draw text at x,y coordinates
void __fastcall  TCNCDesigner::Draw2DTextXY(double x, double y, AnsiString Text)
{
  glRasterPos2d(x, y);
  Draw2DText(Text);
}
//---------------------------------------------------------------------------
/// \brief Draw arc
/// \param x1 - begin X coordinate
/// \param y1 - begin Y coordinate
/// \param x2 - end X coordinate
/// \param y2 - end Y coordinate
/// \param ox - center X arc coordinate
/// \param oy - center Y arc coordinate
/// \param r - radius
/// \param dir - direction (CW or CCW)
void __fastcall TCNCDesigner::DrawArc(double x1, double y1, double x2, double y2,
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

  double prev_X = x1;
  double prev_Y = y1;

  int n = (fabs(a2-a1)/360 * 2 * M_PI * r1) / 2;
  if(n == 0) n = 1;
  double step = fabs(a2-a1)/n;

  if(dir == G_CW) {
    for(double angle=a1-step; angle>a2; angle-=step) {
      double rad = DegToRad(angle);
      x = r1 * cos(rad) + ox;
	  y = r1 * sin(rad) + oy;

	  if((float)x < min_x) min_x = (float)x;
	  if((float)y < min_y) min_y = (float)y;
	  if((float)x > max_x) max_x = (float)x;
	  if((float)y > max_y) max_y = (float)y;

      glBegin(GL_LINES);
        glVertex3f( prev_X, prev_Y, 0.0f);
        glVertex3f( x, y, 0.0f);
      glEnd();

      prev_X = x;
      prev_Y = y;
    }
  }
  else { // dir == G_CCW
    for(double angle=a1+step; angle<a2; angle+=step) {
      double rad = DegToRad(angle);
      x = r1 * cos(rad) + ox;
	  y = r1 * sin(rad) + oy;

	  if((float)x < min_x) min_x = (float)x;
	  if((float)y < min_y) min_y = (float)y;
	  if((float)x > max_x) max_x = (float)x;
	  if((float)y > max_y) max_y = (float)y;

      glBegin(GL_LINES);
        glVertex3f( prev_X, prev_Y, 0.0f);
        glVertex3f( x, y, 0.0f);
      glEnd();

      prev_X = x;
      prev_Y = y;
    }
  }

  glBegin(GL_LINES);
    glVertex3f( prev_X, prev_Y, 0.0f);
    glVertex3f( x2, y2, 0.0f);
  glEnd();
}
//---------------------------------------------------------------------------
/// Draw cod CCW
void __fastcall TCNCDesigner::DrawCodCCW(TGCod *cod)
{
  if(cod->State == csSelected) glColor3fv(CL_RED);
  else if(cod->State == csDragged) glColor3fv(CL_CYAN);
  else glColor3fv(CL_YELLOW);

  DrawArc(cod->Prev->X, cod->Prev->Y, cod->X, cod->Y,
		  cod->OX, cod->OY, cod->R, cod->G);
  DrawPoint(cod->X, cod->Y);
  if(cod->State) DrawPoint(cod->OX, cod->OY);

  if(!numbers || cod->State == csDragged) return;
  double pw = 4 / zoom;
  glRasterPos2d(cod->X+pw, cod->Y+pw);
  Draw2DText(IntToStr(Program->Codes->IndexOf(cod)));
}
//---------------------------------------------------------------------------
/// Draw cod CW
void __fastcall TCNCDesigner::DrawCodCW(TGCod *cod)
{
  if(cod->State == csSelected) glColor3fv(CL_RED);
  else if(cod->State == csDragged) glColor3fv(CL_CYAN);
  else glColor3fv(CL_YELLOW);

  DrawArc(cod->Prev->X, cod->Prev->Y, cod->X, cod->Y,
		  cod->OX, cod->OY, cod->R, cod->G);
  DrawPoint(cod->X, cod->Y);
  if(cod->State) DrawPoint(cod->OX, cod->OY);

  if(!numbers || cod->State == csDragged) return;
  double pw = 4 / zoom;
  glRasterPos2d(cod->X+pw, cod->Y+pw);
  Draw2DText(IntToStr(Program->Codes->IndexOf(cod)));
}
//---------------------------------------------------------------------------
/// Draw cod DRV
void __fastcall TCNCDesigner::DrawCodDRV(TGCod *cod)
{
	const float* cl1 = CL_GREEN;
	if(cod->State == csSelected) cl1 = CL_RED;
	else if(cod->State == csDragged) cl1 = CL_CYAN;
	float cl2[3] = { cl1[0], cl1[1], cl1[2] };
	glColor3fv(cl1);

	double pp = 1.75 / zoom;
	double pw = 3.5 / zoom;
	if(cod->Return || cod->T > 0) glBegin(GL_POLYGON);
	else glBegin(GL_LINE_LOOP);
		glVertex2f( cod->X,    cod->Y+pp );
		glVertex2f( cod->X+pw, cod->Y+2*pw+pp );
		glVertex2f( cod->X-pw, cod->Y+2*pw+pp );
	glEnd();

	if(drv_path && cod->Prev) {
		cl2[0] /= 2;
		cl2[1] /= 2;
		cl2[2] /= 2;
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(2, 0x5555);
		glColor3fv(cl2);
		DrawLine(cod->Prev->X, cod->Prev->Y, cod->X, cod->Y);
		glDisable(GL_LINE_STIPPLE);
		glColor3fv(cl1);
	}
	DrawPoint(cod->X, cod->Y);

	if(!numbers || cod->State == csDragged) return;
	double px = 5 / zoom;
	glRasterPos2d(cod->X+px, cod->Y+pw);
	Draw2DText(IntToStr(Program->Codes->IndexOf(cod)));
}
//---------------------------------------------------------------------------
/// Draw cod LIN
void __fastcall TCNCDesigner::DrawCodLIN(TGCod *cod)
{
  if(cod->State == csSelected) glColor3fv(CL_RED);
  else if(cod->State == csDragged) glColor3fv(CL_CYAN);
  else glColor3fv(CL_YELLOW);

  DrawLine(cod->Prev->X, cod->Prev->Y, cod->X, cod->Y);
  DrawPoint(cod->X, cod->Y);

  if(!numbers || cod->State == csDragged) return;
  double pw = 4 / zoom;
  glRasterPos2d(cod->X+pw, cod->Y+pw);
  Draw2DText(IntToStr(Program->Codes->IndexOf(cod)));
}
//---------------------------------------------------------------------------
// Draw OpenGL scene with render mode
void __fastcall TCNCDesigner::DrawDesignerScene(GLenum RenderMode)
{
  if(antialiasing) {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glLineWidth(1.5);
  }

  glScalef(zoom, zoom, 0.0f);
  if(offsetx != 0.0 || offsety != 0.0)
    glTranslatef(offsetx, offsety, 0.0f);

  // draw grid
  if(grid) DrawGrid();

  // draw X=0, Y=0 cross
  float crossL = 20.0 / zoom;
  glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f( -crossL, 0.0, 0.0f);
    glVertex3f( crossL, 0.0, 0.0f);
    glVertex3f( 0.0, -crossL, 0.0f);
    glVertex3f( 0.0, crossL, 0.0f);
  glEnd();
  glRasterPos2d(crossL, -crossL);
  Draw2DText("X");
  glRasterPos2d(-crossL, crossL);
  Draw2DText("Y");

  // draw codes
  if(!Program) return;

  min_x = 100000.0;
  min_y = 100000.0;
  max_x = -100000.0;
  max_y = -100000.0;

  for(int i=Program->Codes->Count-1; i>=0; i--) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
	if(cod->G != G_DRV) {
		if(cod->X < min_x) min_x = cod->X;
		if(cod->Y < min_y) min_y = cod->Y;
		if(cod->X > max_x) max_x = cod->X;
		if(cod->Y > max_y) max_y = cod->Y;
	}
	else if(cod->Next) {
		if(cod->Next->G != G_DRV) {
			if(cod->X < min_x) min_x = cod->X;
			if(cod->Y < min_y) min_y = cod->Y;
			if(cod->X > max_x) max_x = cod->X;
			if(cod->Y > max_y) max_y = cod->Y;
        }
    }
	if (RenderMode == GL_SELECT) glLoadName(i);
	switch(cod->G) {
	  case G_CW:
		DrawCodCW(cod);
        break;
	  case G_CCW:
		DrawCodCCW(cod);
        break;
	  case G_DRV:
		DrawCodDRV(cod);
        break;
	  case G_LIN:
		DrawCodLIN(cod);
        break;
    }
  }

  if (RenderMode == GL_SELECT) {
	return;
  }

  if(antialiasing) {
	glDisable(GL_BLEND);
	glDisable(GL_LINE_SMOOTH);
	glLineWidth(1.0);
  }

  // draw selection rectangle
  if(DesignAction == DA_DRAG_SEL) {
    glColor3fv(CL_CYAN);
    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_XOR);
    DrawFocusRect(drag_mx, drag_my, mx, my);
	glDisable(GL_COLOR_LOGIC_OP);
    return;
  }
  // draw cursor
  if(DesignAction != DA_NONE) {
    glColor3fv(CL_CYAN);
    switch(DesignAction) {
      case DA_CW:
        break;
      case DA_CCW:
        break;
      case DA_DRV:
        break;
      case DA_LIN: {
        TGCod *prev_cod = (TGCod*)Program->Codes->Last();
        DrawLine(prev_cod->X, prev_cod->Y, curs_x, curs_y);
        break;
      }
      case DA_DRAG_COD:
      case DA_DRAG_MULT_COD:
        DrawDraggedCod();
        break;
      default:
        return;
    }

    glEnable(GL_COLOR_LOGIC_OP);
    glLogicOp(GL_XOR);

    glEnable(GL_LINE_STIPPLE);
    glLineStipple(3, 0x3F3F);
    glColor3fv(CL_CYAN);
    glBegin(GL_LINES);
      glVertex3f( min_wx, curs_y, 0.0f);
      glVertex3f( max_wx, curs_y, 0.0f);
      glVertex3f( curs_x, min_wy, 0.0f);
      glVertex3f( curs_x, max_wy, 0.0f);
    glEnd();
    glDisable(GL_LINE_STIPPLE);
    glDisable(GL_COLOR_LOGIC_OP);
    return;
  }

  min_x = RoundTo(min_x, 0);
  min_y = RoundTo(min_y, 0);
  max_x = RoundTo(max_x, 0);
  max_y = RoundTo(max_y, 0);
}
//---------------------------------------------------------------------------
/// Draw dragged cod
void __fastcall TCNCDesigner::DrawDraggedCod()
{
  for(unsigned int i=0; i<drag_list.size(); i++) {
	TGCod *cod = drag_list[i];
	cod->State = csDragged;
	switch(cod->G) {
	  case G_CW:
		DrawCodCW(cod);
		break;
	  case G_CCW:
		DrawCodCCW(cod);
        break;
	  case G_DRV:
		DrawCodDRV(cod);
        break;
	  case G_LIN:
        DrawCodLIN(cod);
        break;
    }
  }
}
//---------------------------------------------------------------------------
/// \brief Draw focus recatndle
/// \param x1 - begin X coordinate
/// \param y1 - begin Y coordinate
/// \param x2 - end X coordinate
/// \param y2 - end Y coordinate
void __fastcall TCNCDesigner::DrawFocusRect(double x1 , double y1, double x2 , double y2)
{
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(2, 0x5555);
  glBegin(GL_LINE_LOOP);
    glVertex3f( x1, y1, 0.0f);
    glVertex3f( x1, y2, 0.0f);
    glVertex3f( x2, y2, 0.0f);
    glVertex3f( x2, y1, 0.0f);
  glEnd();
  glDisable(GL_LINE_STIPPLE);
}
//---------------------------------------------------------------------------
/// Draw grid
void __fastcall TCNCDesigner::DrawGrid()
{

  min_wx = -((double)XPanel->Width/2) / zoom - offsetx;
  min_wy = -((double)XPanel->Height/2) / zoom - offsety;
  max_wx = ((double)XPanel->Width/2) / zoom - offsetx;
  max_wy = ((double)XPanel->Height/2) / zoom - offsety;

  if(zoom < 0.08) grid_size = 1000.0;
  else if(zoom < 0.8) grid_size = 100.0;
  else if(zoom < 8.0) grid_size = 10.0;
  else grid_size = 1.0;

  int maxi = 3000 / grid_size;
  double x = 0.0;
  double y = 0.0;

  // drawing auxiliary lines - every 10th
  if(2500.0/grid_size > 10.0) {
	double mxy = 3000.0-grid_size*10;
	glColor3f(0.32, 0.32, 0.32);
	glEnable(GL_LINE_STIPPLE);
    glLineStipple(1, 0x6666);
	// vertical lines
    x = 0.0;
    y = 0.0;

    glColor3f(0.32, 0.32, 0.32);
    for(int i=0; x<=mxy && x<max_wx; i+=10) {
      x = (double)i * grid_size;
      glBegin(GL_LINES);
        glVertex3f(x, 0.0, 1.0f);
        glVertex3f(x, 3000.0, 1.0f);
      glEnd();
      if(ruler) {
        glColor3f(0.5, 0.5, 0.5);
        Draw2DTextXY(x, min_wy + 4/zoom, String(x));
        glColor3f(0.32, 0.32, 0.32);
      }
    }

	// horizontal lines
	x = 0.0;
    y = 0.0;
    for(int i=0; y<=mxy && y<max_wy; i+=10) {
      y = (double)i * grid_size;
      glBegin(GL_LINES);
        glVertex3f(0.0, y, 1.0f);
        glVertex3f(3000.0, y, 1.0f);
      glEnd();
      if(ruler) {
        glColor3f(0.5, 0.5, 0.5);
		Draw2DTextXY(min_wx+4/zoom, y, String(y));
        glColor3f(0.32, 0.32, 0.32);
      }
    }
    glDisable(GL_LINE_STIPPLE);
  }

  // draw grid
  glColor3f(0.5, 0.5, 0.5);
  glBegin(GL_POINTS);
  int i = min_wx / grid_size;
  int j = min_wy / grid_size;
  for(i=0; i<=maxi && x<max_wx; i++) {
    x = (double)i * grid_size;
    if(x < min_wx) continue;
    y = 0.0;
    for(j=0; j<=maxi && y<max_wy; j++) {
	  y = (double)j * grid_size;
	  if(y < min_wy) continue;
	  glVertex3f(x, y, 1.0f);
	}
  }
  glEnd();
}
//---------------------------------------------------------------------------
/// \brief Draw line
/// \param x1 - begin X coordinate
/// \param y1 - begin Y coordinate
/// \param x2 - end X coordinate
/// \param y2 - end Y coordinate
void __fastcall TCNCDesigner::DrawLine(double x1, double y1, double x2 , double y2)
{
  glBegin(GL_LINES);
    glVertex3f( x1, y1, 0.0 );
    glVertex3f( x2, y2, 0.0 );
  glEnd();
}
//---------------------------------------------------------------------------
/// \brief Draw point
/// \param x1 - X coordinate
/// \param y1 - Y coordinate
void __fastcall TCNCDesigner::DrawPoint(double x, double y)
{
  glPointSize(4);
  glBegin(GL_POINTS);
    glVertex2f( x, y );
  glEnd();
  glPointSize(1);
}
//---------------------------------------------------------------------------
/// Load program codes from clipboard
TProgram* __fastcall TCNCDesigner::GetProgramFromClipboard()
{
  char data[100000];//8192];
  memset(data, 0, sizeof(data));
  Clipboard()->Open();
  Clipboard()->GetTextBuf(data, sizeof(data));
  Clipboard()->Close();
  if(strlen(data) < 9) return 0;

  std::istrstream iss(data, sizeof(data));
  char buf[1024];
  char str[1024];
  AnsiString line;

  // get program info
  memset(buf, 0, sizeof(buf));
  iss.getline(buf, sizeof(buf), ';');
  array a = explode(buf, '|');
  if(a.size() < 4) return 0;
  if(a[0] != "CNCEditor") return 0;
  TProgram *Prog = new TProgram();
  Prog->Name = a[1].c_str();
  Prog->Number = StrToInt(a[2].c_str());
  Prog->Frame = StrToInt(a[3].c_str());

  // get codes
  while(iss) {
	memset(buf, 0, sizeof(buf));
	iss.getline(buf, sizeof(buf), ';');
	line = buf;
	if(line.IsEmpty()) continue;

	TGCod *cod = new TGCod;
	sscanf(buf, "%d|%f|%f|%f|%d|%d|%d",
		   &cod->G, &cod->X, &cod->Y, &cod->R, &cod->F, &cod->M, &cod->T);

	if(Prog->Codes->Count) {
	  cod->Prev = (TGCod*)Prog->Codes->Last();
	  cod->Prev->Next = cod;
	}
	Prog->Codes->Add(cod);
	cod->UpdateOXY();
  }
  return Prog;
}
//---------------------------------------------------------------------------
/// Get number of selected codes
int __fastcall TCNCDesigner::GetSelectedCount()
{
  if(!Program->Codes) return 0;
  int count = 0;
  for(int i=0; i<Program->Codes->Count; i++) {
    TGCod *cod = (TGCod*)Program->Codes->Items[i];
    if(cod->State & csSelected == csSelected) count++;
  }
  return count;
}
//---------------------------------------------------------------------------
/// Init OpenGL scene
void __fastcall TCNCDesigner::InitOpenGL()
{
  PIXELFORMATDESCRIPTOR pfd;
  int format_pixel;

  // window handle
  hdc = GetDC(XPanel->Handle);

  // set pixel format
  ZeroMemory( &pfd, sizeof( pfd));
  pfd.nSize = sizeof( pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 16;
  pfd.cDepthBits = 24;
  pfd.iLayerType = PFD_MAIN_PLANE;

  format_pixel = ChoosePixelFormat(hdc, &pfd);
  SetPixelFormat(hdc, format_pixel, &pfd);

  hrc = wglCreateContext(hdc); // ID (context) of the OpenGL graphics engine

  if(hrc == NULL)
 	ShowMessage( "Nie z³apa³em kontekstu grafiki: hrc == NULL");

  if(wglMakeCurrent(hdc, hrc) == false)	// Activate OpenGL graphics engine
	ShowMessage("Nie uaktywni³em grafiki");

   GLFont2D=CreateFont2D(Font,32,96);

  // Initial settings of OpenGL scene
  glViewport(0,0,(GLsizei)XPanel->Width,(GLsizei)XPanel->Height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glDepthFunc(GL_LEQUAL);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_AUTO_NORMAL);
  glClearColor(0.0f,0.0f,0.0f,1.0f);

  UpdateLabels();
}
//---------------------------------------------------------------------------
/// Returns the cod at the x, y position
bool __fastcall TCNCDesigner::IsCodPointAt(TGCod *cod, double x, double y)
{
  double delta = 3 / zoom;
  if(cod->X>=x-delta && cod->X<=x+delta &&
     cod->Y>=y-delta && cod->Y<=y+delta) {
    return true;
  }
  return false;
}
//---------------------------------------------------------------------------
/// Moves cod to new index
void __fastcall TCNCDesigner::MovePoint()
{
  if(!CurrCod) return;
  String s = InputBox("Przesuñ punkt",
					  "Wpisz nowy numer punktu:",
					  String(Program->Codes->IndexOf(CurrCod)));
  if(s.IsEmpty()) return;
  int dst_id = StrToInt(s);
  if(dst_id < 0 || dst_id > Program->Codes->Count-1) return;
  TGCod *cod1 = (TGCod*)Program->Codes->Items[dst_id];
  if(CurrCod == cod1) return;
  TGCod *tmp;
  cod1 = new TGCod(*CurrCod);
  DeleteCod(CurrCod);
  Program->Codes->Insert(dst_id, cod1);
  if(dst_id == 0) {
	tmp = (TGCod*)Program->Codes->Items[1];
    cod1->Prev = 0;
    cod1->Next = tmp;
    tmp->Prev = cod1;
  } else if(cod1 == Program->Codes->Last()) {
	tmp = (TGCod*)Program->Codes->Items[Program->Codes->Count-2];
    tmp->Next = cod1;
    cod1->Prev = tmp;
  } else {
	tmp = (TGCod*)Program->Codes->Items[dst_id-1];
    tmp->Next = cod1;
    cod1->Prev = tmp;
    tmp = (TGCod*)Program->Codes->Items[dst_id+1];
    tmp->Prev = cod1;
    cod1->Next = tmp;
  }

  CurrCod = cod1;
  CurrCod->UpdateOXY();
  if(CurrCod->Next) CurrCod->Next->UpdateOXY(); 
  Render();
}
//---------------------------------------------------------------------------
/// Reverse the program path direction
void __fastcall TCNCDesigner::ReverseStitch()
{
	if(!CurrCod) return;

	// find the beginning of the marked DRV point
	TGCod* cod = (TGCod*)Program->Codes->First();
	while(cod) {
		if(cod->G == G_DRV && cod->State == csSelected) break;
		cod = cod->Next;
	}

	// check if the path is selected
	if(!cod) return;
	if(!cod->Next) return;
	if(cod->Next->State != csSelected) return;

	// find the last selected cod
	while(cod->Next) {
		if(cod->Next->State != csSelected) break;
		cod = cod->Next;
	}
	if(cod->Next && cod->Next->G != G_DRV) return;
}
//---------------------------------------------------------------------------
/// Rotate program with designed angle
void __fastcall TCNCDesigner::Rotate(TRotateAngle RAngle)
{
  if(!Program) return;

  int _minx = 100000;
  int _miny = 100000;
  int _maxx = -100000;
  int _maxy = -100000;
  int sel_count = 0;
  
  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
    if(cod->State == csSelected) {
      sel_count++;
      if(cod->X < _minx) _minx = cod->X;
      if(cod->Y < _miny) _miny = cod->Y;
      if(cod->X > _maxx) _maxx = cod->X;
      if(cod->Y > _maxy) _maxy = cod->Y;
    }
  }

  if(sel_count < 2) return;
  Program->SaveUndo();
  int ox  = _minx + (_maxx - _minx)/2;
  int oy  = _miny + (_maxy - _miny)/2;

  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
    if(cod->State == csSelected) {
      int dx = ox - cod->X;
      int dy = oy - cod->Y;
	  switch(RAngle) {
		case RA_270:   // 90 degrees right
          cod->Y = oy + dx;
		  cod->X = ox - dy;
          break;
		case RA_90:  // 90 degrees left
          cod->Y = oy - dx;
          cod->X = ox + dy;
          break;
		case RA_180:  // 180 degrees
          cod->Y = oy + dy;
          cod->X = ox + dx;
          break;
      }
      cod->UpdateOXY();
    }
  }
  SetModified(true);
  Render();
}
//---------------------------------------------------------------------------
/// Rotate program with any angle
void __fastcall TCNCDesigner::RotateAngle()
{
  if(!Program) return;

  String s = InputBox("Obróæ",
					  "Wpisz k¹t obrotu",
					  String(Program->Codes->IndexOf(CurrCod)));
  if(s.IsEmpty()) return;
  double angle = (double)StrToInt(s);
  if(angle < 1 || angle > 359) return;

  int sel_count = 0;

  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
	if(cod->State == csSelected) {
	  sel_count++;
	}
  }

  if(sel_count < 2) return;
  Program->SaveUndo();


  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
	if(cod->State == csSelected) {
	  double a1 = RadToDeg( ArcTan2((double)cod->Y, (double)cod->X) );
	  double r = sqrt( (double)(cod->X*cod->X + cod->Y*cod->Y) );
	  a1 += angle;

	  double rad = DegToRad(a1);
	  double x = r * cos(rad);
	  double y = r * sin(rad);

	  cod->X = (int)x;
	  cod->Y = (int)y;
	  cod->UpdateOXY();
	}
  }


  SetModified(true);
  Render();
}
//---------------------------------------------------------------------------
/// Mirror the program with horizontal or vertical direction
void __fastcall TCNCDesigner::Mirror(TMirrorDirection MDir)
{
  if(!Program) return;

  int _minx = 10000;
  int _miny = 10000;
  int _maxx = -10000;
  int _maxy = -10000;
  int sel_count = 0;
  
  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
    if(cod->State == csSelected) {
      sel_count++;
      if(cod->X < _minx) _minx = cod->X;
      if(cod->Y < _miny) _miny = cod->Y;
      if(cod->X > _maxx) _maxx = cod->X;
      if(cod->Y > _maxy) _maxy = cod->Y;
    }
  }

  if(sel_count < 2) return;
  Program->SaveUndo();
  int ox  = _minx + (_maxx - _minx)/2;
  int oy  = _miny + (_maxy - _miny)/2;

  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
    if(cod->State == csSelected) {
      int dx = ox - cod->X;
      int dy = oy - cod->Y;
	  switch(MDir) {
		case MD_HORIZONTAL:
		  cod->X = ox + dx;
          break;
		case MD_VERTICAL:
		  cod->Y = oy + dy;
		  break;
	  }
	  if(cod->G == G_CW) cod->G = G_CCW;
	  else if(cod->G == G_CCW) cod->G = G_CW;
	  cod->UpdateOXY();
	}
  }
  SetModified(true);
  Render();
}
//---------------------------------------------------------------------------
/// Paste codes from clipboard
void __fastcall TCNCDesigner::Paste()
{
	if(!Program) return;
	DeselectAll();
	char data[65536];
	memset(data, 0, sizeof(data));
	Clipboard()->Open();
	Clipboard()->GetTextBuf(data, sizeof(data));
	Clipboard()->Close();

	std::istrstream iss(data, sizeof(data));
	char buf[1024];
	char str[1024];
	AnsiString line;

	// get program info
	memset(buf, 0, sizeof(buf));
	iss.getline(buf, sizeof(buf), ';');
	array a = explode(buf, '|');
	if(a.size() < 4) return;
	if(a[0] != "CNCEditor") return;

	// get codes
	TGCod *cod = 0;
	int curr_id = -1;
	if(CurrCod) curr_id = Program->Codes->IndexOf(CurrCod);
	if(!Program->Codes->Count) curr_id = -1;

	Program->SaveUndo();
	
	while(iss) {
		memset(buf, 0, sizeof(buf));
		iss.getline(buf, sizeof(buf), ';');
		if(!*buf) continue;
		line = buf;
		if(line.IsEmpty()) continue;

		cod = new TGCod;
		sscanf(buf, "%d|%f|%f|%f|%d|%d|%d",
			   &cod->G, &cod->X, &cod->Y, &cod->R, &cod->F, &cod->M, &cod->T);

		if(curr_id == -1) { // append
			if(Program->Codes->Count) {
				cod->Prev = (TGCod*)Program->Codes->Last();
				cod->Prev->Next = cod;
			}
			Program->Codes->Add(cod);
			cod->State = csSelected;
			cod->UpdateOXY();
		}
		else { // insert before CurrCod
			cod->Prev = CurrCod->Prev;
			cod->Next = CurrCod;
			CurrCod->Prev = cod;
			cod->State = csSelected;
			Program->Codes->Insert(curr_id, cod);
			curr_id++;
		}

		cod->UpdateOXY();
		if(CurrCod) CurrCod->UpdateOXY();
	}

	if(CurrCod) CurrCod->State = csNormal;
	CurrCod = cod;
	SetValues();
	SetModified(true);
	Render();
}
//---------------------------------------------------------------------------
/// Release OpenGL memory context
void __fastcall TCNCDesigner::ReleaseOpenGL()
{
  wglMakeCurrent(NULL, NULL);
  wglDeleteContext(hrc);
  ReleaseDC(Handle, hdc);
}
//---------------------------------------------------------------------------
/// Render all
void __fastcall TCNCDesigner::Render()
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-(GLdouble)XPanel->Width/2,  (GLdouble)XPanel->Width/2,
		  -(GLdouble)XPanel->Height/2, (GLdouble)XPanel->Height/2,
		  -100.0f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
  glLoadIdentity();					// Reset The Current Modelview Matrix

  DrawDesignerScene(GL_RENDER);
  
  SwapBuffers(hdc);
}
//---------------------------------------------------------------------------
/// Select all codes
void __fastcall TCNCDesigner::SelectAll()
{
  if(!Program) return;
  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
	cod->State = csSelected;
  }
  Render();
}
//---------------------------------------------------------------------------
/// Select codes inside rectangle
void __fastcall TCNCDesigner::SelectAt(double left, double bottom,
									   double right, double top)
{
  for(int i=0; i<Program->Codes->Count; i++) {
	TGCod *cod = (TGCod*)Program->Codes->Items[i];
	if(cod->X>=left && cod->X<=right && cod->Y>=bottom && cod->Y<=top) {
	  cod->State = csSelected;
    }
  }
}
//---------------------------------------------------------------------------
/// Select program path
void __fastcall TCNCDesigner::SelectStitch()
{
	if(!CurrCod) return;

	// go up codes
	TGCod* cod = CurrCod;
	while(cod) {
		cod->State = csSelected;
		if(cod->G == G_DRV) break;
		cod = cod->Prev;
	}
	// go down codes
	cod = CurrCod;
	while(cod) {
		if(cod->G == G_DRV) break;
		cod->State = csSelected;
		cod = cod->Next;
	}
}
//---------------------------------------------------------------------------
/// Set antialiased rendering
void __fastcall TCNCDesigner::SetAntialiasing(bool aa)
{
  antialiasing = aa;
  Render();
}
//---------------------------------------------------------------------------
/// Set design action
void __fastcall TCNCDesigner::SetDesignAction(TDesignAction dAction)
{
  if(!Program) return;
  if(!Program->Codes->Count) {
	if(dAction==DA_LIN || dAction==DA_CW || dAction==DA_CCW) {
	  ShowMessage("Pierwszy obiekt musi byæ typu DRV !");
	  if(FOnDesignAction) FOnDesignAction(DA_NONE);
	  return;
    }
  }
  DesignAction = dAction;
}
//---------------------------------------------------------------------------
/// Set modified attribute
void __fastcall TCNCDesigner::SetModified(bool mod)
{
  if(modified != mod) {
	modified = mod;
	if(FOnModify) FOnModify(modified);
  }
}
//---------------------------------------------------------------------------
/// Set current designed program
void __fastcall TCNCDesigner::SetProgram(TProgram* prog)
{
  Program = prog;
  CurrCod = 0;
  SetValues();
  ZoomAll();
}
//---------------------------------------------------------------------------
/// Set read only mode
void __fastcall TCNCDesigner::SetReadOnly(bool ro)
{
  read_only = ro;
  if(read_only) {
	DesignAction = DA_NONE;
	Render();
  }
}
//---------------------------------------------------------------------------
/// Set FOnSelValues handler
void __fastcall TCNCDesigner::SetValues()
{
  if(FOnSetValues) FOnSetValues(this);
}
//---------------------------------------------------------------------------
/// Update information labels
void __fastcall TCNCDesigner::UpdateLabels()
{
  LabelX->Caption = "X: " + String(RoundTo(curs_x, 0));
  LabelY->Caption = "Y: " + String(RoundTo(curs_y, 0));
  LabelS->Caption = "S: " + String(grid_size);
}
//---------------------------------------------------------------------------
/// View grid mode
void __fastcall TCNCDesigner::ViewGrid(bool view)
{
  grid = view;
  Render();
}
//---------------------------------------------------------------------------
/// View codes numbers mode
void __fastcall TCNCDesigner::ViewNumbers(bool view)
{
  numbers = view;
  Render();
}
//---------------------------------------------------------------------------
/// View ruler mode
void __fastcall TCNCDesigner::ViewRuler(bool view)
{
  ruler = view;
  Render();
}
//---------------------------------------------------------------------------
/// View DRV path mode
void __fastcall TCNCDesigner::ViewDrvPath(bool view)
{
  drv_path = view;
  Render();
}
//---------------------------------------------------------------------------
/// Zoom all
void __fastcall TCNCDesigner::ZoomAll()
{
 int x=0, y=0;
  if(Program && Program->Codes->Count) {
	for(int i=0; i<Program->Codes->Count; i++) {
	  TGCod *cod = (TGCod*)Program->Codes->Items[i];
	  if(cod->X > x) x = cod->X;
	  if(cod->Y > y) y = cod->Y;
	}
  }
  else {
    x = 3000;
	y = 3000;
  }

  x += 100;
  y += 100;

  int w = XPanel->Width;
  int h = XPanel->Height;

  double zoomx = (double)w / x;
  double zoomy = (double)h / y;
  zoom = zoomx < zoomy ? zoomx : zoomy;
  prev_zoom = zoom;

  offsetx = 50.0;
  offsety = 50.0;
  min_wx = -((double)w/2) / zoom + offsetx;
  min_wy = -((double)h/2) / zoom + offsety;
  max_wx = ((double)w/2) / zoom - offsetx;
  max_wy = ((double)h/2) / zoom - offsety;
  offsetx = min_wx;
  offsety = min_wy;
  ScrollX->Position = -offsetx;
  ScrollY->Position = offsety;

  Render();
  UpdateLabels();
}
//---------------------------------------------------------------------------
/// Zoom in
void __fastcall TCNCDesigner::ZoomIn()
{
  zoom = zoom * 1.2;
  Render();
}
//---------------------------------------------------------------------------
/// Zoom out
void __fastcall TCNCDesigner::ZoomOut()
{
  zoom = zoom / 1.2;
  Render();
}
//---------------------------------------------------------------------------
/// Form resize event handler
void __fastcall TCNCDesigner::FrameResize(TObject *Sender)
{
  glViewport(0,0,(GLsizei)XPanel->Width,(GLsizei)XPanel->Height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-(GLdouble)XPanel->Width/2,  (GLdouble)XPanel->Width/2,
		  -(GLdouble)XPanel->Height/2, (GLdouble)XPanel->Height/2,
		  -100.0f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glGetIntegerv(GL_VIEWPORT, VP); // get for select
  Render();
}
//---------------------------------------------------------------------------
/// Form mouse wheel event handler
void __fastcall TCNCDesigner::FrameMouseWheel(TObject *Sender,
      TShiftState Shift, int WheelDelta, TPoint &MousePos, bool &Handled)
{
  if(WheelDelta > 0 && zoom > 1000.0) return;
  if(WheelDelta < 0 && zoom < 0.001) return;

  TPoint p = XPanel->ScreenToClient(MousePos);

  prev_mx = mx;
  prev_my = my;
  wx = ((double)p.x - ((double)XPanel->Width/2)) / zoom;
  wy = - (((double)p.y - ((double)XPanel->Height/2)) + 1) / zoom;
  mx = wx - offsetx;
  my = wy - offsety;

  prev_zoom = zoom;

  if(WheelDelta > 0)
	zoom = zoom * 1.2;
  else
	zoom = zoom / 1.2;

  //----------------- calculate offset -------------------
  offsetx = (wx / (zoom/prev_zoom)) - mx;
  offsety = (wy / (zoom/prev_zoom)) - my;

  min_wx = -((double)XPanel->Width/2) / zoom + offsetx;
  min_wy = -((double)XPanel->Height/2) / zoom + offsety;
  max_wx = ((double)XPanel->Width/2) / zoom - offsetx;
  max_wy = ((double)XPanel->Height/2) / zoom - offsety;
  //------------------------------------------------------

  ScrollX->Position = -offsetx;
  ScrollY->Position = offsety;
  UpdateLabels();

  Render();
  Handled = true;
}
//---------------------------------------------------------------------------
/// XScroll scroll event handler
void __fastcall TCNCDesigner::ScrollXScroll(TObject *Sender,
      TScrollCode ScrollCode, int &ScrollPos)
{
  offsetx = -ScrollX->Position;
  Render();
  UpdateLabels();
}
//---------------------------------------------------------------------------
/// XScroll scroll event handler
void __fastcall TCNCDesigner::ScrollYScroll(TObject *Sender,
      TScrollCode ScrollCode, int &ScrollPos)
{
  offsety = ScrollY->Position;
  Render();
  UpdateLabels();
}
//---------------------------------------------------------------------------
/// XPanel mouse double click event handler
void __fastcall TCNCDesigner::XPanelDblClick(TObject *Sender)
{
	SelectStitch();
	DragCod = 0;
	DragMultCod = 0;
	CurrCod = 0;
	SetValues();
	UpdateLabels();
}
//---------------------------------------------------------------------------
/// XPanel mouse move event handler
void __fastcall TCNCDesigner::XPanelMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
  prev_mx = mx;
  prev_my = my;
  wx = ((double)X - ((double)XPanel->Width/2)) / zoom;
  wy = - (((double)Y - ((double)XPanel->Height/2)) + 1) / zoom;
  mx = wx - offsetx;
  my = wy - offsety;

  bool need_redraw = false;
  if(grid && DesignAction != DA_NONE) {
    double floor_x = floor(mx/grid_size)*grid_size;
    double ceil_x = ceil(mx/grid_size)*grid_size;
    double floor_y = floor(my/grid_size)*grid_size;
    double ceil_y = ceil(my/grid_size)*grid_size;
    double cx, cy;

    if((mx - floor_x) < (grid_size/2)) cx = floor_x;
    else cx = ceil_x;
    if(my - floor_y < grid_size/2) cy = floor_y;
    else cy = ceil_y;
    if(cx != curs_x || cy != curs_y) {
      need_redraw = true;
      curs_x = cx;
      curs_y = cy;
    }
  } else {
    curs_x = mx;
    curs_y = my;
	if(DesignAction != DA_NONE) need_redraw = true;
  }

  if(DesignAction != DA_NONE) {
	switch(DesignAction) {
      case DA_CW:
        break;
      case DA_CCW:
		break;
      case DA_DRV:
        break;
      case DA_LIN:
		break;
	  case DA_DRAG_COD: {
		if(drag_list.empty()) {   // added 18.02.2014
			if(need_redraw) Render();
			return;
		}
		TGCod *cod1 = drag_list[0];
		cod1->X = curs_x;
		cod1->Y = curs_y;
		cod1->UpdateOXY();
		if(cod1->Next) cod1->Next->UpdateOXY();
		need_redraw = true;
        break;
      }
	  case DA_DRAG_MULT_COD: {
        for(unsigned int i=0; i<drag_list.size(); i++) {
		  TGCod *cod = drag_list[i];
		  if(cod->ID > -1) {
			TGCod* old_cod = (TGCod*)Program->Codes->Items[cod->ID];
            cod->X = old_cod->X + curs_x - drag_mx;
            cod->Y = old_cod->Y + curs_y - drag_my;
          }
		  cod->UpdateOXY();
        }
		need_redraw = true;
        break;
	  }
      case DA_DRAG_SEL:
        need_redraw = true;
		break;
      default:
        return;
    }

	if(need_redraw) Render();
    return;
  }


  double delta = 5 / zoom;
  double dx = fabs(curs_x - drag_mx);
  double dy = fabs(curs_y - drag_my);

  if(Shift.Contains(ssLeft) && (dx > delta || dy > delta)) {
	int prev_action = DesignAction;
	if(DesignAction == DA_NONE) DesignAction = DA_DRAG_SEL;
	if(DragCod) DesignAction = DA_DRAG_COD;
	if(DragMultCod) DesignAction = DA_DRAG_MULT_COD;
	if(DesignAction != prev_action && FOnDesignAction)
	  FOnDesignAction(DesignAction);
  }

  if(need_redraw) Render();
  UpdateLabels();
}
//---------------------------------------------------------------------------
/// XPanel mouse button down event handler
void __fastcall TCNCDesigner::XPanelMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if(read_only) return;
  double x1, y1, x2, y2;
  HiddenEdit->SetFocus(); // for handling key events
  
  if(Button == mbLeft && DesignAction != DA_NONE) {
	TGCod *cod = 0;
	switch(DesignAction) {
      case DA_CW: {
        String s(insert_r);
        if(!InputQuery("Rysowanie ³uku", "Wpisz promieñ ³uku: ", s)) return;
        int r = StrToInt(s);
		cod = new TGCod(G_CW, Round(curs_x), Round(curs_y), r);
		insert_r = r;
        break;
      }
	  case DA_CCW: {
		String s(insert_r);
		if(!InputQuery("Rysowanie ³uku", "Wpisz promieñ ³uku: ", s)) return;
		int r = StrToInt(s);
		cod = new TGCod(G_CCW, Round(curs_x), Round(curs_y), r);
		insert_r = r;
		break;
	  }
      case DA_DRV:
		cod = new TGCod(G_DRV, Round(curs_x), Round(curs_y), 0);
        break;
      case DA_LIN:
		cod = new TGCod(G_LIN, Round(curs_x), Round(curs_y), 0);
		break;
      case DA_DRAG_COD:
      case DA_DRAG_MULT_COD:
        drop_mx = curs_x;
        drop_my = curs_y;
        DragDropCod();
        DragCod = 0;
        DragMultCod = 0;
		DesignAction = DA_NONE;
		if(FOnDesignAction) FOnDesignAction(DesignAction);
		Render();
        return;
      case DA_DRAG_SEL:
        drop_mx = mx;
        drop_my = my;
        x1 = drop_mx > drag_mx ? drag_mx : drop_mx;
        y1 = drop_my > drag_my ? drag_my : drop_my;
        x2 = drop_mx > drag_mx ? drop_mx : drag_mx;
        y2 = drop_my > drag_my ? drop_my : drag_my;
		SelectAt(x1, y1, x2, y2);
		if(FOnDesignAction) FOnDesignAction(DesignAction);
		DesignAction = DA_NONE;
		Render();
		return;
      default:
        return;
    }
	if(cod) {
	  Program->SaveUndo();
      AddCod(cod);
	  Render();
	  SetModified(true);
      return;
    }
  }

  if(Button == mbRight && DesignAction != DA_NONE) {
	DesignAction = DA_NONE;
	if(FOnDesignAction) FOnDesignAction(DesignAction);
	XPanel->Cursor = crCross;
	Render();
    return;
  }

  drag_mx = mx;
  drag_my = my;

  GLint hit;
  if(Button == mbLeft) {
	CurrCod = 0;
	hit = DoSelect(X, XPanel->Height-Y);
    if(hit != -1) {
	  TGCod *cod = (TGCod*)Program->Codes->Items[hit];
      CurrCod = cod;

      if(cod) {
        cod->State = csSelected;
        if(IsCodPointAt(cod, mx, my)) {
          DragCod = cod;
          DragMultCod = 0;
		  TGCod *cod1 = new TGCod(*cod);
		  TGCod *cod2 = cod->Next ? new TGCod(*cod->Next) : 0;
          cod1->Next = cod2;
          if(cod2) cod2->Prev = cod1;
          for(unsigned int i=0; i<drag_list.size(); i++)
            delete drag_list[i];
          drag_list.clear();
          drag_list.push_back(cod1);
          if(cod2) drag_list.push_back(cod2);
        }
        else if(GetSelectedCount() > 1) {
          DragCod = 0;
          DragMultCod = cod;
          for(unsigned int i=0; i<drag_list.size(); i++)
            delete drag_list[i];
          drag_list.clear();
          //---
		  TGCod *prev_sel = 0;
          for(int i=0; i<Program->Codes->Count; i++) {
			TGCod *cod = (TGCod*)Program->Codes->Items[i];
            cod->ID = i; // save cod identifier
            if(cod->State != csSelected) {
              if(prev_sel && cod == prev_sel->Next) {
				TGCod *cod2 = new TGCod(*cod);
                prev_sel->Next = cod2;
                cod2->Prev = prev_sel;
                cod2->ID = -1;
                drag_list.push_back(cod2);
              }
              continue;
            }
			TGCod *cod1 = new TGCod(*cod);
            cod1->State = csSelected | csDragged;
            if(prev_sel && prev_sel->Next && prev_sel->Next == cod) {
              prev_sel->Next = cod1;
              cod1->Prev = prev_sel;
            }
            drag_list.push_back(cod1);
            prev_sel = cod1;
          }
          //---
        }
        else {
          DragCod = 0;
		  DragMultCod = 0;
		  for(unsigned int i=0; i<drag_list.size(); i++)
            delete drag_list[i];
		  drag_list.clear();
        }
        Render();
      }
    }
	else {  // added 18.02.2014
		DragCod = 0;
		DragMultCod = 0;
	}
    SetValues();
  }

  UpdateLabels();
}
//---------------------------------------------------------------------------
/// XPanel start drag event handler
void __fastcall TCNCDesigner::XPanelStartDrag(TObject *Sender,
	  TDragObject *&DragObject)
{
  if(read_only) return;
  DesignAction = DA_DRAG_COD;
  if(FOnDesignAction) FOnDesignAction(DesignAction);
}
//---------------------------------------------------------------------------
/// HiddenEdit key down event handler
void __fastcall TCNCDesigner::HiddenEditKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
  if(read_only) return;
  switch(Key) {
    case VK_ESCAPE:
	  DeselectAll();
	  DragCod = 0;
	  DragMultCod = 0;
	  for(unsigned int i=0; i<drag_list.size(); i++)
		delete drag_list[i];
	  drag_list.clear();
      break;
	case VK_DELETE:
	  break;
	case VK_LEFT:
	case VK_RIGHT:
	case VK_UP:
	case VK_DOWN:
	  Program->SaveUndo();
	  for(int i=0; i<Program->Codes->Count; i++) {
		TGCod *cod = (TGCod*)Program->Codes->Items[i];
		if(cod->State == csSelected) {
		  int step = Shift.Contains(ssCtrl) ? 10 : 1;
		  switch(Key) {
			case VK_LEFT:  cod->X = (cod->X-step); break;
			case VK_RIGHT: cod->X = (cod->X+step); break;
			case VK_UP:    cod->Y = (cod->Y+step); break;
			case VK_DOWN:  cod->Y = (cod->Y-step); break;
		  }
		}
	  }
	  for(int i=0; i<Program->Codes->Count; i++) {
		TGCod *cod = (TGCod*)Program->Codes->Items[i];
		cod->UpdateOXY();
	  }
	  SetModified(true);
	  Render();
	  break;
  }

  SetValues();
}
//---------------------------------------------------------------------------
/// XPanel paint event handler
void __fastcall TCNCDesigner::XPanelPaint(TObject *Sender)
{
  Render();
}
//---------------------------------------------------------------------------

