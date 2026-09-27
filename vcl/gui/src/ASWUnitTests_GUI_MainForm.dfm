object FormASWUnitTestsGUIMain: TFormASWUnitTestsGUIMain
  Left = 0
  Top = 0
  Caption = 'ASWUnitTests'
  ClientHeight = 661
  ClientWidth = 1008
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -12
  Font.Name = 'Segoe UI'
  Font.Style = []
  Menu = MM_Main
  Position = poDesigned
  ShowHint = True
  OnClose = FormClose
  OnCloseQuery = FormCloseQuery
  TextHeight = 15
  object Spl_Tests: TSplitter
    Left = 380
    Top = 42
    Width = 5
    Height = 580
    MinSize = 150
    ResizeStyle = rsUpdate
  end
  object Pnl_Toolbar: TPanel
    Left = 0
    Top = 0
    Width = 1008
    Height = 42
    Align = alTop
    BevelOuter = bvNone
    ShowCaption = False
    TabOrder = 0
    object Lbl_Filter: TLabel
      Left = 666
      Top = 13
      Width = 29
      Height = 15
      Caption = 'F&ilter:'
      FocusControl = Edt_Filter
    end
    object Btn_RunSelected: TButton
      Left = 8
      Top = 8
      Width = 110
      Height = 26
      Action = Act_RunSelected
      Caption = 'R&un Selected'
      TabOrder = 0
    end
    object Btn_RunFailed: TButton
      Left = 124
      Top = 8
      Width = 100
      Height = 26
      Action = Act_RunFailed
      Caption = 'Run Faile&d'
      TabOrder = 1
    end
    object Btn_Stop: TButton
      Left = 230
      Top = 8
      Width = 80
      Height = 26
      Action = Act_Stop
      TabOrder = 2
    end
    object Btn_SelectAll: TButton
      Left = 328
      Top = 8
      Width = 90
      Height = 26
      Action = Act_SelectAll
      TabOrder = 3
    end
    object Btn_SelectNone: TButton
      Left = 424
      Top = 8
      Width = 90
      Height = 26
      Action = Act_SelectNone
      TabOrder = 4
    end
    object Btn_CopyDetails: TButton
      Left = 532
      Top = 8
      Width = 110
      Height = 26
      Action = Act_CopyDetails
      TabOrder = 5
    end
    object Edt_Filter: TEdit
      Left = 704
      Top = 9
      Width = 240
      Height = 23
      Hint = 
        'Show only tests whose "Group.Test" name contains this text, whic' +
        'h may use the * and ? wildcards of --filter; not case-sensitive'
      TabOrder = 6
      TextHint = 'e.g. String or *Handler*.Run_*'
      OnChange = Edt_FilterChange
    end
  end
  object TV_Tests: TTreeView
    Left = 0
    Top = 42
    Width = 380
    Height = 580
    Align = alLeft
    CheckBoxes = True
    CheckStyles = [csPartial]
    HideSelection = False
    Images = IL_Status
    Indent = 19
    ReadOnly = True
    TabOrder = 1
    OnChange = TV_TestsChange
    OnCheckStateChanging = TV_TestsCheckStateChanging
  end
  object Pnl_Results: TPanel
    Left = 385
    Top = 42
    Width = 623
    Height = 580
    Align = alClient
    BevelOuter = bvNone
    ShowCaption = False
    TabOrder = 2
    object Spl_Detail: TSplitter
      Left = 0
      Top = 170
      Width = 623
      Height = 5
      Cursor = crVSplit
      Align = alTop
      MinSize = 60
      ResizeStyle = rsUpdate
    end
    object Mem_Detail: TMemo
      Left = 0
      Top = 0
      Width = 623
      Height = 170
      Align = alTop
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Consolas'
      Font.Style = []
      ParentFont = False
      ReadOnly = True
      ScrollBars = ssVertical
      TabOrder = 0
    end
    object PC_Results: TPageControl
      Left = 0
      Top = 175
      Width = 623
      Height = 405
      ActivePage = TS_Failures
      Align = alClient
      TabOrder = 1
      object TS_Failures: TTabSheet
        Caption = 'Failures && Skips'
        object LV_Failures: TListView
          Left = 0
          Top = 0
          Width = 615
          Height = 375
          Align = alClient
          Columns = <
            item
              Caption = 'Result'
              Width = 70
            end
            item
              Caption = 'Test'
              Width = 240
            end
            item
              AutoSize = True
              Caption = 'Detail'
            end>
          ReadOnly = True
          RowSelect = True
          TabOrder = 0
          ViewStyle = vsReport
          OnCustomDrawItem = LV_FailuresCustomDrawItem
          OnDblClick = LV_FailuresDblClick
        end
      end
      object TS_Log: TTabSheet
        Caption = 'Log'
        ImageIndex = 1
        object RE_Log: TRichEdit
          Left = 0
          Top = 0
          Width = 615
          Height = 375
          Align = alClient
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = []
          ParentFont = False
          ReadOnly = True
          ScrollBars = ssBoth
          TabOrder = 0
          WordWrap = False
        end
      end
    end
  end
  object PB_Progress: TProgressBar
    Left = 0
    Top = 622
    Width = 1008
    Height = 20
    Align = alBottom
    TabOrder = 3
  end
  object SB_Status: TStatusBar
    Left = 0
    Top = 642
    Width = 1008
    Height = 19
    Panels = <
      item
        Width = 120
      end
      item
        Width = 100
      end
      item
        Width = 90
      end
      item
        Width = 90
      end
      item
        Width = 90
      end
      item
        Width = 70
      end
      item
        Width = 300
      end>
  end
  object AL_Main: TActionList
    Left = 40
    Top = 80
    object Act_RunSelected: TAction
      Caption = '&Run Selected'
      Hint = 'Run the checked tests that are shown (F9)'
      ShortCut = 120
      OnExecute = Act_RunSelectedExecute
    end
    object Act_Stop: TAction
      Caption = '&Stop'
      Enabled = False
      Hint = 'Stop the run after the current test'
      OnExecute = Act_StopExecute
    end
    object Act_SelectAll: TAction
      Caption = 'Select &All'
      Hint = 'Check every shown test'
      OnExecute = Act_SelectAllExecute
    end
    object Act_SelectNone: TAction
      Caption = 'Select &None'
      Hint = 'Uncheck every shown test'
      OnExecute = Act_SelectNoneExecute
    end
    object Act_RunFailed: TAction
      Caption = 'Run &Failed'
      Enabled = False
      Hint = 'Rerun the tests that failed in the last run'
      OnExecute = Act_RunFailedExecute
    end
    object Act_CopyDetails: TAction
      Caption = '&Copy Details'
      Enabled = False
      Hint = 'Copy the selected test'#39's details to the clipboard (Ctrl+Shift+C)'
      ShortCut = 24643
      OnExecute = Act_CopyDetailsExecute
    end
    object Act_Exit: TAction
      Caption = 'E&xit'
      Hint = 'Close the window (during a run, after the current test)'
      OnExecute = Act_ExitExecute
    end
    object Act_ExportJUnitReport: TAction
      Caption = '&Export JUnit Report...'
      Enabled = False
      Hint = 'Save the latest run'#39's results as a JUnit XML report'
      OnExecute = Act_ExportJUnitReportExecute
    end
    object Act_FocusFilter: TAction
      Caption = '&Filter'
      Hint = 'Move to the filter box (Ctrl+F)'
      ShortCut = 16454
      OnExecute = Act_FocusFilterExecute
    end
    object Act_CommandLineOptions: TAction
      Caption = '&Command Line Options...'
      Hint = 'Show the command line options the GUI accepts'
      OnExecute = Act_CommandLineOptionsExecute
    end
    object Act_About: TAction
      Caption = '&About...'
      Hint = 'Show the framework version'
      OnExecute = Act_AboutExecute
    end
    object Act_ResetLayout: TAction
      Caption = '&Reset Layout'
      Hint = 'Restore the default window size and panel sizes'
      OnExecute = Act_ResetLayoutExecute
    end
  end
  object IL_Status: TImageList
    Left = 120
    Top = 80
  end
  object MM_Main: TMainMenu
    Left = 200
    Top = 80
    object MI_File: TMenuItem
      Caption = '&File'
      object MI_FileExportJUnitReport: TMenuItem
        Action = Act_ExportJUnitReport
      end
      object MI_FileSeparator1: TMenuItem
        Caption = '-'
      end
      object MI_FileExit: TMenuItem
        Action = Act_Exit
      end
    end
    object MI_Run: TMenuItem
      Caption = '&Run'
      object MI_RunSelected: TMenuItem
        Action = Act_RunSelected
      end
      object MI_RunFailed: TMenuItem
        Action = Act_RunFailed
      end
      object MI_RunStop: TMenuItem
        Action = Act_Stop
      end
    end
    object MI_Tests: TMenuItem
      Caption = '&Tests'
      object MI_TestsSelectAll: TMenuItem
        Action = Act_SelectAll
      end
      object MI_TestsSelectNone: TMenuItem
        Action = Act_SelectNone
      end
      object MI_TestsFilter: TMenuItem
        Action = Act_FocusFilter
      end
      object MI_TestsSeparator1: TMenuItem
        Caption = '-'
      end
      object MI_TestsCopyDetails: TMenuItem
        Action = Act_CopyDetails
      end
    end
    object MI_View: TMenuItem
      Caption = '&View'
      object MI_ViewResetLayout: TMenuItem
        Action = Act_ResetLayout
      end
    end
    object MI_Help: TMenuItem
      Caption = '&Help'
      object MI_HelpCommandLineOptions: TMenuItem
        Action = Act_CommandLineOptions
      end
      object MI_HelpAbout: TMenuItem
        Action = Act_About
      end
    end
  end
  object SD_JUnitReport: TSaveDialog
    DefaultExt = 'xml'
    Filter = 'JUnit XML reports (*.xml)|*.xml|All files (*.*)|*.*'
    Options = [ofOverwritePrompt, ofHideReadOnly, ofPathMustExist, ofEnableSizing]
    Title = 'Export JUnit Report'
    Left = 280
    Top = 80
  end
end
