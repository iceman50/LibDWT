/*
  DC++ Widget Toolkit

  Copyright (c) 2007-2026, Jacek Sieka

  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

      * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
      * Neither the name of the DWT nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
*/

#include <dwt/util/win32/AccessibilityProvider.h>

#include <dwt/Widget.h>

#include <UIAutomationCore.h>
#include <UIAutomationCoreApi.h>

#include <atomic>
#include <cstring>

namespace dwt {

namespace {

static const PROPERTYID nameProperty = 30005;
static const PROPERTYID controlTypeProperty = 30003;
static const PROPERTYID hasKeyboardFocusProperty = 30008;
static const PROPERTYID isKeyboardFocusableProperty = 30009;
static const PROPERTYID isEnabledProperty = 30010;
static const PROPERTYID helpTextProperty = 30013;
static const PROPERTYID nativeWindowHandleProperty = 30020;
static const PROPERTYID isOffscreenProperty = 30022;

template<typename T>
T getAutomationProc(const char* name) {
	static HMODULE module = ::LoadLibrary(_T("uiautomationcore.dll"));
	T function = nullptr;
	auto address = module ? ::GetProcAddress(module, name) : nullptr;
	static_assert(sizeof(function) == sizeof(address), "Function pointer size mismatch");
	std::memcpy(&function, &address, sizeof(function));
	return function;
}

void setStringVariant(VARIANT* value, const tstring& text) {
	value->vt = VT_BSTR;
	value->bstrVal = ::SysAllocString(text.c_str());
}

void setBoolVariant(VARIANT* value, bool state) {
	value->vt = VT_BOOL;
	value->boolVal = state ? VARIANT_TRUE : VARIANT_FALSE;
}

}

class AccessibilityProvider : public IRawElementProviderSimple {
public:
	explicit AccessibilityProvider(Widget* widget_) : refs(1), widget(widget_) { }
	virtual ~AccessibilityProvider() { }

	void detach() {
		widget = nullptr;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** object) override {
		if(!object) {
			return E_POINTER;
		}
		if(::IsEqualIID(iid, IID_IUnknown) ||
			::IsEqualIID(iid, IID_IRawElementProviderSimple)) {
			*object = static_cast<IRawElementProviderSimple*>(this);
			AddRef();
			return S_OK;
		}
		*object = nullptr;
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef() override {
		return ++refs;
	}

	ULONG STDMETHODCALLTYPE Release() override {
		auto value = --refs;
		if(!value) {
			delete this;
		}
		return value;
	}

	HRESULT STDMETHODCALLTYPE get_ProviderOptions(ProviderOptions* value) override {
		if(!value) {
			return E_POINTER;
		}
		*value = ProviderOptions_ServerSideProvider;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID, IUnknown** value) override {
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID property, VARIANT* value) override {
		if(!value) {
			return E_POINTER;
		}
		::VariantInit(value);
		if(!widget || !widget->handle()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}

		switch(property) {
		case nameProperty:
			setStringVariant(value, widget->getAccessibleName());
			break;
		case controlTypeProperty:
			value->vt = VT_I4;
			value->lVal = widget->getAccessibleControlType();
			break;
		case hasKeyboardFocusProperty:
			setBoolVariant(value, ::GetFocus() == widget->handle());
			break;
		case isKeyboardFocusableProperty:
			setBoolVariant(value, widget->getAccessibleKeyboardFocusable());
			break;
		case isEnabledProperty:
			setBoolVariant(value, widget->getEnabled());
			break;
		case helpTextProperty:
			setStringVariant(value, widget->getAccessibleHelpText());
			break;
		case nativeWindowHandleProperty:
			value->vt = VT_I4;
			value->lVal = static_cast<LONG>(reinterpret_cast<LONG_PTR>(widget->handle()));
			break;
		case isOffscreenProperty:
			setBoolVariant(value, !::IsWindowVisible(widget->handle()));
			break;
		default:
			break;
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_HostRawElementProvider(
		IRawElementProviderSimple** value) override
	{
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		if(!widget || !widget->handle()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		typedef HRESULT (WINAPI *Function)(HWND, IRawElementProviderSimple**);
		auto function = getAutomationProc<Function>("UiaHostProviderFromHwnd");
		return function ? function(widget->handle(), value) : E_NOTIMPL;
	}

private:
	std::atomic<ULONG> refs;
	Widget* widget;
};

namespace util { namespace win32 {

AccessibilityProvider* createAccessibilityProvider(Widget* widget) {
	return new AccessibilityProvider(widget);
}

void detachAccessibilityProvider(AccessibilityProvider* provider) {
	if(provider) {
		provider->detach();
	}
}

void releaseAccessibilityProvider(AccessibilityProvider* provider) {
	if(provider) {
		provider->Release();
	}
}

LRESULT returnAccessibilityProvider(AccessibilityProvider* provider,
	HWND window, WPARAM wParam, LPARAM lParam)
{
	typedef LRESULT (WINAPI *Function)(HWND, WPARAM, LPARAM, IRawElementProviderSimple*);
	auto function = getAutomationProc<Function>("UiaReturnRawElementProvider");
	return function ? function(window, wParam, lParam, provider) : 0;
}

void raiseAccessibilityEvent(AccessibilityProvider* provider, long eventId) {
	if(!provider) {
		return;
	}
	typedef HRESULT (WINAPI *Function)(IRawElementProviderSimple*, EVENTID);
	auto function = getAutomationProc<Function>("UiaRaiseAutomationEvent");
	if(function) {
		function(provider, eventId);
	}
}

} }

}
