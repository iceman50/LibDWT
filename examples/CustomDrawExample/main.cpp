#include <dwt/Application.h>
#include <dwt/CanvasClasses.h>
#include <dwt/Message.h>
#include <dwt/WidgetCreator.h>

#include <dwt/resources/Brush.h>
#include <dwt/resources/Font.h>
#include <dwt/resources/Pen.h>

#include <dwt/widgets/Button.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Header.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/ProgressBar.h>
#include <dwt/widgets/Rebar.h>
#include <dwt/widgets/Slider.h>
#include <dwt/widgets/Table.h>
#include <dwt/widgets/TableTree.h>
#include <dwt/widgets/ToolBar.h>
#include <dwt/widgets/ToolTip.h>
#include <dwt/widgets/Tree.h>
#include <dwt/widgets/VirtualTree.h>
#include <dwt/widgets/Window.h>

#include <memory>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

using dwt::Brush;
using dwt::Button;
using dwt::FreeCanvas;
using dwt::Grid;
using dwt::GridInfo;
using dwt::Header;
using dwt::Label;
using dwt::Pen;
using dwt::ProgressBar;
using dwt::Rebar;
using dwt::Slider;
using dwt::Table;
using dwt::TableTree;
using dwt::ToolBar;
using dwt::ToolTip;
using dwt::Tree;
using dwt::VirtualTree;
using dwt::WidgetCreator;
using dwt::Window;

struct Palette {
	COLORREF windowBg;
	COLORREF panelBg;
	COLORREF text;
	COLORREF mutedText;
	COLORREF accent;
	COLORREF accentText;
	COLORREF rowEven;
	COLORREF rowOdd;
	COLORREF headerBg;
	COLORREF headerHot;
	COLORREF buttonFace;
	COLORREF buttonHot;
	COLORREF border;
	COLORREF progressTrack;
	COLORREF progressFill;
};

Palette makePalette(bool dark) {
	if(dark) {
		return {
			RGB(24, 27, 31),
			RGB(34, 39, 45),
			RGB(229, 233, 238),
			RGB(164, 172, 181),
			RGB(86, 172, 255),
			RGB(5, 16, 28),
			RGB(39, 44, 51),
			RGB(32, 36, 42),
			RGB(45, 52, 60),
			RGB(57, 73, 91),
			RGB(56, 64, 72),
			RGB(65, 76, 88),
			RGB(87, 97, 108),
			RGB(30, 35, 41),
			RGB(72, 169, 255)
		};
	}

	return {
		RGB(247, 249, 252),
		RGB(235, 240, 246),
		RGB(35, 42, 50),
		RGB(98, 108, 120),
		RGB(0, 102, 204),
		RGB(255, 255, 255),
		RGB(255, 255, 255),
		RGB(244, 248, 252),
		RGB(222, 234, 247),
		RGB(202, 224, 247),
		RGB(237, 242, 248),
		RGB(220, 232, 244),
		RGB(170, 180, 192),
		RGB(229, 235, 242),
		RGB(0, 122, 204)
	};
}

void fillRect(HDC hdc, const RECT& rc, COLORREF color) {
	if(rc.left >= rc.right || rc.top >= rc.bottom) {
		return;
	}

	FreeCanvas canvas { hdc };
	Brush brush { color };
	canvas.fill(dwt::Rectangle(rc), brush);
}

COLORREF blendColor(COLORREF base, COLORREF overlay, BYTE overlayWeight) {
	const BYTE baseWeight = static_cast<BYTE>(255 - overlayWeight);
	return RGB(
		(static_cast<int>(GetRValue(base)) * baseWeight + static_cast<int>(GetRValue(overlay)) * overlayWeight) / 255,
		(static_cast<int>(GetGValue(base)) * baseWeight + static_cast<int>(GetGValue(overlay)) * overlayWeight) / 255,
		(static_cast<int>(GetBValue(base)) * baseWeight + static_cast<int>(GetBValue(overlay)) * overlayWeight) / 255
	);
}

void drawBorder(HDC hdc, const RECT& rc, COLORREF color) {
	RECT edge = rc;
	edge.bottom = edge.top + 1;
	fillRect(hdc, edge, color);
	edge = rc;
	edge.top = edge.bottom - 1;
	fillRect(hdc, edge, color);
	edge = rc;
	edge.right = edge.left + 1;
	fillRect(hdc, edge, color);
	edge = rc;
	edge.left = edge.right - 1;
	fillRect(hdc, edge, color);
}

void drawTextInRect(HDC hdc, const dwt::tstring& text, RECT rc, COLORREF color, UINT format) {
	FreeCanvas canvas { hdc };
	auto transparentBk = canvas.setBkMode(true);
	canvas.setTextColor(color);
	dwt::Rectangle textRect(rc);
	canvas.drawText(text, textRect, format | DT_END_ELLIPSIS | DT_NOPREFIX);
}

dwt::FontPtr createExampleFont(unsigned dpi = 96) {
	LOGFONT lf = {};
	lf.lfHeight = -::MulDiv(9, static_cast<int>(dpi), 72);
	lf.lfWeight = FW_NORMAL;
	lf.lfQuality = CLEARTYPE_QUALITY;
	_tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Segoe UI"));
	return dwt::FontPtr(new dwt::Font(lf));
}

LRESULT drawButton(NMCUSTOMDRAW& data, bool dark) {
	auto palette = makePalette(dark);
	if(data.dwDrawStage != CDDS_PREPAINT) {
		return CDRF_DODEFAULT;
	}

	const bool disabled = (data.uItemState & CDIS_DISABLED) != 0;
	const bool pressed = (data.uItemState & CDIS_SELECTED) != 0;
	const bool hot = (data.uItemState & CDIS_HOT) != 0;
	auto face = palette.buttonFace;
	if(pressed) {
		face = blendColor(face, palette.accent, dark ? 95 : 55);
	} else if(hot) {
		face = palette.buttonHot;
	}

	fillRect(data.hdc, data.rc, face);
	drawBorder(data.hdc, data.rc, palette.border);

	TCHAR caption[256] = { 0 };
	::GetWindowText(data.hdr.hwndFrom, caption, 255);

	RECT textRect = data.rc;
	::InflateRect(&textRect, -10, -4);
	if(pressed) {
		::OffsetRect(&textRect, 1, 1);
	}

	drawTextInRect(data.hdc, caption, textRect,
		disabled ? palette.mutedText : palette.text,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	if((data.uItemState & CDIS_FOCUS) != 0) {
		RECT focusRect = data.rc;
		::InflateRect(&focusRect, -4, -4);
		::DrawFocusRect(data.hdc, &focusRect);
	}

	return CDRF_SKIPDEFAULT;
}

void drawSortArrow(HDC hdc, const RECT& rc, COLORREF color, bool ascending) {
	const int centerX = (rc.left + rc.right) / 2;
	const int centerY = (rc.top + rc.bottom) / 2;
	POINT points[3] = {};
	if(ascending) {
		points[0] = { centerX, centerY - 4 };
		points[1] = { centerX - 5, centerY + 3 };
		points[2] = { centerX + 5, centerY + 3 };
	} else {
		points[0] = { centerX - 5, centerY - 3 };
		points[1] = { centerX + 5, centerY - 3 };
		points[2] = { centerX, centerY + 4 };
	}

	FreeCanvas canvas { hdc };
	Pen pen { color, Pen::Solid, 1 };
	Brush brush { color };
	auto penSel = canvas.select(pen);
	auto brushSel = canvas.select(brush);
	canvas.polygon(points, 3);
}

LRESULT drawHeader(NMCUSTOMDRAW& data, bool dark) {
	auto palette = makePalette(dark);
	if(data.dwDrawStage == CDDS_PREPAINT) {
		return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
	}
	if(data.dwDrawStage == CDDS_POSTPAINT) {
		RECT clientRect = {};
		::GetClientRect(data.hdr.hwndFrom, &clientRect);

		const int count = Header_GetItemCount(data.hdr.hwndFrom);
		RECT lastItemRect = {};
		if(count > 0 && Header_GetItemRect(data.hdr.hwndFrom, count - 1, &lastItemRect)) {
			if(lastItemRect.right < clientRect.right) {
				clientRect.left = lastItemRect.right;
				fillRect(data.hdc, clientRect, palette.headerBg);
			}
		} else {
			fillRect(data.hdc, clientRect, palette.headerBg);
		}
		return CDRF_DODEFAULT;
	}
	if(data.dwDrawStage != CDDS_ITEMPREPAINT) {
		return CDRF_DODEFAULT;
	}

	auto background = palette.headerBg;
	if((data.uItemState & CDIS_SELECTED) != 0) {
		background = blendColor(palette.headerBg, palette.accent, dark ? 95 : 70);
	} else if((data.uItemState & CDIS_HOT) != 0) {
		background = palette.headerHot;
	}

	fillRect(data.hdc, data.rc, background);
	drawBorder(data.hdc, data.rc, palette.border);

	TCHAR text[256] = { 0 };
	HDITEM item = { HDI_TEXT | HDI_FORMAT };
	item.pszText = text;
	item.cchTextMax = 255;
	Header_GetItem(data.hdr.hwndFrom, static_cast<int>(data.dwItemSpec), &item);

	RECT textRect = data.rc;
	textRect.left += 10;
	textRect.right -= 10;

	const bool sortUp = (item.fmt & HDF_SORTUP) != 0;
	const bool sortDown = (item.fmt & HDF_SORTDOWN) != 0;
	if(sortUp || sortDown) {
		RECT arrowRect = textRect;
		arrowRect.left = (arrowRect.right - 14 > arrowRect.left) ? arrowRect.right - 14 : arrowRect.left;
		textRect.right = arrowRect.left - 4;
		drawSortArrow(data.hdc, arrowRect, palette.text, sortUp);
	}

	UINT format = DT_LEFT;
	if((item.fmt & HDF_CENTER) == HDF_CENTER) {
		format = DT_CENTER;
	} else if((item.fmt & HDF_RIGHT) == HDF_RIGHT) {
		format = DT_RIGHT;
	}
	drawTextInRect(data.hdc, text, textRect, palette.text, format | DT_VCENTER | DT_SINGLELINE);

	return CDRF_SKIPDEFAULT;
}

LRESULT drawSlider(NMCUSTOMDRAW& data, bool dark) {
	auto palette = makePalette(dark);
	if(data.dwDrawStage == CDDS_PREPAINT) {
		return CDRF_NOTIFYITEMDRAW;
	}
	if(data.dwDrawStage != CDDS_ITEMPREPAINT) {
		return CDRF_DODEFAULT;
	}

	if(data.dwItemSpec == TBCD_CHANNEL) {
		RECT track = data.rc;
		const int center = (track.top + track.bottom) / 2;
		track.top = center - 3;
		track.bottom = center + 3;
		fillRect(data.hdc, track, palette.progressTrack);
		drawBorder(data.hdc, track, palette.border);
		return CDRF_SKIPDEFAULT;
	}

	if(data.dwItemSpec == TBCD_THUMB) {
		fillRect(data.hdc, data.rc, palette.progressFill);
		drawBorder(data.hdc, data.rc, palette.border);
		return CDRF_SKIPDEFAULT;
	}

	if(data.dwItemSpec == TBCD_TICS) {
		return CDRF_DODEFAULT;
	}

	return CDRF_DODEFAULT;
}

LRESULT drawProgress(NMCUSTOMDRAW& data, ProgressBar::ObjectType progress, bool dark) {
	if(data.dwDrawStage != CDDS_PREPAINT || !progress) {
		return CDRF_DODEFAULT;
	}

	auto palette = makePalette(dark);
	fillRect(data.hdc, data.rc, palette.progressTrack);
	drawBorder(data.hdc, data.rc, palette.border);

	const int minimum = progress->getMinValue();
	const int maximum = progress->getMaxValue();
	const int value = progress->getPosition();
	const int range = maximum > minimum ? maximum - minimum : 1;
	const int clamped = value < minimum ? minimum : (value > maximum ? maximum : value);

	RECT fill = data.rc;
	fill.left += 2;
	fill.top += 2;
	fill.bottom -= 2;
	fill.right = fill.left + ((data.rc.right - data.rc.left - 4) * (clamped - minimum)) / range;
	fillRect(data.hdc, fill, palette.progressFill);

	RECT textRect = data.rc;
	const int percent = ((clamped - minimum) * 100) / range;
	drawTextInRect(data.hdc, std::to_wstring(percent) + _T("%"), textRect,
		dark ? palette.text : palette.accentText,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	return CDRF_SKIPDEFAULT;
}

void applyTheme(Label::ObjectType status, ProgressBar::ObjectType progress,
	ToolTip::ObjectType toolTip, bool dark)
{
	auto palette = makePalette(dark);
	if(progress) {
		progress->setBarColor(palette.progressFill);
		progress->setBackgroundColor(palette.progressTrack);
	}
	if(toolTip) {
		toolTip->sendMessage(TTM_SETTIPBKCOLOR, static_cast<WPARAM>(palette.panelBg));
		toolTip->sendMessage(TTM_SETTIPTEXTCOLOR, static_cast<WPARAM>(palette.text));
	}
	if(status) {
		status->setText(dark
			? _T("Dark custom draw active across Button, Header, Rebar, Slider, Table, TableTree, ToolBar, ToolTip, Tree, VirtualTree, and ProgressBar.")
			: _T("Light custom draw active across Button, Header, Rebar, Slider, Table, TableTree, ToolBar, ToolTip, Tree, VirtualTree, and ProgressBar."));
	}
}

} // namespace

int dwtMain(dwt::Application& app) {
	Window::Seed seed(_T("DWT CustomDrawExample"));
	seed.location = dwt::Rectangle(80, 70, 1480, 900);
	auto* window = WidgetCreator<Window>::create(seed);
	window->addRemoveStyle(WS_CLIPCHILDREN, true);

	auto uiFont = createExampleFont(window->getDpi());
	window->setFont(uiFont);

	auto* grid = WidgetCreator<Grid>::create(window, Grid::Seed(7, 4));
	grid->setFont(uiFont);
	grid->addRemoveStyle(WS_CLIPCHILDREN, true);
	grid->addRemoveStyle(WS_CLIPSIBLINGS, true);
	grid->setSpacing(8);
	for(int i = 0; i < 4; ++i) {
		grid->column(i).mode = GridInfo::FILL;
		grid->column(i).align = GridInfo::STRETCH;
	}
	grid->row(0).mode = GridInfo::STATIC;
	grid->row(0).size = 36;
	grid->row(1).mode = GridInfo::AUTO;
	grid->row(2).mode = GridInfo::STATIC;
	grid->row(2).size = 54;
	grid->row(3).mode = GridInfo::STATIC;
	grid->row(3).size = 54;
	grid->row(4).mode = GridInfo::STATIC;
	grid->row(4).size = 34;
	grid->row(5).mode = GridInfo::FILL;
	grid->row(5).align = GridInfo::STRETCH;
	grid->row(6).mode = GridInfo::FILL;
	grid->row(6).align = GridInfo::STRETCH;
	for(int i = 0; i < 7; ++i) {
		grid->row(i).align = GridInfo::STRETCH;
	}

	auto* rebar = WidgetCreator<Rebar>::create(grid, Rebar::Seed());
	rebar->setFont(uiFont);
	auto* toolBar = WidgetCreator<ToolBar>::create(rebar, ToolBar::Seed());
	toolBar->setFont(uiFont);
	auto* status = WidgetCreator<Label>::create(grid, Label::Seed(_T("Custom draw palette active.")));
	status->setFont(uiFont);

	Button::Seed regularButtonSeed(_T("Use Light Palette"), BS_NOTIFY);
	regularButtonSeed.font = uiFont;
	auto* regularButton = WidgetCreator<Button>::create(grid, regularButtonSeed);
	Button::Seed darkButtonSeed(_T("Use Dark Palette"), BS_NOTIFY);
	darkButtonSeed.font = uiFont;
	auto* darkButton = WidgetCreator<Button>::create(grid, darkButtonSeed);

	ProgressBar::Seed progressSeed;
	progressSeed.style |= PBS_SMOOTH;
	auto* progress = WidgetCreator<ProgressBar>::create(grid, progressSeed);
	progress->setFont(uiFont);
	progress->setRange(0, 100);
	progress->setPosition(62);

	Slider::Seed sliderSeed;
	sliderSeed.style |= TBS_ENABLESELRANGE | TBS_AUTOTICKS;
	auto* slider = WidgetCreator<Slider>::create(grid, sliderSeed);
	slider->setFont(uiFont);
	slider->setRange(0, 100);
	slider->setPosition(62);
	slider->setSelection(25, 75);
	slider->setTickFrequency(10);

	auto* sliderLabel = WidgetCreator<Label>::create(grid, Label::Seed(_T("Trackbar custom draw: owner-painted channel and thumb")));
	sliderLabel->setFont(uiFont);

	Header::Seed headerSeed;
	headerSeed.style |= HDS_BUTTONS | HDS_DRAGDROP;
	auto* header = WidgetCreator<Header>::create(grid, headerSeed);
	header->setFont(uiFont);
	header->insert(_T("Control"), 190);
	header->insert(_T("Native draw data"), 220);
	header->insert(_T("Example behavior"), 520);
	header->setSortArrow(0, 1);

	Table::Seed tableSeed;
	tableSeed.font = uiFont;
	auto* table = WidgetCreator<Table>::create(grid, tableSeed);
	table->addColumn(_T("CustomDraw Target"), 190);
	table->addColumn(_T("Aspect Type"), 260);
	table->addColumn(_T("Visible Customization"), 420);
	table->setGridLines(true);
	table->setFullRowSelect(true);
	table->setAlwaysShowSelection(true);
	table->setReadOnly(true);
	table->insert({ _T("Button"), _T("NMCUSTOMDRAW"), _T("Full owner painting with hot/pressed/focus states") }, 1);
	table->insert({ _T("Header"), _T("NMCUSTOMDRAW"), _T("Painted header cells, separators, and sort arrow") }, 2);
	table->insert({ _T("Rebar"), _T("NMCUSTOMDRAW"), _T("Band background palette") }, 3);
	table->insert({ _T("Slider"), _T("NMCUSTOMDRAW"), _T("Trackbar channel and thumb owner painting") }, 4);
	table->insert({ _T("Table"), _T("NMLVCUSTOMDRAW"), _T("Alternating rows and per-subitem colors") }, 5);
	table->insert({ _T("ToolBar"), _T("NMTBCUSTOMDRAW"), _T("Button face, text, and hot-track colors") }, 6);
	table->insert({ _T("ToolTip"), _T("NMTTCUSTOMDRAW"), _T("Tooltip text and background palette on hover") }, 7);
	table->insert({ _T("Tree"), _T("NMTVCUSTOMDRAW"), _T("Tree item colors plus LibDWT column renderer") }, 8);
	table->insert({ _T("ProgressBar"), _T("NMCUSTOMDRAW"), _T("Painted track, fill, border, and percentage text when notified") }, 9);
	table->insert({ _T("TableTree"), _T("Inherited NMLVCUSTOMDRAW"), _T("Hierarchical list glyphs plus list-view coloring") }, 10);
	table->insert({ _T("VirtualTree"), _T("Inherited NMTVCUSTOMDRAW"), _T("Virtualized tree items using the Tree custom-draw hook") }, 11);

	auto* tableTree = WidgetCreator<TableTree>::create(grid, TableTree::Seed(tableSeed));
	tableTree->addColumn(_T("Hierarchy"), 200);
	tableTree->addColumn(_T("Custom Draw Path"), 230);
	tableTree->addColumn(_T("Notes"), 330);
	tableTree->setGridLines(true);
	tableTree->setFullRowSelect(true);
	tableTree->setAlwaysShowSelection(true);
	tableTree->setReadOnly(true);
	tableTree->insert({ _T("Custom-draw surfaces"), _T("TableTree"), _T("Uses Table/ListView custom draw internally") }, 201);
	tableTree->insert({ _T("Button and Header"), _T("Child"), _T("Inherited table row drawing keeps this readable") }, 202);
	tableTree->insert({ _T("Tree and VirtualTree"), _T("Child"), _T("The hierarchy glyph is rendered by TableTree") }, 203);
	tableTree->insertChild(201, 202);
	tableTree->insertChild(201, 203);
	tableTree->expand(201);

	Tree::Seed treeSeed;
	treeSeed.font = uiFont;
	auto* tree = WidgetCreator<Tree>::create(grid, treeSeed);
	tree->addColumn(_T("Tree Item"), 210);
	tree->addColumn(_T("Role"), 160);
	tree->addColumn(_T("Custom Draw Detail"), 260);
	tree->setHasButtons(true);
	tree->setHasLines(true);
	tree->setLinesAtRoot(true);
	auto root = tree->insert(_T("Custom Draw Controls"), TVI_ROOT, TVI_LAST, 100, true);
	auto native = tree->insert(_T("Native notifications"), root, TVI_LAST, 101, true);
	tree->setText(native, 1, _T("Win32"));
	tree->setText(native, 2, _T("NM_CUSTOMDRAW routed through aspects"));
	auto tableItem = tree->insert(_T("Table / Tree"), root, TVI_LAST, 102, false);
	tree->setText(tableItem, 1, _T("Item draw"));
	tree->setText(tableItem, 2, _T("Per-item and subitem color control"));
	auto sliderItem = tree->insert(_T("Slider / Progress"), root, TVI_LAST, 103, false);
	tree->setText(sliderItem, 1, _T("Parts"));
	tree->setText(sliderItem, 2, _T("Channel, thumb, and fill painting"));
	tree->expand(root);

	VirtualTree::Seed virtualSeed(treeSeed);
	auto* virtualTree = WidgetCreator<VirtualTree>::create(grid, virtualSeed);
	virtualTree->addColumn(_T("Virtual Node"), 210);
	virtualTree->addColumn(_T("Draw Role"), 160);
	virtualTree->addColumn(_T("Notes"), 260);
	virtualTree->setHasButtons(true);
	virtualTree->setHasLines(true);
	virtualTree->setLinesAtRoot(true);
	auto virtualRoot = virtualTree->insert(_T("Virtual Custom Draw"), TVI_ROOT, TVI_LAST, 300, true);
	for(int i = 0; i < 5; ++i) {
		auto item = virtualTree->insert(_T("Virtual item ") + std::to_wstring(i + 1),
			virtualRoot, TVI_LAST, 301 + i, false);
		virtualTree->setText(item, 1, _T("Generated"));
		virtualTree->setText(item, 2, _T("Rendered through the Tree path"));
	}
	virtualTree->expand(virtualRoot);

	auto* toolTip = WidgetCreator<ToolTip>::create(window, ToolTip::Seed());
	toolTip->setFont(uiFont);
	toolTip->setMaxTipWidth(460);
	toolTip->setTitle(_T("Custom draw tooltip"), TTI_INFO);
	RECT tooltipMargins = { 10, 8, 10, 8 };
	toolTip->sendMessage(TTM_SETMARGIN, 0, reinterpret_cast<LPARAM>(&tooltipMargins));
	toolTip->addTool(regularButton);
	toolTip->setText(regularButton, _T("Switch to the light custom-draw palette."));
	toolTip->addTool(darkButton);
	toolTip->setText(darkButton, _T("Switch to the dark custom-draw palette."));
	toolTip->addTool(table);
	toolTip->setText(table, _T("ListView rows use NMLVCUSTOMDRAW for alternating colors."));
	toolTip->addTool(tree);
	toolTip->setText(tree, _T("TreeView custom draw supplies item colors before LibDWT renders extra columns."));
	toolTip->addTool(tableTree);
	toolTip->setText(tableTree, _T("TableTree inherits the ListView custom draw path and uses it for hierarchy glyphs."));
	toolTip->addTool(virtualTree);
	toolTip->setText(virtualTree, _T("VirtualTree inherits Tree custom draw while generating nodes on demand."));
	toolTip->addTool(slider);
	toolTip->setText(slider, _T("Trackbar custom draw paints the native channel and thumb parts."));
	toolTip->addTool(progress);
	toolTip->setText(progress, _T("ProgressBar custom draw paints a determinate progress surface when the native control asks for it."));

	constexpr int toolbarCommandBase = 100;
	constexpr int toolbarLightCommand = toolbarCommandBase;
	constexpr int toolbarDarkCommand = toolbarCommandBase + 1;
	constexpr int toolbarAdvanceCommand = toolbarCommandBase + 2;

	auto advanceControls = [slider, progress] {
		if(!slider || !progress) {
			return;
		}
		const int next = slider->getPosition() >= 95 ? 5 : slider->getPosition() + 5;
		slider->setPosition(next);
		progress->setPosition(next);
		::RedrawWindow(slider->handle(), nullptr, nullptr,
			RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
		::RedrawWindow(progress->handle(), nullptr, nullptr,
			RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
	};

	toolBar->addButton("light", -1, _T("Light"), true, 0, [regularButton] {
		::SendMessage(regularButton->handle(), BM_CLICK, 0, 0);
	});
	toolBar->addButton("dark", -1, _T("Dark"), true, 0, [darkButton] {
		::SendMessage(darkButton->handle(), BM_CLICK, 0, 0);
	});
	toolBar->addButton("step", -1, _T("Advance"), true, 0, [advanceControls] {
		advanceControls();
	});
	toolBar->setLayout({ "light", "dark", "", "step" });
	toolBar->refresh();
	rebar->add(toolBar);
	rebar->refresh();

	auto isDark = std::make_shared<bool>(false);

	window->addCallback(dwt::Message(WM_ERASEBKGND), [isDark](const MSG& msg, LRESULT& retVal) {
		auto palette = makePalette(*isDark);
		RECT rc = {};
		::GetClientRect(msg.hwnd, &rc);
		fillRect(reinterpret_cast<HDC>(msg.wParam), rc, palette.windowBg);
		retVal = 1;
		return true;
	});
	grid->addCallback(dwt::Message(WM_ERASEBKGND), [isDark](const MSG& msg, LRESULT& retVal) {
		auto palette = makePalette(*isDark);
		RECT rc = {};
		::GetClientRect(msg.hwnd, &rc);
		fillRect(reinterpret_cast<HDC>(msg.wParam), rc, palette.windowBg);
		retVal = 1;
		return true;
	});

	auto refreshAll = [window, grid, rebar, toolBar, regularButton, darkButton,
		header, slider, progress, table, tableTree, tree, virtualTree]
	{
		std::vector<HWND> handles = {
			window->handle(), grid->handle(), rebar->handle(), toolBar->handle(),
			regularButton->handle(), darkButton->handle(), header->handle(),
			slider->handle(), progress->handle(), table->handle(), tableTree->handle(),
			tree->handle(), tree->treeHandle(), virtualTree->handle(), virtualTree->treeHandle()
		};
		if(auto tableHeader = ListView_GetHeader(table->handle())) {
			handles.push_back(tableHeader);
		}
		for(auto handle : handles) {
			if(handle && ::IsWindow(handle)) {
				::RedrawWindow(handle, nullptr, nullptr,
					RDW_INVALIDATE | RDW_ERASE | RDW_FRAME | RDW_ALLCHILDREN);
			}
		}
	};

	regularButton->onCustomDraw([isDark](NMCUSTOMDRAW& data) -> LRESULT {
		return drawButton(data, *isDark);
	});
	darkButton->onCustomDraw([isDark](NMCUSTOMDRAW& data) -> LRESULT {
		return drawButton(data, *isDark);
	});

	header->onCustomDraw([isDark](NMCUSTOMDRAW& data) -> LRESULT {
		return drawHeader(data, *isDark);
	});

	rebar->onCustomDraw([isDark](NMCUSTOMDRAW& data) -> LRESULT {
		auto palette = makePalette(*isDark);
		if(data.dwDrawStage == CDDS_PREPAINT) {
			fillRect(data.hdc, data.rc, palette.panelBg);
			return CDRF_NOTIFYITEMDRAW;
		}
		if(data.dwDrawStage == CDDS_ITEMPREPAINT) {
			fillRect(data.hdc, data.rc, palette.panelBg);
			return CDRF_DODEFAULT;
		}
		return CDRF_DODEFAULT;
	});

	toolBar->onCustomDraw([isDark](NMTBCUSTOMDRAW& data) -> LRESULT {
		auto palette = makePalette(*isDark);
		if(data.nmcd.dwDrawStage == CDDS_PREPAINT) {
			fillRect(data.nmcd.hdc, data.nmcd.rc, palette.panelBg);
			return CDRF_NOTIFYITEMDRAW | TBCDRF_USECDCOLORS;
		}
		if(data.nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
			const bool hot = (data.nmcd.uItemState & CDIS_HOT) == CDIS_HOT;
			const bool selected = (data.nmcd.uItemState & CDIS_SELECTED) == CDIS_SELECTED;
			const auto face = (hot || selected) ? palette.buttonHot : palette.buttonFace;
			data.clrText = palette.text;
			data.clrBtnFace = face;
			data.clrBtnHighlight = face;
			data.clrHighlightHotTrack = palette.buttonHot;
			return TBCDRF_USECDCOLORS;
		}
		return CDRF_DODEFAULT;
	});

	table->onCustomDraw([isDark](NMLVCUSTOMDRAW& data) -> LRESULT {
		auto palette = makePalette(*isDark);
		if(data.nmcd.dwDrawStage == CDDS_PREPAINT) {
			return CDRF_NOTIFYITEMDRAW;
		}
		if(data.nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {
			return CDRF_NOTIFYSUBITEMDRAW;
		}
		if(data.nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
			const auto row = static_cast<unsigned>(data.nmcd.dwItemSpec);
			data.clrText = data.iSubItem == 0 ? palette.accent : palette.text;
			data.clrTextBk = (row % 2 == 0) ? palette.rowEven : palette.rowOdd;
			return CDRF_NEWFONT;
		}
		return CDRF_DODEFAULT;
	});

	tableTree->onCustomDraw([isDark](NMLVCUSTOMDRAW& data) -> LRESULT {
		auto palette = makePalette(*isDark);
		if(data.nmcd.dwDrawStage == (CDDS_ITEMPREPAINT | CDDS_SUBITEM)) {
			const auto row = static_cast<unsigned>(data.nmcd.dwItemSpec);
			data.clrText = data.iSubItem == 0 ? palette.accent : palette.text;
			data.clrTextBk = (row % 2 == 0) ? blendColor(palette.rowEven, palette.accent, 12) : palette.rowOdd;
			return CDRF_NEWFONT;
		}
		return CDRF_DODEFAULT;
	});

	slider->onCustomDraw([isDark](NMCUSTOMDRAW& data) -> LRESULT {
		return drawSlider(data, *isDark);
	});

	progress->onCustomDraw([progress, isDark](NMCUSTOMDRAW& data) -> LRESULT {
		return drawProgress(data, progress, *isDark);
	});

	toolTip->onCustomDraw([isDark](NMTTCUSTOMDRAW& data) -> LRESULT {
		auto palette = makePalette(*isDark);
		::SetTextColor(data.nmcd.hdc, palette.text);
		::SetBkColor(data.nmcd.hdc, palette.panelBg);
		return CDRF_DODEFAULT;
	});

	tree->onCustomDraw([isDark](NMTVCUSTOMDRAW& data) -> LRESULT {
		auto palette = makePalette(*isDark);
		if(data.nmcd.dwDrawStage == CDDS_ITEMPREPAINT ||
			data.nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
		{
			data.clrText = palette.text;
			data.clrTextBk = ((static_cast<UINT_PTR>(data.nmcd.dwItemSpec) / sizeof(void*)) % 2) ?
				palette.rowOdd : palette.rowEven;
		}
		return CDRF_DODEFAULT;
	});

	virtualTree->onCustomDraw([isDark](NMTVCUSTOMDRAW& data) -> LRESULT {
		auto palette = makePalette(*isDark);
		if(data.nmcd.dwDrawStage == CDDS_ITEMPREPAINT ||
			data.nmcd.dwDrawStage == CDDS_ITEMPOSTPAINT)
		{
			data.clrText = palette.text;
			data.clrTextBk = ((static_cast<UINT_PTR>(data.nmcd.dwItemSpec) / sizeof(void*)) % 2) ?
				blendColor(palette.rowOdd, palette.accent, 18) : palette.rowEven;
		}
		return CDRF_DODEFAULT;
	});

	auto setMode = [status, progress, toolTip, isDark, refreshAll](bool dark) {
		*isDark = dark;
		applyTheme(status, progress, toolTip, *isDark);
		refreshAll();
	};

	window->addCallback(dwt::Message(WM_COMMAND, toolbarLightCommand),
		[setMode](const MSG&, LRESULT&) -> bool {
			setMode(false);
			return true;
		});
	window->addCallback(dwt::Message(WM_COMMAND, toolbarDarkCommand),
		[setMode](const MSG&, LRESULT&) -> bool {
			setMode(true);
			return true;
		});
	window->addCallback(dwt::Message(WM_COMMAND, toolbarAdvanceCommand),
		[advanceControls](const MSG&, LRESULT&) -> bool {
			advanceControls();
			return true;
		});

	regularButton->onClicked([setMode] {
		setMode(false);
	});
	darkButton->onClicked([setMode] {
		setMode(true);
	});
	slider->onScrollHorz([slider, progress, refreshAll] {
		progress->setPosition(slider->getPosition());
		refreshAll();
	});

	window->onDestroy([toolTip] {
		toolTip->close();
		::PostQuitMessage(0);
	});

	grid->setWidget(rebar, 0, 0, 1, 4);
	grid->setWidget(status, 1, 0, 1, 4);
	grid->setWidget(regularButton, 2, 0);
	grid->setWidget(darkButton, 2, 1);
	grid->setWidget(progress, 2, 2, 1, 2);
	grid->setWidget(sliderLabel, 3, 0);
	grid->setWidget(slider, 3, 1, 1, 3);
	grid->setWidget(header, 4, 0, 1, 4);
	grid->setWidget(table, 5, 0, 1, 2);
	grid->setWidget(tree, 5, 2, 1, 2);
	grid->setWidget(tableTree, 6, 0, 1, 2);
	grid->setWidget(virtualTree, 6, 2, 1, 2);

	auto layout = [window, grid, rebar] {
		auto client = window->getClientSize();
		grid->resize(dwt::Rectangle(0, 0, client.x, client.y));
		grid->layout();
		rebar->refresh();
		::RedrawWindow(window->handle(), nullptr, nullptr,
			RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
	};

	window->onSized([layout](const dwt::SizedEvent&) {
		layout();
	});

	setMode(false);

	std::vector<dwt::Control*> visibleControls = {
		grid,
		rebar,
		toolBar,
		status,
		regularButton,
		darkButton,
		progress,
		sliderLabel,
		slider,
		header,
		table,
		tableTree,
		tree,
		virtualTree,
	};
	for(auto* control : visibleControls) {
		if(control) {
			control->setVisible(true);
		}
	}
	toolTip->setVisible(true);
	layout();

	window->setVisible(true);
	window->setFocus();
	app.run();
	return 0;
}
