#include <dwt/Application.h>
#include <dwt/CanvasClasses.h>
#include <dwt/Message.h>
#include <dwt/WidgetCreator.h>

#include <dwt/resources/Font.h>
#include <dwt/resources/Brush.h>
#include <dwt/resources/Pen.h>

#include <dwt/widgets/Button.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Header.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/Rebar.h>
#include <dwt/widgets/Table.h>
#include <dwt/widgets/ToolBar.h>
#include <dwt/widgets/ToolTip.h>
#include <dwt/widgets/Window.h>

#include <memory>
#include <vector>

#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace {

using dwt::Button;
using dwt::Brush;
using dwt::FreeCanvas;
using dwt::Grid;
using dwt::GridInfo;
using dwt::Header;
using dwt::Label;
using dwt::Pen;
using dwt::Rebar;
using dwt::Table;
using dwt::ToolBar;
using dwt::ToolTip;
using dwt::WindowUpdateCanvas;
using dwt::WidgetCreator;
using dwt::Window;

struct Palette {
	COLORREF windowBg;
	COLORREF panelBg;
	COLORREF text;
	COLORREF accent;
	COLORREF rowEven;
	COLORREF rowOdd;
	COLORREF headerBg;
	COLORREF headerText;
	COLORREF buttonFace;
	COLORREF buttonHot;
};

Palette makePalette(bool dark) {
	if(dark) {
		return {
			RGB(24, 27, 31),
			RGB(33, 38, 44),
			RGB(226, 230, 235),
			RGB(90, 170, 255),
			RGB(38, 43, 49),
			RGB(32, 36, 42),
			RGB(44, 50, 58),
			RGB(236, 239, 242),
			RGB(56, 64, 72),
			RGB(62, 72, 82)
		};
	}

	return {
		RGB(248, 249, 251),
		RGB(238, 242, 247),
		RGB(36, 42, 49),
		RGB(0, 102, 204),
		RGB(255, 255, 255),
		RGB(245, 248, 252),
		RGB(221, 232, 245),
		RGB(19, 62, 110),
		RGB(236, 241, 247),
		RGB(216, 226, 237)
	};
}

void fillRect(HDC hdc, const RECT& rc, COLORREF color) {
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

int getHoveredHeaderItem(HWND headerHandle) {
	if(!::IsWindow(headerHandle)) {
		return -1;
	}

	POINT cursorPoint = {};
	if(!::GetCursorPos(&cursorPoint)) {
		return -1;
	}

	RECT headerRect = {};
	if(!::GetWindowRect(headerHandle, &headerRect) || !::PtInRect(&headerRect, cursorPoint)) {
		return -1;
	}

	if(!::ScreenToClient(headerHandle, &cursorPoint)) {
		return -1;
	}

	HDHITTESTINFO hit = {};
	hit.pt = cursorPoint;
	const int item = static_cast<int>(::SendMessage(headerHandle, HDM_HITTEST, 0, reinterpret_cast<LPARAM>(&hit)));
	return item >= 0 ? item : -1;
}

void drawHeaderSortArrow(HDC hdc, const RECT& rect, COLORREF color, bool ascending) {
	FreeCanvas canvas { hdc };
	const int centerX = (rect.left + rect.right) / 2;
	const int centerY = (rect.top + rect.bottom) / 2;
	POINT points[3] = {};

	if(ascending) {
		points[0] = { centerX, centerY - 3 };
		points[1] = { centerX - 4, centerY + 2 };
		points[2] = { centerX + 4, centerY + 2 };
	} else {
		points[0] = { centerX - 4, centerY - 2 };
		points[1] = { centerX + 4, centerY - 2 };
		points[2] = { centerX, centerY + 3 };
	}

	Pen pen { color, Pen::Solid, 1 };
	Brush brush { color };
	auto penSel = canvas.select(pen);
	auto brushSel = canvas.select(brush);
	canvas.polygon(points, 3);
}

LRESULT drawModeButtonCustomDraw(NMCUSTOMDRAW& data, bool dark) {
	auto palette = makePalette(dark);
	if(data.dwDrawStage == CDDS_PREPAINT) {
		return CDRF_NOTIFYITEMDRAW;
	}
	if(data.dwDrawStage != CDDS_ITEMPREPAINT) {
		return CDRF_DODEFAULT;
	}

	const bool disabled = (data.uItemState & CDIS_DISABLED) != 0;
	const bool pressed = (data.uItemState & CDIS_SELECTED) != 0;
	const bool hot = (data.uItemState & CDIS_HOT) != 0;

	COLORREF face = palette.buttonFace;
	if(dark) {
		if(pressed) {
			face = blendColor(palette.buttonFace, palette.accent, 70);
		} else if(hot) {
			face = palette.buttonHot;
		}
	} else {
		if(pressed) {
			face = blendColor(palette.buttonFace, palette.accent, 24);
		} else if(hot) {
			face = palette.buttonHot;
		}
	}

	const COLORREF border = dark ? RGB(96, 106, 116) : RGB(170, 178, 188);
	const COLORREF textColor = disabled ? (dark ? blendColor(palette.text, palette.panelBg, 108) : ::GetSysColor(COLOR_GRAYTEXT)) : palette.text;

	fillRect(data.hdc, data.rc, face);

	RECT top = data.rc;
	top.bottom = top.top + 1;
	fillRect(data.hdc, top, border);

	RECT left = data.rc;
	left.right = left.left + 1;
	fillRect(data.hdc, left, border);

	RECT right = data.rc;
	right.left = right.right - 1;
	fillRect(data.hdc, right, border);

	RECT bottom = data.rc;
	bottom.top = bottom.bottom - 1;
	fillRect(data.hdc, bottom, border);

	TCHAR caption[256] = { 0 };
	::GetWindowText(data.hdr.hwndFrom, caption, 255);

	RECT textRect = data.rc;
	if(pressed) {
		::OffsetRect(&textRect, 1, 1);
	}
	::InflateRect(&textRect, -8, -4);

	FreeCanvas canvas { data.hdc };
	auto transparentBk = canvas.setBkMode(true);
	canvas.setTextColor(textColor);
	dwt::Rectangle drawRect(textRect);
	canvas.drawText(caption, drawRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

	if((data.uItemState & CDIS_FOCUS) != 0) {
		RECT focusRect = data.rc;
		::InflateRect(&focusRect, -4, -4);
		::DrawFocusRect(data.hdc, &focusRect);
	}

	return CDRF_SKIPDEFAULT;
}

void applyTheme(Label::ObjectType status, Table::ObjectType table, ToolTip::ObjectType toolTip, bool dark) {
	auto palette = makePalette(dark);
	table->setColor(palette.text, palette.rowEven);
	toolTip->sendMessage(TTM_SETTIPBKCOLOR, static_cast<WPARAM>(palette.panelBg));
	toolTip->sendMessage(TTM_SETTIPTEXTCOLOR, static_cast<WPARAM>(palette.text));
	status->setText(dark
		? _T("Dark mode custom draw is active for Rebar, ToolBar, Table, ToolTip, and the Table header.")
		: _T("Regular custom draw palette is active for Rebar, ToolBar, Table, ToolTip, and the Table header."));
}

dwt::FontPtr createSystemUiFont() {
	NONCLIENTMETRICS metrics = { sizeof(metrics) };
	if(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0)) {
		return dwt::FontPtr(new dwt::Font(metrics.lfMessageFont));
	}
	return dwt::FontPtr(new dwt::Font(dwt::Font::DefaultGui));
}

} // namespace

int dwtMain(dwt::Application& app) {
	Window::Seed seed(_T("DWT CustomDrawExample"));
	seed.location = dwt::Rectangle(120, 100, 1120, 760);
	auto* window = WidgetCreator<Window>::create(seed);
	window->addRemoveStyle(WS_CLIPCHILDREN, true);
	auto uiFont = window->getFont();
	auto systemUiFont = createSystemUiFont();

	auto* grid = WidgetCreator<Grid>::create(window, Grid::Seed(4, 2));
	grid->addRemoveStyle(WS_CLIPCHILDREN, true);
	grid->addRemoveStyle(WS_CLIPSIBLINGS, true);
	grid->setSpacing(8);
	grid->column(0).mode = GridInfo::FILL;
	grid->column(1).mode = GridInfo::FILL;
	grid->column(0).align = GridInfo::STRETCH;
	grid->column(1).align = GridInfo::STRETCH;
	grid->row(0).mode = GridInfo::STATIC;
	grid->row(0).size = 36;
	grid->row(1).mode = GridInfo::AUTO;
	grid->row(2).mode = GridInfo::AUTO;
	grid->row(3).mode = GridInfo::FILL;
	grid->row(3).align = GridInfo::STRETCH;

	auto* rebar = WidgetCreator<Rebar>::create(grid, Rebar::Seed());
	auto* toolBar = WidgetCreator<ToolBar>::create(rebar, ToolBar::Seed());
	auto* status = WidgetCreator<Label>::create(grid, Label::Seed(_T("Custom draw palette active.")));
	auto* regularButton = WidgetCreator<Button>::create(grid, Button::Seed(_T("Use Regular Colors"), BS_NOTIFY));
	auto* darkButton = WidgetCreator<Button>::create(grid, Button::Seed(_T("Use Dark Mode Look"), BS_NOTIFY));

	Table::Seed tableSeed;
	tableSeed.font = uiFont;
	auto* table = WidgetCreator<Table>::create(grid, tableSeed);

	auto* toolTip = WidgetCreator<ToolTip>::create(window, ToolTip::Seed());
	toolTip->setMaxTipWidth(480);
	RECT tooltipMargins = { 10, 8, 10, 8 };
	toolTip->sendMessage(TTM_SETMARGIN, 0, reinterpret_cast<LPARAM>(&tooltipMargins));
	toolTip->sendMessage(TTM_SETWINDOWTHEME, 0, reinterpret_cast<LPARAM>(_T("")));
	::SetWindowTheme(toolTip->handle(), L"", L"");
	toolTip->addTool(regularButton);
	toolTip->setText(regularButton, _T("Switch to the regular custom-draw palette."));
	toolTip->addTool(darkButton);
	toolTip->setText(darkButton, _T("Switch to the dark custom-draw palette."));
	toolTip->addTool(table);
	toolTip->setText(table, _T("Rows and headers are owner colored by custom draw notifications."));

	status->setFont(systemUiFont ? systemUiFont : uiFont);
	table->addColumn(_T("CustomDraw Target"), 210);
	table->addColumn(_T("Aspect Type"), 230);
	table->addColumn(_T("What To Observe"), 560);
	table->setGridLines(true);
	table->setFullRowSelect(true);
	table->setAlwaysShowSelection(true);
	table->setReadOnly(true);
	table->insert({ _T("Rebar"), _T("CustomDraw<Rebar, NMCUSTOMDRAW>"), _T("Band background is painted to palette colors.") }, 1);
	table->insert({ _T("ToolBar"), _T("CustomDraw<ToolBar, NMTBCUSTOMDRAW>"), _T("Button face/text/hot colors are supplied by custom draw.") }, 2);
	table->insert({ _T("Table"), _T("CustomDraw<Table, NMLVCUSTOMDRAW>"), _T("Alternating row colors and text colors are controlled in custom draw.") }, 3);
	table->insert({ _T("ToolTip"), _T("CustomDraw<ToolTip, NMTTCUSTOMDRAW>"), _T("Tooltip uses native text rendering with custom palette colors.") }, 4);
	table->setChecked(0, true);
    //For whatever reason the header will not CustomDraw using the table attached header, so let's make a new one and attach
    // it with WidgetCreator
	auto headerHandle = ListView_GetHeader(table->handle());
	Header* tableHeader = nullptr;
	if(headerHandle) {
		tableHeader = WidgetCreator<Header>::attach(table, headerHandle);
	}

	toolBar->addButton("regular", -1, _T("Regular"), true, 0, [regularButton] {
		::SendMessage(regularButton->handle(), BM_CLICK, 0, 0);
	});
	toolBar->addButton("dark", -1, _T("Dark"), true, 0, [darkButton] {
		::SendMessage(darkButton->handle(), BM_CLICK, 0, 0);
	});
	toolBar->addButton("tip", -1, _T("Tip"), true, 0, [table] {
		table->setSelected(0);
	});
	toolBar->setLayout({ "regular", "dark", "", "tip" });
	toolBar->refresh();
	rebar->add(toolBar);
	rebar->refresh();

	auto isDark = std::make_shared<bool>(false);
	const auto buttonDraw = [isDark](WPARAM, LPARAM lParam) -> LRESULT {
		auto* draw = reinterpret_cast<NMCUSTOMDRAW*>(lParam);
		if(!draw) {
			return CDRF_DODEFAULT;
		}
		return drawModeButtonCustomDraw(*draw, *isDark);
	};
	regularButton->onRaw(buttonDraw, dwt::Message(WM_NOTIFY, NM_CUSTOMDRAW));
	darkButton->onRaw(buttonDraw, dwt::Message(WM_NOTIFY, NM_CUSTOMDRAW));

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

	auto refreshAll = [window, rebar, toolBar, regularButton, darkButton, table] {
		::InvalidateRect(window->handle(), nullptr, TRUE);
		::InvalidateRect(rebar->handle(), nullptr, TRUE);
		::InvalidateRect(toolBar->handle(), nullptr, TRUE);
		::InvalidateRect(regularButton->handle(), nullptr, TRUE);
		::InvalidateRect(darkButton->handle(), nullptr, TRUE);
		::InvalidateRect(table->handle(), nullptr, TRUE);
		HWND header = ListView_GetHeader(table->handle());
		if(header) {
			::InvalidateRect(header, nullptr, TRUE);
		}
	};

	rebar->onCustomDraw([isDark](NMCUSTOMDRAW& data) -> LRESULT {
		auto palette = makePalette(*isDark);
		if(data.dwDrawStage == CDDS_PREPAINT) {
			fillRect(data.hdc, data.rc, palette.panelBg);
			return CDRF_NOTIFYITEMDRAW;
		}
		if(data.dwDrawStage == CDDS_ITEMPREPAINT) {
			fillRect(data.hdc, data.rc, palette.panelBg);
			return CDRF_SKIPDEFAULT;
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
			auto hot = (data.nmcd.uItemState & CDIS_HOT) == CDIS_HOT;
			auto selected = (data.nmcd.uItemState & CDIS_SELECTED) == CDIS_SELECTED;
			auto face = (hot || selected) ? palette.buttonHot : palette.buttonFace;
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
			auto row = static_cast<unsigned>(data.nmcd.dwItemSpec);
			data.clrText = palette.text;
			data.clrTextBk = (row % 2 == 0) ? palette.rowEven : palette.rowOdd;
			return CDRF_NEWFONT;
		}
		return CDRF_DODEFAULT;
	});

	toolTip->onCustomDraw([](NMTTCUSTOMDRAW&) -> LRESULT {
		// Keep the tooltip in the CustomDraw path, but rely on native tooltip text rendering.
		return CDRF_DODEFAULT;
	});

	auto drawHeader = [isDark](NMCUSTOMDRAW& data) -> LRESULT {
		auto* hdr = reinterpret_cast<const NMHDR*>(&data.hdr);
		if(!hdr) {
			return CDRF_DODEFAULT;
		}

		auto palette = makePalette(*isDark);
		if(data.dwDrawStage == CDDS_PREPAINT) {
			return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYPOSTPAINT;
		}
		if(data.dwDrawStage == CDDS_POSTPAINT) {
			RECT clientRect = {};
			::GetClientRect(hdr->hwndFrom, &clientRect);
			const COLORREF trailingBg = palette.headerBg;

			const int count = Header_GetItemCount(hdr->hwndFrom);
			RECT lastItemRect = {};
			if(count > 0 && Header_GetItemRect(hdr->hwndFrom, count - 1, &lastItemRect)) {
				if(lastItemRect.right < clientRect.right) {
					RECT trailing = clientRect;
					trailing.left = lastItemRect.right;
					fillRect(data.hdc, trailing, trailingBg);
				}
			} else {
				fillRect(data.hdc, clientRect, trailingBg);
			}

			return CDRF_DODEFAULT;
		}
		if(data.dwDrawStage == CDDS_ITEMPREPAINT) {
			const int hoveredItem = getHoveredHeaderItem(hdr->hwndFrom);
			const bool hot = ((data.uItemState & CDIS_HOT) != 0) || (hoveredItem == static_cast<int>(data.dwItemSpec));
			const bool pressed = (data.uItemState & CDIS_SELECTED) != 0;
			const bool disabled = (data.uItemState & CDIS_DISABLED) != 0;

			COLORREF background = palette.headerBg;
			const COLORREF hotBlue = *isDark ? RGB(126, 184, 255) : RGB(190, 224, 255);
			if(*isDark) {
				if(pressed) {
					background = blendColor(palette.panelBg, palette.accent, 96);
				} else if(hot) {
					background = hotBlue;
				}
			} else {
				if(pressed) {
					background = ::GetSysColor(COLOR_HIGHLIGHT);
				} else if(hot) {
					background = hotBlue;
				}
			}

			fillRect(data.hdc, data.rc, background);

			HDITEM item = { 0 };
			TCHAR text[256] = { 0 };
			item.mask = HDI_TEXT | HDI_FORMAT;
			item.pszText = text;
			item.cchTextMax = 255;
			Header_GetItem(hdr->hwndFrom, static_cast<int>(data.dwItemSpec), &item);

			FreeCanvas canvas { data.hdc };
			auto transparentBk = canvas.setBkMode(true);
			const COLORREF textColor = disabled ? (*isDark ? blendColor(palette.text, palette.panelBg, 80) : ::GetSysColor(COLOR_GRAYTEXT))
				: (pressed && !*isDark ? ::GetSysColor(COLOR_HIGHLIGHTTEXT) : palette.headerText);
			canvas.setTextColor(textColor);

			UINT format = DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS;
			if((item.fmt & HDF_CENTER) == HDF_CENTER) {
				format |= DT_CENTER;
			} else if((item.fmt & HDF_RIGHT) == HDF_RIGHT) {
				format |= DT_RIGHT;
			} else {
				format |= DT_LEFT;
			}

			RECT textRect = data.rc;
			textRect.left += 8;
			textRect.right -= 8;

			const bool sortAscending = (item.fmt & HDF_SORTUP) != 0;
			const bool sortDescending = (item.fmt & HDF_SORTDOWN) != 0;
			if(sortAscending || sortDescending) {
				RECT arrowRect = textRect;
				arrowRect.left = (textRect.right - 12 > textRect.left) ? (textRect.right - 12) : textRect.left;
				textRect.right = arrowRect.left - 4;
				drawHeaderSortArrow(data.hdc, arrowRect, textColor, sortAscending);
			}
			dwt::Rectangle textDrawRect(textRect);
			canvas.drawText(text, textDrawRect, format);

			const COLORREF separatorColor = *isDark ? RGB(128, 128, 128) : RGB(150, 150, 150);

			RECT leftEdge = data.rc;
			leftEdge.right = leftEdge.left + 1;
			fillRect(data.hdc, leftEdge, separatorColor);

			RECT rightEdge = data.rc;
			rightEdge.left = rightEdge.right - 1;
			fillRect(data.hdc, rightEdge, separatorColor);

			RECT bottom = data.rc;
			bottom.top = bottom.bottom - 1;
			fillRect(data.hdc, bottom, separatorColor);

			return CDRF_SKIPDEFAULT;
		}
		return CDRF_DODEFAULT;
	};
	if(tableHeader) {
		tableHeader->onRaw([drawHeader](WPARAM, LPARAM lParam) -> LRESULT {
			auto* draw = reinterpret_cast<NMCUSTOMDRAW*>(lParam);
			if(!draw) {
				return CDRF_DODEFAULT;
			}
			return drawHeader(*draw);
		}, dwt::Message(WM_NOTIFY, NM_CUSTOMDRAW));

		tableHeader->onRaw([tableHeader, isDark](WPARAM wParam, LPARAM lParam) -> LRESULT {
			if(!tableHeader || !::IsWindow(tableHeader->handle())) {
				return 0;
			}

			auto palette = makePalette(*isDark);

			WindowUpdateCanvas canvas { tableHeader };

			RECT windowRect = {};
			::GetWindowRect(tableHeader->handle(), &windowRect);
			::OffsetRect(&windowRect, -windowRect.left, -windowRect.top);

			RECT clientRect = {};
			::GetClientRect(tableHeader->handle(), &clientRect);
			::ExcludeClipRect(canvas.handle(), clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

			Brush ncBrush { palette.headerBg };
			canvas.fill(dwt::Rectangle(windowRect), ncBrush);

			return 0;
		}, dwt::Message(WM_NCPAINT));

		tableHeader->onRaw([tableHeader, isDark](WPARAM wParam, LPARAM lParam) -> LRESULT {
			if(tableHeader && ::IsWindow(tableHeader->handle())) {
				::RedrawWindow(tableHeader->handle(), nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
			}
			return 1;
		}, dwt::Message(WM_NCACTIVATE));
	}

	auto setMode = [status, table, toolTip, isDark, refreshAll](bool dark) {
		*isDark = dark;
		applyTheme(status, table, toolTip, *isDark);
		refreshAll();
		if(auto header = ListView_GetHeader(table->handle())) {
			::RedrawWindow(header, nullptr, nullptr, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
		}
	};

	regularButton->onClicked([setMode] {
		setMode(false);
	});
	darkButton->onClicked([setMode] {
		setMode(true);
	});

	window->onDestroy([] {
		::PostQuitMessage(0);
	});

	grid->setWidget(rebar, 0, 0, 1, 2);
	grid->setWidget(status, 1, 0, 1, 2);
	grid->setWidget(regularButton, 2, 0);
	grid->setWidget(darkButton, 2, 1);
	grid->setWidget(table, 3, 0, 1, 2);
	auto layout = [window, grid, rebar] {
		auto client = window->getClientSize();
		grid->resize(dwt::Rectangle(0, 0, client.x, client.y));
		grid->layout();
		rebar->refresh();
		::RedrawWindow(window->handle(), nullptr, nullptr, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
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
		table,
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
