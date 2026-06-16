#include <dwt/Application.h>
#include <dwt/CanvasClasses.h>
#include <dwt/Message.h>
#include <dwt/WidgetCreator.h>

#include <dwt/resources/Bitmap.h>
#include <dwt/resources/Brush.h>
#include <dwt/resources/Font.h>
#include <dwt/resources/Icon.h>
#include <dwt/resources/Pen.h>

#include <dwt/widgets/Button.h>
#include <dwt/widgets/CheckBox.h>
#include <dwt/widgets/ColorDialog.h>
#include <dwt/widgets/ComboBox.h>
#include <dwt/widgets/Control.h>
#include <dwt/widgets/DateTime.h>
#include <dwt/widgets/GroupBox.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/Link.h>
#include <dwt/widgets/LoadDialog.h>
#include <dwt/widgets/Menu.h>
#include <dwt/widgets/MDIChild.h>
#include <dwt/widgets/MDIFrame.h>
#include <dwt/widgets/MDIParent.h>
#include <dwt/widgets/ProgressBar.h>
#include <dwt/widgets/RadioButton.h>
#include <dwt/widgets/RichTextBox.h>
#include <dwt/widgets/Slider.h>
#include <dwt/widgets/Spinner.h>
#include <dwt/widgets/Table.h>
#include <dwt/widgets/TextBox.h>
#include <dwt/widgets/Tree.h>

#include <algorithm>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <tchar.h>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

using dwt::Button;
using dwt::BitmapPtr;
using dwt::Brush;
using dwt::CheckBox;
using dwt::ColorDialog;
using dwt::ComboBox;
using dwt::Control;
using dwt::DateTime;
using dwt::FontPtr;
using dwt::GroupBox;
using dwt::IconPtr;
using dwt::Label;
using dwt::Link;
using dwt::LoadDialog;
using dwt::Menu;
using dwt::MDIChild;
using dwt::MDIFrame;
using dwt::MDIParent;
using dwt::PaintCanvas;
using dwt::Pen;
using dwt::ProgressBar;
using dwt::RadioButton;
using dwt::RichTextBox;
using dwt::Slider;
using dwt::Spinner;
using dwt::Table;
using dwt::TextBox;
using dwt::Tree;
using dwt::WidgetCreator;

constexpr UINT ID_MDI_FIRST_CHILD = 50000;

FontPtr createExampleFont(unsigned dpi = 96) {
	LOGFONT lf = { };
	lf.lfHeight = -::MulDiv(9, static_cast<int>(dpi), 72);
	lf.lfWeight = FW_NORMAL;
	lf.lfQuality = CLEARTYPE_QUALITY;
	_tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Segoe UI"));
	return new dwt::Font(lf);
}

dwt::tstring lowerExtension(const dwt::tstring& file) {
	const auto pos = file.find_last_of(_T('.'));
	if(pos == dwt::tstring::npos) {
		return dwt::tstring();
	}

	dwt::tstring ext = file.substr(pos);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](dwt::tstring::value_type ch) {
		return static_cast<dwt::tstring::value_type>(_totlower(ch));
	});
	return ext;
}

class DemoChild : public MDIChild {
public:
	DemoChild(MDIParent* parent, const dwt::tstring& caption, FontPtr font) :
		MDIChild(parent),
		caption(caption),
		uiFont(font)
	{
	}

	void create(const dwt::Rectangle& bounds, bool activate = true) {
		Seed seed(caption);
		seed.location = bounds;
		seed.activate = activate;
		createMDIChild(seed);
		build();
	}

protected:
	virtual void build() = 0;

	FontPtr getUIFont() const {
		return uiFont;
	}

	template<typename Widget>
	void applyUIFont(Widget* widget) const {
		if(widget && uiFont) {
			widget->setFont(uiFont);
		}
	}

private:
	dwt::tstring caption;
	FontPtr uiFont;
};

class TextDemoChild : public DemoChild {
public:
	TextDemoChild(MDIParent* parent, const dwt::tstring& caption, FontPtr font) :
		DemoChild(parent, caption, font),
		rich(nullptr)
	{
	}

private:
	void build() override {
		RichTextBox::Seed seed;
		seed.font = getUIFont();
		seed.scrollBarHorizontallyFlag = true;
		rich = WidgetCreator<RichTextBox>::create(this, seed);
		rich->setColor(RGB(31, 41, 55), RGB(255, 255, 255));
		rich->addText(rtfDocument());
		rich->setReadOnly(true);
		rich->findText(_T("colors"));

		onSized([this](const dwt::SizedEvent&) { layoutChild(); });
		layoutChild();
	}

	void layoutChild() {
		if(rich) {
			rich->resize(dwt::Rectangle(getClientSize()));
		}
	}

	RichTextBox* rich;

	static std::string rtfDocument() {
		return
			"{\\urtf\\ansi\\deff0"
			"{\\fonttbl{\\f0 Segoe UI;}{\\f1 Consolas;}{\\f2 Georgia;}}"
			"{\\colortbl ;\\red31\\green41\\blue55;\\red0\\green102\\blue204;\\red32\\green128\\blue72;\\red196\\green80\\blue32;\\red96\\green45\\blue145;\\red255\\green242\\blue204;\\red232\\green244\\blue253;}"
			"\\viewkind4\\uc1\\pard\\f0\\fs21\\cf1 "
			"\\fs32\\b RichTextBox RTF feature sample\\b0\\fs21\\par"
			"\\cf2 Bold blue text\\cf1 , \\i italic text\\i0 , \\ul underlined text\\ul0 , \\strike strikeout\\strike0 , "
			"and \\super superscript\\nosupersub / \\sub subscript\\nosupersub runs.\\par\\par"
			"\\cf3 This paragraph is green, \\cf4 this clause is orange, \\cf5 and this one is purple.\\cf1\\par"
			"\\highlight6 Highlighted background\\highlight0 and \\chcbpat7 shaded character backgrounds show per-run colors.\\par\\par"
			"\\pard\\qc\\b Centered heading\\b0\\par"
			"\\pard\\qr Right aligned note\\par"
			"\\pard\\li360\\fi-180\\tx360\\bullet\\tab Bullet list item with indentation\\par"
			"\\pard\\li360\\fi-180\\tx360\\bullet\\tab Another item after a tab stop\\par"
			"\\pard\\li0\\fi0\\par"
			"\\f1 Code font: for(auto window : mdiChildren) cascade(window);\\f0\\par"
			"\\f2 Alternate serif font face for mixed font rendering.\\f0\\par\\par"
			"Escaped braces \\{ \\}, backslashes \\\\, Unicode: \\u8212? dash, \\u9733? star.\\par"
			"{\\field{\\*\\fldinst{HYPERLINK \"https://dcplusplus.sourceforge.io/\"}}{\\fldrslt{\\cf2\\ul RichEdit hyperlink field\\ul0\\cf1}}}\\par"
			"}";
	}
};

class ControlsDemoChild : public DemoChild {
public:
	ControlsDemoChild(MDIParent* parent, const dwt::tstring& caption, FontPtr font) :
		DemoChild(parent, caption, font),
		title(nullptr),
		optionsGroup(nullptr),
		modeGroup(nullptr),
		input(nullptr),
		combo(nullptr),
		checkOption(nullptr),
		radioA(nullptr),
		radioB(nullptr),
		dateTime(nullptr),
		slider(nullptr),
		spinnerEdit(nullptr),
		spinner(nullptr),
		progress(nullptr),
		step(nullptr),
		link(nullptr),
		tree(nullptr),
		value(25)
	{
	}

private:
	void build() override {
		title = WidgetCreator<Label>::create(this, Label::Seed(_T("Controls inside an MDI child")));
		optionsGroup = WidgetCreator<GroupBox>::create(this, GroupBox::Seed(_T("Input and state")));
		modeGroup = WidgetCreator<GroupBox>::create(this, GroupBox::Seed(_T("Choice controls")));
		input = WidgetCreator<TextBox>::create(this, TextBox::Seed(_T("Edit text, then switch MDI windows.")));
		combo = WidgetCreator<ComboBox>::create(this, ComboBox::Seed());
		checkOption = WidgetCreator<CheckBox>::create(this, CheckBox::Seed(_T("Enable enhanced mode")));
		radioA = WidgetCreator<RadioButton>::create(this, RadioButton::Seed(_T("Mode A")));
		radioB = WidgetCreator<RadioButton>::create(this, RadioButton::Seed(_T("Mode B")));
		DateTime::Seed dateSeed;
		dateSeed.style |= DTS_SHOWNONE;
		dateSeed.format = _T("ddd, MMM dd yyyy");
		dateSeed.font = getUIFont();
		dateTime = WidgetCreator<DateTime>::create(this, dateSeed);
		slider = WidgetCreator<Slider>::create(this, Slider::Seed());
		spinnerEdit = WidgetCreator<TextBox>::create(this, TextBox::Seed(_T("25")));
		spinner = WidgetCreator<Spinner>::create(this, Spinner::Seed(0, 100, spinnerEdit));
		progress = WidgetCreator<ProgressBar>::create(this, ProgressBar::Seed());
		step = WidgetCreator<Button>::create(this, Button::Seed(_T("Step Progress")));
		link = WidgetCreator<Link>::create(this, Link::Seed(_T("https://dcplusplus.sourceforge.io/"), true));
		Tree::Seed treeSeed;
		treeSeed.font = getUIFont();
		treeSeed.checkBoxes = true;
		tree = WidgetCreator<Tree>::create(this, treeSeed);

		applyFonts();

		combo->addValue(_T("Cascade windows"));
		combo->addValue(_T("Tile horizontally"));
		combo->addValue(_T("Tile vertically"));
		combo->setSelected(0);

		slider->setRange(0, 100);
		slider->setPosition(value);
		slider->setAutoTicks();
		slider->setTickFrequency(10);

		spinner->setValue(value);

		progress->setRange(0, 100);
		progress->setPosition(value);
		step->onClicked([this] {
			value += 10;
			if(value > 100) {
				value = 0;
			}
			progress->setPosition(value);
			slider->setPosition(value);
			spinner->setValue(value);
			input->setText(_T("Progress is now ") + std::to_wstring(value) + _T("%."));
		});
		spinner->onUpdate([this](int, int position) {
			value = position;
			progress->setPosition(value);
			slider->setPosition(value);
			return true;
		});

		auto root = tree->insert(_T("MDI child controls"), TVI_ROOT, TVI_LAST, 0, true);
		tree->insert(_T("TextBox, Button, ProgressBar"), root);
		tree->insert(_T("ComboBox, CheckBox, RadioButton"), root);
		tree->insert(_T("DateTime, Slider, Spinner, Link"), root);
		tree->setChecked(root, true);

		onSized([this](const dwt::SizedEvent&) { layoutChild(); });
		layoutChild();
	}

	void applyFonts() {
		applyUIFont(title);
		applyUIFont(optionsGroup);
		applyUIFont(modeGroup);
		applyUIFont(input);
		applyUIFont(combo);
		applyUIFont(checkOption);
		applyUIFont(radioA);
		applyUIFont(radioB);
		applyUIFont(spinnerEdit);
		applyUIFont(step);
		applyUIFont(link);
	}

	void layoutChild() {
		auto size = getClientSize();
		const int margin = scale(12);
		const int row = scale(28);
		const int buttonWidth = scale(132);
		const int buttonHeight = scale(32);
		const int gap = scale(8);
		const int width = size.x > margin * 2 ? size.x - margin * 2 : 0;
		const int leftWidth = width > scale(380) ? scale(360) : width;
		const int rightX = margin + leftWidth + gap;
		const int rightWidth = width > leftWidth + gap ? width - leftWidth - gap : 0;

		if(title) {
			title->resize(dwt::Rectangle(margin, margin, width, row));
		}
		if(optionsGroup) {
			optionsGroup->resize(dwt::Rectangle(margin, margin + row + gap, leftWidth, scale(174)));
		}
		if(input) {
			input->resize(dwt::Rectangle(margin + gap, margin + row + gap * 3,
				leftWidth - gap * 2, row));
		}
		if(combo) {
			combo->resize(dwt::Rectangle(margin + gap, margin + row + gap * 4 + row,
				leftWidth - gap * 2, scale(120)));
		}
		if(checkOption) {
			checkOption->resize(dwt::Rectangle(margin + gap, margin + row + gap * 5 + row * 2,
				leftWidth - gap * 2, row));
		}
		if(dateTime) {
			dateTime->resize(dwt::Rectangle(margin + gap, margin + row + gap * 6 + row * 3,
				leftWidth - gap * 2, row));
		}
		if(modeGroup) {
			modeGroup->resize(dwt::Rectangle(rightX, margin + row + gap, rightWidth, scale(174)));
		}
		if(radioA) {
			radioA->resize(dwt::Rectangle(rightX + gap, margin + row + gap * 3,
				rightWidth - gap * 2, row));
		}
		if(radioB) {
			radioB->resize(dwt::Rectangle(rightX + gap, margin + row + gap * 4 + row,
				rightWidth - gap * 2, row));
		}
		if(slider) {
			slider->resize(dwt::Rectangle(rightX + gap, margin + row + gap * 5 + row * 2,
				rightWidth - gap * 2, scale(36)));
		}
		if(spinnerEdit) {
			spinnerEdit->resize(dwt::Rectangle(rightX + gap, margin + row + gap * 6 + row * 3,
				scale(80), row));
		}
		if(spinner) {
			spinner->resize(dwt::Rectangle(rightX + gap + scale(82), margin + row + gap * 6 + row * 3,
				scale(24), row));
		}
		if(progress) {
			progress->resize(dwt::Rectangle(margin, margin + scale(230),
				width > buttonWidth + gap ? width - buttonWidth - gap : width, row));
		}
		if(step) {
			step->resize(dwt::Rectangle(size.x - margin - buttonWidth,
				margin + scale(227), buttonWidth, buttonHeight));
		}
		if(link) {
			link->resize(dwt::Rectangle(margin, margin + scale(266), width, row));
		}
		if(tree) {
			tree->resize(dwt::Rectangle(margin, margin + scale(302),
				width, std::max<long>(scale(80), size.y - margin - scale(302))));
		}
	}

	Label* title;
	GroupBox* optionsGroup;
	GroupBox* modeGroup;
	TextBox* input;
	ComboBox* combo;
	CheckBox* checkOption;
	RadioButton* radioA;
	RadioButton* radioB;
	DateTime* dateTime;
	Slider* slider;
	TextBox* spinnerEdit;
	Spinner* spinner;
	ProgressBar* progress;
	Button* step;
	Link* link;
	Tree* tree;
	int value;
};

class TableDemoChild : public DemoChild {
public:
	TableDemoChild(MDIParent* parent, const dwt::tstring& caption, FontPtr font) :
		DemoChild(parent, caption, font),
		table(nullptr)
	{
	}

private:
	void build() override {
		Table::Seed seed;
		seed.font = getUIFont();
		table = WidgetCreator<Table>::create(this, seed);
		table->addColumn(_T("MDI Test"), 180);
		table->addColumn(_T("Expected Behavior"), 380);
		table->setFullRowSelect(true);
		table->setGridLines(true);
		table->setAlwaysShowSelection(true);
		table->setReadOnly(true);
		table->insert({ _T("Create"), _T("Child windows are created through CreateMDIWindow.") }, 1);
		table->insert({ _T("Activate"), _T("Clicking or using the Window menu activates the selected child.") }, 2);
		table->insert({ _T("Maximize"), _T("The active child maximizes inside the MDI client.") }, 3);
		table->insert({ _T("Tile/Cascade"), _T("The MDI client arranges all child windows.") }, 4);
		table->insert({ _T("Close"), _T("Closing children destroys the DWT child objects cleanly.") }, 5);

		onSized([this](const dwt::SizedEvent&) { layoutChild(); });
		layoutChild();
	}

	void layoutChild() {
		if(table) {
			table->resize(dwt::Rectangle(getClientSize()));
		}
	}

	Table* table;
};

class PaintDemoChild : public DemoChild {
public:
	PaintDemoChild(MDIParent* parent, const dwt::tstring& caption, FontPtr font) :
		DemoChild(parent, caption, font)
	{
	}

private:
	void build() override {
		addCallback(dwt::Message(WM_ERASEBKGND), [](const MSG&, LRESULT& ret) {
			ret = 1;
			return true;
		});
		onPainting([this](PaintCanvas& canvas) { paint(canvas); });
		onSized([this](const dwt::SizedEvent&) {
			redraw();
		});
	}

	void paint(PaintCanvas& canvas) {
		dwt::Rectangle client(getClientSize());
		dwt::Rectangle panel(
			scale(18), scale(18),
			client.width() - scale(36),
			client.height() - scale(36));
		dwt::Rectangle text(panel.pos + dwt::Point(scale(18), scale(18)),
			panel.size - dwt::Point(scale(36), scale(36)));

		canvas.fill(client, Brush(RGB(248, 250, 252)));
		canvas.fill(panel, Brush(RGB(221, 237, 255)));

		Pen border(RGB(0, 102, 204), Pen::Solid, scale(2));
		auto borderSelection = canvas.select(border);
		canvas.line(panel);

		if(getUIFont()) {
			auto fontSelection = canvas.select(*getUIFont());
			drawText(canvas, text);
		} else {
			drawText(canvas, text);
		}
	}

	void drawText(PaintCanvas& canvas, dwt::Rectangle& text) {
		canvas.setTextColor(RGB(24, 42, 64));
		auto transparentBackground = canvas.setBkMode(true);
		canvas.drawText(
			_T("Custom-painted MDI child\r\n\r\n")
			_T("This window exercises WM_PAINT, WM_ERASEBKGND, resize, activation, ")
			_T("and MDI arrange operations without native child controls."),
			text, DT_LEFT | DT_TOP | DT_WORDBREAK);
	}
};

class FlatMDITabs : public Control {
public:
	typedef FlatMDITabs ThisType;
	typedef ThisType* ObjectType;

	struct Seed : public Control::Seed {
		typedef ThisType WidgetType;

		Seed() :
			Control::Seed(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
		{
		}
	};

	explicit FlatMDITabs(dwt::Widget* parent) :
		Control(parent, dwt::NormalDispatcher::getDefault()),
		active(nullptr)
	{
	}

	void create(const Seed& seed = Seed()) {
		Control::create(seed);

		addCallback(dwt::Message(WM_ERASEBKGND), [](const MSG&, LRESULT& ret) {
			ret = 1;
			return true;
		});
		onPainting([this](PaintCanvas& canvas) { paint(canvas); });
		addCallback(dwt::Message(WM_LBUTTONUP), [this](const MSG& msg, LRESULT&) {
			click(dwt::Point::fromLParam(msg.lParam),
				(::GetKeyState(VK_SHIFT) & 0x8000) != 0);
			return true;
		});
		addCallback(dwt::Message(WM_MBUTTONUP), [this](const MSG& msg, LRESULT&) {
			closeAt(dwt::Point::fromLParam(msg.lParam));
			return true;
		});
		addCallback(dwt::Message(WM_CONTEXTMENU), [this](const MSG& msg, LRESULT&) {
			contextMenu(msg);
			return true;
		});
	}

	int preferredHeight() const {
		return scale(32);
	}

	void setNewCallback(std::function<void ()> callback) {
		newCallback = callback;
	}

	void addTab(MDIChild* child) {
		if(!child || findTab(child) != tabs.end()) {
			return;
		}

		Tab tab;
		tab.child = child;
		tab.title = titleOf(child);
		tabs.push_back(tab);
		if(!active) {
			active = child;
		}
		redraw();
	}

	void removeTab(MDIChild* child) {
		tabs.erase(std::remove_if(tabs.begin(), tabs.end(), [child](const Tab& tab) {
			return tab.child == child;
		}), tabs.end());

		if(active == child) {
			active = tabs.empty() ? nullptr : tabs.back().child;
		}
		redraw();
	}

	void setActive(MDIChild* child) {
		if(findTab(child) != tabs.end()) {
			active = child;
			redraw();
		}
	}

	void updateTab(MDIChild* child) {
		auto tab = findTab(child);
		if(tab != tabs.end()) {
			tab->title = titleOf(child);
			redraw();
		}
	}

private:
	struct Tab {
		MDIChild* child;
		dwt::tstring title;
		dwt::Rectangle bounds;
	};

	typedef std::vector<Tab>::iterator TabIter;

	TabIter findTab(MDIChild* child) {
		return std::find_if(tabs.begin(), tabs.end(), [child](const Tab& tab) {
			return tab.child == child;
		});
	}

	static dwt::tstring titleOf(MDIChild* child) {
		if(!child || !child->handle()) {
			return dwt::tstring();
		}

		const int length = ::GetWindowTextLength(child->handle());
		if(length <= 0) {
			return _T("Untitled");
		}

		std::vector<TCHAR> text(static_cast<size_t>(length) + 1);
		::GetWindowText(child->handle(), text.data(), static_cast<int>(text.size()));
		return text.data();
	}

	void paint(PaintCanvas& canvas) {
		const dwt::Rectangle client(getClientSize());
		canvas.fill(client, Brush(RGB(242, 244, 247)));

		Pen topLine(RGB(188, 196, 207));
		auto topLineSelection = canvas.select(topLine);
		canvas.line(0, 0, client.width(), 0);

		if(getFont()) {
			auto fontSelection = canvas.select(*getFont());
			layoutTabs(canvas);
			for(const auto& tab: tabs) {
				if(tab.bounds.width() > 0) {
					drawTab(canvas, tab, tab.child == active);
				}
			}
		} else {
			layoutTabs(canvas);
			for(const auto& tab: tabs) {
				if(tab.bounds.width() > 0) {
					drawTab(canvas, tab, tab.child == active);
				}
			}
		}
		drawNewButton(canvas);
	}

	void layoutTabs(PaintCanvas& canvas) {
		const dwt::Rectangle client(getClientSize());
		const int margin = scale(4);
		const int gap = scale(2);
		const int buttonSize = scale(24);
		const int tabHeight = std::max<long>(0, client.height() - margin * 2);

		newButton = dwt::Rectangle(
			std::max<long>(margin, client.width() - buttonSize - margin),
			margin,
			buttonSize,
			tabHeight);

		int x = margin;
		const int rightLimit = std::max<long>(margin, newButton.left() - gap);

		for(auto& tab: tabs) {
			const int textWidth = static_cast<int>(canvas.getTextExtent(tab.title).x);
			int width = std::clamp(textWidth + scale(30), scale(96), scale(220));
			if(x + width > rightLimit) {
				width = std::max<long>(0, rightLimit - x);
			}

			tab.bounds = dwt::Rectangle(x, margin, width, tabHeight);
			x += width + gap;
		}
	}

	void drawTab(PaintCanvas& canvas, const Tab& tab, bool selected) {
		const COLORREF fill = selected ? RGB(255, 255, 255) : RGB(228, 233, 240);
		const COLORREF border = selected ? RGB(69, 117, 180) : RGB(184, 193, 205);
		const COLORREF text = selected ? RGB(20, 33, 49) : RGB(69, 80, 94);

		canvas.fill(tab.bounds, Brush(fill));

		Pen borderPen(border);
		auto borderSelection = canvas.select(borderPen);
		canvas.line(tab.bounds);

		dwt::Rectangle label(
			tab.bounds.pos + dwt::Point(scale(10), 0),
			dwt::Point(std::max<long>(0, tab.bounds.width() - scale(20)), tab.bounds.height()));

		canvas.setTextColor(text);
		auto transparent = canvas.setBkMode(true);
		canvas.drawText(tab.title, label,
			DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
	}

	void drawNewButton(PaintCanvas& canvas) {
		canvas.fill(newButton, Brush(RGB(232, 236, 242)));

		Pen borderPen(RGB(184, 193, 205));
		auto borderSelection = canvas.select(borderPen);
		canvas.line(newButton);

		Pen plusPen(RGB(43, 72, 108), Pen::Solid, scale(2));
		auto plusSelection = canvas.select(plusPen);
		const int cx = static_cast<int>(newButton.left() + newButton.width() / 2);
		const int cy = static_cast<int>(newButton.top() + newButton.height() / 2);
		const int arm = scale(6);
		canvas.line(cx - arm, cy, cx + arm, cy);
		canvas.line(cx, cy - arm, cx, cy + arm);
	}

	void click(const dwt::Point& point, bool closeRequested) {
		if(newButton.contains(point)) {
			if(newCallback) {
				newCallback();
			}
			return;
		}

		if(auto child = childAt(point)) {
			if(closeRequested) {
				child->sendMessage(WM_CLOSE);
			} else {
				child->activate();
			}
		}
	}

	void closeAt(const dwt::Point& point) {
		if(auto child = childAt(point)) {
			child->sendMessage(WM_CLOSE);
		}
	}

	void contextMenu(const MSG& msg) {
		dwt::Point point = msg.lParam == -1 ?
			dwt::Point(scale(8), scale(8)) :
			dwt::Point::fromLParam(msg.lParam);

		if(msg.lParam != -1) {
			POINT pt = point;
			::ScreenToClient(handle(), &pt);
			point = pt;
		}

		MDIChild* child = childAt(point);
		Menu::Seed seed(false);
		seed.font = getFont();
		auto menu = WidgetCreator<Menu>::create(this, seed);
		menu->appendItem(_T("New &Text Child"), [this] {
			if(newCallback) {
				newCallback();
			}
		});

		if(child) {
			menu->appendSeparator();
			menu->appendItem(_T("&Activate"), [child] { child->activate(); });
			menu->appendItem(_T("&Close"), [child] { child->sendMessage(WM_CLOSE); });
		}

		POINT screen = point;
		::ClientToScreen(handle(), &screen);
		menu->open(dwt::ScreenCoordinate(dwt::Point(screen)));
	}

	MDIChild* childAt(const dwt::Point& point) {
		for(auto& tab: tabs) {
			if(tab.bounds.width() > 0 && tab.bounds.contains(point)) {
				return tab.child;
			}
		}
		return nullptr;
	}

	std::vector<Tab> tabs;
	dwt::Rectangle newButton;
	MDIChild* active;
	std::function<void ()> newCallback;
};

class DemoFrame : public MDIFrame {
public:
	DemoFrame() :
		MDIFrame(nullptr),
		uiFont(createExampleFont()),
		menuBar(nullptr),
		fileMenu(nullptr),
		windowMenu(nullptr),
		backgroundMenu(nullptr),
		helpMenu(nullptr),
		tabs(nullptr),
		backgroundColor(RGB(235, 239, 245)),
		childCounter(1)
	{
	}

	void createFrame() {
		createMenus();
		bindCommands();

		Seed seed(_T("DWT MDIExample"));
		seed.location = dwt::Rectangle(80, 80, 1100, 760);
		seed.menuHandle = menuBar->handle();
		seed.mdiWindowMenu = windowMenu->handle();
		seed.mdiIdFirstChild = ID_MDI_FIRST_CHILD;
		create(seed);
		setFont(uiFont);
		createTabs();
		getMDIParent()->setBackgroundColor(backgroundColor);

		onDestroy([] { ::PostQuitMessage(0); });

		createTextChild(false);
		createControlsChild(false);
		createTableChild(false);
		createPaintChild(true);
		getMDIParent()->cascade();

		setVisible(true);
		setFocus();
	}

	void layout() override {
		const auto size = getClientSize();
		const int tabHeight = tabs && tabs->handle() ? tabs->preferredHeight() : 0;
		const int mdiHeight = std::max<long>(0, size.y - tabHeight);

		if(getMDIParent() && getMDIParent()->handle()) {
			getMDIParent()->resize(dwt::Rectangle(0, 0, size.x, mdiHeight));
		}
		if(tabs && tabs->handle()) {
			tabs->resize(dwt::Rectangle(0, mdiHeight, size.x, tabHeight));
		}
	}

private:
	void createTabs() {
		tabs = new FlatMDITabs(this);
		tabs->create();
		tabs->setFont(uiFont);
		tabs->setNewCallback([this] { createTextChild(true); });
		layout();
	}

	void createMenus() {
		Menu::Seed menuSeed(false);
		menuSeed.popup = false;
		menuSeed.commandMessages = true;
		menuSeed.font = uiFont;
		menuBar = WidgetCreator<Menu>::create(this, menuSeed);

		fileMenu = menuBar->appendPopup(_T("&File"));
		windowMenu = menuBar->appendPopup(_T("&Window"));
		backgroundMenu = menuBar->appendPopup(_T("&Background"));
		helpMenu = menuBar->appendPopup(_T("&Help"));

		fileMenu->appendItem(_T("New &Text Child\tCtrl+T"), [this] { createTextChild(true); });
		fileMenu->appendItem(_T("New &Controls Child\tCtrl+N"), [this] { createControlsChild(true); });
		fileMenu->appendItem(_T("New Ta&ble Child\tCtrl+B"), [this] { createTableChild(true); });
		fileMenu->appendItem(_T("New &Paint Child\tCtrl+P"), [this] { createPaintChild(true); });
		fileMenu->appendSeparator();
		fileMenu->appendItem(_T("&Close Active\tCtrl+F4"), [this] { getMDIParent()->closeActive(); });
		fileMenu->appendItem(_T("Close A&ll"), [this] { getMDIParent()->closeAll(); });
		fileMenu->appendSeparator();
		fileMenu->appendItem(_T("E&xit"), [this] { postMessage(WM_CLOSE); });

		windowMenu->appendItem(_T("&Cascade"), [this] { getMDIParent()->cascade(); });
		windowMenu->appendItem(_T("Tile &Horizontal"), [this] { getMDIParent()->tile(true); });
		windowMenu->appendItem(_T("Tile &Vertical"), [this] { getMDIParent()->tile(false); });
		windowMenu->appendItem(_T("&Arrange Icons"), [this] { getMDIParent()->arrange(); });
		windowMenu->appendItem(_T("&Minimize All"), [this] { getMDIParent()->minimizeAll(); });
		windowMenu->appendSeparator();
		windowMenu->appendItem(_T("&Next\tCtrl+F6"), [this] { getMDIParent()->next(); });
		windowMenu->appendItem(_T("&Previous\tCtrl+Shift+F6"), [this] { getMDIParent()->previous(); });
		windowMenu->appendItem(_T("&Maximize Active"), [this] { getMDIParent()->maximizeActive(); });
		windowMenu->appendItem(_T("&Restore Active"), [this] { getMDIParent()->restoreActive(); });

		backgroundMenu->appendItem(_T("&Load Image..."), [this] { loadBackgroundImage(); });
		backgroundMenu->appendItem(_T("&Choose Color..."), [this] { chooseBackgroundColor(); });
		backgroundMenu->appendSeparator();
		backgroundMenu->appendItem(_T("&Clear Image"), [this] { getMDIParent()->clearBackgroundImage(); });
		backgroundMenu->appendItem(_T("&Reset Color"), [this] {
			backgroundColor = RGB(235, 239, 245);
			getMDIParent()->setBackgroundColor(backgroundColor);
		});

		helpMenu->appendItem(_T("&About MDIExample"), [this] {
			::MessageBox(handle(),
				_T("MDIExample creates multiple DWT MDI child windows and exercises native MDI commands."),
				_T("About MDIExample"), MB_OK | MB_ICONINFORMATION);
		});
	}

	void bindCommands() {
		addAccel(FCONTROL, 'T', [this] { createTextChild(true); });
		addAccel(FCONTROL, 'N', [this] { createControlsChild(true); });
		addAccel(FCONTROL, 'B', [this] { createTableChild(true); });
		addAccel(FCONTROL, 'P', [this] { createPaintChild(true); });
	}

	void loadBackgroundImage() {
		LoadDialog dialog(this);
		dialog.setTitle(_T("Load MDI background image"));
		dialog.addFilter(_T("Background images"), _T("*.bmp;*.dib;*.ico"));
		dialog.addFilter(_T("Bitmap images"), _T("*.bmp;*.dib"));
		dialog.addFilter(_T("Icon images"), _T("*.ico"));
		dialog.addFilter(_T("All files"), _T("*.*"));
		dialog.setDefaultExtension(_T("bmp"));

		dwt::tstring file;
		if(!dialog.open(file)) {
			return;
		}

		try {
			if(lowerExtension(file) == _T(".ico")) {
				IconPtr icon(new dwt::Icon(file));
				getMDIParent()->setBackgroundImage(icon);
				return;
			}

			BitmapPtr bitmap(new dwt::Bitmap(file, LR_CREATEDIBSECTION));
			if(bitmap && bitmap->handle()) {
				getMDIParent()->setBackgroundImage(bitmap);
				return;
			}
		} catch(const std::exception&) {
		}

		{
			::MessageBox(handle(),
				_T("The selected file could not be loaded as a background image."),
				_T("MDI background"), MB_OK | MB_ICONWARNING);
			return;
		}
	}

	void chooseBackgroundColor() {
		ColorDialog dialog(this);
		ColorDialog::ColorParams params(backgroundColor);
		if(dialog.open(params)) {
			backgroundColor = params.getColor();
			getMDIParent()->setBackgroundColor(backgroundColor);
		}
	}

	dwt::Rectangle nextBounds() {
		const int offset = static_cast<int>((childCounter % 8) * 28);
		return dwt::Rectangle(30 + offset, 30 + offset, 520, 320);
	}

	dwt::tstring numbered(const dwt::tstring& base) {
		return base + _T(" ") + std::to_wstring(childCounter++);
	}

	void createTextChild(bool activate) {
		auto* child = new TextDemoChild(getMDIParent(), numbered(_T("Text")), uiFont);
		child->create(nextBounds(), activate);
		trackChild(child);
	}

	void createControlsChild(bool activate) {
		auto* child = new ControlsDemoChild(getMDIParent(), numbered(_T("Controls")), uiFont);
		child->create(nextBounds(), activate);
		trackChild(child);
	}

	void createTableChild(bool activate) {
		auto* child = new TableDemoChild(getMDIParent(), numbered(_T("Table")), uiFont);
		child->create(nextBounds(), activate);
		trackChild(child);
	}

	void createPaintChild(bool activate) {
		auto* child = new PaintDemoChild(getMDIParent(), numbered(_T("Paint")), uiFont);
		child->create(nextBounds(), activate);
		trackChild(child);
	}

	void trackChild(DemoChild* child) {
		if(!tabs) {
			return;
		}

		tabs->addTab(child);
		syncActiveTab();

		child->addCallback(dwt::Message(WM_MDIACTIVATE), [this, child](const MSG& msg, LRESULT&) {
			if(tabs && reinterpret_cast<HWND>(msg.lParam) == child->handle()) {
				tabs->setActive(child);
			}
			return false;
		});
		child->addCallback(dwt::Message(WM_SETTEXT), [this, child](const MSG&, LRESULT&) {
			if(tabs) {
				tabs->updateTab(child);
			}
			return false;
		});
		child->addCallback(dwt::Message(WM_DESTROY), [this, child](const MSG&, LRESULT&) {
			if(tabs) {
				tabs->removeTab(child);
				syncActiveTab();
			}
			return false;
		});
	}

	void syncActiveTab() {
		if(!tabs || !getMDIParent()) {
			return;
		}

		if(auto activeChild = dynamic_cast<MDIChild*>(getMDIParent()->getActive())) {
			tabs->setActive(activeChild);
		}
	}

	FontPtr uiFont;
	dwt::MenuPtr menuBar;
	Menu* fileMenu;
	Menu* windowMenu;
	Menu* backgroundMenu;
	Menu* helpMenu;
	FlatMDITabs* tabs;
	COLORREF backgroundColor;
	unsigned childCounter;
};

} // namespace

int dwtMain(dwt::Application& app) {
	auto* frame = new DemoFrame();
	frame->createFrame();
	app.run();
	return 0;
}
