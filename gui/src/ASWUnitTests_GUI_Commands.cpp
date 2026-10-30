/* **************************************************************************
ASWUnitTests_GUI_Commands.cpp
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
#include "ASWUnitTests_GUI_Commands.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
TGUICommandStates GUICommandStates(TGUICommandContext const& context)
{
    bool const idle = !context.Running;
    bool const canRun = idle && !context.RunAborted;
    bool const anyFailed = context.FailedCount > 0;

    TGUICommandStates states;

    states.About = idle;
    states.CollapseAll = context.TreeHasNodes;
    states.CommandLineOptions = idle;
    states.CopyDetails = context.HasSelection;
    states.Exit = true;
    states.ExpandAll = context.TreeHasNodes;
    states.ExportJUnitReport = idle && context.HasReport;
    states.FilterBox = idle;
    states.FocusFilter = idle;
    states.ResetLayout = true;
    states.RunFailed = canRun && anyFailed;
    states.RunSelected = canRun;
    states.SelectAll = idle;
    states.SelectFailed = idle && anyFailed;
    states.SelectNone = idle;
    states.ShuffleOrder = idle;
    states.ShuffleSeed = idle;
    states.Stop = context.Running;
    states.Theme = idle;

    return states;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
