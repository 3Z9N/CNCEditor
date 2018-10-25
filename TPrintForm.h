//---------------------------------------------------------------------------

#ifndef TPrintFormH
#define TPrintFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <ImgList.hpp>
#include <ToolWin.hpp>
#include <Dialogs.hpp>
//---------------------------------------------------------------------------
class TPrintForm : public TForm
{
__published:	// IDE-managed Components
        TPanel *Panel1;
        TToolBar *ToolBar1;
        TToolButton *ToolButton1;
        TImageList *ImageList1;
        TToolButton *ToolButton2;
        TPanel *Panel2;
        TPaintBox *Box;
	TToolButton *ToolButton3;
	TToolButton *ToolButton4;
	TSaveDialog *SaveDialog;
	TToolButton *ToolButton5;
	TPanel *Panel3;
	TLabeledEdit *EIlosc;
	TLabeledEdit *EOdleglosc;
	TLabeledEdit *ERozstaw;
        void __fastcall ToolButton1Click(TObject *Sender);
        void __fastcall FormResize(TObject *Sender);
        void __fastcall BoxPaint(TObject *Sender);
        void __fastcall ToolButton2Click(TObject *Sender);
	void __fastcall ToolButton3Click(TObject *Sender);
	void __fastcall EIloscChange(TObject *Sender);
	void __fastcall EOdlegloscChange(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ERozstawChange(TObject *Sender);
private:	// User declarations
public:		// User declarations
        __fastcall TPrintForm(TComponent* Owner);
        void __fastcall DrawArc(TCanvas *Canvas, double x1, double y1, double x2, double y2,
								double ox, double oy, double r, int dir);
		void __fastcall ExportToJPEG();
};
//---------------------------------------------------------------------------
extern PACKAGE TPrintForm *PrintForm;
//---------------------------------------------------------------------------
#endif
