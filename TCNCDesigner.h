//---------------------------------------------------------------------------

#ifndef TCNCDesignerH
#define TCNCDesignerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include "TXPanel.h"
#include <gl\gl.h>
#include <gl\glu.h>

#include "TGCod.h"
#include "TProgram.h"
#define MAXSELECT 1000
enum TDesignAction {DA_NONE, DA_DRV, DA_LIN, DA_CW, DA_CCW,
					DA_DRAG_COD, DA_DRAG_MULT_COD, DA_DRAG_SEL, DA_ANIMATION};
enum TRotateAngle { RA_90, RA_180, RA_270 };
enum TMirrorDirection { MD_HORIZONTAL, MD_VERTICAL };
//typedef void __fastcall (__closure *TSetValuesNotify)(TGCod *cod);
typedef void __fastcall (__closure *TDesinActionEvent)(int);
typedef void __fastcall (__closure *TModifyEvent)(bool);
//---------------------------------------------------------------------------
class TGLFont2D
{
  public:
	TGLFont2D()    {}
	~TGLFont2D()   { glDeleteLists(ListBase,NumGylph); }
	int FirstGylph;
	int NumGylph;
	int ListBase;
};
//---------------------------------------------------------------------------
class TCNCDesigner : public TFrame
{
__published:	// IDE-managed Components
	TPanel *PanelX;
	TPanel *Panel2;
	TLabel *LabelX;
	TLabel *LabelY;
	TLabel *LabelS;
	TPanel *Panel4;
	TScrollBar *ScrollX;
	TPanel *PanelY;
	TScrollBar *ScrollY;
	TEdit *HiddenEdit;
	void __fastcall FrameResize(TObject *Sender);
	void __fastcall FrameMouseWheel(TObject *Sender, TShiftState Shift,
          int WheelDelta, TPoint &MousePos, bool &Handled);
	void __fastcall ScrollXScroll(TObject *Sender, TScrollCode ScrollCode,
          int &ScrollPos);
	void __fastcall ScrollYScroll(TObject *Sender, TScrollCode ScrollCode,
		  int &ScrollPos);
	void __fastcall XPanelDblClick(TObject *Sender);
	void __fastcall XPanelMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
	void __fastcall XPanelMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
	void __fastcall XPanelStartDrag(TObject *Sender, TDragObject *&DragObject);
	void __fastcall HiddenEditKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
	void __fastcall XPanelPaint(TObject *Sender);
private:	// User declarations
	double zoom;
	double prev_zoom;
	double mx;
	double my;
	double prev_mx;
	double prev_my;
	double wx;
	double wy;
	double max_wx;
	double max_wy;
	double min_wx;
	double min_wy;
	double drag_mx;
	double drag_my;
	double drop_mx;
	double drop_my;
	double curs_x;
	double curs_y;
	double offsetx;
	double offsety;
	bool start_drag;
	double grid_size;
	int insert_r;
	bool modified;
	bool read_only;
	bool antialiasing;
	
	// select mode
    GLuint SelectBuf[MAXSELECT];
	GLint VP[4];
	
	// animation
	//TCod* AnimCod;
	int AnimStep;
	bool AnimDone;
	double AnimX;
	double AnimY;

	static bool grid;
	static bool ruler;
	static bool numbers;
	static bool drv_path;

	// OpenGL
	HDC hdc;
	HGLRC hrc;
	TGLFont2D *GLFont2D;
	TGLFont2D * __fastcall CreateFont2D(TFont *Font,
										int FirstGylph,
										int NumGylph);

	int DesignAction;
	TGCod *DragCod;
	TGCod *DragMultCod;
	TQueue *UndoQueue;

public:		// User declarations
	__fastcall TCNCDesigner(TComponent* Owner);
	void __fastcall AddCod(TGCod *cod);
	void __fastcall CalculateOffset(double _mx , double _my, double _wx , double _wy);
	void __fastcall Copy();
	void __fastcall CopyProgramToClipboard(TProgram *Prog);
	void __fastcall Delete();
	void __fastcall DeleteCod(TGCod *cod);
	void __fastcall DeselectAll();
	GLint __fastcall DoSelect(int x, int y);
	void __fastcall DragDropCod();
	void __fastcall Draw2DText(AnsiString Text);
	void __fastcall Draw2DTextXY(double X, double Y, AnsiString Text);
	void __fastcall DrawArc(double x1, double y1, double x2, double y2,
							double ox, double oy, double r, int dir);
	void __fastcall DrawCodCCW(TGCod *gcod);
	void __fastcall DrawCodCW(TGCod *gcod);
	void __fastcall DrawCodDRV(TGCod *gcod);
	void __fastcall DrawCodLIN(TGCod *gcod);
	void __fastcall DrawDesignerScene(GLenum RenderMode);
	void __fastcall DrawDraggedCod();
	void __fastcall DrawFocusRect(double x1 , double y1, double x2 , double y2);
	void __fastcall DrawGrid();
	void __fastcall DrawLine(double x1, double y1, double x2 , double y2);
	void __fastcall DrawPoint(double x, double y);
	int __fastcall GetSelectedCount();
	TProgram* __fastcall GetProgramFromClipboard();
	void __fastcall InitOpenGL();
	bool __fastcall IsCodPointAt(TGCod *cod, double x, double y);
	void __fastcall MovePoint();
	void __fastcall ReverseStitch();
	void __fastcall Rotate(TRotateAngle Angle);
	void __fastcall RotateAngle();
	void __fastcall Mirror(TMirrorDirection MDir);
	void __fastcall Paste();
	void __fastcall ReleaseOpenGL();
	void __fastcall Render();
	void __fastcall SelectAll();
	void __fastcall SelectAt(double left, double bottom,
							 double right, double top);
	void __fastcall SelectStitch();
	void __fastcall SetAntialiasing(bool aa);
	void __fastcall SetDesignAction(TDesignAction dAction);
	void __fastcall SetModified(bool modified);
	void __fastcall SetProgram(TProgram* prog);
	void __fastcall SetReadOnly(bool ro);
	void __fastcall UpdateLabels();
	void __fastcall ViewGrid(bool view);
	void __fastcall ViewNumbers(bool view);
	void __fastcall ViewRuler(bool view);
	void __fastcall ViewDrvPath(bool view);
	void __fastcall ZoomAll();
	void __fastcall ZoomIn();
	void __fastcall ZoomOut();

	TProgram *Program;
	TGCod *CurrCod;
	TXPanel *XPanel;

	float min_x;
	float min_y;
	float max_x;
	float max_y;
	void __fastcall MoveToZero();
    void __fastcall Rescale();

protected:
   TNotifyEvent FOnSetValues;
   TDesinActionEvent FOnDesignAction;
   TModifyEvent FOnModify;
	void __fastcall SetValues();
__published:
	__property TNotifyEvent OnSetValues  = {read=FOnSetValues,
											write=FOnSetValues};
	__property TDesinActionEvent OnDesignAction  = {read=FOnDesignAction,
													write=FOnDesignAction};
	__property TModifyEvent OnModify  = {read=FOnModify,write=FOnModify};
	__property bool Modified = {read=modified};
};
//---------------------------------------------------------------------------
extern PACKAGE TCNCDesigner *CNCDesigner;
//---------------------------------------------------------------------------
#endif
