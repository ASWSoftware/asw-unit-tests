/* **************************************************************************
ASWUnitTests_GUI_ShuffleSeedDialog.h
Author: Anthony S. West - ASW Software

The VCL GUI runner's Options > Shuffle Seed... dialog.

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
#ifndef ASWUnitTests_GUI_ShuffleSeedDialogH
#define ASWUnitTests_GUI_ShuffleSeedDialogH
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Shuffle.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

// Shows a modal dialog for choosing between a new random seed for each shuffled run and one seed for every run,
// starting from 'shuffle's current choice. Clicking OK applies the choice to 'shuffle', which also turns
// shuffling on, and returns true; Cancel changes nothing and returns false.
bool ShowShuffleSeedDialog(TGUIShuffle& shuffle);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_ShuffleSeedDialogH
