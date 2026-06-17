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

#ifndef DWT_UTIL_WIN32_FILEDIALOG_H
#define DWT_UTIL_WIN32_FILEDIALOG_H

#include "../../WindowsHeaders.h"
#include "../../tstring.h"

#include <functional>
#include <utility>
#include <vector>

namespace dwt { namespace util { namespace win32 {

template<typename T>
class ComPtr {
public:
	ComPtr() : value(nullptr) { }

	explicit ComPtr(T* ptr, bool addRef = true) : value(ptr) {
		if(value && addRef) {
			value->AddRef();
		}
	}

	ComPtr(const ComPtr& rhs) : value(rhs.value) {
		if(value) {
			value->AddRef();
		}
	}

	ComPtr(ComPtr&& rhs) noexcept : value(rhs.value) {
		rhs.value = nullptr;
	}

	~ComPtr() {
		reset();
	}

	ComPtr& operator=(const ComPtr& rhs) {
		if(this != &rhs) {
			reset(rhs.value);
		}
		return *this;
	}

	ComPtr& operator=(ComPtr&& rhs) noexcept {
		if(this != &rhs) {
			reset();
			value = rhs.value;
			rhs.value = nullptr;
		}
		return *this;
	}

	T* get() const { return value; }
	T** put() {
		reset();
		return &value;
	}
	T* operator->() const { return value; }
	explicit operator bool() const { return value != nullptr; }

	void reset(T* ptr = nullptr, bool addRef = true) {
		if(ptr && addRef) {
			ptr->AddRef();
		}
		if(value) {
			value->Release();
		}
		value = ptr;
	}

	T* detach() {
		auto result = value;
		value = nullptr;
		return result;
	}

private:
	T* value;
};

typedef ComPtr<IShellItem> ShellItemPtr;

struct FileDialogResult {
	ShellItemPtr item;
	tstring path;

	bool hasPath() const { return !path.empty(); }
	bool getDisplayName(SIGDN sigdn, tstring& value) const;
};

struct FileDialogEvents {
	typedef std::function<HRESULT (IFileDialog*)> FileOkCallback;
	typedef std::function<HRESULT (IFileDialog*, IShellItem*)> FolderChangingCallback;
	typedef std::function<void (IFileDialog*)> DialogCallback;
	typedef std::function<void (IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE&)> ShareViolationCallback;
	typedef std::function<void (IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE&)> OverwriteCallback;

	FileOkCallback fileOk;
	FolderChangingCallback folderChanging;
	DialogCallback folderChanged;
	DialogCallback selectionChanged;
	ShareViolationCallback shareViolation;
	DialogCallback typeChanged;
	OverwriteCallback overwrite;

	bool empty() const {
		return !fileOk && !folderChanging && !folderChanged && !selectionChanged &&
			!shareViolation && !typeChanged && !overwrite;
	}
};

typedef std::function<void (IFileDialogCustomize&)> FileDialogCustomizeCallback;

struct FileDialogOptions {
	HWND owner = nullptr;
	bool save = false;
	bool pickFolders = false;
	bool allowMultiple = false;
	bool forceFilesystem = true;
	DWORD legacyFlags = 0;
	FILEOPENDIALOGOPTIONS options = 0;
	unsigned activeFilter = 0;
	tstring title;
	tstring initialDirectory;
	tstring defaultExtension;
	tstring initialFileName;
	std::vector<std::pair<tstring, tstring>> filters;
	std::vector<std::pair<tstring, FDAP>> places;
	std::vector<std::pair<ShellItemPtr, FDAP>> shellPlaces;
	PCIDLIST_ABSOLUTE initialItem = nullptr;
	ShellItemPtr initialFolder;
	const GUID* clientGuid = nullptr;
	const FileDialogEvents* events = nullptr;
	FileDialogCustomizeCallback customize;
};

bool showFileDialog(const FileDialogOptions& options, std::vector<tstring>& paths);
bool showFileDialogItems(const FileDialogOptions& options, std::vector<FileDialogResult>& results);

} } }

#endif
