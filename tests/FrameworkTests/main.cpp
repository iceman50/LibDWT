#include <dwt/Accessibility.h>
#include <dwt/Application.h>
#include <dwt/Events.h>
#include <dwt/Message.h>
#include <dwt/Taskbar.h>
#include <dwt/Widget.h>
#include <dwt/WidgetCreator.h>
#include <dwt/util/win32/Dpi.h>
#include <dwt/util/win32/FileDialog.h>
#include <dwt/widgets/Header.h>
#include <dwt/widgets/Table.h>
#include <dwt/widgets/Notification.h>
#include <dwt/widgets/Tree.h>
#include <dwt/widgets/VirtualTree.h>
#include <dwt/widgets/Window.h>

#include <algorithm>
#include <cstring>
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

void pumpMessages() {
	MSG message;
	while(::PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
		::TranslateMessage(&message);
		::DispatchMessage(&message);
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

	THUMBBUTTON thumbnailButton = { };
	thumbnailButton.dwMask = THB_TOOLTIP | THB_FLAGS;
	thumbnailButton.dwFlags = THBF_ENABLED;
	thumbnailButton.iId = 1;
	check((thumbnailButton.dwMask & THB_FLAGS) &&
		thumbnailButton.dwFlags == THBF_ENABLED &&
		thumbnailButton.iId == 1,
		"taskbar thumbnail button value contract");

	JumpList jumpList;
	jumpList.appId = _T("LibDWT.FrameworkTests");
	jumpList.showFrequent = true;
	JumpListCategory category(_T("Examples"));
	JumpListLink link;
	link.title = _T("Open Example");
	link.arguments = _T("--example");
	category.links.push_back(link);
	jumpList.categories.push_back(category);
	check(jumpList.showFrequent && !jumpList.showRecent &&
		jumpList.categories.front().links.front().title == _T("Open Example"),
		"taskbar jump list value contract");

	Notification::MessageOptions notificationOptions;
	notificationOptions.realTime = true;
	notificationOptions.respectQuietTime = true;
	notificationOptions.largeIcon = true;
	notificationOptions.noSound = true;
	check(notificationOptions.realTime && notificationOptions.respectQuietTime &&
		notificationOptions.largeIcon && notificationOptions.noSound,
		"notification message options value contract");
}

void testInputEventContracts() {
	using namespace dwt;

	MSG pointer = { };
	pointer.message = WM_POINTERUP;
	pointer.wParam = MAKEWPARAM(42, POINTER_MESSAGE_FLAG_PRIMARY |
		POINTER_MESSAGE_FLAG_INCONTACT | POINTER_MESSAGE_FLAG_CANCELED);
	pointer.lParam = MAKELPARAM(10, 20);
	PointerEvent pointerEvent(pointer);
	check(pointerEvent.id == 42, "pointer id decoding");
	check(pointerEvent.primary && pointerEvent.inContact && pointerEvent.canceled,
		"pointer flag decoding");
	check(pointerEvent.pos.x() == 10 && pointerEvent.pos.y() == 20,
		"pointer position decoding");

	GESTURENOTIFYSTRUCT notify = { sizeof(GESTURENOTIFYSTRUCT) };
	notify.dwFlags = GF_BEGIN;
	notify.ptsLocation.x = 30;
	notify.ptsLocation.y = 40;
	notify.dwInstanceID = 7;
	MSG gestureNotify = { };
	gestureNotify.message = WM_GESTURENOTIFY;
	gestureNotify.lParam = reinterpret_cast<LPARAM>(&notify);
	GestureNotifyEvent notifyEvent(gestureNotify);
	check(notifyEvent.valid && notifyEvent.flags == GF_BEGIN &&
		notifyEvent.instanceId == 7, "gesture notify decoding");
	check(notifyEvent.pos.x() == 30 && notifyEvent.pos.y() == 40,
		"gesture notify position decoding");
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

	FileDialogControlItem item(7, _T("Choice"));
	check(item.id == 7 && item.label == _T("Choice"),
		"file dialog control item value contract");

	options.controls = [](FileDialogControls&) { };
	check(static_cast<bool>(options.controls),
		"file dialog typed controls callback contract");
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

BOOL CALLBACK countMonitor(HMONITOR, HDC, LPRECT, LPARAM value) {
	auto count = reinterpret_cast<unsigned*>(value);
	++*count;
	return TRUE;
}

void testLiveDpiAndSettingsValidation() {
	using namespace dwt;

	Window::Seed windowSeed(_T("FrameworkTests Live Validation"));
	windowSeed.style &= ~WS_VISIBLE;
	windowSeed.location = dwt::Rectangle(10, 10, 320, 240);
	auto* window = WidgetCreator<Window>::create(windowSeed);

	check(window && ::IsWindow(window->handle()),
		"live validation window creation");
	if(!window || !::IsWindow(window->handle())) {
		return;
	}

	const auto initialDpi = window->getDpi();
	check(initialDpi > 0, "live validation window DPI");
	check(window->getSystemMetric(SM_CXVSCROLL) > 0,
		"live validation DPI-aware system metrics");

	RECT adjusted = { 0, 0, 320, 240 };
	check(window->adjustWindowRect(adjusted),
		"live validation DPI-aware window rectangle");

	bool resourceChanged = false;
	bool dpiChanged = false;
	window->onDpiResourcesChanged([&](const DpiResourceEvent& event) {
		resourceChanged = true;
		check(event.oldDpi == initialDpi,
			"live validation previous resource DPI");
		check(event.newDpi != event.oldDpi,
			"live validation changed resource DPI");
	});
	window->onDpiChanged([&](const DpiChangedEvent& event) {
		dpiChanged = true;
		check(event.oldDpi == initialDpi,
			"live validation previous DPI");
		check(event.newDpi != event.oldDpi,
			"live validation changed DPI");
		check(event.suggestedBounds.size.x > 0 &&
			event.suggestedBounds.size.y > 0,
			"live validation suggested bounds");
	});

	RECT suggested = { 20, 20, 420, 320 };
	const auto nextDpi = initialDpi == 144 ? 96U : 144U;
	window->sendMessage(WM_DPICHANGED, MAKEWPARAM(nextDpi, nextDpi),
		reinterpret_cast<LPARAM>(&suggested));
	check(dpiChanged, "live validation WM_DPICHANGED callbacks");
	check(resourceChanged,
		"live validation DPI resource callbacks");
	check(window->getDpi() > 0,
		"live validation effective window DPI");

	FontPtr headerFont(new Font(Font::DefaultGui));
	Header::Seed headerSeed;
	headerSeed.style |= HDS_FILTERBAR;
	headerSeed.font = headerFont;
	auto* header = WidgetCreator<Header>::create(window, headerSeed);
	header->insert(_T("Filter"), 120);
	check(reinterpret_cast<HFONT>(header->sendMessage(WM_GETFONT)) ==
		headerFont->handle(), "live validation header seed font");

	bool settingsChanged = false;
	window->onSystemSettingsChanged([&](const SystemSettingsEvent& event) {
		settingsChanged = true;
		check(event.action == SPI_SETHIGHCONTRAST,
			"live validation system setting action");
		check(event.accessibilityChanged(),
			"live validation accessibility settings classification");
	});
	const TCHAR section[] = _T("Accessibility");
	window->sendMessage(WM_SETTINGCHANGE, SPI_SETHIGHCONTRAST,
		reinterpret_cast<LPARAM>(section));
	check(settingsChanged, "live validation WM_SETTINGCHANGE callbacks");

	unsigned monitorCount = 0;
	::EnumDisplayMonitors(nullptr, nullptr, countMonitor,
		reinterpret_cast<LPARAM>(&monitorCount));
	check(monitorCount > 0, "live validation display monitor enumeration");

	window->close();
	pumpMessages();
}

void testLiveAccessibilityValidation() {
	using namespace dwt;

	Window::Seed windowSeed(_T("FrameworkTests UIA Validation"));
	windowSeed.style &= ~WS_VISIBLE;
	windowSeed.location = dwt::Rectangle(10, 10, 240, 160);
	auto* window = WidgetCreator<Window>::create(windowSeed);

	check(window && ::IsWindow(window->handle()),
		"UIA validation window creation");
	if(!window || !::IsWindow(window->handle())) {
		return;
	}

	window->enableAccessibility(accessibility::Window);
	window->setAccessibleName(_T("FrameworkTests UIA Validation"));
	window->setAccessibleHelpText(_T("Validation provider"));
	window->setAccessibleKeyboardFocusable(true);

	check(window->accessibilityEnabled(), "UIA validation provider enabled");
	check(window->getAccessibleName() == _T("FrameworkTests UIA Validation"),
		"UIA validation provider name");
	check(window->getAccessibleHelpText() == _T("Validation provider"),
		"UIA validation provider help text");
	check(window->getAccessibleControlType() == accessibility::Window,
		"UIA validation provider control type");
	check(window->getAccessibleKeyboardFocusable(),
		"UIA validation keyboard-focusable state");

	// UiaRootObjectId is -25. Sending WM_GETOBJECT verifies the live provider
	// return path without requiring an external UIA client in ordinary test runs.
	auto result = window->sendMessage(WM_GETOBJECT, 0, static_cast<LPARAM>(-25));
	check(result != 0, "UIA validation raw element provider");

	window->close();
	pumpMessages();
}

void runHeadlessTests() {
	testDpiMath();
	testSystemSettings();
	testAccessibilityContract();
	testControlContracts();
	testInputEventContracts();
	testVirtualTreeSelection();
	testFileDialogContracts();
	testMessageContracts();
}

void runLiveValidationTests() {
	testLiveDpiAndSettingsValidation();
	testLiveAccessibilityValidation();
}

bool hasArgument(int argc, char* argv[], const char* value) {
	for(int i = 1; i < argc; ++i) {
		if(std::strcmp(argv[i], value) == 0) {
			return true;
		}
	}
	return false;
}

}

int dwtMain(dwt::Application&) {
	return 0;
}

int main(int argc, char* argv[]) {
	dwt::Application::init();

	runHeadlessTests();
	if(hasArgument(argc, argv, "--live-validation")) {
		runLiveValidationTests();
	}

	dwt::Application::uninit();

	if(failures) {
		std::cerr << failures << " framework test(s) failed\n";
		return 1;
	}
	std::cout << "All framework tests passed\n";
	return 0;
}
