# Changelog

All notable changes to LibDWT are documented in this file.

## Unreleased

### Added

- Added a visible `FrameworkValidation` application for manual DPI, UI
  Automation, high-contrast, text-scaling, and scrolling validation, with a
  bounded event log and non-interactive lifecycle self-test for build targets.
- Added `Application::processMessages()` for keeping the UI responsive during
  lengthy work on the GUI thread by processing pending messages and callbacks.
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
- Added public shell-item result APIs, shell-item places and initial folders,
  file-dialog event callbacks, and `IFileDialogCustomize` hooks for modern load,
  save, and folder dialogs.
- Added a `TaskDialog` wrapper with custom and common buttons, command links,
  radio buttons, verification text, progress modes, icons, and callbacks.
- Added unified Windows 8+ pointer events to the mouse aspect.
- Added pointer history, cancellation, wheel callbacks, touch/pen detail
  readback, capture helpers, typed `WM_TOUCH`, typed `WM_GESTURE`, and gesture
  configuration helpers to the mouse aspect.
- Added command-link notes, elevation shields, image lists, split-button
  configuration, and dropdown events to `Button`.
- Added marquee, normal/error/paused state, and color APIs to `ProgressBar`.
- Added Tree extended styles, multiselect enumeration, double buffering, and
  item-changing notifications.
- Added ListView view/tile, empty-markup, group, item-index, insertion-mark, and
  activation/drag/change APIs to `Table`.
- Completed the audited Table/ListView P1 surface with tile readback, footer
  information and link events, group metrics/state/focus, work areas, hot-item
  and hover settings, geometry, background images, image-list getters, and
  label-edit callbacks.
- Completed the audited native Tree P1 surface with partial, exclusion, and
  dimmed checkbox states, multiselect-aware counts, info-tip and label-edit
  callbacks, drag and asynchronous-draw events, drag images, insertion marks,
  colors, indentation, scroll settings, sorting, item-part rectangles, and
  accessibility-ID mapping.
- Added VirtualTree translation for the new item states, geometry, drag images,
  insertion marks, sorting, accessibility IDs, and notification payloads.
- Added typed Tree selected-state helpers and model-backed VirtualTree
  multiselect storage, enumeration, selected counts, hidden-node preservation,
  and framework behavior tests.
- Added taskbar progress state and value APIs.
- Added notification icon version 4, focus, icon rectangle, keyboard selection,
  and popup lifecycle support.
- Added tooltip titles, margins, colors, themes, links, pop, and update APIs.
- Added nullable values, ranges, ideal sizing, month-calendar styles, and picker
  information to `DateTime`.
- Added `MonthCalendar`, including value and range APIs, multiselect ranges,
  colors, sizing helpers, view state, grid information, and selection/view
  events.
- Added `DateTime` month-calendar handle/font wrappers and format-query/format
  callbacks.
- Added 32-bit range, selection range, line/page size, thumb length, geometry,
  tick, tooltip, buddy, Unicode-format, style, and movement-validation APIs to
  `Slider`.
- Added audited Header coverage, including item text/data wrappers, checkbox
  and checked-state helpers, split/drop-down affordances, sort arrows, image
  lists, drag images, geometry, order/focus/filter/Unicode APIs, and typed
  header notifications.
- Added the `CustomDraw` aspect to `Button`, `Header`, `ProgressBar`, and
  `Slider`; custom-draw callbacks now ignore null notification payloads.
- Added a public `Tree::onCustomDraw()` hook for native tree-view custom draw
  customization before LibDWT's built-in column renderer runs.
- Expanded `CustomDrawExample` to demonstrate Button, Header, Rebar, Slider,
  Table/ListView, TableTree, ToolBar, ToolTip, Tree, VirtualTree, and
  ProgressBar custom-draw paths with a shared Segoe UI font, visible
  slider/progress layout, embedded Table/Tree header custom draw, visual styles
  enabled, dark native list/tree backgrounds, fully owner-painted trackbar
  surfaces, guarded progress-bar repaint fallback, slider/progress-only
  invalidation while dragging, working toolbar commands, and native tree expand
  glyphs.
- Added a working MDI implementation for `MDIFrame`, `MDIParent`, and
  `MDIChild`, including native MDI dispatching, system accelerator translation,
  menu merging support, active child management, cascade, tiling, icon
  arrangement, next/previous activation, maximize, restore, close, close-all,
  and minimize-all operations.
- Added configurable `MDIParent` background painting with solid colors, bitmap
  images, and icon images, using DWT RAII resource wrappers.
- Added an `MDIExample` project demonstrating DWT MDI children, native MDI
  window commands, menu commands through `dwt::Menu`, rich text formatting,
  standard controls, owner-painted content, a custom bottom tab strip, and
  configurable MDI background images and colors.
- Added a generic mouse-wheel callback to the mouse aspect.
- Expanded `MultiControlExample` to demonstrate task dialogs, command-link
  buttons, modern file dialogs, progress states and marquee mode, nullable date
  values, slider selection and movement validation, tooltip styling, pointer and
  DPI events, accessibility metadata, and richer Table, Tree, MonthCalendar, and
  notification callbacks, including extended Tree and VirtualTree checkbox
  states.

### Changed

- Tree and VirtualTree now initialize and propagate their default or assigned
  font to both the native tree body and the embedded column header.
- `TabView::Seed::closeable` can now disable tab-strip close affordances and
  close gestures; `FrameworkValidation` uses it and relies on normal nested
  layout propagation to remain stable while resizing.
- MSVC Release static-library objects now use self-contained debug records,
  avoiding fragile compiler-PDB dependencies when linking applications.
- MSVC builds compile DWT once per configuration before building dependent
  examples and tests.
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
- Menu command dispatch now handles regular `WM_COMMAND` menu items without
  colliding with accelerator/control command IDs.
- MDI client background repainting now handles both erase and paint passes so
  custom backgrounds redraw correctly when child frames move over them.
- `RichTextBox::findText()` now calls the registered not-found callback when a
  search wraps and still fails.
- `RichTextBox::textUnderCursor()` now matches upstream behavior when the
  cursor is on whitespace or at the end of a line.
- `TabView` now handles mouse-wheel tab switching through the mouse aspect.
- `Button::getPreferredSize()` now checks for `BCM_GETIDEALSIZE` failure.

### Removed

- Removed Windows XP and Windows Vista support.
- Removed obsolete XP/Vista runtime checks and compatibility code.
