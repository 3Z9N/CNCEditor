object CodeEditor: TCodeEditor
  Left = 0
  Top = 0
  Width = 451
  Height = 355
  TabOrder = 0
  TabStop = True
  object Panel1: TPanel
    Left = 0
    Top = 0
    Width = 451
    Height = 355
    Align = alClient
    BevelInner = bvLowered
    BevelOuter = bvNone
    Caption = 'Panel1'
    TabOrder = 0
    object Memo: TMemo
      Left = 1
      Top = 25
      Width = 449
      Height = 329
      Align = alClient
      BorderStyle = bsNone
      Font.Charset = EASTEUROPE_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Fixedsys'
      Font.Style = []
      ParentFont = False
      ReadOnly = True
      ScrollBars = ssBoth
      TabOrder = 0
      WordWrap = False
      ExplicitTop = 52
      ExplicitHeight = 302
    end
    object Panel2: TPanel
      Left = 1
      Top = 1
      Width = 449
      Height = 24
      Align = alTop
      Alignment = taRightJustify
      BevelEdges = [beBottom]
      BevelKind = bkFlat
      BevelOuter = bvNone
      TabOrder = 1
      object SpeedButton1: TSpeedButton
        Left = 145
        Top = 0
        Width = 108
        Height = 22
        Align = alLeft
        Caption = 'Kopiuj do schowka'
        Flat = True
        ExplicitHeight = 24
      end
      object Check20GM: TCheckBox
        Left = 0
        Top = 0
        Width = 145
        Height = 22
        Align = alLeft
        Caption = 'MITSUBISHI FX2-20GM'
        TabOrder = 0
        OnClick = Check20GMClick
        ExplicitHeight = 24
      end
    end
  end
end
