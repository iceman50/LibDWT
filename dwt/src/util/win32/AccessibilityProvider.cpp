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

#include <algorithm>
#include <atomic>
#include <cstring>

namespace dwt {

namespace {

static const PROPERTYID controlTypeProperty = 30003;
static const PROPERTYID nameProperty = 30005;
static const PROPERTYID hasKeyboardFocusProperty = 30008;
static const PROPERTYID isKeyboardFocusableProperty = 30009;
static const PROPERTYID isEnabledProperty = 30010;
static const PROPERTYID helpTextProperty = 30013;
static const PROPERTYID isControlElementProperty = 30016;
static const PROPERTYID isContentElementProperty = 30017;
static const PROPERTYID nativeWindowHandleProperty = 30020;
static const PROPERTYID isOffscreenProperty = 30022;

static const PATTERNID rangeValuePattern = 10003;
static const PATTERNID scrollPattern = 10004;
static const double noScroll = -1.0;

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

HRESULT hostProvider(HWND window, IRawElementProviderSimple** value) {
	if(!value) {
		return E_POINTER;
	}
	*value = nullptr;
	typedef HRESULT (WINAPI *Function)(HWND, IRawElementProviderSimple**);
	auto function = getAutomationProc<Function>("UiaHostProviderFromHwnd");
	return function ? function(window, value) : E_NOTIMPL;
}

HRESULT hostFragment(HWND window, IRawElementProviderFragment** value) {
	if(!value) {
		return E_POINTER;
	}
	*value = nullptr;
	IRawElementProviderSimple* simple = nullptr;
	auto result = hostProvider(window, &simple);
	if(SUCCEEDED(result) && simple) {
		result = simple->QueryInterface(IID_IRawElementProviderFragment,
			reinterpret_cast<void**>(value));
		simple->Release();
	}
	return result;
}

}

class AccessibilityProvider :
	public IRawElementProviderSimple,
	public IRawElementProviderFragment,
	public IRawElementProviderFragmentRoot,
	public IRangeValueProvider,
	public IScrollProvider
{
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
		} else if(::IsEqualIID(iid, IID_IRawElementProviderFragment)) {
			*object = static_cast<IRawElementProviderFragment*>(this);
		} else if(::IsEqualIID(iid, IID_IRawElementProviderFragmentRoot) &&
			isFragmentRoot()) {
			*object = static_cast<IRawElementProviderFragmentRoot*>(this);
		} else if(::IsEqualIID(iid, IID_IRangeValueProvider) &&
			widget && widget->getAccessibleRangeValue()) {
			*object = static_cast<IRangeValueProvider*>(this);
		} else if(::IsEqualIID(iid, IID_IScrollProvider) &&
			widget && widget->getAccessibleScroll()) {
			*object = static_cast<IScrollProvider*>(this);
		} else {
			*object = nullptr;
			return E_NOINTERFACE;
		}
		AddRef();
		return S_OK;
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

	HRESULT STDMETHODCALLTYPE GetPatternProvider(PATTERNID pattern,
		IUnknown** value) override
	{
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		if(!available()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		if(pattern == rangeValuePattern && widget->getAccessibleRangeValue()) {
			*value = static_cast<IRangeValueProvider*>(this);
		} else if(pattern == scrollPattern && widget->getAccessibleScroll()) {
			*value = static_cast<IScrollProvider*>(this);
		}
		if(*value) {
			AddRef();
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetPropertyValue(PROPERTYID property,
		VARIANT* value) override
	{
		if(!value) {
			return E_POINTER;
		}
		::VariantInit(value);
		if(!available()) {
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
		case isControlElementProperty:
		case isContentElementProperty:
			setBoolVariant(value, true);
			break;
		case nativeWindowHandleProperty:
			value->vt = VT_I4;
			value->lVal = static_cast<LONG>(
				reinterpret_cast<LONG_PTR>(widget->handle()));
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
		if(!available()) {
			if(value) {
				*value = nullptr;
			}
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		return hostProvider(widget->handle(), value);
	}

	HRESULT STDMETHODCALLTYPE Navigate(NavigateDirection direction,
		IRawElementProviderFragment** value) override
	{
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		if(!available()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}

		switch(direction) {
		case NavigateDirection_Parent:
			if(isFragmentRoot()) {
				return S_OK;
			}
			return fragmentForWindow(::GetParent(widget->handle()), value);

		case NavigateDirection_FirstChild:
			return childFragment(false, value);

		case NavigateDirection_LastChild:
			return childFragment(true, value);

		case NavigateDirection_NextSibling:
			return siblingFragment(false, value);

		case NavigateDirection_PreviousSibling:
			return siblingFragment(true, value);
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetRuntimeId(SAFEARRAY** value) override {
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		if(!available()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		auto array = ::SafeArrayCreateVector(VT_I4, 0, 2);
		if(!array) {
			return E_OUTOFMEMORY;
		}
		LONG index = 0;
		int part = UiaAppendRuntimeId;
		::SafeArrayPutElement(array, &index, &part);
		index = 1;
		part = static_cast<int>(reinterpret_cast<INT_PTR>(widget->handle()));
		::SafeArrayPutElement(array, &index, &part);
		*value = array;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_BoundingRectangle(UiaRect* value) override {
		if(!value) {
			return E_POINTER;
		}
		if(!available()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		RECT rect = { 0 };
		::GetWindowRect(widget->handle(), &rect);
		value->left = rect.left;
		value->top = rect.top;
		value->width = rect.right - rect.left;
		value->height = rect.bottom - rect.top;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetEmbeddedFragmentRoots(SAFEARRAY** value) override {
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetFocus() override {
		if(!available()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		::SetFocus(widget->handle());
		return ::GetFocus() == widget->handle() ? S_OK : E_FAIL;
	}

	HRESULT STDMETHODCALLTYPE get_FragmentRoot(
		IRawElementProviderFragmentRoot** value) override
	{
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		auto root = fragmentRoot();
		return root ? root->QueryInterface(IID_IRawElementProviderFragmentRoot,
			reinterpret_cast<void**>(value)) : UIA_E_ELEMENTNOTAVAILABLE;
	}

	HRESULT STDMETHODCALLTYPE ElementProviderFromPoint(double x, double y,
		IRawElementProviderFragment** value) override
	{
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		if(!available()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		POINT point = { static_cast<LONG>(x), static_cast<LONG>(y) };
		auto target = ::WindowFromPoint(point);
		if(target && (target == widget->handle() ||
			::IsChild(widget->handle(), target))) {
			while(target && target != widget->handle()) {
				auto result = fragmentForWindow(target, value);
				if(SUCCEEDED(result) && *value) {
					return S_OK;
				}
				target = ::GetParent(target);
			}
		}
		*value = static_cast<IRawElementProviderFragment*>(this);
		AddRef();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE GetFocus(
		IRawElementProviderFragment** value) override
	{
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		if(!available()) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		auto focus = ::GetFocus();
		if(focus && (focus == widget->handle() ||
			::IsChild(widget->handle(), focus))) {
			return fragmentForWindow(focus, value);
		}
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetValue(double value) override {
		auto range = available() ? widget->getAccessibleRangeValue() : nullptr;
		if(!range) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		if(range->readOnly || !range->setValue) {
			return UIA_E_NOTSUPPORTED;
		}
		if(value < range->minimum || value > range->maximum) {
			return E_INVALIDARG;
		}
		range->setValue(value);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_Value(double* value) override {
		auto range = available() ? widget->getAccessibleRangeValue() : nullptr;
		if(!value) {
			return E_POINTER;
		}
		if(!range || !range->getValue) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		*value = range->getValue();
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_IsReadOnly(BOOL* value) override {
		auto range = available() ? widget->getAccessibleRangeValue() : nullptr;
		if(!value) {
			return E_POINTER;
		}
		if(!range) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		*value = range->readOnly ? TRUE : FALSE;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_Maximum(double* value) override {
		return rangeConstant(value, &accessibility::RangeValueProvider::maximum);
	}

	HRESULT STDMETHODCALLTYPE get_Minimum(double* value) override {
		return rangeConstant(value, &accessibility::RangeValueProvider::minimum);
	}

	HRESULT STDMETHODCALLTYPE get_LargeChange(double* value) override {
		return rangeConstant(value, &accessibility::RangeValueProvider::largeChange);
	}

	HRESULT STDMETHODCALLTYPE get_SmallChange(double* value) override {
		return rangeConstant(value, &accessibility::RangeValueProvider::smallChange);
	}

	HRESULT STDMETHODCALLTYPE Scroll(::ScrollAmount horizontal,
		::ScrollAmount vertical) override
	{
		auto scroll = available() ? widget->getAccessibleScroll() : nullptr;
		if(!scroll || !scroll->scroll) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		scroll->scroll(
			static_cast<accessibility::ScrollAmount>(horizontal),
			static_cast<accessibility::ScrollAmount>(vertical));
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE SetScrollPercent(double horizontal,
		double vertical) override
	{
		auto scroll = available() ? widget->getAccessibleScroll() : nullptr;
		if(!scroll || !scroll->setPercent) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		if((horizontal != noScroll && (horizontal < 0 || horizontal > 100)) ||
			(vertical != noScroll && (vertical < 0 || vertical > 100))) {
			return E_INVALIDARG;
		}
		scroll->setPercent(horizontal, vertical);
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE get_HorizontalScrollPercent(double* value) override {
		return scrollValue(value, &accessibility::ScrollProvider::horizontalPercent,
			noScroll);
	}

	HRESULT STDMETHODCALLTYPE get_VerticalScrollPercent(double* value) override {
		return scrollValue(value, &accessibility::ScrollProvider::verticalPercent,
			noScroll);
	}

	HRESULT STDMETHODCALLTYPE get_HorizontalViewSize(double* value) override {
		return scrollValue(value, &accessibility::ScrollProvider::horizontalViewSize,
			100);
	}

	HRESULT STDMETHODCALLTYPE get_VerticalViewSize(double* value) override {
		return scrollValue(value, &accessibility::ScrollProvider::verticalViewSize,
			100);
	}

	HRESULT STDMETHODCALLTYPE get_HorizontallyScrollable(BOOL* value) override {
		return scrollBool(value,
			&accessibility::ScrollProvider::horizontallyScrollable);
	}

	HRESULT STDMETHODCALLTYPE get_VerticallyScrollable(BOOL* value) override {
		return scrollBool(value,
			&accessibility::ScrollProvider::verticallyScrollable);
	}

private:
	bool available() const {
		return widget && widget->handle() && ::IsWindow(widget->handle());
	}

	bool isFragmentRoot() const {
		if(!available()) {
			return false;
		}
		for(auto parent = widget->getParent(); parent; parent = parent->getParent()) {
			if(parent->accessibilityProvider) {
				return false;
			}
		}
		return true;
	}

	AccessibilityProvider* fragmentRoot() {
		if(!available()) {
			return nullptr;
		}
		auto root = this;
		for(auto parent = widget->getParent(); parent; parent = parent->getParent()) {
			if(parent->accessibilityProvider) {
				root = parent->accessibilityProvider;
			}
		}
		return root;
	}

	static HRESULT fragmentForWindow(HWND window,
		IRawElementProviderFragment** value)
	{
		if(!value) {
			return E_POINTER;
		}
		*value = nullptr;
		if(!window) {
			return S_OK;
		}
		auto candidate = hwnd_cast<Widget*>(window);
		if(candidate && candidate->accessibilityProvider) {
			return candidate->accessibilityProvider->QueryInterface(
				IID_IRawElementProviderFragment, reinterpret_cast<void**>(value));
		}
		auto result = hostFragment(window, value);
		return result == E_NOINTERFACE ? S_OK : result;
	}

	HRESULT childFragment(bool reverse,
		IRawElementProviderFragment** value) const
	{
		HWND child = nullptr;
		while((child = ::FindWindowEx(widget->handle(), child, nullptr, nullptr))) {
			IRawElementProviderFragment* candidate = nullptr;
			auto result = fragmentForWindow(child, &candidate);
			if(FAILED(result)) {
				return result;
			}
			if(candidate) {
				if(!reverse) {
					*value = candidate;
					return S_OK;
				}
				if(*value) {
					(*value)->Release();
				}
				*value = candidate;
			}
		}
		return S_OK;
	}

	HRESULT siblingFragment(bool previous,
		IRawElementProviderFragment** value) const
	{
		auto parent = ::GetParent(widget->handle());
		if(!parent) {
			return S_OK;
		}
		HWND child = nullptr;
		bool found = false;
		IRawElementProviderFragment* prior = nullptr;
		while((child = ::FindWindowEx(parent, child, nullptr, nullptr))) {
			IRawElementProviderFragment* candidate = nullptr;
			auto result = fragmentForWindow(child, &candidate);
			if(FAILED(result)) {
				if(prior) {
					prior->Release();
				}
				return result;
			}
			if(!candidate) {
				continue;
			}
			if(child == widget->handle()) {
				candidate->Release();
				if(previous) {
					*value = prior;
					return S_OK;
				}
				if(prior) {
					prior->Release();
				}
				prior = nullptr;
				found = true;
				continue;
			}
			if(found && !previous) {
				*value = candidate;
				return S_OK;
			}
			if(!found) {
				if(prior) {
					prior->Release();
				}
				prior = candidate;
			} else {
				candidate->Release();
			}
		}
		if(prior) {
			prior->Release();
		}
		return S_OK;
	}

	HRESULT rangeConstant(double* value,
		double accessibility::RangeValueProvider::* member) const
	{
		if(!value) {
			return E_POINTER;
		}
		auto range = available() ? widget->getAccessibleRangeValue() : nullptr;
		if(!range) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		*value = range->*member;
		return S_OK;
	}

	HRESULT scrollValue(double* value,
		std::function<double ()> accessibility::ScrollProvider::* member,
		double fallback) const
	{
		if(!value) {
			return E_POINTER;
		}
		auto scroll = available() ? widget->getAccessibleScroll() : nullptr;
		if(!scroll) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		auto& callback = scroll->*member;
		*value = callback ? callback() : fallback;
		return S_OK;
	}

	HRESULT scrollBool(BOOL* value,
		std::function<bool ()> accessibility::ScrollProvider::* member) const
	{
		if(!value) {
			return E_POINTER;
		}
		auto scroll = available() ? widget->getAccessibleScroll() : nullptr;
		if(!scroll) {
			return UIA_E_ELEMENTNOTAVAILABLE;
		}
		auto& callback = scroll->*member;
		*value = callback && callback() ? TRUE : FALSE;
		return S_OK;
	}

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
	typedef LRESULT (WINAPI *Function)(HWND, WPARAM, LPARAM,
		IRawElementProviderSimple*);
	auto function = getAutomationProc<Function>("UiaReturnRawElementProvider");
	return function ? function(window, wParam, lParam,
		static_cast<IRawElementProviderSimple*>(provider)) : 0;
}

void raiseAccessibilityEvent(AccessibilityProvider* provider, long eventId) {
	if(!provider) {
		return;
	}
	typedef HRESULT (WINAPI *Function)(IRawElementProviderSimple*, EVENTID);
	auto function = getAutomationProc<Function>("UiaRaiseAutomationEvent");
	if(function) {
		function(static_cast<IRawElementProviderSimple*>(provider), eventId);
	}
}

void raiseAccessibilityStructureChanged(AccessibilityProvider* provider) {
	if(!provider) {
		return;
	}
	typedef HRESULT (WINAPI *Function)(IRawElementProviderSimple*,
		StructureChangeType, int*, int);
	auto function = getAutomationProc<Function>("UiaRaiseStructureChangedEvent");
	if(function) {
		function(static_cast<IRawElementProviderSimple*>(provider),
			StructureChangeType_ChildrenInvalidated, nullptr, 0);
	}
}

} }

}
