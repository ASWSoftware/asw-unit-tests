/* **************************************************************************
ASWUnitTests_GUI_TextDialog.cpp
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
#include "ASWUnitTests_GUI_TextDialog.h"
//---------------------------------------------------------------------------
#include <memory>
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
void ShowTextDialog(System::UnicodeString const& caption, std::string const& text)
{
    // Built in code rather than from a .dfm, since it's this simple and is also shown before the main form
    // exists (e.g. for --help). CreateNew (the int overload) makes a form with no .dfm resource.
    std::unique_ptr<Vcl::Forms::TForm> const form(new Vcl::Forms::TForm(nullptr, 0));
    form->Caption = caption;
    form->BorderStyle = Vcl::Forms::bsSizeable;
    form->BorderIcons = Vcl::Forms::TBorderIcons() << Vcl::Forms::biSystemMenu;
    form->Position = Vcl::Forms::poScreenCenter;
    form->ClientWidth = form->ScaleValue(760);
    form->ClientHeight = form->ScaleValue(520);

    Vcl::Extctrls::TPanel* const buttonPanel = new Vcl::Extctrls::TPanel(form.get());
    buttonPanel->Parent = form.get();
    buttonPanel->Align = Vcl::Controls::alBottom;
    buttonPanel->BevelOuter = Vcl::Controls::bvNone;
    buttonPanel->Height = form->ScaleValue(44);

    Vcl::Stdctrls::TButton* const okButton = new Vcl::Stdctrls::TButton(form.get());
    okButton->Parent = buttonPanel;
    okButton->Caption = "OK";
    okButton->ModalResult = System::Uitypes::mrOk;
    okButton->Default = true;
    okButton->Cancel = true;
    okButton->Anchors = Vcl::Controls::TAnchors() << Vcl::Controls::akTop << Vcl::Controls::akRight;
    okButton->SetBounds(buttonPanel->ClientWidth - form->ScaleValue(96), form->ScaleValue(9), form->ScaleValue(84),
        form->ScaleValue(26));

    Vcl::Stdctrls::TMemo* const memo = new Vcl::Stdctrls::TMemo(form.get());
    memo->Parent = form.get();
    memo->Align = Vcl::Controls::alClient;
    memo->ReadOnly = true;
    memo->ScrollBars = Vcl::Stdctrls::ssBoth;
    memo->WordWrap = false;
    memo->Font->Name = "Consolas";
    memo->Font->Size = 10;
    memo->Text = System::Sysutils::AdjustLineBreaks(FromUTF8(text));

    form->ActiveControl = okButton;
    form->ShowModal();
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
