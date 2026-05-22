#include <dwt/Application.h>
#include <dwt/WidgetCreator.h>

#include <dwt/widgets/Button.h>
#include <dwt/widgets/CheckBox.h>
#include <dwt/widgets/ComboBox.h>
#include <dwt/widgets/Control.h>
#include <dwt/widgets/DateTime.h>
#include <dwt/widgets/GroupBox.h>
#include <dwt/widgets/Header.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/Link.h>
#include <dwt/widgets/ProgressBar.h>
#include <dwt/widgets/RadioButton.h>
#include <dwt/widgets/RichTextBox.h>
#include <dwt/widgets/Slider.h>
#include <dwt/widgets/Spinner.h>
#include <dwt/widgets/StatusBar.h>
#include <dwt/widgets/Table.h>
#include <dwt/widgets/TextBox.h>
#include <dwt/widgets/Tree.h>
#include <dwt/widgets/Window.h>
#include <dwt/resources/Font.h>

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

long clampMin(long value, long minValue) {
	return value < minValue ? minValue : value;
}

using dwt::Button;
using dwt::CheckBox;
using dwt::ComboBox;
using dwt::Control;
using dwt::DateTime;
using dwt::GroupBox;
using dwt::Header;
using dwt::Label;
using dwt::Link;
using dwt::ProgressBar;
using dwt::RadioButton;
using dwt::RichTextBox;
using dwt::Slider;
using dwt::Spinner;
using dwt::StatusBar;
using dwt::Table;
using dwt::TextBox;
using dwt::Tree;
using dwt::WidgetCreator;
using dwt::Window;

void setStatus(StatusBar::ObjectType status, const dwt::tstring& text) {
	status->setText(0, text, true);
	status->setText(1, _T("MultiControlExample"));
}

void setRichSummary(RichTextBox::ObjectType richText) {
	const char* rtf =
		"{\\rtf1\\ansi\\ansicpg1252\\deff0"
		"{\\fonttbl{\\f0 Segoe UI;}{\\f1 Consolas;}}"
		"{\\colortbl;\\red0\\green0\\blue0;\\red0\\green102\\blue204;\\red0\\green128\\blue0;}"
		"\\viewkind4\\uc1\\pard\\sa100\\sl260\\slmult1\\f0\\fs20 "
		"\\b MultiControlExample\\b0\\par "
		"This window demonstrates a broad set of DWT controls:\\par "
		"- Button, CheckBox, RadioButton\\par "
		"- TextBox, ComboBox, Spinner, Slider, ProgressBar, DateTime\\par "
		"- Header control, Table control, and Tree control\\par "
		"- Link, Label, GroupBox, StatusBar, and RichTextBox\\par "
		"\\par "
		"Click controls to update the status bar and interact with the sample data.\\par"
		"}";

	SETTEXTEX config = { ST_DEFAULT, CP_ACP };
	richText->sendMessage(EM_SETTEXTEX, reinterpret_cast<WPARAM>(&config), reinterpret_cast<LPARAM>(rtf));
	richText->setReadOnly(true);
	richText->setSelection(0, 0);
}

} // namespace

int dwtMain(dwt::Application& app) {
	Window::Seed seed(_T("DWT MultiControlExample"));
	seed.location = dwt::Rectangle(70, 70, 1280, 840);
	auto* window = WidgetCreator<Window>::create(seed);
	auto uiFont = window->getFont();
	auto headerFont = uiFont ? uiFont->makeBold() : uiFont;

	auto* groupButtons = WidgetCreator<GroupBox>::create(window, GroupBox::Seed(_T("Button Controls")));
	auto* buttonRun = WidgetCreator<Button>::create(window, Button::Seed(_T("Run Action")));
	auto* checkOption = WidgetCreator<CheckBox>::create(window, CheckBox::Seed(_T("Enable Option")));
	auto* radioA = WidgetCreator<RadioButton>::create(window, RadioButton::Seed(_T("Mode A")));
	auto* radioB = WidgetCreator<RadioButton>::create(window, RadioButton::Seed(_T("Mode B")));

	auto* labelInput = WidgetCreator<Label>::create(window, Label::Seed(_T("Input:")));
	auto* textInput = WidgetCreator<TextBox>::create(window, TextBox::Seed(_T("Sample text")));
	auto* combo = WidgetCreator<ComboBox>::create(window, ComboBox::Seed());
	auto* dateTime = WidgetCreator<DateTime>::create(window, DateTime::Seed());
	auto* spinner = WidgetCreator<Spinner>::create(window, Spinner::Seed(0, 100));
	auto* slider = WidgetCreator<Slider>::create(window, Slider::Seed());
	auto* progress = WidgetCreator<ProgressBar>::create(window, ProgressBar::Seed());
	auto* link = WidgetCreator<Link>::create(window, Link::Seed(_T("https://dcplusplus.com"), true));
	auto* richText = WidgetCreator<RichTextBox>::create(window, RichTextBox::Seed());

	auto* header = WidgetCreator<Header>::create(window, Header::Seed());
	header->setFont(headerFont);

	Table::Seed tableSeed;
	tableSeed.font = uiFont;
	auto* table = WidgetCreator<Table>::create(window, tableSeed);

	Tree::Seed treeSeed;
	treeSeed.font = uiFont;
	auto* tree = WidgetCreator<Tree>::create(window, treeSeed);

	auto* status = WidgetCreator<StatusBar>::create(window, StatusBar::Seed(2, 1, true));
	status->setSize(0, 300);
	status->setText(1, _T("MultiControlExample"));

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
	spinner->setValue(25);

	header->insert(_T("Header Name"), 180);
	header->insert(_T("Header Value"), 140);
	header->insert(_T("Header State"), 120);

	table->addColumn(_T("Control"), 170);
	table->addColumn(_T("Value"), 150);
	table->addColumn(_T("Notes"), 220);
	if(headerFont) {
		auto tableHeaderHwnd = reinterpret_cast<HWND>(table->sendMessage(LVM_GETHEADER));
		if(tableHeaderHwnd) {
			auto* tableHeader = WidgetCreator<Header>::attach(table, tableHeaderHwnd);
			tableHeader->setFont(headerFont);
		}
	}
	table->setFullRowSelect(true);
	table->setGridLines(true);
	table->setAlwaysShowSelection(true);
	table->setHeaderDragDrop(true);
	table->setCheckBoxes(true);
	table->setReadOnly(true);
	table->insert({ _T("Button"), _T("Run Action"), _T("Clickable") }, 101);
	table->insert({ _T("CheckBox"), _T("Enable Option"), _T("Toggle state") }, 102);
	table->insert({ _T("RadioButton"), _T("Mode A/B"), _T("Single selection") }, 103);
	table->insert({ _T("ComboBox"), _T("First/Second/Third"), _T("Drop-down list") }, 104);
	table->insert({ _T("ProgressBar"), _T("0-100"), _T("Updated by slider/spinner") }, 105);
	table->setChecked(1, true);
	table->onColumnClick([status](int col) {
		setStatus(status, _T("Table column clicked: ") + std::to_wstring(col));
	});
	table->onClicked([status] {
		setStatus(status, _T("Table clicked"));
	});

	if(headerFont) {
		// Tree creates its internal Header on first addColumn; use the current tree font for that.
		tree->setFont(headerFont);
	}
	tree->addColumn(_T("Node"), 260);
	tree->addColumn(_T("Detail"), 180);
	tree->setFont(uiFont);
	auto root = tree->insert(_T("Controls"), TVI_ROOT, TVI_LAST, 0, true);
	tree->insert(_T("Buttons"), root, TVI_LAST, 0, false);
	tree->insert(_T("Inputs"), root, TVI_LAST, 0, false);
	tree->insert(_T("Lists"), root, TVI_LAST, 0, false);
	tree->expand(root);

	setRichSummary(richText);
	auto syncingRange = std::make_shared<bool>(false);

	buttonRun->onClicked([status] {
		setStatus(status, _T("Button clicked"));
	});

	checkOption->onClicked([checkOption, status] {
		setStatus(status, checkOption->getChecked() ? _T("CheckBox enabled") : _T("CheckBox disabled"));
	});

	radioA->onClicked([status] {
		setStatus(status, _T("Mode A selected"));
	});

	radioB->onClicked([status] {
		setStatus(status, _T("Mode B selected"));
	});

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

	dateTime->onDateTimeChanged([status](const SYSTEMTIME& st) {
		setStatus(status, _T("Date changed: ") + std::to_wstring(st.wYear) + _T("-") + std::to_wstring(st.wMonth) + _T("-") + std::to_wstring(st.wDay));
	});

	auto layoutControls = [
		window,
		groupButtons,
		buttonRun,
		checkOption,
		radioA,
		radioB,
		labelInput,
		textInput,
		combo,
		dateTime,
		spinner,
		slider,
		progress,
		link,
		richText,
		header,
		table,
		tree,
		status
	]() {
		auto size = window->getClientSize();
		const long margin = 8;
		const long statusH = 26;
		const long leftMin = 340;
		const long leftMax = 520;
		const long leftW = clampMin((std::min)(leftMax, size.x / 2), leftMin);
		const long topH = 280;
		const long contentH = clampMin(size.y - statusH, 40);

		status->resize(dwt::Rectangle(0, clampMin(size.y - statusH, 0), clampMin(size.x, 80), statusH));
		status->refresh();

		const long leftPaneW = clampMin(leftW - margin * 2, 160);
		groupButtons->resize(dwt::Rectangle(margin, margin, leftPaneW, 120));
		buttonRun->resize(dwt::Rectangle(margin + 12, margin + 28, 128, 28));
		checkOption->resize(dwt::Rectangle(margin + 152, margin + 31, 140, 24));
		radioA->resize(dwt::Rectangle(margin + 300, margin + 31, 70, 24));
		radioB->resize(dwt::Rectangle(margin + 372, margin + 31, 70, 24));

		labelInput->resize(dwt::Rectangle(margin, margin + 132, 72, 24));
		textInput->resize(dwt::Rectangle(margin + 76, margin + 132, clampMin(leftW - 84 - margin, 160), 24));
		combo->resize(dwt::Rectangle(margin, margin + 162, 200, 280));
		dateTime->resize(dwt::Rectangle(margin + 208, margin + 162, clampMin(leftW - 216 - margin, 120), 24));

		spinner->resize(dwt::Rectangle(margin, margin + 194, 100, 24));
		slider->resize(dwt::Rectangle(margin + 108, margin + 194, clampMin(leftW - 116 - margin, 120), 36));
		progress->resize(dwt::Rectangle(margin, margin + 234, leftPaneW, 24));
		link->resize(dwt::Rectangle(margin, margin + 262, leftPaneW, 24));

		richText->resize(dwt::Rectangle(margin, topH + margin, leftPaneW, clampMin(contentH - topH - margin, 80)));

		const long rightX = leftW + margin;
		const long rightW = clampMin(size.x - rightX - margin, 140);
		header->resize(dwt::Rectangle(rightX, margin, rightW, 24));
		table->resize(dwt::Rectangle(rightX, margin + 28, rightW, 190));
		tree->resize(dwt::Rectangle(rightX, margin + 224, rightW, clampMin(contentH - 224 - margin, 80)));
	};

	window->onSized([layoutControls](const dwt::SizedEvent&) {
		layoutControls();
	});

	window->onDestroy([] {
		::PostQuitMessage(0);
	});

	std::vector<Control*> visibleControls = {
		groupButtons,
		buttonRun,
		checkOption,
		radioA,
		radioB,
		labelInput,
		textInput,
		combo,
		dateTime,
		spinner,
		slider,
		progress,
		link,
		richText,
		header,
		table,
		tree,
		status
	};
	for(auto* control : visibleControls) {
		if(control) {
			control->setVisible(true);
		}
	}

	layoutControls();

	setStatus(status, _T("Ready"));
	window->setVisible(true);
	window->setFocus();

	app.run();
	return 0;
}
