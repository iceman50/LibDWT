#include <dwt/Application.h>
#include <dwt/CanvasClasses.h>
#include <dwt/Message.h>
#include <dwt/WidgetCreator.h>

#include <dwt/resources/Brush.h>
#include <dwt/resources/Pen.h>

#include <dwt/widgets/Button.h>
#include <dwt/widgets/CheckBox.h>
#include <dwt/widgets/ComboBox.h>
#include <dwt/widgets/Container.h>
#include <dwt/widgets/DateTime.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/GroupBox.h>
#include <dwt/widgets/Header.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/Link.h>
#include <dwt/widgets/MonthCalendar.h>
#include <dwt/widgets/ProgressBar.h>
#include <dwt/widgets/RadioButton.h>
#include <dwt/widgets/Rebar.h>
#include <dwt/widgets/RichTextBox.h>
#include <dwt/widgets/ScrolledContainer.h>
#include <dwt/widgets/Slider.h>
#include <dwt/widgets/Spinner.h>
#include <dwt/widgets/SplitterContainer.h>
#include <dwt/widgets/StatusBar.h>
#include <dwt/widgets/Table.h>
#include <dwt/widgets/TableTree.h>
#include <dwt/widgets/TabView.h>
#include <dwt/widgets/TextBox.h>
#include <dwt/widgets/ToolBar.h>
#include <dwt/widgets/Tree.h>
#include <dwt/widgets/VirtualTree.h>
#include <dwt/widgets/Window.h>

#include <algorithm>
#include <array>
#include <functional>
#include <initializer_list>
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

using dwt::Brush;
using dwt::Button;
using dwt::CheckBox;
using dwt::ComboBox;
using dwt::Container;
using dwt::Control;
using dwt::DateTime;
using dwt::Grid;
using dwt::GridInfo;
using dwt::GroupBox;
using dwt::Header;
using dwt::Label;
using dwt::Link;
using dwt::MonthCalendar;
using dwt::PaintCanvas;
using dwt::Pen;
using dwt::ProgressBar;
using dwt::RadioButton;
using dwt::Rebar;
using dwt::RichTextBox;
using dwt::ScrolledContainer;
using dwt::Slider;
using dwt::Spinner;
using dwt::SplitterContainer;
using dwt::StatusBar;
using dwt::Table;
using dwt::TableTree;
using dwt::TabView;
using dwt::TextBox;
using dwt::ToolBar;
using dwt::Tree;
using dwt::VirtualTree;
using dwt::Widget;
using dwt::WidgetCreator;
using dwt::Window;

constexpr size_t designerRows = 4;
constexpr size_t designerColumns = 2;
constexpr size_t noSelection = (std::numeric_limits<size_t>::max)();

enum class ControlKind {
	Button,
	CheckBox,
	ComboBox,
	Container,
	DateTime,
	Grid,
	GroupBox,
	Header,
	Label,
	Link,
	MonthCalendar,
	ProgressBar,
	RadioButton,
	Rebar,
	RichTextBox,
	ScrolledContainer,
	Slider,
	Spinner,
	SplitterContainer,
	StatusBar,
	Table,
	TableTree,
	TabView,
	TextBox,
	ToolBar,
	Tree,
	VirtualTree,
	Count
};

constexpr size_t controlKindCount = static_cast<size_t>(ControlKind::Count);

struct ControlDescriptor {
	const TCHAR* name;
	const TCHAR* variable;
	const TCHAR* caption;
};

const std::array<ControlDescriptor, controlKindCount> controlDescriptors = {{
	{ _T("Button"), _T("button"), _T("New button") },
	{ _T("CheckBox"), _T("checkBox"), _T("New option") },
	{ _T("ComboBox"), _T("comboBox"), _T("First choice") },
	{ _T("Container"), _T("container"), _T("Container content") },
	{ _T("DateTime"), _T("dateTime"), _T("Date and time") },
	{ _T("Grid"), _T("grid"), _T("Nested grid") },
	{ _T("GroupBox"), _T("groupBox"), _T("New group") },
	{ _T("Header"), _T("header"), _T("Column heading") },
	{ _T("Label"), _T("label"), _T("New label") },
	{ _T("Link"), _T("link"), _T("https://dcplusplus.sourceforge.io/") },
	{ _T("MonthCalendar"), _T("monthCalendar"), _T("Calendar") },
	{ _T("ProgressBar"), _T("progressBar"), _T("Progress") },
	{ _T("RadioButton"), _T("radioButton"), _T("New choice") },
	{ _T("Rebar"), _T("rebar"), _T("Rebar band") },
	{ _T("RichTextBox"), _T("richTextBox"), _T("Rich text value") },
	{ _T("ScrolledContainer"), _T("scrolledContainer"), _T("Scrollable content") },
	{ _T("Slider"), _T("slider"), _T("Slider") },
	{ _T("Spinner"), _T("spinner"), _T("50") },
	{ _T("SplitterContainer"), _T("splitterContainer"), _T("Split panes") },
	{ _T("StatusBar"), _T("statusBar"), _T("Ready") },
	{ _T("Table"), _T("table"), _T("Table item") },
	{ _T("TableTree"), _T("tableTree"), _T("Hierarchy item") },
	{ _T("TabView"), _T("tabView"), _T("Page") },
	{ _T("TextBox"), _T("textBox"), _T("Text value") },
	{ _T("ToolBar"), _T("toolBar"), _T("Action") },
	{ _T("Tree"), _T("tree"), _T("Tree item") },
	{ _T("VirtualTree"), _T("virtualTree"), _T("Virtual item") }
}};

size_t kindIndex(ControlKind kind) {
	return static_cast<size_t>(kind);
}

const ControlDescriptor& descriptor(ControlKind kind) {
	return controlDescriptors[kindIndex(kind)];
}

const TCHAR* kindName(ControlKind kind) {
	return descriptor(kind).name;
}

const TCHAR* variableBase(ControlKind kind) {
	return descriptor(kind).variable;
}

dwt::tstring defaultCaption(ControlKind kind) {
	return descriptor(kind).caption;
}

const TCHAR* gridModeName(GridInfo::Modes mode) {
	switch(mode) {
	case GridInfo::STATIC: return _T("STATIC");
	case GridInfo::FILL: return _T("FILL");
	case GridInfo::AUTO: return _T("AUTO");
	}
	return _T("AUTO");
}

const TCHAR* gridAlignName(GridInfo::Align align) {
	switch(align) {
	case GridInfo::TOP_LEFT: return _T("TOP_LEFT");
	case GridInfo::BOTTOM_RIGHT: return _T("BOTTOM_RIGHT");
	case GridInfo::CENTER: return _T("CENTER");
	case GridInfo::STRETCH: return _T("STRETCH");
	}
	return _T("TOP_LEFT");
}

dwt::tstring quoteLiteral(const dwt::tstring& value) {
	dwt::tstring escaped;
	escaped.reserve(value.size());
	for(auto character : value) {
		switch(character) {
		case _T('\\'): escaped += _T("\\\\"); break;
		case _T('"'): escaped += _T("\\\""); break;
		case _T('\r'): escaped += _T("\\r"); break;
		case _T('\n'): escaped += _T("\\n"); break;
		case _T('\t'): escaped += _T("\\t"); break;
		default: escaped += character; break;
		}
	}
	return escaped;
}

Control* createSimpleControl(Widget* parent, ControlKind kind,
	const dwt::tstring& caption, bool readOnlyText = false)
{
	switch(kind) {
	case ControlKind::Button:
		return WidgetCreator<Button>::create(parent, Button::Seed(caption));
	case ControlKind::CheckBox:
		return WidgetCreator<CheckBox>::create(parent, CheckBox::Seed(caption));
	case ControlKind::ComboBox: {
		auto* combo = WidgetCreator<ComboBox>::create(parent, ComboBox::Seed());
		combo->addValue(caption);
		combo->addValue(_T("Second choice"));
		combo->setSelected(0);
		return combo;
	}
	case ControlKind::Container: {
		auto* container = WidgetCreator<Container>::create(parent, Container::Seed());
		WidgetCreator<Label>::create(container, Label::Seed(caption));
		return container;
	}
	case ControlKind::DateTime:
		return WidgetCreator<DateTime>::create(parent, DateTime::Seed());
	case ControlKind::Grid: {
		auto* grid = WidgetCreator<Grid>::create(parent, Grid::Seed(1, 1));
		grid->row(0) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		grid->column(0) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		auto* label = WidgetCreator<Label>::create(grid, Label::Seed(caption));
		grid->setWidget(label, 0, 0);
		return grid;
	}
	case ControlKind::GroupBox: {
		auto* group = WidgetCreator<GroupBox>::create(parent, GroupBox::Seed(caption));
		group->addChild(Label::Seed(_T("Group content")));
		return group;
	}
	case ControlKind::Header: {
		auto* header = WidgetCreator<Header>::create(parent, Header::Seed());
		header->insert(caption, 180);
		return header;
	}
	case ControlKind::Label: {
		Label::Seed seed(caption);
		seed.style |= SS_CENTERIMAGE;
		return WidgetCreator<Label>::create(parent, seed);
	}
	case ControlKind::Link:
		return WidgetCreator<Link>::create(parent, Link::Seed(caption, true));
	case ControlKind::MonthCalendar:
		return WidgetCreator<MonthCalendar>::create(parent, MonthCalendar::Seed());
	case ControlKind::ProgressBar: {
		auto* progress = WidgetCreator<ProgressBar>::create(parent, ProgressBar::Seed());
		progress->setRange(0, 100);
		progress->setPosition(50);
		return progress;
	}
	case ControlKind::RadioButton:
		return WidgetCreator<RadioButton>::create(parent, RadioButton::Seed(caption));
	case ControlKind::Rebar: {
		auto* rebar = WidgetCreator<Rebar>::create(parent, Rebar::Seed());
		auto* toolBar = WidgetCreator<ToolBar>::create(rebar, ToolBar::Seed());
		toolBar->addButton("designer", -1, caption, true);
		toolBar->setLayout({ "designer" });
		rebar->add(toolBar, RBBS_CHILDEDGE, caption);
		return rebar;
	}
	case ControlKind::RichTextBox: {
		auto* richText = WidgetCreator<RichTextBox>::create(parent, RichTextBox::Seed());
		richText->setText(caption);
		richText->setReadOnly(readOnlyText);
		return richText;
	}
	case ControlKind::ScrolledContainer: {
		auto* scrolled = WidgetCreator<ScrolledContainer>::create(parent,
			ScrolledContainer::Seed());
		scrolled->addChild(Label::Seed(caption));
		return scrolled;
	}
	case ControlKind::Slider: {
		auto* slider = WidgetCreator<Slider>::create(parent, Slider::Seed());
		slider->setRange(0, 100);
		slider->setPosition(50);
		return slider;
	}
	case ControlKind::Spinner: {
		auto* spinner = WidgetCreator<Spinner>::create(parent, Spinner::Seed(0, 100));
		spinner->setValue(50);
		return spinner;
	}
	case ControlKind::SplitterContainer: {
		auto* splitter = WidgetCreator<SplitterContainer>::create(parent,
			SplitterContainer::Seed());
		auto* first = WidgetCreator<Container>::create(splitter, Container::Seed());
		WidgetCreator<Label>::create(first, Label::Seed(_T("Pane 1")));
		auto* second = WidgetCreator<Container>::create(splitter, Container::Seed());
		WidgetCreator<Label>::create(second, Label::Seed(_T("Pane 2")));
		return splitter;
	}
	case ControlKind::StatusBar: {
		auto* status = WidgetCreator<StatusBar>::create(parent, StatusBar::Seed());
		status->setText(0, caption);
		return status;
	}
	case ControlKind::Table: {
		auto* table = WidgetCreator<Table>::create(parent, Table::Seed());
		table->addColumn(_T("Item"), 180);
		table->insert({ caption }, 1);
		return table;
	}
	case ControlKind::TableTree: {
		auto* table = WidgetCreator<TableTree>::create(parent,
			TableTree::Seed(Table::Seed()));
		table->addColumn(_T("Hierarchy"), 180);
		table->insert({ caption }, 1);
		return table;
	}
	case ControlKind::TabView: {
		auto* tabs = WidgetCreator<TabView>::create(parent, TabView::Seed());
		auto* page = WidgetCreator<Container>::create(tabs, Container::Seed());
		page->setText(caption);
		WidgetCreator<Label>::create(page, Label::Seed(_T("Tab content")));
		tabs->add(page);
		return tabs;
	}
	case ControlKind::TextBox: {
		auto* textBox = WidgetCreator<TextBox>::create(parent, TextBox::Seed(caption));
		textBox->setReadOnly(readOnlyText);
		return textBox;
	}
	case ControlKind::ToolBar: {
		auto* toolBar = WidgetCreator<ToolBar>::create(parent, ToolBar::Seed());
		toolBar->addButton("designer", -1, caption, true);
		toolBar->setLayout({ "designer" });
		return toolBar;
	}
	case ControlKind::Tree: {
		auto* tree = WidgetCreator<Tree>::create(parent, Tree::Seed());
		tree->addColumn(_T("Item"), 180);
		tree->insert(caption, TVI_ROOT, TVI_LAST, 1, true);
		return tree;
	}
	case ControlKind::VirtualTree: {
		auto* tree = WidgetCreator<VirtualTree>::create(parent,
			VirtualTree::Seed(Tree::Seed()));
		tree->addColumn(_T("Virtual item"), 180);
		tree->insert(caption, TVI_ROOT, TVI_LAST, 1, true);
		return tree;
	}
	case ControlKind::Count:
		break;
	}
	return nullptr;
}

class DesignerTile : public Container {
public:
	typedef DesignerTile ThisType;
	typedef ThisType* ObjectType;

	struct Seed : public Container::Seed {
		typedef ThisType WidgetType;

		Seed(ControlKind kind_, const dwt::tstring& caption_) :
			Container::Seed(WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
				WS_EX_CONTROLPARENT),
			kind(kind_), caption(caption_)
		{
		}

		ControlKind kind;
		dwt::tstring caption;
	};

	void create(const Seed& seed) {
		kind = seed.kind;
		caption = seed.caption;
		Container::create(seed);
		setFont(nullptr);
		setAccessibleName(dwt::tstring(kindName(kind)) + _T(" designer item"));

		addCallback(dwt::Message(WM_ERASEBKGND), [](const MSG&, LRESULT& result) {
			result = 1;
			return true;
		});
		onPainting([this](PaintCanvas& canvas) { paint(canvas); });
		onLeftMouseDown([this](const dwt::MouseEvent&) {
			notifySelected();
			return false;
		});

		inner = createSimpleControl(this, kind, caption, true);
		if(inner) {
			inner->setAccessibleHelpText(_T("Select this control to edit its designer properties."));
			inner->onLeftMouseDown([this](const dwt::MouseEvent&) {
				notifySelected();
				return false;
			});
		}
		layout();
	}

	void setSelectionCallback(std::function<void ()> callback) {
		selectionCallback = std::move(callback);
	}

	void requestSelection() {
		notifySelected();
	}

	void setSelected(bool value) {
		if(selected != value) {
			selected = value;
			redraw(true);
		}
	}

	void setCaption(const dwt::tstring& value) {
		caption = value;
		if(inner) {
			inner->sendMessage(WM_SETTEXT, 0,
				reinterpret_cast<LPARAM>(caption.c_str()));
		}
	}

	ControlKind getKind() const {
		return kind;
	}

	dwt::Point getPreferredSize() override {
		return scale(dwt::Point(190, 92));
	}

	void layout() override {
		if(!inner) {
			return;
		}
		const auto client = getClientSize();
		const int side = scale(7);
		const int top = scale(27);
		const int bottom = scale(7);
		inner->resize(dwt::Rectangle(
			side,
			top,
			(std::max)(0L, client.x - side * 2L),
			(std::max)(0L, client.y - top - bottom)));
	}

protected:
	friend class dwt::WidgetCreator<DesignerTile>;

	explicit DesignerTile(Widget* parent) :
		Container(parent), kind(ControlKind::Button), inner(nullptr), selected(false)
	{
	}

private:
	void notifySelected() {
		if(selectionCallback) {
			selectionCallback();
		}
	}

	void paint(PaintCanvas& canvas) {
		dwt::Rectangle client(getClientSize());
		canvas.fill(client, Brush(::GetSysColor(COLOR_WINDOW)));

		Brush border(selected ? RGB(0, 120, 215) : ::GetSysColor(COLOR_3DSHADOW));
		canvas.frame(client, border);
		if(selected && client.width() > 2 && client.height() > 2) {
			canvas.frame(client.inflate(-1, -1), border);
		}

		dwt::Rectangle title(scale(7), scale(4),
			(std::max)(0L, client.width() - scale(14)), scale(18));
		auto font = canvas.select(*getFont());
		auto transparent = canvas.setBkMode(true);
		canvas.setTextColor(selected ? RGB(0, 90, 158) :
			::GetSysColor(COLOR_WINDOWTEXT));
		canvas.drawText(dwt::tstring(kindName(kind)) +
			(selected ? _T("  (selected)") : _T("")), title,
			DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
	}

	ControlKind kind;
	dwt::tstring caption;
	Control* inner;
	bool selected;
	std::function<void ()> selectionCallback;
};

struct DesignerItem {
	ControlKind kind;
	dwt::tstring caption;
	size_t row;
	size_t column;
	size_t rowSpan;
	size_t columnSpan;
	DesignerTile* tile;
};

struct DesignerSpec {
	ControlKind kind;
	dwt::tstring caption;
	size_t row;
	size_t column;
	size_t rowSpan;
	size_t columnSpan;
};

class DesignerController {
public:
	explicit DesignerController(Window* window_) :
		window(window_), rootGrid(nullptr), surfaceGroup(nullptr), designGrid(nullptr), codeOutput(nullptr),
		widgetPicker(nullptr), selectionLabel(nullptr), captionEditor(nullptr),
		rowEditor(nullptr), columnEditor(nullptr), rowSpanEditor(nullptr),
		columnSpanEditor(nullptr), rowModeEditor(nullptr), rowSizeEditor(nullptr),
		rowAlignEditor(nullptr), columnModeEditor(nullptr), columnSizeEditor(nullptr),
		columnAlignEditor(nullptr), applyButton(nullptr), removeButton(nullptr),
		statusLabel(nullptr), previewWindow(nullptr), selected(noSelection)
	{
		build();
	}

	void show() {
		layout();
		window->setVisible(true);
		window->setFocus();
	}

	int runSelfTest(dwt::Application& app) {
		int failure = 0;
		auto check = [&failure](bool condition, int code) {
			if(!condition && failure == 0) {
				failure = code;
			}
		};
		layout();
		app.processMessages();
		check(itemCount() == 4 && selected != noSelection, 1);
		const bool textAdded = addItem(ControlKind::TextBox, _T("Self-test value"));
		check(textAdded && itemCount() == 5 && selected != noSelection, 2);

		captionEditor->setText(_T("Edited by self-test"));
		rowEditor->setSelected(3);
		columnEditor->setSelected(0);
		rowSpanEditor->setSelected(0);
		columnSpanEditor->setSelected(1);
		rowModeEditor->setSelected(static_cast<int>(GridInfo::STATIC));
		rowSizeEditor->setText(_T("120"));
		rowAlignEditor->setSelected(static_cast<int>(GridInfo::CENTER));
		columnModeEditor->setSelected(static_cast<int>(GridInfo::AUTO));
		columnSizeEditor->setText(_T("0"));
		columnAlignEditor->setSelected(static_cast<int>(GridInfo::BOTTOM_RIGHT));
		applyProperties();
		check(selected < items.size() &&
			items[selected].caption == _T("Edited by self-test") &&
			items[selected].row == 3 && items[selected].column == 0 &&
			items[selected].rowSpan == 1 && items[selected].columnSpan == 2 &&
			rowSettings[3].mode == GridInfo::STATIC && rowSettings[3].size == 120 &&
			rowSettings[3].align == GridInfo::CENTER &&
			columnSettings[0].mode == GridInfo::AUTO &&
			columnSettings[0].align == GridInfo::BOTTOM_RIGHT, 3);
		check(codeOutput->getText().find(_T("Edited by self-test")) !=
			dwt::tstring::npos && codeOutput->getText().find(_T("3, 0, 1, 2")) !=
			dwt::tstring::npos, 4);

		// Regression: replacing the GroupBox child during removal must lay out the
		// new grid, and the rebuilt tile callbacks must remain selectable.
		removeSelected();
		app.processMessages();
		check(itemCount() == 4 && selected == noSelection &&
			designGrid->getClientSize().x > 0 && designGrid->getClientSize().y > 0, 5);
		if(!items.empty()) {
			items.front().tile->requestSelection();
			app.processMessages();
			check(selected == 0, 6);
		}

		// Construct every palette entry through the same add/remove path used by
		// the interactive designer so new widget adapters cannot silently regress.
		for(size_t index = 0; index < controlKindCount; ++index) {
			const bool added = addItem(static_cast<ControlKind>(index));
			check(added && selected != noSelection, 20 + static_cast<int>(index));
			check(codeOutput->getText().find(kindName(static_cast<ControlKind>(index))) !=
				dwt::tstring::npos, 120 + static_cast<int>(index));
			removeSelected();
			app.processMessages();
			check(itemCount() == 4, 60 + static_cast<int>(index));
		}
		resetExample();

		showPreview();
		app.processMessages();
		check(previewWindow && ::IsWindow(previewWindow->handle()), 100);
		if(previewWindow) {
			previewWindow->close();
			app.processMessages();
		}

		window->resize(dwt::Rectangle(80, 80, 900, 620));
		app.processMessages();
		window->close();
		app.processMessages();
		return failure;
	}

private:
	void build() {
		rootGrid = WidgetCreator<Grid>::create(window, Grid::Seed(4, 3));
		rootGrid->setSpacing(10);
		rootGrid->row(0) = GridInfo(0, GridInfo::AUTO, GridInfo::STRETCH);
		rootGrid->row(1) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		rootGrid->row(2) = GridInfo(225, GridInfo::STATIC, GridInfo::STRETCH);
		rootGrid->row(3) = GridInfo(0, GridInfo::AUTO, GridInfo::STRETCH);
		rootGrid->column(0) = GridInfo(220, GridInfo::STATIC, GridInfo::STRETCH);
		rootGrid->column(1) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		rootGrid->column(2) = GridInfo(320, GridInfo::STATIC, GridInfo::STRETCH);

		auto* header = WidgetCreator<Grid>::create(rootGrid, Grid::Seed(1, 3));
		header->setSpacing(8);
		header->row(0) = GridInfo(0, GridInfo::AUTO, GridInfo::STRETCH);
		header->column(0) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		header->column(1) = GridInfo(0, GridInfo::AUTO, GridInfo::STRETCH);
		header->column(2) = GridInfo(0, GridInfo::AUTO, GridInfo::STRETCH);
		auto* heading = WidgetCreator<Label>::create(header, Label::Seed(
			_T("Visual UI Designer - place DWT widgets, customize their grid cell, then preview the result.")));
		heading->setAccessibleName(_T("Designer instructions"));
		auto* previewButton = WidgetCreator<Button>::create(header,
			Button::Seed(_T("Preview window")));
		auto* resetButton = WidgetCreator<Button>::create(header,
			Button::Seed(_T("Reset example")));
		header->setWidget(heading, 0, 0);
		header->setWidget(previewButton, 0, 1);
		header->setWidget(resetButton, 0, 2);

		auto* paletteGroup = WidgetCreator<GroupBox>::create(rootGrid,
			GroupBox::Seed(_T("Control palette")));
		auto* palette = paletteGroup->addChild(Grid::Seed(5, 1));
		palette->setSpacing(7);
		palette->column(0) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		for(size_t row = 0; row < 5; ++row) {
			palette->row(row) = GridInfo(0, GridInfo::AUTO, GridInfo::STRETCH);
		}
		palette->row(4) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);

		auto* paletteHelp = WidgetCreator<Label>::create(palette, Label::Seed(
			_T("Choose any grid-placeable DWT widget.")));
		ComboBox::Seed pickerSeed;
		pickerSeed.style &= ~static_cast<DWORD>(CBS_DROPDOWN);
		pickerSeed.style |= CBS_DROPDOWNLIST;
		widgetPicker = WidgetCreator<ComboBox>::create(palette, pickerSeed);
		for(const auto& item : controlDescriptors) {
			widgetPicker->addValue(item.name);
		}
		widgetPicker->setSelected(0);
		widgetPicker->setMinimumVisibleItems(16);
		auto* addControl = WidgetCreator<Button>::create(palette,
			Button::Seed(_T("+ Add selected widget")));
		auto* paletteHint = WidgetCreator<Label>::create(palette, Label::Seed(
			_T("27 child widgets; host-level windows, dialogs, menus, notifications, and ToolTips are excluded.")));
		palette->setWidget(paletteHelp, 0, 0);
		palette->setWidget(widgetPicker, 1, 0);
		palette->setWidget(addControl, 2, 0);
		palette->setWidget(paletteHint, 3, 0);

		surfaceGroup = WidgetCreator<GroupBox>::create(rootGrid,
			GroupBox::Seed(_T("Design surface - 4 rows x 2 columns")));

		auto* propertiesGroup = WidgetCreator<GroupBox>::create(rootGrid,
			GroupBox::Seed(_T("Selected control and grid cell")));
		auto* properties = propertiesGroup->addChild(Grid::Seed(15, 2));
		properties->setSpacing(7);
		properties->column(0) = GridInfo(105, GridInfo::STATIC, GridInfo::STRETCH);
		properties->column(1) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		for(size_t row = 0; row < 15; ++row) {
			properties->row(row) = GridInfo(0, GridInfo::AUTO, GridInfo::STRETCH);
		}

		selectionLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Nothing selected")));
		auto* captionLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Caption")));
		captionEditor = WidgetCreator<TextBox>::create(properties, TextBox::Seed());
		captionEditor->setCue(_T("Control caption"), false);
		auto* rowLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Row")));
		rowEditor = createIndexEditor(properties, designerRows);
		auto* columnLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Column")));
		columnEditor = createIndexEditor(properties, designerColumns);
		auto* rowSpanLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Row span")));
		rowSpanEditor = createIndexEditor(properties, designerRows);
		auto* columnSpanLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Column span")));
		columnSpanEditor = createIndexEditor(properties, designerColumns);
		auto* rowModeLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Row mode")));
		rowModeEditor = createChoiceEditor(properties,
			{ _T("Static"), _T("Fill"), _T("Auto") });
		auto* rowSizeLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Row size")));
		rowSizeEditor = createNumberEditor(properties);
		auto* rowAlignLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Vertical align")));
		rowAlignEditor = createChoiceEditor(properties,
			{ _T("Top"), _T("Bottom"), _T("Center"), _T("Stretch") });
		auto* columnModeLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Column mode")));
		columnModeEditor = createChoiceEditor(properties,
			{ _T("Static"), _T("Fill"), _T("Auto") });
		auto* columnSizeLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Column size")));
		columnSizeEditor = createNumberEditor(properties);
		auto* columnAlignLabel = WidgetCreator<Label>::create(properties,
			Label::Seed(_T("Horizontal align")));
		columnAlignEditor = createChoiceEditor(properties,
			{ _T("Left"), _T("Right"), _T("Center"), _T("Stretch") });
		applyButton = WidgetCreator<Button>::create(properties,
			Button::Seed(_T("Apply properties")));
		removeButton = WidgetCreator<Button>::create(properties,
			Button::Seed(_T("Remove control")));
		auto* propertyHint = WidgetCreator<Label>::create(properties, Label::Seed(
			_T("Modes and alignment apply to the selected row and column.")));

		properties->setWidget(selectionLabel, 0, 0, 1, 2);
		properties->setWidget(captionLabel, 1, 0);
		properties->setWidget(captionEditor, 1, 1);
		properties->setWidget(rowLabel, 2, 0);
		properties->setWidget(rowEditor, 2, 1);
		properties->setWidget(columnLabel, 3, 0);
		properties->setWidget(columnEditor, 3, 1);
		properties->setWidget(rowSpanLabel, 4, 0);
		properties->setWidget(rowSpanEditor, 4, 1);
		properties->setWidget(columnSpanLabel, 5, 0);
		properties->setWidget(columnSpanEditor, 5, 1);
		properties->setWidget(rowModeLabel, 6, 0);
		properties->setWidget(rowModeEditor, 6, 1);
		properties->setWidget(rowSizeLabel, 7, 0);
		properties->setWidget(rowSizeEditor, 7, 1);
		properties->setWidget(rowAlignLabel, 8, 0);
		properties->setWidget(rowAlignEditor, 8, 1);
		properties->setWidget(columnModeLabel, 9, 0);
		properties->setWidget(columnModeEditor, 9, 1);
		properties->setWidget(columnSizeLabel, 10, 0);
		properties->setWidget(columnSizeEditor, 10, 1);
		properties->setWidget(columnAlignLabel, 11, 0);
		properties->setWidget(columnAlignEditor, 11, 1);
		properties->setWidget(applyButton, 12, 0, 1, 2);
		properties->setWidget(removeButton, 13, 0, 1, 2);
		properties->setWidget(propertyHint, 14, 0, 1, 2);

		auto* codeGroup = WidgetCreator<GroupBox>::create(rootGrid,
			GroupBox::Seed(_T("Generated LibDWT layout code")));
		TextBox::Seed codeSeed;
		codeSeed.style &= ~ES_AUTOHSCROLL;
		codeSeed.style |= ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY |
			WS_HSCROLL | WS_VSCROLL;
		codeSeed.lines = 10;
		codeOutput = codeGroup->addChild(codeSeed);
		codeOutput->setReadOnly(true);
		codeOutput->setAccessibleName(_T("Generated LibDWT C++ code"));

		statusLabel = WidgetCreator<Label>::create(rootGrid,
			Label::Seed(_T("Ready")));
		statusLabel->setAccessibleName(_T("Designer status"));

		rootGrid->setWidget(header, 0, 0, 1, 3);
		rootGrid->setWidget(paletteGroup, 1, 0);
		rootGrid->setWidget(surfaceGroup, 1, 1);
		rootGrid->setWidget(propertiesGroup, 1, 2);
		rootGrid->setWidget(codeGroup, 2, 0, 1, 3);
		rootGrid->setWidget(statusLabel, 3, 0, 1, 3);

		addControl->onClicked([this] {
			const int choice = widgetPicker->getSelected();
			if(choice >= 0 && static_cast<size_t>(choice) < controlKindCount) {
				addItem(static_cast<ControlKind>(choice));
			}
		});
		applyButton->onClicked([this] { applyProperties(); });
		removeButton->onClicked([this] { removeSelected(); });
		previewButton->onClicked([this] { showPreview(); });
		resetButton->onClicked([this] { resetExample(); });

		window->onSized([this](const dwt::SizedEvent&) { layout(); });
		window->onDestroy([this] {
			previewWindow = nullptr;
			::PostQuitMessage(0);
		});

		resetExample();
	}

	ComboBox* createIndexEditor(Widget* parent, size_t count) {
		ComboBox::Seed seed;
		seed.style &= ~static_cast<DWORD>(CBS_DROPDOWNLIST);
		seed.style |= CBS_DROPDOWNLIST;
		auto* editor = WidgetCreator<ComboBox>::create(parent, seed);
		for(size_t index = 0; index < count; ++index) {
			editor->addValue(std::to_wstring(index + 1));
		}
		return editor;
	}

	ComboBox* createChoiceEditor(Widget* parent,
		std::initializer_list<const TCHAR*> choices)
	{
		ComboBox::Seed seed;
		seed.style &= ~static_cast<DWORD>(CBS_DROPDOWNLIST);
		seed.style |= CBS_DROPDOWNLIST;
		auto* editor = WidgetCreator<ComboBox>::create(parent, seed);
		for(const auto* choice : choices) {
			editor->addValue(choice);
		}
		editor->setSelected(0);
		return editor;
	}

	TextBox* createNumberEditor(Widget* parent) {
		TextBox::Seed seed;
		seed.style |= ES_NUMBER;
		auto* editor = WidgetCreator<TextBox>::create(parent, seed);
		editor->setCue(_T("Pixels"), false);
		return editor;
	}

	bool readGridSize(TextBox* editor, size_t& value) const {
		const auto text = editor->getText();
		if(text.empty()) {
			return false;
		}
		TCHAR* end = nullptr;
		const unsigned long parsed = _tcstoul(text.c_str(), &end, 10);
		if(end == text.c_str() || *end != _T('\0') || parsed > 4096) {
			return false;
		}
		value = static_cast<size_t>(parsed);
		return true;
	}

	void paintDesignGrid(PaintCanvas& canvas) {
		dwt::Rectangle client(designGrid->getClientSize());
		canvas.fill(client, Brush(RGB(248, 250, 253)));
		Pen gridPen(RGB(220, 226, 234), Pen::Dot, 1);
		auto selectedPen = canvas.select(gridPen);
		const int step = (std::max)(4, designGrid->scale(12));
		for(int x = step; x < client.width(); x += step) {
			canvas.line(x, 0, x, client.height());
		}
		for(int y = step; y < client.height(); y += step) {
			canvas.line(0, y, client.width(), y);
		}
	}

	void resetGridSettings() {
		for(auto& row : rowSettings) {
			row = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		}
		for(auto& column : columnSettings) {
			column = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);
		}
	}

	void applyGridSettings(Grid* grid) {
		for(size_t row = 0; row < designerRows; ++row) {
			grid->row(row) = rowSettings[row];
		}
		for(size_t column = 0; column < designerColumns; ++column) {
			grid->column(column) = columnSettings[column];
		}
	}

	void recreateDesignGrid() {
		designGrid = surfaceGroup->addChild(
			Grid::Seed(designerRows, designerColumns));
		designGrid->setSpacing(10);
		applyGridSettings(designGrid);
		designGrid->addCallback(dwt::Message(WM_ERASEBKGND),
			[](const MSG&, LRESULT& result) {
				result = 1;
				return true;
			});
		designGrid->onPainting([this](PaintCanvas& canvas) {
			paintDesignGrid(canvas);
		});
		items.clear();
		selected = noSelection;
		// GroupBox owns a single child. Replacing that child after the initial
		// window layout requires an immediate layout pass or the new grid remains
		// at its zero-sized creation rectangle and cannot receive selection clicks.
		surfaceGroup->layout();
	}

	void layout() {
		if(rootGrid && ::IsWindow(rootGrid->handle())) {
			rootGrid->resize(dwt::Rectangle(window->getClientSize()));
		}
	}

	bool findFreeCell(size_t& row, size_t& column) const {
		for(row = 0; row < designerRows; ++row) {
			for(column = 0; column < designerColumns; ++column) {
				if(!occupied(row, column, 1, 1, noSelection)) {
					return true;
				}
			}
		}
		return false;
	}

	bool occupied(size_t row, size_t column, size_t rowSpan,
		size_t columnSpan, size_t ignored) const
	{
		for(size_t index = 0; index < items.size(); ++index) {
			const auto& item = items[index];
			if(index != ignored && row < item.row + item.rowSpan &&
				row + rowSpan > item.row && column < item.column + item.columnSpan &&
				column + columnSpan > item.column)
			{
				return true;
			}
		}
		return false;
	}

	bool addItem(ControlKind kind,
		const dwt::tstring& caption = dwt::tstring())
	{
		size_t row = 0;
		size_t column = 0;
		if(!findFreeCell(row, column)) {
			setStatus(_T("The design grid is full. Remove a control before adding another."));
			return false;
		}

		const auto text = caption.empty() ? defaultCaption(kind) : caption;
		const auto index = addItemAt(kind, text, row, column);
		designGrid->layout();
		selectItem(index);
		setStatus(dwt::tstring(kindName(kind)) + _T(" added at row ") +
			std::to_wstring(row + 1) + _T(", column ") +
			std::to_wstring(column + 1) + _T("."));
		regenerateCode();
		return true;
	}

	size_t addItemAt(ControlKind kind, const dwt::tstring& caption,
		size_t row, size_t column, size_t rowSpan = 1, size_t columnSpan = 1)
	{
		auto* tile = WidgetCreator<DesignerTile>::create(designGrid,
			DesignerTile::Seed(kind, caption));
		const auto index = items.size();
		items.push_back({ kind, caption, row, column, rowSpan, columnSpan, tile });
		tile->setSelectionCallback([this, index] { selectItem(index); });
		designGrid->setWidget(tile, row, column, rowSpan, columnSpan);
		tile->setVisible(true);
		return index;
	}

	void selectItem(size_t index) {
		if(index >= items.size()) {
			return;
		}
		if(selected != noSelection && selected < items.size()) {
			items[selected].tile->setSelected(false);
		}
		selected = index;
		auto& item = items[selected];
		item.tile->setSelected(true);
		selectionLabel->setText(dwt::tstring(kindName(item.kind)) +
			_T(" at row ") + std::to_wstring(item.row + 1) +
			_T(", column ") + std::to_wstring(item.column + 1) +
			_T(" (") + std::to_wstring(item.rowSpan) + _T(" x ") +
			std::to_wstring(item.columnSpan) + _T(" cells)"));
		captionEditor->setText(item.caption);
		rowEditor->setSelected(static_cast<int>(item.row));
		columnEditor->setSelected(static_cast<int>(item.column));
		rowSpanEditor->setSelected(static_cast<int>(item.rowSpan - 1));
		columnSpanEditor->setSelected(static_cast<int>(item.columnSpan - 1));
		const auto& rowInfo = rowSettings[item.row];
		const auto& columnInfo = columnSettings[item.column];
		rowModeEditor->setSelected(static_cast<int>(rowInfo.mode));
		rowSizeEditor->setText(std::to_wstring(rowInfo.size));
		rowAlignEditor->setSelected(static_cast<int>(rowInfo.align));
		columnModeEditor->setSelected(static_cast<int>(columnInfo.mode));
		columnSizeEditor->setText(std::to_wstring(columnInfo.size));
		columnAlignEditor->setSelected(static_cast<int>(columnInfo.align));
		setPropertiesEnabled(true);
		setStatus(_T("Selected ") + dwt::tstring(kindName(item.kind)) + _T("."));
	}

	void applyProperties() {
		if(selected == noSelection || selected >= items.size())
		{
			return;
		}

		const int requestedRow = rowEditor->getSelected();
		const int requestedColumn = columnEditor->getSelected();
		const int requestedRowSpan = rowSpanEditor->getSelected();
		const int requestedColumnSpan = columnSpanEditor->getSelected();
		if(requestedRow < 0 || requestedColumn < 0 || requestedRowSpan < 0 ||
			requestedColumnSpan < 0)
		{
			setStatus(_T("Choose a row, column, and valid spans."));
			return;
		}
		const auto row = static_cast<size_t>(requestedRow);
		const auto column = static_cast<size_t>(requestedColumn);
		const auto rowSpan = static_cast<size_t>(requestedRowSpan) + 1;
		const auto columnSpan = static_cast<size_t>(requestedColumnSpan) + 1;
		if(row + rowSpan > designerRows || column + columnSpan > designerColumns) {
			setStatus(_T("The requested span extends beyond the design grid."));
			rowSpanEditor->setSelected(static_cast<int>(items[selected].rowSpan - 1));
			columnSpanEditor->setSelected(static_cast<int>(items[selected].columnSpan - 1));
			return;
		}
		if(occupied(row, column, rowSpan, columnSpan, selected)) {
			setStatus(_T("The requested grid area overlaps another control."));
			rowEditor->setSelected(static_cast<int>(items[selected].row));
			columnEditor->setSelected(static_cast<int>(items[selected].column));
			rowSpanEditor->setSelected(static_cast<int>(items[selected].rowSpan - 1));
			columnSpanEditor->setSelected(static_cast<int>(items[selected].columnSpan - 1));
			return;
		}

		size_t rowSize = 0;
		size_t columnSize = 0;
		if(!readGridSize(rowSizeEditor, rowSize) ||
			!readGridSize(columnSizeEditor, columnSize))
		{
			setStatus(_T("Grid sizes must be whole numbers from 0 through 4096."));
			return;
		}
		const int rowMode = rowModeEditor->getSelected();
		const int rowAlign = rowAlignEditor->getSelected();
		const int columnMode = columnModeEditor->getSelected();
		const int columnAlign = columnAlignEditor->getSelected();
		if(rowMode < 0 || rowAlign < 0 || columnMode < 0 || columnAlign < 0) {
			setStatus(_T("Choose grid modes and alignment values."));
			return;
		}

		auto& item = items[selected];
		item.caption = captionEditor->getText();
		item.row = row;
		item.column = column;
		item.rowSpan = rowSpan;
		item.columnSpan = columnSpan;
		rowSettings[row] = GridInfo(static_cast<int>(rowSize),
			static_cast<GridInfo::Modes>(rowMode),
			static_cast<GridInfo::Align>(rowAlign));
		columnSettings[column] = GridInfo(static_cast<int>(columnSize),
			static_cast<GridInfo::Modes>(columnMode),
			static_cast<GridInfo::Align>(columnAlign));
		item.tile->setCaption(item.caption);
		applyGridSettings(designGrid);
		designGrid->setWidget(item.tile, row, column, rowSpan, columnSpan);
		designGrid->layout();
		selectItem(selected);
		setStatus(_T("Properties applied and generated code updated."));
		regenerateCode();
	}

	void removeSelected() {
		if(selected == noSelection || selected >= items.size())
		{
			return;
		}
		std::vector<DesignerSpec> remaining;
		remaining.reserve(items.size() - 1);
		for(size_t index = 0; index < items.size(); ++index) {
			const auto& item = items[index];
			if(index != selected) {
				remaining.push_back({ item.kind, item.caption, item.row, item.column,
					item.rowSpan, item.columnSpan });
			}
		}
		recreateDesignGrid();
		for(const auto& item : remaining) {
			addItemAt(item.kind, item.caption, item.row, item.column,
				item.rowSpan, item.columnSpan);
		}
		designGrid->layout();
		selectionLabel->setText(_T("Nothing selected"));
		captionEditor->setText(_T(""));
		setPropertiesEnabled(false);
		setStatus(_T("Control removed from the generated design."));
		regenerateCode();
	}

	void setPropertiesEnabled(bool enabled) {
		captionEditor->setEnabled(enabled);
		rowEditor->setEnabled(enabled);
		columnEditor->setEnabled(enabled);
		rowSpanEditor->setEnabled(enabled);
		columnSpanEditor->setEnabled(enabled);
		rowModeEditor->setEnabled(enabled);
		rowSizeEditor->setEnabled(enabled);
		rowAlignEditor->setEnabled(enabled);
		columnModeEditor->setEnabled(enabled);
		columnSizeEditor->setEnabled(enabled);
		columnAlignEditor->setEnabled(enabled);
		applyButton->setEnabled(enabled);
		removeButton->setEnabled(enabled);
	}

	void resetExample() {
		resetGridSettings();
		recreateDesignGrid();
		addItemAt(ControlKind::Label, _T("Customer name"), 0, 0);
		addItemAt(ControlKind::TextBox, _T("Ada Lovelace"), 0, 1);
		addItemAt(ControlKind::CheckBox, _T("Send product updates"), 1, 0);
		addItemAt(ControlKind::Button, _T("Save changes"), 1, 1);
		designGrid->layout();
		selectLastItem();
		setStatus(_T("Example reset to a two-column customer form."));
		regenerateCode();
	}

	void selectLastItem() {
		if(!items.empty()) {
			selectItem(items.size() - 1);
		}
	}

	void writeControlCreation(std::wostringstream& output, ControlKind kind,
		const dwt::tstring& variable, const dwt::tstring& caption)
	{
		const auto literal = quoteLiteral(caption);
		switch(kind) {
		case ControlKind::Button:
		case ControlKind::CheckBox:
		case ControlKind::GroupBox:
		case ControlKind::Label:
		case ControlKind::RadioButton:
		case ControlKind::TextBox:
			output << L"auto* " << variable << L" = WidgetCreator<"
				<< kindName(kind) << L">::create(formGrid, " << kindName(kind)
				<< L"::Seed(_T(\"" << literal << L"\")));\r\n";
			break;
		case ControlKind::Link:
			output << L"auto* " << variable << L" = WidgetCreator<Link>::create(formGrid, "
				<< L"Link::Seed(_T(\"" << literal << L"\"), true));\r\n";
			break;
		case ControlKind::ComboBox:
			output << L"auto* " << variable << L" = WidgetCreator<ComboBox>::create(formGrid, ComboBox::Seed());\r\n"
				<< variable << L"->addValue(_T(\"" << literal << L"\"));\r\n"
				<< variable << L"->setSelected(0);\r\n";
			break;
		case ControlKind::Container:
			output << L"auto* " << variable << L" = WidgetCreator<Container>::create(formGrid, Container::Seed());\r\n"
				<< L"WidgetCreator<Label>::create(" << variable << L", Label::Seed(_T(\""
				<< literal << L"\")));\r\n";
			break;
		case ControlKind::DateTime:
		case ControlKind::MonthCalendar:
		case ControlKind::ProgressBar:
		case ControlKind::RichTextBox:
		case ControlKind::ScrolledContainer:
		case ControlKind::Slider:
		case ControlKind::Spinner:
		case ControlKind::SplitterContainer:
		case ControlKind::StatusBar:
		case ControlKind::Table:
		case ControlKind::TabView:
		case ControlKind::ToolBar:
		case ControlKind::Tree:
			output << L"auto* " << variable << L" = WidgetCreator<"
				<< kindName(kind) << L">::create(formGrid, " << kindName(kind)
				<< L"::Seed());\r\n";
			break;
		case ControlKind::Grid:
			output << L"auto* " << variable << L" = WidgetCreator<Grid>::create(formGrid, Grid::Seed(1, 1));\r\n"
				<< variable << L"->row(0) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);\r\n"
				<< variable << L"->column(0) = GridInfo(0, GridInfo::FILL, GridInfo::STRETCH);\r\n";
			break;
		case ControlKind::Header:
			output << L"auto* " << variable << L" = WidgetCreator<Header>::create(formGrid, Header::Seed());\r\n"
				<< variable << L"->insert(_T(\"" << literal << L"\"), 180);\r\n";
			break;
		case ControlKind::Rebar:
			output << L"auto* " << variable << L" = WidgetCreator<Rebar>::create(formGrid, Rebar::Seed());\r\n"
				<< L"auto* " << variable << L"ToolBar = WidgetCreator<ToolBar>::create("
				<< variable << L", ToolBar::Seed());\r\n"
				<< variable << L"ToolBar->addButton(\"action\", -1, _T(\"" << literal
				<< L"\"), true);\r\n"
				<< variable << L"ToolBar->setLayout({ \"action\" });\r\n"
				<< variable << L"->add(" << variable << L"ToolBar, RBBS_CHILDEDGE, _T(\""
				<< literal << L"\"));\r\n";
			break;
		case ControlKind::TableTree:
			output << L"auto* " << variable << L" = WidgetCreator<TableTree>::create(formGrid, "
				<< L"TableTree::Seed(Table::Seed()));\r\n";
			break;
		case ControlKind::VirtualTree:
			output << L"auto* " << variable << L" = WidgetCreator<VirtualTree>::create(formGrid, "
				<< L"VirtualTree::Seed(Tree::Seed()));\r\n";
			break;
		case ControlKind::Count:
			return;
		}

		switch(kind) {
		case ControlKind::ProgressBar:
		case ControlKind::Slider:
			output << variable << L"->setRange(0, 100);\r\n"
				<< variable << L"->setPosition(50);\r\n";
			break;
		case ControlKind::RichTextBox:
			output << variable << L"->setText(_T(\"" << literal << L"\"));\r\n";
			break;
		case ControlKind::ScrolledContainer:
			output << variable << L"->addChild(Label::Seed(_T(\"" << literal << L"\")));\r\n";
			break;
		case ControlKind::Spinner:
			output << variable << L"->setRange(0, 100);\r\n"
				<< variable << L"->setValue(50);\r\n";
			break;
		case ControlKind::SplitterContainer:
			output << L"WidgetCreator<Container>::create(" << variable << L", Container::Seed());\r\n"
				<< L"WidgetCreator<Container>::create(" << variable << L", Container::Seed());\r\n";
			break;
		case ControlKind::StatusBar:
			output << variable << L"->setText(0, _T(\"" << literal << L"\"));\r\n";
			break;
		case ControlKind::Table:
		case ControlKind::TableTree:
			output << variable << L"->addColumn(_T(\"Item\"), 180);\r\n"
				<< variable << L"->insert({ _T(\"" << literal << L"\") }, 1);\r\n";
			break;
		case ControlKind::TabView:
			output << L"auto* " << variable << L"Page = WidgetCreator<Container>::create("
				<< variable << L", Container::Seed());\r\n"
				<< variable << L"Page->setText(_T(\"" << literal << L"\"));\r\n"
				<< variable << L"->add(" << variable << L"Page);\r\n";
			break;
		case ControlKind::ToolBar:
			output << variable << L"->addButton(\"action\", -1, _T(\"" << literal
				<< L"\"), true);\r\n"
				<< variable << L"->setLayout({ \"action\" });\r\n";
			break;
		case ControlKind::Tree:
		case ControlKind::VirtualTree:
			output << variable << L"->addColumn(_T(\"Item\"), 180);\r\n"
				<< variable << L"->insert(_T(\"" << literal
				<< L"\"), TVI_ROOT, TVI_LAST, 1, true);\r\n";
			break;
		default:
			break;
		}
	}

	void regenerateCode() {
		std::wostringstream output;
		output << L"// Generated by UIDesignerExample\r\n"
			<< L"auto* formGrid = WidgetCreator<Grid>::create(window, Grid::Seed("
			<< designerRows << L", " << designerColumns << L"));\r\n"
			<< L"formGrid->setSpacing(10);\r\n";
		for(size_t row = 0; row < designerRows; ++row) {
			const auto& info = rowSettings[row];
			output << L"formGrid->row(" << row << L") = GridInfo(" << info.size
				<< L", GridInfo::" << gridModeName(info.mode) << L", GridInfo::"
				<< gridAlignName(info.align) << L");\r\n";
		}
		for(size_t column = 0; column < designerColumns; ++column) {
			const auto& info = columnSettings[column];
			output << L"formGrid->column(" << column << L") = GridInfo(" << info.size
				<< L", GridInfo::" << gridModeName(info.mode) << L", GridInfo::"
				<< gridAlignName(info.align) << L");\r\n";
		}
		output << L"\r\n";

		std::array<unsigned, controlKindCount> counters = {};
		for(const auto& item : items) {
			const auto number = ++counters[kindIndex(item.kind)];
			const dwt::tstring variable = dwt::tstring(variableBase(item.kind)) +
				std::to_wstring(number);
			writeControlCreation(output, item.kind, variable, item.caption);
			output << L"formGrid->setWidget(" << variable << L", "
				<< item.row << L", " << item.column << L", " << item.rowSpan
				<< L", " << item.columnSpan << L");\r\n\r\n";
		}
		output << L"window->onSized([window, formGrid](const dwt::SizedEvent&) {\r\n"
			<< L"    formGrid->resize(dwt::Rectangle(window->getClientSize()));\r\n"
			<< L"});\r\n"
			<< L"formGrid->resize(dwt::Rectangle(window->getClientSize()));\r\n";
		codeOutput->setText(output.str());
	}

	void showPreview() {
		if(previewWindow && ::IsWindow(previewWindow->handle())) {
			previewWindow->close();
		}

		Window::Seed seed(_T("LibDWT Designer Preview"));
		seed.location = dwt::Rectangle(180, 140, 700, 460);
		previewWindow = WidgetCreator<Window>::create(window, seed);
		auto* previewGrid = WidgetCreator<Grid>::create(previewWindow,
			Grid::Seed(designerRows, designerColumns));
		previewGrid->setSpacing(10);
		applyGridSettings(previewGrid);

		for(const auto& item : items) {
			auto* control = createSimpleControl(previewGrid, item.kind, item.caption);
			previewGrid->setWidget(control, item.row, item.column,
				item.rowSpan, item.columnSpan);
		}

		previewWindow->onSized([previewWindow = previewWindow, previewGrid](
			const dwt::SizedEvent&)
		{
			previewGrid->resize(dwt::Rectangle(previewWindow->getClientSize()));
		});
		previewWindow->onDestroy([this] { previewWindow = nullptr; });
		previewGrid->resize(dwt::Rectangle(previewWindow->getClientSize()));
		previewWindow->setVisible(true);
		previewWindow->setFocus();
		setStatus(_T("Live preview opened from the current designer model."));
	}

	size_t itemCount() const {
		return items.size();
	}

	void setStatus(const dwt::tstring& text) {
		if(statusLabel) {
			statusLabel->setText(text);
		}
	}

	Window* window;
	Grid* rootGrid;
	GroupBox* surfaceGroup;
	Grid* designGrid;
	TextBox* codeOutput;
	ComboBox* widgetPicker;
	Label* selectionLabel;
	TextBox* captionEditor;
	ComboBox* rowEditor;
	ComboBox* columnEditor;
	ComboBox* rowSpanEditor;
	ComboBox* columnSpanEditor;
	ComboBox* rowModeEditor;
	TextBox* rowSizeEditor;
	ComboBox* rowAlignEditor;
	ComboBox* columnModeEditor;
	TextBox* columnSizeEditor;
	ComboBox* columnAlignEditor;
	Button* applyButton;
	Button* removeButton;
	Label* statusLabel;
	Window* previewWindow;
	std::vector<DesignerItem> items;
	std::array<GridInfo, designerRows> rowSettings;
	std::array<GridInfo, designerColumns> columnSettings;
	size_t selected;
};

} // namespace

int dwtMain(dwt::Application& app) {
	const bool selfTest = _tcsstr(::GetCommandLine(), _T("--self-test")) != nullptr;

	Window::Seed seed(_T("LibDWT Visual UI Designer Example"));
	seed.location = dwt::Rectangle(50, 50, 1380, 900);
	auto* window = WidgetCreator<Window>::create(seed);
	window->setAccessibleName(_T("LibDWT visual UI designer example"));
	DesignerController designer(window);

	if(selfTest) {
		return designer.runSelfTest(app);
	}

	designer.show();
	app.run();
	return 0;
}
