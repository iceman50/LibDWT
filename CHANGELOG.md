# Changelog

All notable changes to LibDWT are documented in this file.

## Unreleased

### Added

- Added checkbox support to `dwt::Tree`.
- Added `Tree::Seed::checkBoxes` for enabling checkboxes when creating a tree.
- Added `Tree::setCheckBoxes()`, with checkbox state available through
  `Tree::getChecked()` and `Tree::setChecked()`.
- Added a tree checkbox demonstration to `MultiControlExample`.

### Changed

- Windows 7 is now the minimum supported Windows version.
- Set `WINVER` and `_WIN32_WINNT` to `0x0601`.
- Set `_WIN32_IE` to `0x0A00`.
- Updated the MSVC projects and MinGW makefile to use the new Windows target
  definitions.
- Set generated executable subsystem versions to Windows 7 (`6.01`).

### Removed

- Removed Windows XP and Windows Vista support.
- Removed obsolete XP/Vista runtime checks and compatibility code.
