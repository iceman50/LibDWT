# Changelog

All notable changes to LibDWT are documented in this file.

## Unreleased

### Added

- Added checkbox support to `dwt::Tree`.
- Added `Tree::Seed::checkBoxes` for enabling checkboxes when creating a tree.
- Added `Tree::setCheckBoxes()`, with checkbox state available through
  `Tree::getChecked()` and `Tree::setChecked()`.
- Added a tree checkbox demonstration to `MultiControlExample`.
- Added Per-Monitor V2 DPI manifests and runtime DPI-awareness negotiation.
- Added per-widget DPI queries, scaling helpers, DPI-aware system metrics,
  window-rectangle adjustment, and `WM_DPICHANGED` callbacks.
- Added pre-layout DPI resource callbacks, child
  `WM_DPICHANGED_AFTERPARENT` handling, automatic assigned-font recreation,
  resized-copy helpers for icons and image lists, and window-aware Grid,
  splitter, and scroll metrics.
- Added an opt-in UI Automation provider foundation for custom HWND controls.
- Added UI Automation fragment navigation for custom HWND controls, including
  native child-provider bridging, structure-change notifications, Splitter
  `RangeValue`, and ScrolledContainer `Scroll` patterns.
- Added logical UI Automation item providers for `TableTree`, `VirtualTree`,
  and `TabView`, with selection, selection-item, expand/collapse, invoke,
  focus, bounds, hit-testing, and item event support.
- Added scoped thread DPI-awareness contexts and DPI-aware system-parameter
  queries.
- Added typed theme, system-color, and system-settings change callbacks,
  including high-contrast and client-area-animation state.
- Added automatic relayout and repaint handling for theme, system-color, and
  system-settings changes.
- Added keyboard scrolling and accessibility focus metadata to
  `ScrolledContainer`.
- Added headless framework tests for DPI helpers, system settings, and logical
  UI Automation item-provider contracts, integrated into MSVC and MinGW builds.
- Added automatic DPI image-list recreation for Button, Table, Tree, ToolBar,
  and TabView, including TabView owner-draw metrics and close icons.
- Added modern `IFileOpenDialog`/`IFileSaveDialog` backends for load, save, and
  folder dialogs, including custom places, client GUIDs, and `FOS_*` options.
- Added a `TaskDialog` wrapper with custom and common buttons, command links,
  radio buttons, verification text, progress modes, icons, and callbacks.
- Added unified Windows 8+ pointer events to the mouse aspect.
- Added command-link notes, elevation shields, image lists, split-button
  configuration, and dropdown events to `Button`.
- Added marquee, normal/error/paused state, and color APIs to `ProgressBar`.
- Added Tree extended styles, multiselect enumeration, double buffering, and
  item-changing notifications.
- Added ListView view/tile, empty-markup, group, item-index, insertion-mark, and
  activation/drag/change APIs to `Table`.
- Added taskbar progress state and value APIs.
- Added notification icon version 4, focus, icon rectangle, keyboard selection,
  and popup lifecycle support.
- Added tooltip titles, margins, colors, themes, links, pop, and update APIs.
- Added nullable values, ranges, ideal sizing, month-calendar styles, and picker
  information to `DateTime`.
- Expanded `MultiControlExample` to demonstrate task dialogs, command-link
  buttons, modern file dialogs, progress states and marquee mode, nullable date
  values, tooltip styling, pointer and DPI events, accessibility metadata, and
  richer Table, Tree, and notification callbacks.

### Changed

- Windows 7 is now the minimum supported Windows version.
- Set `WINVER` and `_WIN32_WINNT` to `0x0601`.
- Set `_WIN32_IE` to `0x0A00`.
- Updated the MSVC projects and MinGW makefile to use the new Windows target
  definitions.
- Set generated executable subsystem versions to Windows 7 (`6.01`).
- Shared executable manifests now declare Windows 7 through Windows 11
  compatibility, Per-Monitor V2 DPI awareness, common-controls v6, and long
  path awareness.
- `ProgressBar::getStep()` no longer mutates the native control while querying
  the configured step.
- Existing file and folder dialog APIs now use the modern shell dialog
  implementation without changing their path-based result types.

### Removed

- Removed Windows XP and Windows Vista support.
- Removed obsolete XP/Vista runtime checks and compatibility code.
