#include <dwt/Accessibility.h>
#include <dwt/Application.h>
#include <dwt/Message.h>
#include <dwt/WidgetCreator.h>
#include <dwt/widgets/Button.h>
#include <dwt/widgets/CheckBox.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/ScrolledContainer.h>
#include <dwt/widgets/Splitter.h>
#include <dwt/widgets/SplitterContainer.h>
#include <dwt/widgets/TabView.h>
#include <dwt/widgets/TableTree.h>
#include <dwt/widgets/TextBox.h>
#include <dwt/widgets/VirtualTree.h>
#include <dwt/widgets/Window.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace {

using dwt::Button;
using dwt::CheckBox;
using dwt::Control;
using dwt::Grid;
using dwt::GridInfo;
using dwt::Label;
using dwt::ScrolledContainer;
using dwt::Splitter;
using dwt::SplitterContainer;
using dwt::TabView;
using dwt::Table;
using dwt::TableTree;
using dwt::TextBox;
using dwt::Tree;
using dwt::VirtualTree;
using dwt::WidgetCreator;
using dwt::Window;

constexpr size_t maxLogCharacters = 60000;
using TableTreeRows = std::unordered_map<LPARAM, std::vector<dwt::tstring>>;

void appendLog(TextBox::ObjectType log, const dwt::tstring& message) {
	if(!log || !::IsWindow(log->handle())) {
		return;
	}

	if(log->length() >= maxLogCharacters) {
		log->setText(_T("[log truncated to keep validation responsive]\r\n"));
	}

	SYSTEMTIME now = {};
	::GetLocalTime(&now);
	TCHAR timestamp[32] = {};
	_sntprintf_s(timestamp, _countof(timestamp), _TRUNCATE,
		_T("[%02u:%02u:%02u.%03u] "), now.wHour, now.wMinute,
		now.wSecond, now.wMilliseconds);
	log->addText(dwt::tstring(timestamp) + message + _T("\r\n"));
	log->scrollToBottom();
}

BOOL CALLBACK countMonitor(HMONITOR, HDC, LPRECT, LPARAM value) {
	auto* count = reinterpret_cast<unsigned*>(value);
	if(count) {
		++*count;
	}
	return TRUE;
}

void installTableTreeRows(TableTree::ObjectType tableTree,
	const std::shared_ptr<TableTreeRows>& rows)
{
	tableTree->addCallback(dwt::Message(WM_NOTIFY, LVN_GETDISPINFO),
		[tableTree, rows](const MSG& msg, LRESULT&) -> bool {
			auto* info = reinterpret_cast<NMLVDISPINFO*>(msg.lParam);
			if(!info || info->hdr.hwndFrom != tableTree->handle()) {
				return false;
			}

			if(info->item.mask & LVIF_TEXT) {
				auto id = info->item.lParam ? info->item.lParam :
					tableTree->getData(info->item.iItem);
				auto row = rows->find(id);
				auto column = static_cast<size_t>(info->item.iSubItem);
				if(row != rows->end() && column < row->second.size() &&
					info->item.pszText && info->item.cchTextMax > 0) {
					_tcsncpy_s(info->item.pszText, info->item.cchTextMax,
						row->second[column].c_str(), _TRUNCATE);
				}
			}

			if(info->item.mask & LVIF_IMAGE) {
				info->item.iImage = I_IMAGENONE;
			}
			return true;
		});
}

void insertTableTreeRow(TableTree::ObjectType tableTree,
	const std::shared_ptr<TableTreeRows>& rows, LPARAM id,
	const std::vector<dwt::tstring>& text)
{
	(*rows)[id] = text;
	tableTree->insert(LVIF_TEXT | LVIF_PARAM, -1, LPSTR_TEXTCALLBACK,
		0, 0, I_IMAGECALLBACK, id);
}

void configureGrid(Grid::ObjectType grid) {
	grid->setSpacing(8);
	for(size_t column = 0; column < 3; ++column) {
		grid->column(column).mode = GridInfo::FILL;
		grid->column(column).align = GridInfo::STRETCH;
	}
	grid->row(0).mode = GridInfo::AUTO;
	grid->row(1).mode = GridInfo::AUTO;
	grid->row(2).mode = GridInfo::FILL;
	grid->row(3).mode = GridInfo::STATIC;
	grid->row(3).size = 220;
	for(size_t row = 0; row < 4; ++row) {
		grid->row(row).align = GridInfo::STRETCH;
	}
}

bool runSafeChecks(Window::ObjectType window, Grid::ObjectType grid,
	TableTree::ObjectType tableTree, VirtualTree::ObjectType virtualTree,
	TabView::ObjectType tabs, SplitterContainer::ObjectType splitterContainer,
	TextBox::ObjectType log)
{
	bool allPassed = true;
	auto report = [log, &allPassed](bool passed, const dwt::tstring& description) {
		allPassed &= passed;
		appendLog(log, (passed ? _T("[PASS] ") : _T("[FAIL] ")) + description);
	};

	report(window && ::IsWindow(window->handle()), _T("top-level HWND is valid"));
	report(grid && grid->accessibilityEnabled(),
		_T("Grid UI Automation provider is enabled"));
	report(tableTree && tableTree->accessibilityEnabled(),
		_T("TableTree UI Automation provider is enabled"));
	report(virtualTree && virtualTree->accessibilityEnabled(),
		_T("VirtualTree UI Automation provider is enabled"));
	report(tabs && tabs->accessibilityEnabled(),
		_T("TabView UI Automation provider is enabled"));
	auto splitters = splitterContainer->getChildren<Splitter>();
	const bool splitterReady = splitters.first != splitters.second &&
		(*splitters.first)->accessibilityEnabled() &&
		(*splitters.first)->getAccessibleRangeValue();
	report(splitterReady, _T("Splitter RangeValue provider is enabled"));

	RECT adjusted = { 0, 0, 800, 600 };
	report(window->adjustWindowRect(adjusted),
		_T("DPI-aware window rectangle adjustment succeeds"));
	report(window->getDpi() > 0, _T("window reports a non-zero DPI"));
	report(window->getSystemMetric(SM_CXVSCROLL) > 0,
		_T("DPI-aware system metrics are available"));

	const auto provider = window->sendMessage(WM_GETOBJECT, 0,
		static_cast<LPARAM>(-25));
	report(provider != 0, _T("WM_GETOBJECT returns the root UIA provider"));

	unsigned monitors = 0;
	::EnumDisplayMonitors(nullptr, nullptr, countMonitor,
		reinterpret_cast<LPARAM>(&monitors));
	report(monitors > 0, _T("at least one display monitor is available"));
	appendLog(log, _T("Safe checks complete; no system settings were changed."));
	return allPassed;
}

} // namespace

int dwtMain(dwt::Application& app) {
	const bool selfTest = _tcsstr(::GetCommandLine(), _T("--self-test")) != nullptr;

	Window::Seed windowSeed(_T("LibDWT Framework Validation"));
	windowSeed.location = dwt::Rectangle(60, 60, 1280, 820);
	auto* window = WidgetCreator<Window>::create(windowSeed);
	window->enableAccessibility(dwt::accessibility::Window);
	window->setAccessibleName(_T("LibDWT Framework Validation"));
	window->setAccessibleHelpText(
		_T("Visible diagnostics for DPI, accessibility, themes, and custom controls."));

	auto* grid = WidgetCreator<Grid>::create(window, Grid::Seed(4, 3));
	configureGrid(grid);
	grid->setAccessibleName(_T("LibDWT framework validation surfaces"));

	auto* instructions = WidgetCreator<Label>::create(grid, Label::Seed(
		_T("Move this window between differently scaled monitors, inspect it with a UI Automation client, "
			"and change contrast or text-size settings. Events are recorded below.")));
	auto* captureButton = WidgetCreator<Button>::create(grid,
		Button::Seed(_T("Capture environment")));
	auto* checksButton = WidgetCreator<Button>::create(grid,
		Button::Seed(_T("Run safe checks")));
	auto* clearButton = WidgetCreator<Button>::create(grid,
		Button::Seed(_T("Clear event log")));

	Tree::Seed virtualSeed;
	virtualSeed.style |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	auto* virtualTree = WidgetCreator<VirtualTree>::create(grid,
		VirtualTree::Seed(virtualSeed));
	virtualTree->setAccessibleName(_T("VirtualTree validation hierarchy"));
	virtualTree->setMultiSelect(true);
	virtualTree->addColumn(_T("Virtual item"), 250);
	auto virtualRoot = virtualTree->insert(_T("Virtual root"), TVI_ROOT,
		TVI_LAST, 1000, true);
	auto virtualDpi = virtualTree->insert(_T("DPI resources"), virtualRoot,
		TVI_LAST, 1001, true);
	virtualTree->insert(_T("Fonts and image lists"), virtualDpi,
		TVI_LAST, 1002);
	virtualTree->insert(_T("Suggested window bounds"), virtualDpi,
		TVI_LAST, 1003);
	auto virtualAccessibility = virtualTree->insert(_T("Accessibility"),
		virtualRoot, TVI_LAST, 1010, true);
	virtualTree->insert(_T("Selection and focus"), virtualAccessibility,
		TVI_LAST, 1011);
	virtualTree->insert(_T("Expand and collapse"), virtualAccessibility,
		TVI_LAST, 1012);

	Table::Seed tableSeed;
	auto* tableTree = WidgetCreator<TableTree>::create(grid,
		TableTree::Seed(tableSeed));
	tableTree->setAccessibleName(_T("TableTree validation hierarchy"));
	tableTree->addColumn(_T("Validation area"), 190);
	tableTree->addColumn(_T("Expected behavior"), 260);
	tableTree->setGridLines(true);
	auto tableRows = std::make_shared<TableTreeRows>();
	installTableTreeRows(tableTree, tableRows);
	insertTableTreeRow(tableTree, tableRows, 2000,
		{ _T("UI Automation"), _T("Logical items and patterns are visible") });
	insertTableTreeRow(tableTree, tableRows, 2001,
		{ _T("Selection"), _T("SelectionItem updates without stale providers") });
	insertTableTreeRow(tableTree, tableRows, 2002,
		{ _T("Structure"), _T("Expand/collapse updates the fragment tree") });
	insertTableTreeRow(tableTree, tableRows, 2010,
		{ _T("Visual settings"), _T("Colors and text remain readable") });
	tableTree->insertChild(2000, 2001);
	tableTree->insertChild(2000, 2002);
	tableTree->expand(2000);

	TabView::Seed tabSeed(160, false, true);
	tabSeed.closeable = false;
	auto* tabs = WidgetCreator<TabView>::create(grid, tabSeed);
	tabs->setAccessibleName(_T("Validation tab view"));

	auto* accessibilityPage = WidgetCreator<Grid>::create(tabs, Grid::Seed(4, 1));
	accessibilityPage->setText(_T("Accessibility"));
	accessibilityPage->setSpacing(8);
	accessibilityPage->column(0).mode = GridInfo::FILL;
	accessibilityPage->column(0).align = GridInfo::STRETCH;
	for(size_t row = 0; row < 4; ++row) {
		accessibilityPage->row(row).mode = GridInfo::AUTO;
		accessibilityPage->row(row).align = GridInfo::STRETCH;
	}
	auto* accessibilityLabel = WidgetCreator<Label>::create(accessibilityPage,
		Label::Seed(_T("Inspect names, focus, selection, and invocation.")));
	auto* focusableButton = WidgetCreator<Button>::create(accessibilityPage,
		Button::Seed(_T("Invokable validation button")));
	auto* enabledCheck = WidgetCreator<CheckBox>::create(accessibilityPage,
		CheckBox::Seed(_T("Focusable checked state")));
	auto* providerLabel = WidgetCreator<Label>::create(accessibilityPage,
		Label::Seed(_T("Expected: no stale-element or missing-provider errors.")));
	accessibilityPage->setWidget(accessibilityLabel, 0, 0);
	accessibilityPage->setWidget(focusableButton, 1, 0);
	accessibilityPage->setWidget(enabledCheck, 2, 0);
	accessibilityPage->setWidget(providerLabel, 3, 0);

	auto* scrollingPage = WidgetCreator<Grid>::create(tabs, Grid::Seed(1, 1));
	scrollingPage->setText(_T("Scrolling"));
	scrollingPage->row(0).mode = GridInfo::FILL;
	scrollingPage->row(0).align = GridInfo::STRETCH;
	scrollingPage->column(0).mode = GridInfo::FILL;
	scrollingPage->column(0).align = GridInfo::STRETCH;
	auto* scrolled = WidgetCreator<ScrolledContainer>::create(scrollingPage,
		ScrolledContainer::Seed());
	scrolled->setAccessibleName(_T("Scrollable validation content"));
	auto* scrollContent = scrolled->addChild(Grid::Seed(8, 1));
	scrollContent->setSpacing(8);
	scrollContent->column(0).mode = GridInfo::STATIC;
	scrollContent->column(0).size = 700;
	scrollContent->column(0).align = GridInfo::STRETCH;
	std::vector<Label::ObjectType> scrollLabels;
	for(size_t row = 0; row < 8; ++row) {
		scrollContent->row(row).mode = GridInfo::STATIC;
		scrollContent->row(row).size = 55;
		scrollContent->row(row).align = GridInfo::STRETCH;
		auto* label = WidgetCreator<Label>::create(scrollContent,
			Label::Seed(_T("Scrollable UIA row ") + std::to_wstring(row + 1)));
		scrollContent->setWidget(label, row, 0);
		scrollLabels.push_back(label);
	}
	scrollingPage->setWidget(scrolled, 0, 0);

	tabs->add(accessibilityPage);
	tabs->add(scrollingPage);

	auto* splitterPage = WidgetCreator<Grid>::create(tabs, Grid::Seed(1, 1));
	splitterPage->setText(_T("Splitter"));
	splitterPage->row(0).mode = GridInfo::FILL;
	splitterPage->row(0).align = GridInfo::STRETCH;
	splitterPage->column(0).mode = GridInfo::FILL;
	splitterPage->column(0).align = GridInfo::STRETCH;
	auto* splitterContainer = WidgetCreator<SplitterContainer>::create(splitterPage,
		SplitterContainer::Seed(0.5, false));
	splitterContainer->setAccessibleName(_T("RangeValue splitter validation"));
	auto* leftPane = splitterContainer->addChild(Grid::Seed(1, 1));
	auto* rightPane = splitterContainer->addChild(Grid::Seed(1, 1));
	leftPane->row(0).mode = rightPane->row(0).mode = GridInfo::FILL;
	leftPane->row(0).align = rightPane->row(0).align = GridInfo::STRETCH;
	leftPane->column(0).mode = rightPane->column(0).mode = GridInfo::FILL;
	leftPane->column(0).align = rightPane->column(0).align = GridInfo::STRETCH;
	auto* leftLabel = WidgetCreator<Label>::create(leftPane,
		Label::Seed(_T("Move the splitter and inspect its RangeValue pattern.")));
	auto* rightLabel = WidgetCreator<Label>::create(rightPane,
		Label::Seed(_T("Keyboard focus and bounds should remain coherent.")));
	leftPane->setWidget(leftLabel, 0, 0);
	rightPane->setWidget(rightLabel, 0, 0);
	// Materialize the splitter before the hidden page is laid out asynchronously.
	splitterContainer->layout();
	for(auto splitters = splitterContainer->getChildren<Splitter>();
		splitters.first != splitters.second; ++splitters.first) {
		(*splitters.first)->setVisible(true);
	}
	splitterPage->setWidget(splitterContainer, 0, 0);
	tabs->add(splitterPage);
	tabs->setActive(accessibilityPage);

	TextBox::Seed logSeed;
	logSeed.style &= ~ES_AUTOHSCROLL;
	logSeed.style |= ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL;
	logSeed.lines = 10;
	auto* log = WidgetCreator<TextBox>::create(grid, logSeed);
	log->setAccessibleName(_T("LibDWT framework validation event log"));
	log->setReadOnly(true);

	grid->setWidget(instructions, 0, 0, 1, 3);
	grid->setWidget(captureButton, 1, 0);
	grid->setWidget(checksButton, 1, 1);
	grid->setWidget(clearButton, 1, 2);
	grid->setWidget(virtualTree, 2, 0);
	grid->setWidget(tableTree, 2, 1);
	grid->setWidget(tabs, 2, 2);
	grid->setWidget(log, 3, 0, 1, 3);

	auto captureEnvironment = [window, log] {
		unsigned monitors = 0;
		::EnumDisplayMonitors(nullptr, nullptr, countMonitor,
			reinterpret_cast<LPARAM>(&monitors));
		auto client = window->getClientSize();
		appendLog(log, _T("Environment: DPI=") + std::to_wstring(window->getDpi()) +
			_T(", client=") + std::to_wstring(client.x) + _T("x") +
			std::to_wstring(client.y) + _T(", monitors=") +
			std::to_wstring(monitors) + _T(", high contrast=") +
			(window->isHighContrast() ? _T("on") : _T("off")));
	};

	captureButton->onClicked(captureEnvironment);
	checksButton->onClicked([window, grid, tableTree, virtualTree, tabs,
		splitterContainer, log] {
		runSafeChecks(window, grid, tableTree, virtualTree, tabs,
			splitterContainer, log);
	});
	clearButton->onClicked([log] {
		log->setText(_T(""));
		appendLog(log, _T("Event log cleared."));
	});
	focusableButton->onClicked([log] {
		appendLog(log, _T("Invokable validation button activated."));
	});
	enabledCheck->onClicked([enabledCheck, log] {
		appendLog(log, enabledCheck->getChecked() ?
			_T("Validation checkbox checked.") :
			_T("Validation checkbox unchecked."));
	});
	virtualTree->onClicked([virtualTree, log] {
		appendLog(log, _T("VirtualTree selection count: ") +
			std::to_wstring(virtualTree->countSelected()));
	});
	tableTree->onClicked([log] {
		appendLog(log, _T("TableTree item interaction received."));
	});

	window->onDpiResourcesChanged([log](const dwt::DpiResourceEvent& event) {
		appendLog(log, _T("DPI resources: ") + std::to_wstring(event.oldDpi) +
			_T(" -> ") + std::to_wstring(event.newDpi));
	});
	window->onDpiChanged([log](const dwt::DpiChangedEvent& event) {
		appendLog(log, _T("DPI changed: ") + std::to_wstring(event.oldDpi) +
			_T(" -> ") + std::to_wstring(event.newDpi) +
			_T(", suggested bounds ") +
			std::to_wstring(event.suggestedBounds.size.x) + _T("x") +
			std::to_wstring(event.suggestedBounds.size.y));
	});
	window->onThemeChanged([log] {
		appendLog(log, _T("WM_THEMECHANGED received."));
	});
	window->onSystemColorsChanged([log] {
		appendLog(log, _T("WM_SYSCOLORCHANGE received."));
	});
	window->onSystemSettingsChanged([log](const dwt::SystemSettingsEvent& event) {
		appendLog(log, _T("WM_SETTINGCHANGE action=") +
			std::to_wstring(event.action) + _T(", section=") + event.section +
			_T(", high contrast=") + (event.highContrast ? _T("on") : _T("off")));
	});

	auto layout = [window, grid] {
		grid->resize(dwt::Rectangle(window->getClientSize()));
	};
	window->onSized([layout](const dwt::SizedEvent&) {
		layout();
	});
	window->onDestroy([] {
		::PostQuitMessage(0);
	});

	std::vector<Control*> visibleControls = {
		grid, instructions, captureButton, checksButton, clearButton,
		virtualTree, tableTree, tabs, accessibilityLabel, focusableButton,
		enabledCheck, providerLabel, scrolled, scrollContent, splitterContainer,
		leftPane, rightPane, leftLabel, rightLabel, log
	};
	for(auto* label : scrollLabels) {
		visibleControls.push_back(label);
	}
	for(auto* control : visibleControls) {
		if(control) {
			control->setVisible(true);
		}
	}

	layout();
	appendLog(log, _T("LibDWT framework validation surface ready."));
	captureEnvironment();
	appendLog(log,
		_T("Tip: run Accessibility Insights or Inspect while interacting with each pane."));
	if(selfTest) {
		const dwt::Rectangle resizeCases[] = {
			dwt::Rectangle(60, 60, 900, 600),
			dwt::Rectangle(60, 60, 500, 350),
			dwt::Rectangle(60, 60, 1400, 900),
			dwt::Rectangle(60, 60, 320, 240),
			dwt::Rectangle(60, 60, 1280, 820)
		};
		for(const auto& bounds : resizeCases) {
			window->resize(bounds);
			app.processMessages();
		}
		appendLog(log, _T("Resize lifecycle checks complete."));
		const double splitterPositions[] = {
			0.05, 0.95, 0.15, 0.85, 0.25, 0.75, 0.35, 0.65, 0.5
		};
		for(unsigned pass = 0; pass < 8; ++pass) {
			for(auto position : splitterPositions) {
				splitterContainer->setSplitter(0, position);
				splitterContainer->layout();
				app.processMessages();
			}
		}
		appendLog(log, _T("Splitter repaint stress checks complete."));
		const bool passed = runSafeChecks(window, grid, tableTree, virtualTree,
			tabs, splitterContainer, log);
		window->close();
		app.processMessages();
		return passed ? 0 : 1;
	}

	window->setVisible(true);
	window->setFocus();
	app.run();
	return 0;
}
