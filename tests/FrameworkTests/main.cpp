#include <dwt/Accessibility.h>
#include <dwt/Application.h>
#include <dwt/Events.h>
#include <dwt/Message.h>
#include <dwt/Widget.h>
#include <dwt/WidgetCreator.h>
#include <dwt/util/win32/Dpi.h>
#include <dwt/util/win32/FileDialog.h>
#include <dwt/widgets/Table.h>
#include <dwt/widgets/Tree.h>
#include <dwt/widgets/VirtualTree.h>
#include <dwt/widgets/Window.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
	if(!condition) {
		std::cerr << "FAIL: " << message << '\n';
		++failures;
	}
}

void testDpiMath() {
	using namespace dwt;
	using namespace dwt::util::win32;

	check(scale(100, 144) == 150, "144 DPI integer scaling");
	check(scale(-10, 192) == -20, "negative DPI scaling");
	check(scale(Point(10, 20), 144) == Point(15, 30),
		"point DPI scaling");
	check(scale(dwt::Rectangle(1, 2, 10, 20), 192) ==
		dwt::Rectangle(2, 4, 20, 40), "rectangle DPI scaling");

	DpiResourceEvent resource(96, 144);
	check(resource.scale(20) == 30, "DPI resource integer scaling");
	check(resource.scale(Point(8, 12)) == Point(12, 18),
		"DPI resource point scaling");

	RECT suggested = { 10, 20, 210, 120 };
	MSG message = { };
	message.message = WM_DPICHANGED;
	message.wParam = MAKEWPARAM(144, 144);
	message.lParam = reinterpret_cast<LPARAM>(&suggested);
	DpiChangedEvent changed(96, message);
	check(changed.oldDpi == 96 && changed.newDpi == 144,
		"DPI changed values");
	check(changed.suggestedBounds == dwt::Rectangle(suggested),
		"DPI suggested bounds");
}

void testSystemSettings() {
	using namespace dwt;
	using namespace dwt::util::win32;

	const TCHAR section[] = _T("Accessibility");
	MSG message = { };
	message.message = WM_SETTINGCHANGE;
	message.wParam = SPI_SETHIGHCONTRAST;
	message.lParam = reinterpret_cast<LPARAM>(section);
	SystemSettingsEvent event(message);
	check(event.action == SPI_SETHIGHCONTRAST,
		"settings action decoding");
	check(event.section == section, "settings section decoding");
	check(event.accessibilityChanged(),
		"accessibility settings classification");
	check(event.highContrast == Widget::isHighContrast(),
		"high contrast state consistency");

	NONCLIENTMETRICS metrics = { sizeof(NONCLIENTMETRICS) };
	check(systemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,
		sizeof(metrics), &metrics, 0, defaultDpi),
		"DPI-aware system parameter query");

	ScopedThreadDpiAwareness scope(
		ThreadDpiAwareness::PerMonitorAwareV2);
	(void)scope.changed();
}

void testAccessibilityContract() {
	using namespace dwt;

	std::vector<accessibility::ItemId> selected;
	accessibility::ItemProvider provider;
	provider.exists = [](accessibility::ItemId item) {
		return item == 1 || item == 2;
	};
	provider.children = [](accessibility::ItemId parent) {
		return parent == 0 ?
			std::vector<accessibility::ItemId> { 1, 2 } :
			std::vector<accessibility::ItemId>();
	};
	provider.name = [](accessibility::ItemId item) {
		return item == 1 ? tstring(_T("First")) : tstring(_T("Second"));
	};
	provider.select = [&selected](accessibility::ItemId item) {
		selected.assign(1, item);
	};
	provider.selection = [&selected] { return selected; };
	provider.selected = [&selected](accessibility::ItemId item) {
		return !selected.empty() && selected.front() == item;
	};
	provider.expandState = [](accessibility::ItemId item) {
		return item == 1 ? accessibility::Collapsed :
			accessibility::LeafNode;
	};

	check(provider.exists(1), "logical item existence");
	check(provider.children(0).size() == 2, "logical item hierarchy");
	check(provider.name(2) == _T("Second"), "logical item name");
	provider.select(2);
	check(provider.selected(2) && provider.selection().front() == 2,
		"logical item selection");
	check(provider.expandState(1) == accessibility::Collapsed,
		"logical item expansion state");
}

void testControlContracts() {
	using namespace dwt;

	check(Tree::NoCheckBox == 0 && Tree::Unchecked == 1 &&
		Tree::Checked == 2 && Tree::PartiallyChecked == 3 &&
		Tree::Excluded == 4 && Tree::Dimmed == 5,
		"tree checkbox state image indexes");

	Table::FooterInfo footer { _T("Footer"), 2 };
	Table::FooterItem item { 1, _T("Item"), LVFIS_FOCUSED };
	check(footer.itemCount == 2 && item.index == 1 &&
		(item.state & LVFIS_FOCUSED), "table footer value contracts");
}

void testVirtualTreeSelection() {
	using namespace dwt;

	Window::Seed windowSeed(_T("FrameworkTests"));
	windowSeed.style &= ~WS_VISIBLE;
	windowSeed.location = dwt::Rectangle(0, 0, 320, 240);
	auto* window = WidgetCreator<Window>::create(windowSeed);

	Tree::Seed treeSeed;
	treeSeed.style &= ~WS_VISIBLE;
	treeSeed.tvExStyle = TVS_EX_MULTISELECT;
	auto* tree = WidgetCreator<VirtualTree>::create(window, VirtualTree::Seed(treeSeed));
	tree->resize(dwt::Rectangle(0, 0, 320, 240));

	auto root = tree->insert(_T("Root"), TVI_ROOT, TVI_LAST, 1, true);
	auto first = tree->insert(_T("First"), root, TVI_LAST, 2, false);
	auto branch = tree->insert(_T("Branch"), root, TVI_LAST, 3, true);
	auto hidden = tree->insert(_T("Hidden child"), branch, TVI_LAST, 4, false);

	tree->setItemSelected(first);
	tree->setItemSelected(hidden);
	check(tree->countSelected() == 2, "virtual tree selected count");
	check(tree->getItemSelected(first) && tree->getItemSelected(hidden),
		"virtual tree selected item state");

	auto selected = tree->getSelectedItems();
	check(selected.size() == 2, "virtual tree selected enumeration size");
	check(std::find(selected.begin(), selected.end(), first) != selected.end() &&
		std::find(selected.begin(), selected.end(), hidden) != selected.end(),
		"virtual tree selected enumeration values");

	tree->collapse(branch);
	check(tree->countSelected() == 2, "virtual tree hidden selection count");
	selected = tree->getSelectedItems();
	check(selected.size() == 2 &&
		std::find(selected.begin(), selected.end(), hidden) != selected.end(),
		"virtual tree hidden selection enumeration");

	tree->expand(branch);
	tree->setSelected(hidden);
	check(tree->countSelected() == 1 && tree->getSelected() == hidden,
		"virtual tree setSelected resets to a single selected item");
	check(!tree->getItemSelected(first) && tree->getItemSelected(hidden),
		"virtual tree single selection state reset");

	window->close();
}

void testFileDialogContracts() {
	using namespace dwt::util::win32;

	FileDialogOptions options;
	check(options.forceFilesystem, "file dialogs force filesystem paths by default");

	FileDialogEvents events;
	check(events.empty(), "empty file dialog event set");
	events.fileOk = [](IFileDialog*) { return S_OK; };
	check(!events.empty(), "file dialog event set detects callbacks");

	FileDialogResult result;
	check(!result.hasPath(), "empty file dialog result has no path");
	dwt::tstring displayName;
	check(!result.getDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, displayName),
		"empty file dialog result has no display name");
}

void testMessageContracts() {
	using namespace dwt;

	MSG menuMessage = { };
	menuMessage.message = WM_COMMAND;
	menuMessage.wParam = 1234;
	check(Message(menuMessage) == Message(WM_COMMAND, 1234),
		"menu WM_COMMAND matches the command id");

	MSG controlMessage = { };
	controlMessage.message = WM_COMMAND;
	controlMessage.wParam = MAKEWPARAM(42, BN_CLICKED);
	controlMessage.lParam = reinterpret_cast<LPARAM>(HWND(1));
	check(Message(controlMessage) == Message(WM_COMMAND, BN_CLICKED),
		"control WM_COMMAND matches the notification code");
}

}

int dwtMain(dwt::Application&) {
	return 0;
}

int main() {
	dwt::Application::init();

	testDpiMath();
	testSystemSettings();
	testAccessibilityContract();
	testControlContracts();
	testVirtualTreeSelection();
	testFileDialogContracts();
	testMessageContracts();

	dwt::Application::uninit();

	if(failures) {
		std::cerr << failures << " framework test(s) failed\n";
		return 1;
	}
	std::cout << "All framework tests passed\n";
	return 0;
}
