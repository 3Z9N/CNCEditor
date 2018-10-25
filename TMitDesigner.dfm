object MitDesigner: TMitDesigner
  Left = 0
  Top = 0
  Width = 504
  Height = 388
  TabOrder = 0
  TabStop = True
  object PanelX: TPanel
    Left = 0
    Top = 370
    Width = 504
    Height = 18
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitLeft = -29
    ExplicitTop = 367
    ExplicitWidth = 538
    object Panel2: TPanel
      Left = 0
      Top = 0
      Width = 153
      Height = 18
      Align = alLeft
      BevelOuter = bvLowered
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      object LabelX: TLabel
        Left = 6
        Top = 2
        Width = 45
        Height = 13
        AutoSize = False
        Caption = 'X: 5000'
        Transparent = True
      end
      object LabelY: TLabel
        Left = 58
        Top = 2
        Width = 48
        Height = 13
        AutoSize = False
        Caption = 'Y: 5000'
        Transparent = True
      end
      object LabelS: TLabel
        Left = 110
        Top = 2
        Width = 37
        Height = 13
        AutoSize = False
        Caption = 'S: 1000'
        Transparent = True
      end
    end
    object Panel4: TPanel
      Left = 487
      Top = 0
      Width = 17
      Height = 18
      Align = alRight
      BevelOuter = bvNone
      TabOrder = 1
      ExplicitLeft = 521
    end
    object ScrollX: TScrollBar
      Left = 153
      Top = 0
      Width = 334
      Height = 18
      Align = alClient
      LargeChange = 10
      Max = 3000
      Min = -3000
      PageSize = 0
      TabOrder = 2
      TabStop = False
      ExplicitWidth = 368
    end
  end
  object PanelY: TPanel
    Left = 486
    Top = 0
    Width = 18
    Height = 370
    Align = alRight
    BevelOuter = bvNone
    TabOrder = 1
    ExplicitLeft = 491
    ExplicitHeight = 382
    object ScrollY: TScrollBar
      Left = 0
      Top = 0
      Width = 18
      Height = 370
      Align = alClient
      Kind = sbVertical
      LargeChange = 10
      Max = 3000
      Min = -3000
      PageSize = 0
      TabOrder = 0
      TabStop = False
      ExplicitHeight = 382
    end
  end
  object GLPanel: TPanel
    Left = 0
    Top = 0
    Width = 486
    Height = 370
    Align = alClient
    BevelOuter = bvNone
    Color = clBlack
    TabOrder = 2
    ExplicitLeft = 96
    ExplicitTop = 96
    ExplicitWidth = 185
    ExplicitHeight = 41
  end
end
