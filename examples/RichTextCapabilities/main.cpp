#include <dwt/Application.h>
#include <dwt/WidgetCreator.h>
#include <dwt/widgets/RichTextBox.h>
#include <dwt/widgets/Window.h>

#include <string>
#include <vector>

namespace {

using dwt::RichTextBox;
using dwt::WidgetCreator;
using dwt::Window;

void wireLinkHandler(Window::ObjectType window, RichTextBox::ObjectType box) {
	window->addCallback(dwt::Message(WM_NOTIFY, EN_LINK), [box](const MSG& msg, LRESULT& result) {
		auto* enlink = reinterpret_cast<const ENLINK*>(msg.lParam);
		if(!enlink || enlink->nmhdr.hwndFrom != box->handle()) {
			return false;
		}

		if(enlink->msg != WM_LBUTTONUP && enlink->msg != WM_LBUTTONDBLCLK) {
			return false;
		}

		long len = enlink->chrg.cpMax - enlink->chrg.cpMin;
		if(len <= 0) {
			return false;
		}

		std::vector<TCHAR> buffer(static_cast<size_t>(len) + 1, 0);
		TEXTRANGE range = { 0 };
		range.chrg = enlink->chrg;
		range.lpstrText = buffer.data();
		box->sendMessage(EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&range));

		dwt::tstring url(buffer.data());
		while(!url.empty() && (url.back() == _T(' ') || url.back() == _T('\t') || url.back() == _T('\r') ||
			url.back() == _T('\n') || url.back() == _T(')') || url.back() == _T(']') || url.back() == _T('}') ||
			url.back() == _T('.') || url.back() == _T(',') || url.back() == _T(';'))) {
			url.pop_back();
		}
		if(url.empty()) {
			return false;
		}

		if(url.find(_T("http://")) != 0 && url.find(_T("https://")) != 0) {
			url = _T("https://") + url;
		}

		auto openRet = reinterpret_cast<INT_PTR>(::ShellExecute(nullptr, _T("open"), url.c_str(), nullptr, nullptr, SW_SHOWNORMAL));
		if(openRet <= 32) {
			return false;
		}
		result = 1;
		return true;
	});
}

void setRtfDocument(RichTextBox::ObjectType box) {
	const char* rtf =
		"{\\rtf1\\ansi\\ansicpg1252\\deff0"
		"{\\fonttbl{\\f0 Segoe UI;}{\\f1 Consolas;}{\\f2 Times New Roman;}}"
		"{\\colortbl;"
		"\\red0\\green0\\blue0;"
		"\\red0\\green102\\blue204;"
		"\\red192\\green0\\blue0;"
		"\\red0\\green128\\blue0;"
		"\\red255\\green235\\blue156;"
		"\\red235\\green245\\blue255;"
		"}"
		"\\viewkind4\\uc1\\pard\\sa140\\sl300\\slmult1\\f0\\fs22 "
		"\\b DWT RichEdit RTF Showcase\\b0\\par "
		"This control is loaded from one hardcoded RTF string in main.cpp.\\par "
		"\\par "
		"\\b Character formatting\\b0\\par "
		"\\b Bold\\b0, \\i italic\\i0, \\ul underline\\ulnone, \\strike strikeout\\strike0, "
		"\\super superscript\\nosupersub and \\sub subscript\\nosupersub.\\par "
		"Foreground colors: \\cf2 blue\\cf1, \\cf3 red\\cf1, \\cf4 green\\cf1.\\par "
		"Background highlight: \\highlight5 highlighted text\\highlight0.\\par "
		"Monospace run: \\f1 for(int i = 0; i < 3; ++i) { log(i); }\\f0\\par "
		"Unicode escapes: \\u945? \\u946? \\u947? (alpha beta gamma).\\par "
		"\\par "
		"\\b Paragraph formatting\\b0\\par "
		"\\qc Center aligned paragraph.\\par "
		"\\qr Right aligned paragraph.\\par "
		"\\qj Justified text paragraph demonstrating richer layout behavior in RichEdit controls.\\par "
		"\\ql\\li480\\ri240\\sb120\\sa120 Indented paragraph with left and right margins.\\par "
		"\\li0\\ri0\\sb0\\sa0 "
		"\\par "
		"\\b Bullets and numbering\\b0\\par "
		"\\bullet\\tab Bullet item one\\par "
		"\\bullet\\tab Bullet item two\\par "
		"1.\\tab Numbered item one\\par "
		"2.\\tab Numbered item two\\par "
		"\\par "
		"\\b Hyperlink\\b0\\par "
		"\\ul\\cf2 https://learn.microsoft.com/windows/win32/controls/about-rich-edit-controls\\ulnone\\cf1\\par "
		"\\par "
		"\\b Simple table\\b0\\par "
		"\\trowd\\trgaph108\\trleft0\\cellx2400\\cellx4800\\cellx7200 "
		"\\intbl\\b Feature\\b0\\cell\\intbl\\b Example\\b0\\cell\\intbl\\b Status\\b0\\cell\\row "
		"\\trowd\\trgaph108\\trleft0\\cellx2400\\cellx4800\\cellx7200 "
		"\\intbl Color\\cell\\intbl \\cf2 Blue text\\cf1\\cell\\intbl OK\\cell\\row "
		"\\trowd\\trgaph108\\trleft0\\cellx2400\\cellx4800\\cellx7200 "
		"\\intbl Highlight\\cell\\intbl \\highlight6 Background\\highlight0\\cell\\intbl OK\\cell\\row "
		"\\par "
		"\\b Editing features\\b0\\par "
		"Ctrl+A, Ctrl+C, Ctrl+V, Undo/Redo and selection APIs still work.\\par "
		"}";

	SETTEXTEX config = { ST_DEFAULT, CP_ACP };
	box->sendMessage(EM_SETTEXTEX, reinterpret_cast<WPARAM>(&config), reinterpret_cast<LPARAM>(rtf));
	box->sendMessage(EM_AUTOURLDETECT, TRUE);
	box->sendMessage(EM_SETEVENTMASK, 0, ENM_LINK | ENM_MOUSEEVENTS | ENM_SCROLL | ENM_SELCHANGE);
	box->setSelection(0, 0);
}

} // namespace

int dwtMain(dwt::Application& app) {
	Window::Seed seed(_T("DWT RichTextBox Capabilities"));
	seed.location = dwt::Rectangle(80, 80, 980, 680);
	auto* window = WidgetCreator<Window>::create(seed);

	auto* richText = WidgetCreator<RichTextBox>::create(window, RichTextBox::Seed());
	richText->resize(dwt::Rectangle(window->getClientSize()));
	richText->setVisible(true);

	window->onSized([window, richText](const dwt::SizedEvent&) {
		richText->resize(dwt::Rectangle(window->getClientSize()));
	});

	window->onDestroy([] {
		::PostQuitMessage(0);
	});

	wireLinkHandler(window, richText);
	setRtfDocument(richText);
	richText->setReadOnly(true);

	window->setVisible(true);
	window->setFocus();

	app.run();
	return 0;
}
