#include <dwt/Application.h>
#include <dwt/WidgetCreator.h>

#include <dwt/resources/Font.h>
#include <dwt/resources/Icon.h>
#include <dwt/widgets/Button.h>
#include <dwt/widgets/CheckBox.h>
#include <dwt/widgets/ColorDialog.h>
#include <dwt/widgets/ComboBox.h>
#include <dwt/widgets/Control.h>
#include <dwt/widgets/DateTime.h>
#include <dwt/widgets/FolderDialog.h>
#include <dwt/widgets/FontDialog.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Header.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/Link.h>
#include <dwt/widgets/LoadDialog.h>
#include <dwt/widgets/MessageBox.h>
#include <dwt/widgets/Notification.h>
#include <dwt/widgets/ProgressBar.h>
#include <dwt/widgets/RadioButton.h>
#include <dwt/widgets/Rebar.h>
#include <dwt/widgets/RichTextBox.h>
#include <dwt/widgets/SaveDialog.h>
#include <dwt/widgets/ScrolledContainer.h>
#include <dwt/widgets/Slider.h>
#include <dwt/widgets/Spinner.h>
#include <dwt/widgets/StatusBar.h>
#include <dwt/widgets/Table.h>
#include <dwt/widgets/TableTree.h>
#include <dwt/widgets/TaskDialog.h>
#include <dwt/widgets/TextBox.h>
#include <dwt/widgets/ToolBar.h>
#include <dwt/widgets/ToolTip.h>
#include <dwt/widgets/Tree.h>
#include <dwt/widgets/VirtualTree.h>
#include <dwt/widgets/Window.h>

#include "resource.h"

#include <memory>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

using dwt::Button;
using dwt::CheckBox;
using dwt::ColorDialog;
using dwt::ComboBox;
using dwt::Control;
using dwt::DateTime;
using dwt::FolderDialog;
using dwt::FontDialog;
using dwt::Grid;
using dwt::GridInfo;
using dwt::Header;
using dwt::Label;
using dwt::Link;
using dwt::LoadDialog;
using dwt::Notification;
using dwt::ProgressBar;
using dwt::RadioButton;
using dwt::Rebar;
using dwt::RichTextBox;
using dwt::SaveDialog;
using dwt::ScrolledContainer;
using dwt::Slider;
using dwt::Spinner;
using dwt::StatusBar;
using dwt::Table;
using dwt::TableTree;
using dwt::TextBox;
using dwt::ToolBar;
using dwt::ToolTip;
using dwt::Tree;
using dwt::VirtualTree;
using dwt::WidgetCreator;
using dwt::Window;

void setStatus(StatusBar::ObjectType status, const dwt::tstring& text) {
	status->setText(0, text, true);
	status->setText(1, _T("MultiControlExample"));
}

dwt::tstring pointerTypeName(dwt::PointerEvent::Type type) {
	switch(type) {
	case dwt::PointerEvent::Touch:
		return _T("touch");
	case dwt::PointerEvent::Pen:
		return _T("pen");
	case dwt::PointerEvent::Mouse:
		return _T("mouse");
	case dwt::PointerEvent::Touchpad:
		return _T("touchpad");
	case dwt::PointerEvent::Generic:
		return _T("generic");
	default:
		return _T("unknown");
	}
}

void setRichSummary(RichTextBox::ObjectType richText) {
	const char* rtf =
		"{\\rtf1\\ansi\\ansicpg1252\\deff0"
		"{\\fonttbl{\\f0 Segoe UI;}{\\f1 Consolas;}{\\f2 Cambria;}}"
		"{\\colortbl;"
		"\\red30\\green30\\blue30;"
		"\\red0\\green102\\blue204;"
		"\\red198\\green40\\blue40;"
		"\\red0\\green128\\blue72;"
		"\\red120\\green64\\blue160;"
		"\\red245\\green245\\blue210;"
		"}"
		"\\viewkind4\\uc1\\pard\\sa120\\sl280\\slmult1\\f0\\fs20\\cf1 "
		"\\b RichTextBox Example\\b0\\par "
		"\\cf2 Grid-based layout with a broad collection of controls.\\cf1\\par\\par "
		"\\b Text formatting showcase\\b0\\par "
		"Regular, \\b bold\\b0, \\i italic\\i0, \\ul underline\\ul0, and \\strike strikeout\\strike0 text.\\par "
		"\\cf3 Colorized text\\cf1, \\cf4 semantic highlights\\cf1, and \\highlight6 highlighted regions\\highlight0.\\par "
		"Math style: H\\sub 2\\nosupersubO and x\\super 2\\nosupersub + y\\super 2\\nosupersub = z\\super 2\\nosupersub.\\par "
		"Code sample: \\f1\\fs18 for(int i = 0; i < 3; ++i) { update(); }\\f0\\fs20\\par\\par "
		"\\b Paragraph and list styles\\b0\\par "
		"\\pard\\li360\\tx720\\fi-240\\bullet\\tab Toolbar and Rebar actions\\par "
		"\\bullet\\tab Table, TableTree, Tree, and VirtualTree\\par "
		"\\bullet\\tab Native dialogs (open/save/folder/color/font)\\par "
		"\\pard\\sa120\\qc\\cf5 Center aligned accent line\\par "
		"\\pard\\sa120\\qr\\cf3 Right aligned warning tone\\par "
		"\\pard\\sa120\\ql\\cf1 Back to normal flow."
		"}";

	SETTEXTEX config = { ST_DEFAULT, CP_ACP };
	richText->sendMessage(EM_SETTEXTEX, reinterpret_cast<WPARAM>(&config), reinterpret_cast<LPARAM>(rtf));
	richText->setReadOnly(true);
	richText->setSelection(0, 0);
}

void configureGrid(Grid::ObjectType grid) {
	grid->setSpacing(8);
	grid->column(0).mode = GridInfo::FILL;
	grid->column(1).mode = GridInfo::FILL;
	grid->column(2).mode = GridInfo::FILL;
	grid->column(3).mode = GridInfo::FILL;
	grid->column(0).align = GridInfo::STRETCH;
	grid->column(1).align = GridInfo::STRETCH;
	grid->column(2).align = GridInfo::STRETCH;
	grid->column(3).align = GridInfo::STRETCH;

	grid->row(0).mode = GridInfo::STATIC;
	grid->row(0).size = 36;
	grid->row(1).mode = GridInfo::AUTO;
	grid->row(2).mode = GridInfo::STATIC;
	grid->row(2).size = 64;
	grid->row(3).mode = GridInfo::AUTO;
	grid->row(4).mode = GridInfo::AUTO;
	grid->row(5).mode = GridInfo::AUTO;
	grid->row(6).mode = GridInfo::FILL;
	grid->row(7).mode = GridInfo::STATIC;
	grid->row(7).size = 220;
	grid->row(3).align = GridInfo::STRETCH;
	grid->row(4).align = GridInfo::STRETCH;
	grid->row(2).align = GridInfo::STRETCH;
	grid->row(6).align = GridInfo::STRETCH;
	grid->row(7).align = GridInfo::STRETCH;
}

} // namespace

int dwtMain(dwt::Application& app) {
	Window::Seed seed(_T("DWT MultiControlExample - Grid Showcase"));
	seed.location = dwt::Rectangle(70, 70, 1400, 900);
	auto* window = WidgetCreator<Window>::create(seed);
	auto uiFont = window->getFont();

	auto* grid = WidgetCreator<Grid>::create(window, Grid::Seed(8, 4));
	configureGrid(grid);

	auto* rebar = WidgetCreator<Rebar>::create(grid, Rebar::Seed());
	auto* toolbar = WidgetCreator<ToolBar>::create(rebar, ToolBar::Seed());

	auto* labelInput = WidgetCreator<Label>::create(grid, Label::Seed(_T("Input:")));
	auto* textInput = WidgetCreator<TextBox>::create(grid, TextBox::Seed(_T("Grid layout with all major controls")));
	auto* buttonRun = WidgetCreator<Button>::create(grid,
		Button::Seed(_T("Modern Controls"), BS_COMMANDLINK));
	auto* checkOption = WidgetCreator<CheckBox>::create(grid, CheckBox::Seed(_T("Enable Option")));
	auto* radioA = WidgetCreator<RadioButton>::create(grid, RadioButton::Seed(_T("Mode A")));
	auto* radioB = WidgetCreator<RadioButton>::create(grid, RadioButton::Seed(_T("Mode B")));
	auto* combo = WidgetCreator<ComboBox>::create(grid, ComboBox::Seed());
	DateTime::Seed dateTimeSeed;
	dateTimeSeed.style |= DTS_SHOWNONE;
	auto* dateTime = WidgetCreator<DateTime>::create(grid, dateTimeSeed);
	auto* spinner = WidgetCreator<Spinner>::create(grid, Spinner::Seed(0, 100));
	auto* slider = WidgetCreator<Slider>::create(grid, Slider::Seed());
	auto* progress = WidgetCreator<ProgressBar>::create(grid, ProgressBar::Seed());
	auto* link = WidgetCreator<Link>::create(grid, Link::Seed(_T("https://dcplusplus.sourceforge.io/"), true));

	auto* header = WidgetCreator<Header>::create(grid, Header::Seed());
	header->setFont(uiFont);

	Table::Seed tableSeed;
	tableSeed.font = uiFont;
	auto* table = WidgetCreator<Table>::create(grid, tableSeed);

	Tree::Seed treeSeed;
	treeSeed.font = uiFont;
	treeSeed.checkBoxes = true;
	treeSeed.tvExStyle = TVS_EX_PARTIALCHECKBOXES |
		TVS_EX_EXCLUSIONCHECKBOXES | TVS_EX_DIMMEDCHECKBOXES;
	auto* tree = WidgetCreator<Tree>::create(grid, treeSeed);

	auto* tableTree = WidgetCreator<TableTree>::create(grid, TableTree::Seed(tableSeed));
	Tree::Seed virtualTreeSeed;
	virtualTreeSeed.font = uiFont;
	virtualTreeSeed.checkBoxes = true;
	virtualTreeSeed.tvExStyle = treeSeed.tvExStyle;
	auto* virtualTree = WidgetCreator<VirtualTree>::create(grid, VirtualTree::Seed(virtualTreeSeed));

	auto* richText = WidgetCreator<RichTextBox>::create(grid, RichTextBox::Seed());

	auto* scrolled = WidgetCreator<ScrolledContainer>::create(grid, ScrolledContainer::Seed());
	auto* scrolledGrid = scrolled->addChild(Grid::Seed(4, 1));
	scrolledGrid->setSpacing(6);
	scrolledGrid->row(0).mode = GridInfo::AUTO;
	scrolledGrid->row(1).mode = GridInfo::AUTO;
	scrolledGrid->row(2).mode = GridInfo::AUTO;
	scrolledGrid->row(3).mode = GridInfo::STATIC;
	scrolledGrid->row(3).size = 260;
	scrolledGrid->column(0).mode = GridInfo::STATIC;
	scrolledGrid->column(0).size = 420;
	scrolledGrid->column(0).align = GridInfo::STRETCH;
	auto* scrollLabel1 = WidgetCreator<Label>::create(scrolledGrid, Label::Seed(_T("ScrolledContainer with nested Grid:")));
	auto* scrollLabel2 = WidgetCreator<Label>::create(scrolledGrid, Label::Seed(_T("- Maintains spacing and alignment")));
	auto* scrollLabel3 = WidgetCreator<Label>::create(scrolledGrid, Label::Seed(_T("- Shows scrollbars when content exceeds viewport")));
	auto* scrollLabel4 = WidgetCreator<Label>::create(scrolledGrid, Label::Seed(_T("- Wide and tall content area below should trigger horizontal and vertical scrolling in normal window sizes.")));
	scrolledGrid->setWidget(scrollLabel1, 0, 0);
	scrolledGrid->setWidget(scrollLabel2, 1, 0);
	scrolledGrid->setWidget(scrollLabel3, 2, 0);
	scrolledGrid->setWidget(scrollLabel4, 3, 0);
	scrolledGrid->layout();

	auto* status = WidgetCreator<StatusBar>::create(window, StatusBar::Seed(2, 1, true));
	status->setSize(0, 420);
	status->setText(1, _T("MultiControlExample"));

	dwt::MessageBoxW messageBox(window);
	ColorDialog colorDialog(window);
	ColorDialog::ColorParams colorParams(RGB(0, 102, 204));
	FontDialog fontDialog(window);
	LoadDialog loadDialog(window);
	SaveDialog saveDialog(window);
	FolderDialog folderDialog(window);
	auto* toolTip = WidgetCreator<ToolTip>::create(window, ToolTip::Seed());

	buttonRun->setNote(_T("Task Dialog, progress states, nullable dates, DPI, and pointer events"));
	buttonRun->setAccessibleName(_T("Open modern controls demonstration"));
	buttonRun->setAccessibleHelpText(_T("Opens a Windows task dialog and applies its selections to the example."));

	grid->setAccessibleName(_T("Multi-control example content"));
	grid->setAccessibleHelpText(_T("Demonstrates LibDWT controls and Windows 7 or later APIs."));
	scrolled->setAccessibleName(_T("Scrollable feature summary"));

	toolTip->setTitle(_T("Windows 7+ control features"), TTI_INFO);
	toolTip->setMargin(dwt::Rectangle(6, 4, 6, 4));
	toolTip->setMaxTipWidth(420);
	toolTip->setWindowTheme(_T("Explorer"));
	toolTip->setText(buttonRun,
		_T("Open a Task Dialog and use its radio buttons and verification checkbox to update the controls."));

	auto notification = std::make_shared<Notification>(window);
	dwt::IconPtr trayIcon(new dwt::Icon(IDI_MULTI_TRAY, dwt::Point(16, 16)));
	notification->create(Notification::Seed(trayIcon, _T("DWT MultiControlExample notification")));
	notification->setTooltip(_T("DWT MultiControlExample notification"));
	notification->setVisible(true);
	notification->onIconClicked([status, notification] {
		auto rect = notification->getRect();
		setStatus(status, _T("Tray icon at ") + std::to_wstring(rect.x()) +
			_T(", ") + std::to_wstring(rect.y()));
	});
	notification->onContextMenu([status, notification] {
		setStatus(status, _T("Tray icon context menu requested"));
		notification->setFocus();
	});
	notification->onPopupOpened([status] { setStatus(status, _T("Tray popup opened")); });
	notification->onPopupClosed([status] { setStatus(status, _T("Tray popup closed")); });
	window->onDpiResourcesChanged(
		[notification, dpiTrayIcon = trayIcon](const dwt::DpiResourceEvent& event) mutable {
			auto size = event.scale(dpiTrayIcon->getSize());
			dpiTrayIcon = dpiTrayIcon->resized(size);
			notification->setIcon(dpiTrayIcon);
		});

	combo->addValue(_T("First"));
	combo->addValue(_T("Second"));
	combo->addValue(_T("Third"));
	combo->setSelected(0);

	slider->setRange(0, 100);
	slider->setPosition(25);
	slider->setAutoTicks(true);
	slider->setTickFrequency(10);
	progress->setRange(0, 100);
	progress->setPosition(25);
	progress->setState(ProgressBar::Normal);
	spinner->setValue(25);

	SYSTEMTIME minimumDate = {};
	minimumDate.wYear = 2020;
	minimumDate.wMonth = 1;
	minimumDate.wDay = 1;
	SYSTEMTIME maximumDate = {};
	maximumDate.wYear = 2035;
	maximumDate.wMonth = 12;
	maximumDate.wDay = 31;
	dateTime->setRange(&minimumDate, &maximumDate);

	header->insert(_T("Control"), 180);
	header->insert(_T("Category"), 180);
	header->insert(_T("State"), 160);

	table->addColumn(_T("Control"), 180);
	table->addColumn(_T("Value"), 180);
	table->addColumn(_T("Notes"), 260);
	table->setFullRowSelect(true);
	table->setGridLines(true);
	table->setAlwaysShowSelection(true);
	table->setHeaderDragDrop(true);
	table->setCheckBoxes(true);
	table->setReadOnly(true);
	table->setView(LV_VIEW_DETAILS);
	table->setHoverTime(350);
	table->setOutlineColor(RGB(0, 102, 204));
	table->insert({ _T("Grid"), _T("9x4"), _T("Main responsive layout") }, 101);
	table->insert({ _T("Rebar/Toolbar"), _T("Top row"), _T("Dialog and notify actions") }, 102);
	table->insert({ _T("TableTree"), _T("Hierarchy"), _T("Parent-child list entries") }, 103);
	table->insert({ _T("VirtualTree"), _T("Large trees"), _T("Lazy tree structure") }, 104);
	table->setChecked(0, true);

	tree->addColumn(_T("Node"), 240);
	tree->addColumn(_T("Detail"), 180);
	auto treeRoot = tree->insert(_T("Widgets"), TVI_ROOT, TVI_LAST, 0, true);
	auto treeInput = tree->insert(_T("Input Controls"), treeRoot, TVI_LAST, 0, false);
	auto treeData = tree->insert(_T("Data Views"), treeRoot, TVI_LAST, 0, false);
	auto treeDialogs = tree->insert(_T("Dialogs and Notifications"), treeRoot, TVI_LAST, 0, false);
	tree->setChecked(treeInput);
	tree->setCheckState(treeData, Tree::PartiallyChecked);
	tree->setCheckState(treeDialogs, Tree::Excluded);
	tree->setDoubleBuffered();
	tree->setMultiSelect();
	tree->expand(treeRoot);

	tableTree->addColumn(_T("Item"), 220);
	tableTree->addColumn(_T("Description"), 220);
	tableTree->setFullRowSelect(true);
	tableTree->setGridLines(true);
	tableTree->setReadOnly(true);
	tableTree->insert({ _T("Containers"), _T("Grid, ScrolledContainer") }, 200);
	tableTree->insert({ _T("Views"), _T("Table, Tree, TableTree, VirtualTree") }, 201);
	tableTree->insert({ _T("Utility"), _T("MessageBox, dialogs, tray notifications") }, 202);
	tableTree->insertChild(200, 201);
	tableTree->insertChild(200, 202);
	tableTree->expand(200);

	virtualTree->addColumn(_T("Virtual Node"), 240);
	virtualTree->addColumn(_T("Role"), 180);
	virtualTree->setMultiSelect();
	auto virtualRoot = virtualTree->insert(_T("Virtual Root"), TVI_ROOT, TVI_LAST, 500, true);
	std::vector<HTREEITEM> virtualSelectedItems;
	for(int i = 0; i < 6; ++i) {
		auto section = virtualTree->insert(_T("Section ") + std::to_wstring(i + 1), virtualRoot, TVI_LAST, 600 + i, true);
		for(int j = 0; j < 3; ++j) {
			auto item = virtualTree->insert(_T("Item ") + std::to_wstring(i + 1) + _T(".") + std::to_wstring(j + 1), section, TVI_LAST, 700 + (i * 10) + j, false);
			if((i == 0 && j == 1) || (i == 3 && j == 2)) {
				virtualSelectedItems.push_back(item);
			}
		}
	}
	virtualTree->setCheckState(virtualRoot, Tree::Dimmed);
	virtualTree->expand(virtualRoot);
	for(auto item: virtualSelectedItems) {
		virtualTree->setItemSelected(item);
	}

	setRichSummary(richText);

	auto syncingRange = std::make_shared<bool>(false);
	auto marqueeActive = std::make_shared<bool>(false);

	auto showModernControls = [window, status, textInput, slider, progress, dateTime, marqueeActive] {
		const int applyStateButton = 1001;
		const int toggleMarqueeButton = 1002;
		const int normalRadio = 2001;
		const int pausedRadio = 2002;
		const int errorRadio = 2003;
		const int value = slider->getPosition();

		dwt::TaskDialog dialog(window);
		dialog
			.setTitle(_T("LibDWT Modern Controls"))
			.setMainInstruction(_T("Windows 7+ controls are active"))
			.setContent(textInput->getText())
			.setFooter(_T("The selected radio button updates the native progress-bar state."))
			.setExpandedInformation(
				_T("This dialog also demonstrates command links, custom buttons, radio buttons, ")
				_T("verification state, standard icons, callbacks, and live progress updates."))
			.setCollapsedControlText(_T("Show implementation details"))
			.setExpandedControlText(_T("Hide implementation details"))
			.setVerificationText(_T("Clear the optional date value"))
			.setMainIcon(TD_INFORMATION_ICON)
			.setFooterIcon(TD_SHIELD_ICON)
			.setCommandLinks()
			.setProgressBar()
			.addFlags(TDF_POSITION_RELATIVE_TO_WINDOW)
			.addButton(applyStateButton,
				_T("Apply progress state\nUse the selected normal, paused, or error state."))
			.addButton(toggleMarqueeButton,
				_T("Toggle marquee mode\nSwitch between determinate and indeterminate progress."))
			.addRadioButton(normalRadio, _T("Normal"))
			.addRadioButton(pausedRadio, _T("Paused"))
			.addRadioButton(errorRadio, _T("Error"))
			.setDefaultButton(applyStateButton)
			.setDefaultRadioButton(normalRadio)
			.setCommonButtons(TDCBF_CANCEL_BUTTON)
			.onEvent([value](HWND dialogWindow, UINT notification, WPARAM, LPARAM) -> HRESULT {
				if(notification == TDN_CREATED) {
					::SendMessage(dialogWindow, TDM_SET_PROGRESS_BAR_RANGE, 0, MAKELPARAM(0, 100));
					::SendMessage(dialogWindow, TDM_SET_PROGRESS_BAR_POS, value, 0);
				}
				return S_OK;
			});

		auto result = dialog.show();
		if(result.verificationChecked) {
			dateTime->setNone();
		}

		if(result.button == applyStateButton) {
			*marqueeActive = false;
			progress->setMarquee(false);
			auto state = ProgressBar::Normal;
			dwt::tstring stateName = _T("normal");
			if(result.radioButton == pausedRadio) {
				state = ProgressBar::Paused;
				stateName = _T("paused");
			} else if(result.radioButton == errorRadio) {
				state = ProgressBar::Error;
				stateName = _T("error");
			}
			progress->setState(state);
			progress->setPosition(value);
			setStatus(status, _T("Progress state: ") + stateName);
		} else if(result.button == toggleMarqueeButton) {
			*marqueeActive = !*marqueeActive;
			progress->setState(ProgressBar::Normal);
			progress->setMarquee(*marqueeActive, 35);
			setStatus(status, *marqueeActive ? _T("Progress marquee started") : _T("Progress marquee stopped"));
		}
	};

	buttonRun->onClicked(showModernControls);
	buttonRun->onPointerDown([status](const dwt::PointerEvent& event) {
		setStatus(status, _T("Pointer ") + pointerTypeName(event.type) +
			_T(" down, ID ") + std::to_wstring(event.id));
		return false;
	});

	checkOption->onClicked([checkOption, status] {
		setStatus(status, checkOption->getChecked() ? _T("CheckBox enabled") : _T("CheckBox disabled"));
	});

	radioA->onClicked([status] { setStatus(status, _T("Mode A selected")); });
	radioB->onClicked([status] { setStatus(status, _T("Mode B selected")); });

	combo->onSelectionChanged([combo, status] {
		auto index = combo->getSelected();
		auto text = combo->getValue(index >= 0 ? index : 0);
		setStatus(status, _T("ComboBox: ") + text);
	});

	slider->onScrollHorz([slider, progress, spinner, status, syncingRange] {
		if(*syncingRange) {
			return;
		}
		*syncingRange = true;
		auto value = slider->getPosition();
		progress->setPosition(value);
		spinner->setValue(value);
		setStatus(status, _T("Slider: ") + std::to_wstring(value));
		*syncingRange = false;
	});

	spinner->onUpdate([slider, progress, status, syncingRange](int value, int) {
		if(*syncingRange) {
			return true;
		}
		*syncingRange = true;
		slider->setPosition(value);
		progress->setPosition(value);
		setStatus(status, _T("Spinner: ") + std::to_wstring(value));
		*syncingRange = false;
		return true;
	});

	dateTime->onDateTimeChanged([dateTime, status](const SYSTEMTIME&) {
		auto value = dateTime->getValue();
		if(!value) {
			setStatus(status, _T("Date cleared"));
			return;
		}
		setStatus(status, _T("Date changed: ") + std::to_wstring(value->wYear) +
			_T("-") + std::to_wstring(value->wMonth) + _T("-") + std::to_wstring(value->wDay));
	});

	table->onColumnClick([status](int col) {
		setStatus(status, _T("Table column clicked: ") + std::to_wstring(col));
	});
	table->onItemActivate([status](const NMITEMACTIVATE& item) {
		setStatus(status, _T("Table item activated: ") + std::to_wstring(item.iItem));
	});
	table->onBeginDrag([status](const NMLISTVIEW& item) {
		setStatus(status, _T("Table drag started: ") + std::to_wstring(item.iItem));
	});
	table->onItemChanged([status](const NMLISTVIEW& item) {
		if(item.uChanged & LVIF_STATE) {
			setStatus(status, _T("Table item state changed: ") +
				std::to_wstring(item.iItem));
		}
	});
	table->onListKeyDown([status](const NMLVKEYDOWN& key) {
		setStatus(status, _T("Table key: ") + std::to_wstring(key.wVKey));
	});

	tree->onSelectionChanged([tree, status] {
		setStatus(status, _T("Tree selected items: ") +
			std::to_wstring(tree->getSelectedItems().size()));
	});
	tree->onItemChanged([status](const NMTVITEMCHANGE&) {
		setStatus(status, _T("Tree item state changed"));
	});
	tree->onBeginDrag([status](const NMTREEVIEW&) {
		setStatus(status, _T("Tree drag started"));
	});
	tree->onGetInfoTip([tree](HTREEITEM item, LPARAM) {
		return _T("Tree item: ") + tree->getText(item);
	});
	tree->onClicked([tree, status] {
		auto item = tree->getSelected();
		if(item) {
			setStatus(status, _T("Tree checkbox state: ") +
				std::to_wstring(tree->getCheckState(item)));
		}
	});
	virtualTree->onSelectionChanged([virtualTree, status] {
		setStatus(status, _T("VirtualTree selected items: ") +
			std::to_wstring(virtualTree->getSelectedItems().size()));
	});

	const GUID loadDialogGuid =
		{ 0x7de5da90, 0x5a19, 0x48be, { 0xa8, 0x87, 0x5e, 0x1b, 0xe0, 0x20, 0x61, 0x51 } };
	const GUID saveDialogGuid =
		{ 0x25bd6f94, 0x8b7f, 0x45b0, { 0xa4, 0x98, 0xb8, 0x8d, 0xc6, 0x5f, 0x52, 0xb2 } };
	const GUID folderDialogGuid =
		{ 0x114f332a, 0xc51b, 0x46ec, { 0xb2, 0x2c, 0xb3, 0xc5, 0x4a, 0xc5, 0x8e, 0xa0 } };

	dwt::util::win32::FileDialogEvents loadDialogEvents;
	loadDialogEvents.selectionChanged = [status](IFileDialog*) {
		setStatus(status, _T("Open dialog selection changed"));
	};
	loadDialogEvents.typeChanged = [status](IFileDialog*) {
		setStatus(status, _T("Open dialog file type changed"));
	};

	loadDialog
		.setTitle(_T("Open with IFileOpenDialog"))
		.setClientGuid(loadDialogGuid)
		.setFileDialogEvents(loadDialogEvents)
		.onFileDialogCustomize([](IFileDialogCustomize& customize) {
			customize.StartVisualGroup(3000, _T("DWT custom controls"));
			customize.AddCheckButton(3001, _T("Custom checkbox from IFileDialogCustomize"), TRUE);
			customize.EndVisualGroup();
		})
		.addFilter(_T("All Files"), _T("*.*"));
	saveDialog
		.setTitle(_T("Save with IFileSaveDialog"))
		.setClientGuid(saveDialogGuid)
		.setDefaultExtension(_T("txt"))
		.addFilter(_T("Text Files"), _T("*.txt"))
		.addFilter(_T("All Files"), _T("*.*"));
	folderDialog
		.setTitle(_T("Choose a folder with IFileOpenDialog"))
		.setClientGuid(folderDialogGuid);

	toolbar->addButton("msg", -1, _T("Message"), true, 0, [status, &messageBox] {
		messageBox.show(_T("Toolbar action triggered."), _T("DWT MessageBox"), dwt::MessageBox::BOX_OK, dwt::MessageBox::BOX_ICONINFORMATION);
		setStatus(status, _T("MessageBox opened"));
	});
	toolbar->addButton("task", -1, _T("Task Dialog"), true, 0, showModernControls);
	toolbar->addButton("color", -1, _T("Color"), true, 0, [status, &colorDialog, &colorParams] {
		if(colorDialog.open(colorParams)) {
			setStatus(status, _T("Color selected"));
		}
	});
	toolbar->addButton("font", -1, _T("Font"), true, 0, [status, &fontDialog] {
		LOGFONT lf = {};
		lf.lfHeight = 18;
		_tcscpy(lf.lfFaceName, _T("Segoe UI"));
		COLORREF color = RGB(30, 30, 30);
		if(fontDialog.open(lf, color)) {
			setStatus(status, _T("Font selected"));
		}
	});
	toolbar->addButton("open", -1, _T("Open"), true, 0, [status, &loadDialog] {
		dwt::tstring file;
		if(loadDialog.open(file)) {
			setStatus(status, _T("Opened: ") + file);
		}
	});
	toolbar->addButton("shell", -1, _T("Shell Item"), true, 0, [status, &loadDialog] {
		dwt::util::win32::FileDialogResult result;
		if(loadDialog.openShellItem(result)) {
			dwt::tstring name;
			if(!result.getDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, name)) {
				name = result.path;
			}
			setStatus(status, _T("Shell item: ") + name);
		}
	});
	toolbar->addButton("save", -1, _T("Save"), true, 0, [status, &saveDialog] {
		dwt::tstring file;
		if(saveDialog.open(file)) {
			setStatus(status, _T("Saved to: ") + file);
		}
	});
	toolbar->addButton("folder", -1, _T("Folder"), true, 0, [status, &folderDialog] {
		dwt::tstring dir;
		if(folderDialog.open(dir)) {
			setStatus(status, _T("Folder: ") + dir);
		}
	});
	toolbar->addButton("notify", -1, _T("Notify"), true, 0, [status, notification] {
		notification->addMessage(_T("DWT Notification"), _T("Toolbar-triggered balloon message"), [status] {
			setStatus(status, _T("Balloon clicked"));
		});
		setStatus(status, _T("Tray notification sent"));
	});
	toolbar->setLayout({ "msg", "task", "color", "font", "", "open", "shell", "save", "folder", "", "notify" });
	toolbar->refresh();
	rebar->add(toolbar);
	rebar->refresh();

	grid->setWidget(rebar, 0, 0, 1, 4);
	grid->setWidget(labelInput, 1, 0);
	grid->setWidget(textInput, 1, 1, 1, 3);
	grid->setWidget(buttonRun, 2, 0);
	grid->setWidget(checkOption, 2, 1);
	grid->setWidget(radioA, 2, 2);
	grid->setWidget(radioB, 2, 3);
	grid->setWidget(combo, 3, 0);
	grid->setWidget(dateTime, 3, 1);
	grid->setWidget(spinner, 3, 2);
	grid->setWidget(slider, 3, 3);
	grid->setWidget(progress, 4, 0, 1, 2);
	grid->setWidget(link, 4, 2, 1, 2);
	grid->setWidget(header, 5, 0, 1, 4);
	grid->setWidget(table, 6, 0, 1, 2);
	grid->setWidget(tree, 6, 2, 1, 2);
	grid->setWidget(tableTree, 7, 0);
	grid->setWidget(virtualTree, 7, 1);
	grid->setWidget(scrolled, 7, 2);
	grid->setWidget(richText, 7, 3);
	auto layout = [window, grid, rebar, scrolled, scrolledGrid, status] {
		auto client = window->getClientSize();
		auto statusSize = status->getPreferredSize();
		const long statusHeight = statusSize.y > 0 ? statusSize.y : 24;
		const long contentHeight = client.y > (statusHeight + 4) ? (client.y - statusHeight - 4) : 0;

		grid->resize(dwt::Rectangle(0, 0, client.x, contentHeight));

		// Keep a stable 60/40 split between the two large content rows.
		const long lowerTopRowsMin = 230; // rows 0-5 combined practical minimum
		const long row7Min = 160;
		long row7Target = static_cast<long>((contentHeight * 40) / 100);
		if(row7Target < row7Min) {
			row7Target = row7Min;
		}
		if(contentHeight - row7Target < lowerTopRowsMin) {
			row7Target = contentHeight > lowerTopRowsMin ? contentHeight - lowerTopRowsMin : row7Min;
		}
		if(row7Target < row7Min) {
			row7Target = row7Min;
		}
		grid->row(7).size = row7Target;

		grid->layout();
		rebar->refresh();
		scrolled->layout();
		scrolledGrid->layout();

		status->resize(dwt::Rectangle(0, contentHeight + 4, client.x, statusHeight));
		status->refresh();
	};

	window->onSized([layout](const dwt::SizedEvent&) {
		layout();
	});
	window->onDpiChanged([status](const dwt::DpiChangedEvent& event) {
		setStatus(status, _T("DPI changed: ") + std::to_wstring(event.oldDpi) +
			_T(" -> ") + std::to_wstring(event.newDpi));
	});

	window->onDestroy([notification, toolTip] {
		toolTip->close();
		notification->setVisible(false);
		::PostQuitMessage(0);
	});

	std::vector<Control*> visibleControls = {
		grid,
		status,
		rebar,
		toolbar,
		labelInput,
		textInput,
		buttonRun,
		checkOption,
		radioA,
		radioB,
		combo,
		dateTime,
		spinner,
		slider,
		progress,
		link,
		header,
		table,
		tree,
		tableTree,
		virtualTree,
		scrolled,
		scrolledGrid,
		scrollLabel1,
		scrollLabel2,
		scrollLabel3,
		scrollLabel4,
		richText,
	};
	for(auto* control : visibleControls) {
		if(control) {
			control->setVisible(true);
		}
	}

	layout();
	setStatus(status, _T("Ready"));

	window->setVisible(true);
	window->setFocus();

	app.run();
	return 0;
}
