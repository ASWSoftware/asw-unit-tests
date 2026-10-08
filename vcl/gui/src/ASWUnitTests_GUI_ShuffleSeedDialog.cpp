/* **************************************************************************
ASWUnitTests_GUI_ShuffleSeedDialog.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 ASW Software

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************** */

//---------------------------------------------------------------------------
// Module header
#include "ASWUnitTests_GUI_ShuffleSeedDialog.h"
//---------------------------------------------------------------------------
#include <memory>
#include <optional>
#include <string>
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIShuffleSeedForm
//
// The dialog ShowShuffleSeedDialog() shows. Built in code rather than from
// a .dfm, like ShowTextDialog()'s, since it's this simple.
/////////////////////////////////////////////////////////////////////////////
class TGUIShuffleSeedForm : public Vcl::Forms::TForm
{
private:
    typedef Vcl::Forms::TForm inherited;

private:
    Vcl::Stdctrls::TEdit* Edt_Seed;
    TGUIShuffle* const m_Shuffle;
    Vcl::Stdctrls::TRadioButton* RB_FixedSeed;
    Vcl::Stdctrls::TRadioButton* RB_NewSeed;

private:
    void __fastcall Btn_OKClick(System::TObject* Sender);
    void __fastcall Edt_SeedChange(System::TObject* Sender);

public:
    explicit __fastcall TGUIShuffleSeedForm(TGUIShuffle& shuffle);
};

//---------------------------------------------------------------------------
__fastcall TGUIShuffleSeedForm::TGUIShuffleSeedForm(TGUIShuffle& shuffle)
    : inherited(static_cast<System::Classes::TComponent*>(nullptr), 0), // CreateNew: a form with no .dfm.
      m_Shuffle(&shuffle)
{
    Caption = "Shuffle Seed";
    BorderStyle = Vcl::Forms::bsDialog;
    Position = Vcl::Forms::poMainFormCenter;

    int const margin = ScaleValue(12);
    int const width = ScaleValue(420);
    ClientWidth = width;

    Vcl::Stdctrls::TLabel* const introLabel = new Vcl::Stdctrls::TLabel(this);
    introLabel->Parent = this;
    introLabel->AutoSize = true;
    introLabel->WordWrap = true;
    introLabel->SetBounds(margin, margin, width - 2 * margin, ScaleValue(15));
    introLabel->Caption = "A shuffled run runs the groups, and each group's tests, in an order its seed decides, "
        "so running again with the same seed repeats that order. Clicking OK also turns on Run in Shuffled Order.";

    int top = introLabel->Top + introLabel->Height + margin;

    RB_NewSeed = new Vcl::Stdctrls::TRadioButton(this);
    RB_NewSeed->Parent = this;
    RB_NewSeed->Caption = "Use a &new random seed for each run";
    RB_NewSeed->SetBounds(margin, top, width - 2 * margin, ScaleValue(20));
    top += ScaleValue(28);

    RB_FixedSeed = new Vcl::Stdctrls::TRadioButton(this);
    RB_FixedSeed->Parent = this;
    RB_FixedSeed->Caption = "Use the same &seed for every run:";
    RB_FixedSeed->SetBounds(margin, top, ScaleValue(220), ScaleValue(20));

    Edt_Seed = new Vcl::Stdctrls::TEdit(this);
    Edt_Seed->Parent = this;
    Edt_Seed->NumbersOnly = true;
    Edt_Seed->MaxLength = 10; // 4294967295, the largest seed.
    Edt_Seed->SetBounds(RB_FixedSeed->Left + RB_FixedSeed->Width + ScaleValue(8), top - ScaleValue(2),
        ScaleValue(120), ScaleValue(23));
    top += ScaleValue(32);

    std::optional<unsigned int> const lastSeed = m_Shuffle->LastSeed();
    Vcl::Stdctrls::TLabel* const lastSeedLabel = new Vcl::Stdctrls::TLabel(this);
    lastSeedLabel->Parent = this;
    lastSeedLabel->Left = margin;
    lastSeedLabel->Top = top;
    lastSeedLabel->Caption = lastSeed.has_value() ?
            FromUTF8("The latest shuffled run's seed was " + std::to_string(*lastSeed) + ".") :
            System::UnicodeString("No run has been shuffled yet.");
    top += lastSeedLabel->Height + margin + ScaleValue(4);

    int const buttonWidth = ScaleValue(80);
    int const buttonHeight = ScaleValue(26);

    Vcl::Stdctrls::TButton* const cancelButton = new Vcl::Stdctrls::TButton(this);
    cancelButton->Parent = this;
    cancelButton->Caption = "Cancel";
    cancelButton->Cancel = true;
    cancelButton->ModalResult = System::Uitypes::mrCancel;
    cancelButton->SetBounds(width - margin - buttonWidth, top, buttonWidth, buttonHeight);

    // Its ModalResult is set by Btn_OKClick(), once the seed has been checked.
    Vcl::Stdctrls::TButton* const okButton = new Vcl::Stdctrls::TButton(this);
    okButton->Parent = this;
    okButton->Caption = "OK";
    okButton->Default = true;
    okButton->OnClick = Btn_OKClick;
    okButton->SetBounds(cancelButton->Left - ScaleValue(8) - buttonWidth, top, buttonWidth, buttonHeight);

    ClientHeight = top + buttonHeight + margin;

    // Set before OnChange is, so only the user's own typing selects RB_FixedSeed.
    Edt_Seed->Text = FromUTF8(std::to_string(m_Shuffle->SeedToShow()));
    Edt_Seed->OnChange = Edt_SeedChange;

    bool const fixedSeed = m_Shuffle->FixedSeed().has_value();
    RB_FixedSeed->Checked = fixedSeed;
    RB_NewSeed->Checked = !fixedSeed;

    if (fixedSeed)
        ActiveControl = Edt_Seed;
    else
        ActiveControl = RB_NewSeed;
}
//---------------------------------------------------------------------------
void __fastcall TGUIShuffleSeedForm::Btn_OKClick(System::TObject* /*Sender*/)
{
    std::string error;

    if (!m_Shuffle->ApplySeedChoice(RB_NewSeed->Checked, ToUTF8(Edt_Seed->Text), error))
    {
        Application->MessageBox(FromUTF8(error).c_str(), Caption.c_str(), MB_OK | MB_ICONWARNING);
        Edt_Seed->SetFocus();
        Edt_Seed->SelectAll();

        return;
    }

    ModalResult = System::Uitypes::mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TGUIShuffleSeedForm::Edt_SeedChange(System::TObject* /*Sender*/)
{
    RB_FixedSeed->Checked = true;
}
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
bool ShowShuffleSeedDialog(TGUIShuffle& shuffle)
{
    std::unique_ptr<TGUIShuffleSeedForm> const form(new TGUIShuffleSeedForm(shuffle));

    return form->ShowModal() == System::Uitypes::mrOk;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
