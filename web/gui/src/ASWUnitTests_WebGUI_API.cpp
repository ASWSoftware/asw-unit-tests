/* **************************************************************************
ASWUnitTests_WebGUI_API.cpp
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
#include "ASWUnitTests_WebGUI_API.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_View.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_WebGUI_JSON.h"
//---------------------------------------------------------------------------

namespace
{

/////////////////////////////////////////////////////////////////////////////
// TCommandInfo
//
// A command the page sends: its kind, its name and the form fields it takes
// besides "command".
/////////////////////////////////////////////////////////////////////////////
struct TCommandInfo
{
    // The fields' names, nullptr where it takes fewer than two.
    char const* Fields[2];
    ASWUnitTests::EWebGUICommand Kind;
    char const* Name;
};

//---------------------------------------------------------------------------

void CheckTestIndex(ASWUnitTests::TWebGUIState const& state, std::size_t index, char const* function);
std::string ChecksText(ASWUnitTests::TGUITestList const& testList,
    std::vector<ASWUnitTests::TGUITreeNode> const& nodes);
std::vector<TCommandInfo> const& CommandInfos();
std::nullopt_t Fail(std::string& error, std::string const& message);
std::string const* FindField(std::vector<ASWUnitTests::TWebGUIField> const& fields, std::string const& name);
bool ParseFlag(std::string const& text, bool& value);
std::string ThemeChoiceName(ASWUnitTests::EGUITheme theme);
void WriteCommandStates(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TGUICommandStates const& states);
void WriteFailureRow(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUIState const& state,
    std::size_t index);
void WriteLogLines(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUILog const& log, std::size_t from);
void WriteStateMembers(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUIState const& state);
void WriteTestEventMembers(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUIState const& state,
    std::size_t index, char const* type);
void WriteTreeMembers(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUIState const& state);

//---------------------------------------------------------------------------
/*
    CheckTestIndex

    Throws std::logic_error, naming 'function', if 'index' isn't one of the state's tests.
*/
void CheckTestIndex(ASWUnitTests::TWebGUIState const& state, std::size_t index, char const* function)
{
    if (index >= state.TestList.Count())
        throw std::logic_error(std::string(function) + ": " + std::to_string(index) + " isn't a test's index");
}
//---------------------------------------------------------------------------
/*
    ChecksText

    One digit per node of 'nodes', its TGUICheckState's value.
*/
std::string ChecksText(ASWUnitTests::TGUITestList const& testList,
    std::vector<ASWUnitTests::TGUITreeNode> const& nodes)
{
    std::string text;
    text.reserve(nodes.size());

    for (ASWUnitTests::TGUITreeNode const& node : nodes)
        text += static_cast<char>('0' + static_cast<int>(ASWUnitTests::GUINodeCheckState(testList, node.Ref)));

    return text;
}
//---------------------------------------------------------------------------
/*
    CommandInfos

    Every command the page sends, in EWebGUICommand's order.
*/
std::vector<TCommandInfo> const& CommandInfos()
{
    static std::vector<TCommandInfo> const infos = {
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::CopyDetails, "CopyDetails" },
        { { "name", nullptr }, ASWUnitTests::EWebGUICommand::DarkTheme, "DarkTheme" },
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::Exit, "Exit" },
        { { "text", nullptr }, ASWUnitTests::EWebGUICommand::Filter, "Filter" },
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::ResetLayout, "ResetLayout" },
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::RunFailed, "RunFailed" },
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::RunSelected, "RunSelected" },
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::SelectAll, "SelectAll" },
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::SelectFailed, "SelectFailed" },
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::SelectNone, "SelectNone" },
        { { "node", "checked" }, ASWUnitTests::EWebGUICommand::SetChecked, "SetChecked" },
        { { "enabled", nullptr }, ASWUnitTests::EWebGUICommand::ShuffleOrder, "ShuffleOrder" },
        { { "newSeedEachRun", "seed" }, ASWUnitTests::EWebGUICommand::ShuffleSeed, "ShuffleSeed" },
        { { nullptr, nullptr }, ASWUnitTests::EWebGUICommand::Stop, "Stop" },
        { { "theme", nullptr }, ASWUnitTests::EWebGUICommand::Theme, "Theme" },
    };

    return infos;
}
//---------------------------------------------------------------------------
/*
    Fail

    Sets 'error' to 'message' and returns std::nullopt, for ParseWebGUICommand() to return.
*/
std::nullopt_t Fail(std::string& error, std::string const& message)
{
    error = message;
    return std::nullopt;
}
//---------------------------------------------------------------------------
/*
    FindField

    The value of the field named 'name', or nullptr if there's none.
*/
std::string const* FindField(std::vector<ASWUnitTests::TWebGUIField> const& fields, std::string const& name)
{
    for (ASWUnitTests::TWebGUIField const& field : fields)
    {
        if (field.Name == name)
            return &field.Value;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
/*
    ParseFlag

    Whether 'text' is "1" or "0", as the page sends a check box's state, setting 'value' to which. Not a
    std::optional<bool>, which RAD Studio's 32-bit compiler can't construct from a bool.
*/
bool ParseFlag(std::string const& text, bool& value)
{
    if (text != "1" && text != "0")
        return false;

    value = text == "1";
    return true;
}
//---------------------------------------------------------------------------
/*
    ThemeChoiceName

    'theme' as SaveGUITheme() writes it.
*/
std::string ThemeChoiceName(ASWUnitTests::EGUITheme theme)
{
    switch (theme)
    {
        case ASWUnitTests::EGUITheme::Light:
            return "Light";

        case ASWUnitTests::EGUITheme::Dark:
            return "Dark";

        default:
            return "Windows";
    }
}
//---------------------------------------------------------------------------
/*
    WriteCommandStates

    An object with a member for each command, named as TGUICommandStates' members, in their order.
*/
void WriteCommandStates(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TGUICommandStates const& states)
{
    struct TFlag
    {
        char const* Name;
        bool ASWUnitTests::TGUICommandStates::* Member;
    };

    static TFlag const flags[] = {
        { "About", &ASWUnitTests::TGUICommandStates::About },
        { "CollapseAll", &ASWUnitTests::TGUICommandStates::CollapseAll },
        { "CommandLineOptions", &ASWUnitTests::TGUICommandStates::CommandLineOptions },
        { "CopyDetails", &ASWUnitTests::TGUICommandStates::CopyDetails },
        { "Exit", &ASWUnitTests::TGUICommandStates::Exit },
        { "ExpandAll", &ASWUnitTests::TGUICommandStates::ExpandAll },
        { "ExportJUnitReport", &ASWUnitTests::TGUICommandStates::ExportJUnitReport },
        { "FilterBox", &ASWUnitTests::TGUICommandStates::FilterBox },
        { "FocusFilter", &ASWUnitTests::TGUICommandStates::FocusFilter },
        { "ResetLayout", &ASWUnitTests::TGUICommandStates::ResetLayout },
        { "RunFailed", &ASWUnitTests::TGUICommandStates::RunFailed },
        { "RunSelected", &ASWUnitTests::TGUICommandStates::RunSelected },
        { "SelectAll", &ASWUnitTests::TGUICommandStates::SelectAll },
        { "SelectFailed", &ASWUnitTests::TGUICommandStates::SelectFailed },
        { "SelectNone", &ASWUnitTests::TGUICommandStates::SelectNone },
        { "ShuffleOrder", &ASWUnitTests::TGUICommandStates::ShuffleOrder },
        { "ShuffleSeed", &ASWUnitTests::TGUICommandStates::ShuffleSeed },
        { "Stop", &ASWUnitTests::TGUICommandStates::Stop },
        { "Theme", &ASWUnitTests::TGUICommandStates::Theme },
    };

    writer.BeginObject();

    for (TFlag const& flag : flags)
    {
        writer.Key(flag.Name);
        writer.Bool(states.*flag.Member);
    }

    writer.EndObject();
}
//---------------------------------------------------------------------------
/*
    WriteFailureRow

    The Failures & Skips row of the test at 'index', as an array: its id, its status's name, its full name and
    the first line of its message, as the Win32 GUI runner's columns show them.
*/
void WriteFailureRow(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUIState const& state,
    std::size_t index)
{
    ASWUnitTests::TTestId const& test = state.TestList.Test(index);
    ASWUnitTests::TGUITestResult const* const result = state.TestList.Result(index);

    writer.BeginArray();
    writer.String(ASWUnitTests::WebGUINodeId(ASWUnitTests::TGUINodeRef { index, ASWUnitTests::TGUINodeKind::Test }));
    writer.String(ASWUnitTests::StatusName(state.TestList.Status(index)));
    writer.String(test.GroupName + "." + test.TestName);
    writer.String(result != nullptr ? ASWUnitTests::GUIFirstLine(result->Record.Message) : std::string());
    writer.EndArray();
}
//---------------------------------------------------------------------------
/*
    WriteLogLines

    The log's lines from 'from' on, as an array of [text, kind] arrays.
*/
void WriteLogLines(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUILog const& log, std::size_t from)
{
    std::vector<std::string> const& lines = log.Lines();
    writer.BeginArray();

    for (std::size_t i = from; i < lines.size(); ++i)
    {
        writer.BeginArray();
        writer.String(lines[i]);
        writer.UInt(static_cast<unsigned int>(ASWUnitTests::ClassifyGUILogLine(lines[i])));
        writer.EndArray();
    }

    writer.EndArray();
}
//---------------------------------------------------------------------------
/*
    WriteStateMembers

    The state event's members (see WebGUIStateEvent()), into the object 'writer' is writing.
*/
void WriteStateMembers(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUIState const& state)
{
    writer.Key("commands");
    WriteCommandStates(writer, ASWUnitTests::WebGUICommandStates(state));

    writer.Key("panels");
    writer.BeginArray();

    for (std::string const& panel : ASWUnitTests::GUIStatusPanels(state.TestList, state.RunFinishedCount,
        state.RunTotalCount, state.ElapsedSeconds, state.Shuffle.StatusText(), state.StatusMessage))
    {
        writer.String(panel);
    }

    writer.EndArray();

    writer.Key("progress");
    writer.BeginObject();
    writer.Key("finished");
    writer.UInt(state.RunFinishedCount);
    writer.Key("total");
    writer.UInt(state.RunTotalCount);
    writer.Key("failed");
    writer.Bool(state.TestList.StatusCount(ASWUnitTests::TGUITestStatus::Failed) > 0);
    writer.EndObject();

    std::optional<unsigned int> const fixedSeed = state.Shuffle.FixedSeed();
    writer.Key("shuffle");
    writer.BeginObject();
    writer.Key("enabled");
    writer.Bool(state.Shuffle.Enabled());
    writer.Key("fixedSeed");

    if (fixedSeed.has_value())
        writer.UInt(*fixedSeed);
    else
        writer.Null();

    writer.Key("seedToShow");
    writer.UInt(state.Shuffle.SeedToShow());
    writer.EndObject();

    writer.Key("theme");
    writer.BeginObject();
    writer.Key("choice");
    writer.String(ThemeChoiceName(state.Theme));
    writer.Key("dark");
    writer.String(state.DarkTheme);
    writer.EndObject();
}
//---------------------------------------------------------------------------
/*
    WriteTestEventMembers

    The started event's members (see WebGUITestStartedEvent()) for the test at 'index', with 'type' as the
    event's type, into the object 'writer' is writing. The test's group and suites have nodes whenever it's
    visible, since a group or suite gets one for any visible test.
*/
void WriteTestEventMembers(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUIState const& state,
    std::size_t index, char const* type)
{
    ASWUnitTests::TGUITestList const& testList = state.TestList;
    std::vector<ASWUnitTests::TGUINodeRef> refs;

    if (testList.IsVisible(index))
    {
        std::string const& groupName = testList.Test(index).GroupName;
        std::vector<std::string> const& groupNames = testList.GroupNames();
        std::size_t const group = static_cast<std::size_t>(std::find(groupNames.begin(), groupNames.end(),
            groupName) - groupNames.begin());

        refs.push_back(ASWUnitTests::TGUINodeRef { index, ASWUnitTests::TGUINodeKind::Test });
        refs.push_back(ASWUnitTests::TGUINodeRef { group, ASWUnitTests::TGUINodeKind::Group });

        for (std::string suite = testList.GroupSuitePath(groupName); !suite.empty();
             suite = ASWUnitTests::ParentSuitePath(suite))
        {
            refs.push_back(ASWUnitTests::TGUINodeRef { testList.SuiteIndexOf(suite).value(),
                                                       ASWUnitTests::TGUINodeKind::Suite });
        }
    }

    writer.Key("type");
    writer.String(type);
    writer.Key("test");
    writer.String(ASWUnitTests::WebGUINodeId(ASWUnitTests::TGUINodeRef { index, ASWUnitTests::TGUINodeKind::Test }));
    writer.Key("nodes");
    writer.BeginObject();

    for (ASWUnitTests::TGUINodeRef const& ref : refs)
    {
        writer.Key(ASWUnitTests::WebGUINodeId(ref));
        writer.UInt(static_cast<unsigned int>(ASWUnitTests::GUINodeStatus(testList, ref)));
    }

    writer.EndObject();
}
//---------------------------------------------------------------------------
/*
    WriteTreeMembers

    The tree event's members (see WebGUITreeEvent()), into the object 'writer' is writing.
*/
void WriteTreeMembers(ASWUnitTests::TWebGUIJSONWriter& writer, ASWUnitTests::TWebGUIState const& state)
{
    std::vector<ASWUnitTests::TGUITreeNode> const nodes = ASWUnitTests::BuildGUITree(state.TestList);
    std::string statuses;
    statuses.reserve(nodes.size());

    writer.Key("filter");
    writer.String(state.FilterText);

    writer.Key("nodes");
    writer.BeginArray();

    for (ASWUnitTests::TGUITreeNode const& node : nodes)
    {
        writer.BeginObject();
        writer.Key("id");
        writer.String(ASWUnitTests::WebGUINodeId(node.Ref));
        writer.Key("parent");

        if (node.Parent.has_value())
            writer.String(ASWUnitTests::WebGUINodeId(nodes[*node.Parent].Ref));
        else
            writer.Null();

        writer.Key("text");
        writer.String(node.Text);
        writer.EndObject();

        statuses += static_cast<char>('0' + static_cast<int>(ASWUnitTests::GUINodeStatus(state.TestList, node.Ref)));
    }

    writer.EndArray();

    writer.Key("statuses");
    writer.String(statuses);
    writer.Key("checks");
    writer.String(ChecksText(state.TestList, nodes));
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
bool IsWebGUICommandEnabled(TGUICommandStates const& states, EWebGUICommand command)
{
    switch (command)
    {
        case EWebGUICommand::CopyDetails:
            return states.CopyDetails;

        case EWebGUICommand::DarkTheme:
        case EWebGUICommand::Theme:
            return states.Theme;

        case EWebGUICommand::Exit:
            return states.Exit;

        case EWebGUICommand::Filter:
            return states.FilterBox;

        case EWebGUICommand::ResetLayout:
            return states.ResetLayout;

        case EWebGUICommand::RunFailed:
            return states.RunFailed;

        case EWebGUICommand::RunSelected:
            return states.RunSelected;

        case EWebGUICommand::SelectAll:
        case EWebGUICommand::SetChecked:
            return states.SelectAll;

        case EWebGUICommand::SelectFailed:
            return states.SelectFailed;

        case EWebGUICommand::SelectNone:
            return states.SelectNone;

        case EWebGUICommand::ShuffleOrder:
            return states.ShuffleOrder;

        case EWebGUICommand::ShuffleSeed:
            return states.ShuffleSeed;

        case EWebGUICommand::Stop:
            return states.Stop;
    }

    return false;
}
//---------------------------------------------------------------------------
/*
    ParseWebGUICommand

    Strict, so a mistake in the page shows at once: every field a command takes is required, and no other may
    come with it.
*/
std::optional<TWebGUICommand> ParseWebGUICommand(std::vector<TWebGUIField> const& fields,
    TWebGUIState const& state, std::string& error)
{
    for (std::size_t i = 0; i < fields.size(); ++i)
    {
        for (std::size_t j = 0; j < i; ++j)
        {
            if (fields[j].Name == fields[i].Name)
                return Fail(error, "\"" + fields[i].Name + "\" given twice");
        }
    }

    std::string const* const name = FindField(fields, "command");

    if (name == nullptr)
        return Fail(error, "no \"command\" field");

    TCommandInfo const* info = nullptr;

    for (TCommandInfo const& candidate : CommandInfos())
    {
        if (*name == candidate.Name)
            info = &candidate;
    }

    if (info == nullptr)
        return Fail(error, "unknown command \"" + *name + "\"");

    for (TWebGUIField const& field : fields)
    {
        bool const taken = field.Name == "command" || (info->Fields[0] != nullptr && field.Name == info->Fields[0]) ||
            (info->Fields[1] != nullptr && field.Name == info->Fields[1]);

        if (!taken)
            return Fail(error, *name + " has no \"" + field.Name + "\" field");
    }

    for (char const* const fieldName : info->Fields)
    {
        if (fieldName != nullptr && FindField(fields, fieldName) == nullptr)
            return Fail(error, *name + " needs a \"" + fieldName + "\" field");
    }

    TWebGUICommand command;
    command.Kind = info->Kind;

    switch (info->Kind)
    {
        case EWebGUICommand::DarkTheme:
            command.Text = *FindField(fields, "name");

            if (command.Text.empty())
                return Fail(error, "DarkTheme's \"name\" is empty");

            break;

        case EWebGUICommand::Filter:
            command.Text = *FindField(fields, "text");
            break;

        case EWebGUICommand::SetChecked:
        {
            std::string const& nodeId = *FindField(fields, "node");
            std::optional<TGUINodeRef> const node = ParseWebGUINodeId(nodeId, state);

            if (!node.has_value())
                return Fail(error, "\"node\" isn't a node: \"" + nodeId + "\"");

            if (!ParseFlag(*FindField(fields, "checked"), command.Checked))
                return Fail(error, "\"checked\" isn't 0 or 1");

            command.Node = *node;
            break;
        }

        case EWebGUICommand::ShuffleOrder:
            if (!ParseFlag(*FindField(fields, "enabled"), command.Enabled))
                return Fail(error, "\"enabled\" isn't 0 or 1");

            break;

        case EWebGUICommand::ShuffleSeed:
            if (!ParseFlag(*FindField(fields, "newSeedEachRun"), command.NewSeedEachRun))
                return Fail(error, "\"newSeedEachRun\" isn't 0 or 1");

            command.Text = *FindField(fields, "seed");
            break;

        case EWebGUICommand::Theme:
        {
            std::string const& theme = *FindField(fields, "theme");
            bool found = false;

            for (EGUITheme const candidate : { EGUITheme::Windows, EGUITheme::Light, EGUITheme::Dark })
            {
                if (theme == ThemeChoiceName(candidate))
                {
                    command.Theme = candidate;
                    found = true;
                }
            }

            if (!found)
                return Fail(error, "\"theme\" isn't Windows, Light or Dark");

            break;
        }

        default:
            break;
    }

    return command;
}
//---------------------------------------------------------------------------
/*
    ParseWebGUINodeId

    The index stops growing as soon as it reaches the count, so a long number can't overflow.
*/
std::optional<TGUINodeRef> ParseWebGUINodeId(std::string const& id, TWebGUIState const& state)
{
    if (id.size() < 2 || (id[1] == '0' && id.size() > 2))
        return std::nullopt;

    TGUINodeRef ref;
    std::size_t count = 0;

    switch (id[0])
    {
        case 's':
            ref.Kind = TGUINodeKind::Suite;
            count = state.TestList.SuitePaths().size();
            break;

        case 'g':
            ref.Kind = TGUINodeKind::Group;
            count = state.TestList.GroupNames().size();
            break;

        case 't':
            ref.Kind = TGUINodeKind::Test;
            count = state.TestList.Count();
            break;

        default:
            return std::nullopt;
    }

    for (std::size_t i = 1; i < id.size(); ++i)
    {
        if (id[i] < '0' || id[i] > '9')
            return std::nullopt;

        ref.Index = ref.Index * 10 + static_cast<std::size_t>(id[i] - '0');

        if (ref.Index >= count)
            return std::nullopt;
    }

    return ref;
}
//---------------------------------------------------------------------------
std::string WebGUIChecksEvent(TWebGUIState const& state)
{
    TWebGUIJSONWriter writer;
    writer.BeginObject();
    writer.Key("type");
    writer.String("checks");
    writer.Key("checks");
    writer.String(ChecksText(state.TestList, BuildGUITree(state.TestList)));
    writer.EndObject();
    return writer.Text();
}
//---------------------------------------------------------------------------
std::string WebGUICommandName(EWebGUICommand command)
{
    for (TCommandInfo const& info : CommandInfos())
    {
        if (info.Kind == command)
            return info.Name;
    }

    return std::string();
}
//---------------------------------------------------------------------------
/*
    WebGUICommandStates

    The test tree has nodes exactly when a test is visible, since BuildGUITree() gives a group or suite a node
    only for its visible tests.
*/
TGUICommandStates WebGUICommandStates(TWebGUIState const& state)
{
    TGUICommandContext context;
    context.FailedCount = state.TestList.StatusCount(TGUITestStatus::Failed);
    context.HasReport = state.HasReport;
    context.HasSelection = true;
    context.RunAborted = state.RunAborted;
    context.Running = state.Running;
    context.TreeHasNodes = state.TestList.VisibleCount() > 0;
    return GUICommandStates(context);
}
//---------------------------------------------------------------------------
std::string WebGUIDetailJSON(TWebGUIState const& state, TGUINodeRef const& ref)
{
    TWebGUIJSONWriter writer;
    writer.BeginObject();
    writer.Key("node");
    writer.String(WebGUINodeId(ref));
    writer.Key("text");
    writer.String(GUINodeDetailText(state.TestList, ref));
    writer.EndObject();
    return writer.Text();
}
//---------------------------------------------------------------------------
std::string WebGUILogEvent(TWebGUIState const& state, std::size_t from)
{
    if (from > state.Log.Lines().size())
        throw std::logic_error("WebGUILogEvent: line " + std::to_string(from) + " is past the log's end");

    TWebGUIJSONWriter writer;
    writer.BeginObject();
    writer.Key("type");
    writer.String("log");
    writer.Key("from");
    writer.UInt(from);
    writer.Key("lines");
    WriteLogLines(writer, state.Log, from);
    writer.EndObject();
    return writer.Text();
}
//---------------------------------------------------------------------------
std::string WebGUINodeId(TGUINodeRef const& ref)
{
    char const letter = ref.Kind == TGUINodeKind::Suite ? 's' : ref.Kind == TGUINodeKind::Group ? 'g' : 't';
    return letter + std::to_string(ref.Index);
}
//---------------------------------------------------------------------------
std::string WebGUISnapshotEvent(TWebGUIState const& state)
{
    TWebGUIJSONWriter writer;
    writer.BeginObject();
    writer.Key("type");
    writer.String("snapshot");
    writer.Key("caption");
    writer.String(GUICaption(state.ProjectName));
    WriteTreeMembers(writer, state);
    WriteStateMembers(writer, state);
    writer.Key("log");
    WriteLogLines(writer, state.Log, 0);

    writer.Key("failures");
    writer.BeginArray();

    for (std::size_t const index : state.FailureOrder)
    {
        CheckTestIndex(state, index, "WebGUISnapshotEvent");
        WriteFailureRow(writer, state, index);
    }

    writer.EndArray();
    writer.EndObject();
    return writer.Text();
}
//---------------------------------------------------------------------------
std::string WebGUIStateEvent(TWebGUIState const& state)
{
    TWebGUIJSONWriter writer;
    writer.BeginObject();
    writer.Key("type");
    writer.String("state");
    WriteStateMembers(writer, state);
    writer.EndObject();
    return writer.Text();
}
//---------------------------------------------------------------------------
std::string WebGUITestFinishedEvent(TWebGUIState const& state, std::size_t index)
{
    CheckTestIndex(state, index, "WebGUITestFinishedEvent");

    TGUITestStatus const status = state.TestList.Status(index);
    TWebGUIJSONWriter writer;
    writer.BeginObject();
    WriteTestEventMembers(writer, state, index, "finished");

    if (status == TGUITestStatus::Failed || status == TGUITestStatus::Skipped)
    {
        writer.Key("failure");
        WriteFailureRow(writer, state, index);
    }

    writer.EndObject();
    return writer.Text();
}
//---------------------------------------------------------------------------
std::string WebGUITestStartedEvent(TWebGUIState const& state, std::size_t index)
{
    CheckTestIndex(state, index, "WebGUITestStartedEvent");

    TWebGUIJSONWriter writer;
    writer.BeginObject();
    WriteTestEventMembers(writer, state, index, "started");
    writer.EndObject();
    return writer.Text();
}
//---------------------------------------------------------------------------
std::string WebGUITreeEvent(TWebGUIState const& state)
{
    TWebGUIJSONWriter writer;
    writer.BeginObject();
    writer.Key("type");
    writer.String("tree");
    WriteTreeMembers(writer, state);
    writer.EndObject();
    return writer.Text();
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWebGUILog
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TWebGUILog::TWebGUILog()
    : m_LastLineOpen(false)
{
}
//---------------------------------------------------------------------------
void TWebGUILog::Append(std::string const& text)
{
    if (text.empty())
        return;

    std::size_t const first = m_LastLineOpen ? m_Lines.size() - 1 : m_Lines.size();

    if (!m_FirstUnsentLine.has_value() || first < *m_FirstUnsentLine)
        m_FirstUnsentLine = first;

    std::size_t start = 0;

    while (start < text.size())
    {
        std::size_t const end = text.find('\n', start);
        std::string const piece = text.substr(start, end == std::string::npos ? std::string::npos : end - start);

        if (m_LastLineOpen)
            m_Lines.back() += piece;
        else
            m_Lines.push_back(piece);

        m_LastLineOpen = end == std::string::npos;
        start = end == std::string::npos ? text.size() : end + 1;
    }
}
//---------------------------------------------------------------------------
void TWebGUILog::Clear()
{
    m_Lines.clear();
    m_LastLineOpen = false;
    m_FirstUnsentLine = 0;
}
//---------------------------------------------------------------------------
std::optional<std::size_t> TWebGUILog::FirstUnsentLine() const
{
    return m_FirstUnsentLine;
}
//---------------------------------------------------------------------------
std::vector<std::string> const& TWebGUILog::Lines() const
{
    return m_Lines;
}
//---------------------------------------------------------------------------
void TWebGUILog::MarkSent()
{
    m_FirstUnsentLine.reset();
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
