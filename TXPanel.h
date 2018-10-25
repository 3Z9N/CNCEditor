//---------------------------------------------------------------------------

#ifndef TXPanelH
#define TXPanelH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Classes.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class PACKAGE TXPanel : public TPanel
{
private:
protected:
    TNotifyEvent FOnPaint;
	virtual void __fastcall Paint();
public:
	__fastcall TXPanel(TComponent* Owner);
__published:
	__property TNotifyEvent OnPaint  = {read=FOnPaint,write=FOnPaint};
};
//---------------------------------------------------------------------------
#endif
