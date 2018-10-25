//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("TMainForm.cpp", MainForm);
USEFORM("TCNCDesigner.cpp", CNCDesigner); /* TFrame: File Type */
USEFORM("TOptionsForm.cpp", OptionsForm);
USEFORM("TCodeEditor.cpp", CodeEditor); /* TFrame: File Type */
USEFORM("TPrintForm.cpp", PrintForm);
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	try
	{
		Application->Initialize();
		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TOptionsForm), &OptionsForm);
		Application->CreateForm(__classid(TPrintForm), &PrintForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
