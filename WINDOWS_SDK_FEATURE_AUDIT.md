# Windows SDK Feature Audit

Audit date: 2026-06-14
Last updated: 2026-06-18

This audit compares LibDWT's public widget APIs with the desktop Win32 API
surface available from Windows 7 through Windows 11. The local Windows SDK used
for the comparison is 10.0.26100.0.

"Missing" means that LibDWT has no typed, ownership-safe, event-integrated API
for a capability. Most control messages remain technically reachable through
`Widget::sendMessage` or `onRaw`, but requiring raw Win32 calls is still treated
as a wrapper gap.

The list covers material application-facing capabilities. It does not enumerate
every redundant getter, compatibility alias, obsolete message, or internal
common-control implementation detail.

## Status Legend

- **Added**: the audited capability has a public typed API and is implemented.
- **Partial**: useful coverage exists, but items named in the remaining column
  still require implementation.
- **Remaining**: no material typed wrapper has been added yet.
- **Existing**: LibDWT already supported the capability before this audit.

Items without an explicit status marker remain unimplemented.

## Implementation Status

### P0 Foundations

| Area | Status | Added | Remaining |
| --- | --- | --- | --- |
| Per-monitor DPI | **Partial** | Per-Monitor V2 manifests, runtime and scoped thread awareness, per-widget DPI queries and scaling, DPI-aware metrics/system parameters/window adjustment, suggested bounds, typed DPI events, child after-parent handling, pre-layout resource callbacks, automatic font recreation, icon/image-list resize helpers, automatic image-list policies for Button/Table/Tree/ToolBar/TabView, and headless DPI contract tests | Remaining owner-drawn caches and direct bitmap/icon policies, plus live multi-monitor transition tests |
| UI Automation | **Partial** | `IRawElementProviderSimple` plus fragment/root and logical-item navigation, native child-provider bridging, properties/events/structure notifications, `Splitter` RangeValue, `ScrolledContainer` Scroll, TableTree/VirtualTree/TabView selection, expansion, invocation, focus, bounds and hit-testing, and logical-item contract tests | Additional semantic patterns, live UIA client validation, text-scaling checks, and visual high-contrast audits |
| Modern file dialogs | **Partial** | `IFileOpenDialog`/`IFileSaveDialog` backends for load, save, and folder selection; filters, filesystem paths, long paths, custom places, shell-item places/results, multiple shell-item results, client GUIDs, events, customization hooks, options, and `FOS_PICKFOLDERS` | Higher-level typed wrappers around common custom-control patterns and live validation of library and virtual-folder selections |
| Task dialogs | **Added** | `TaskDialogIndirect` wrapper with common/custom buttons, command links, radio buttons, verification, expanded/footer text, icons, progress modes, callbacks, and callback-based live updates | Convenience APIs for hyperlinks and typed live progress/text updates |

### P1 Work Completed So Far

| Area | Status | Added | Remaining |
| --- | --- | --- | --- |
| Pointer/input | **Added** | Typed `WM_POINTER` down, update, up, enter, leave, wheel, horizontal wheel, capture-changed, and cancellation events with pointer ID, type, position, flags, history, touch details, pen pressure/tilt/rotation, capture helpers, typed `WM_TOUCH`, typed `WM_GESTURE`, and gesture configuration | No remaining item from the original P1 audit |
| Button | **Partial** | Command-link styles through seeds, note get/set, elevation shield, aligned image lists, split configuration, and `BCN_DROPDOWN` | Image-list getter, text-margin getter, explicit dropdown-state helpers, and hot/focus notifications |
| ProgressBar | **Added** | Marquee mode/speed, normal/error/paused states, colors, and non-mutating cached step retrieval | No remaining item from the original P1 audit |
| Table/ListView | **Added** | View and tile get/set, empty markup, footer information/items/rectangles/link events, complete group info/metrics/state/focus operations, `LVITEMINDEX` state/iteration/rectangles, insertion marks and colors, work areas, hot item/cursor, hover time, outline color, selected column, background image, geometry, image-list getters, and drag/activate/key/change/label-edit events | No remaining item from the original P1 audit |
| Tree/VirtualTree | **Partial** | Native Tree extended styles and checkbox states, multiselect enumeration/counting, item-change/info-tip/label-edit/async-draw/drag/key events, public TreeView custom-draw hook, drag images, insertion marks/colors, indentation, scroll/autoscroll settings, tooltips, sorting, item/part rectangles and accessibility IDs; VirtualTree translates applicable states, geometry, sorting, IDs and events, with model-backed multiselect storage/enumeration and framework behavior tests | Remaining live UI/accessibility validation for VirtualTree multiselect behavior |
| Taskbar | **Partial** | Progress state and value | Thumbnail toolbar buttons/updates, thumbnail tooltip/clipping, tab properties, AppUserModelID, and Jump Lists |
| Notification | **Partial** | `NOTIFYICON_VERSION_4`, keyboard selection, `Shell_NotifyIconGetRect`, `NIM_SETFOCUS`, popup lifecycle events, and basic Explorer recreation handling | GUID identity, real-time/quiet-time/large-icon/sound flags, balloon-show events, and stronger restoration/error handling |
| ToolTip | **Partial** | Title/icon, margins, colors, pop/update, window theme, and `TTN_LINKCLICK` | Balloon/close styles, tracking APIs, current-tool/enumeration/bubble sizing/rectangle adjustment, and per-tool flags |
| DateTime / MonthCalendar | **Partial** | Nullable values, ranges, ideal size, close-calendar, month-calendar styles, picker information, month-calendar handle/font wrappers, format callbacks, and standalone `MonthCalendar` value, range, multiselect, color, sizing, view, grid-info, and selection/view-event APIs | Specialist `MonthCalendar` calendar-ID and day-state APIs |
| Slider/Trackbar | **Added** | 32-bit range APIs, selection range, line/page size, thumb length, channel/thumb rectangles, tick values/positions/counts, tooltip control/placement, buddy getter, Unicode format, reversed/down-is-left/transparent-background/notify-before-move styles, and `TRBN_THUMBPOSCHANGING` validation | No remaining item from the original Slider audit |
| Header | **Added** | Text/data fixes, checkbox and checked-state helpers, split/drop-down affordances, sort arrows, image lists, drag images, item/drop-down/overflow rectangles, hit testing, order/focus/hot-divider/filter/Unicode wrappers, and typed click/drag/filter/drop-down/overflow notifications | No remaining item from the original Header audit |

## Priority Summary

### P0 - Framework Foundations

1. **Per-monitor DPI awareness and dynamic DPI changes - Partial**
   - **Added:** Per-Monitor V2 manifests and process DPI-awareness negotiation.
   - **Added:** Dispatch `WM_DPICHANGED` and expose old/new DPI plus the suggested window
     rectangle.
   - **Added:** Replace the process-global `LOGPIXELSX` scale cached by
     `util::dpiFactor()` with window/monitor-aware scaling.
   - **Partial:** Assigned fonts and owned image lists now recreate
     automatically for Button, Table, Tree, ToolBar, and TabView; pre-layout
     callbacks and icon/image-list resized copies support remaining custom
     caches and direct resources.
   - **Added:** Wrap `GetDpiForWindow`, `GetSystemMetricsForDpi`,
     `SystemParametersInfoForDpi`, and `AdjustWindowRectExForDpi`.
   - **Added:** Scoped thread DPI-awareness helpers.
   - **Added:** Headless DPI scaling, event, and system-parameter tests.
   - **Remaining:** Live multi-monitor transition tests and policies for
     remaining direct bitmap/icon and owner-drawn caches.
   - Earliest relevant versions: Windows 8.1 for per-monitor DPI and Windows 10
     for the complete Per-Monitor V2 API.

2. **UI Automation for custom controls - Partial**
   - Native controls receive much of their accessibility behavior from Windows,
     but LibDWT's custom `Grid`, `TableTree`, `VirtualTree`, `TabView`,
     `Splitter`, and `ScrolledContainer` need UI Automation providers.
   - **Added:** Provider infrastructure, fragment/root navigation, native child
     bridging, names, help text, control types, enabled/focus/offscreen state,
     child bounds, event raising, and structure-change notifications.
   - **Added:** `Grid`, `ScrolledContainer`, `SplitterContainer`, and `Splitter`
     participate in the fragment tree.
   - **Added:** `Splitter` exposes `RangeValue`; `ScrolledContainer` exposes
     `Scroll`.
   - **Added:** Logical item providers for `TableTree`, `VirtualTree`, and
     `TabView`, including Selection, SelectionItem, ExpandCollapse, Invoke,
     focus, bounds, hit-testing, and item events.
   - **Added:** Headless logical-item provider contract tests.
   - **Added:** Typed theme, system-color, and system-settings notifications,
     including current high-contrast and client-area-animation state.
   - **Added:** Keyboard audit and missing keyboard scrolling support for
     `ScrolledContainer`.
   - **Remaining:** Additional control-specific semantic patterns.
   - **Remaining:** Live UIA client validation, text-scaling support, and visual
     high-contrast behavior audits for custom-drawn controls.
   - Available at the Windows 7 minimum.

3. **Modern file and folder dialogs - Partial**
   - **Added:** Replace or supplement `OPENFILENAME` and `SHBrowseForFolder` with
     `IFileOpenDialog` and `IFileSaveDialog`.
   - **Added:** Filesystem path results, long paths, custom places, persisted
     client GUIDs, option flags, and folder picking through `FOS_PICKFOLDERS`.
   - **Added:** Public shell-item result APIs, shell-item places and initial
     folders, multiple shell-item results, event callbacks, and
     `IFileDialogCustomize` hooks for custom controls.
   - **Remaining:** Higher-level typed wrappers around common custom-control
     patterns and live validation of library and virtual-folder selections.
   - Available since Windows Vista, so no fallback is required with a Windows 7
     minimum.

4. **Task dialogs - Added**
   - **Added:** A `TaskDialogIndirect` wrapper alongside `MessageBox`.
   - **Added:** Command links, custom buttons, radio buttons, verification
     checkboxes, expanded/footer text, progress or marquee flags, icons,
     callbacks, and callback-HWND live updates.
   - **Remaining:** Typed hyperlink convenience and dedicated live-update
     methods; both are currently reachable through flags and the callback HWND.
   - Available at the Windows 7 minimum.

### P1 - High-Value Control and Shell Coverage

1. **Added:** Pointer, touch, gesture, richer pen data, history, and
   cancellation coverage is added.
2. **Added:** The audited ListView/Table v6 P1 surface is covered.
3. **Partial:** Native Tree P1 coverage is complete; VirtualTree has
   model-backed multiselection and framework behavior tests. Live UI and
   accessibility validation remain.
4. **Partial:** Taskbar progress is added; thumbnail-toolbar support remains.
5. **Added:** Notification icons now use `NOTIFYICON_VERSION_4`; related tray
   options remain.
6. **Partial:** Button, ProgressBar, ToolTip, DateTime, and MonthCalendar
   additions are in place; ProgressBar is complete against this audit.

## Platform Features by Windows Generation

### Windows 7 Baseline

These capabilities are already available at the new minimum target and should
not require runtime fallback paths:

- Common-controls v6 button features: command links, split buttons, notes,
  shields, and aligned image lists.
- Task dialogs and modern file dialogs.
- ListView group, tile, footer, empty-text, and item-index APIs.
- TreeView extended styles and richer notifications.
- Progress-bar marquee and normal/error/paused states.
- Enhanced tooltip, header, rebar, toolbar, date-time, and trackbar APIs.
- Windows 7 taskbar progress, overlays, tab thumbnails, and thumbnail buttons.
- Shell notification icon version 4.
- `WM_TOUCH`, `WM_GESTURE`, and UI Automation.

### Windows 8

- Unified mouse, touch, and pen input through `WM_POINTER` and pointer APIs.
- Pointer capture, history, device type, pressure/pen information, and
  primary-pointer handling.
- DWM window-state attributes such as cloaking are candidates for low-level
  window wrappers, but are less important than input support.

### Windows 8.1 and Windows 10

- Per-monitor DPI awareness, `WM_DPICHANGED`, and monitor transitions.
- Windows 10 Per-Monitor V2 behavior and APIs such as `GetDpiForWindow`,
  `GetSystemMetricsForDpi`, and `AdjustWindowRectExForDpi`.
- Touchpad and precision-pointer input can build on the pointer event layer.
- Toast notifications are a separate WinRT/Windows App SDK integration and
  should be an optional module, not part of the classic `Notification` wrapper.

### Windows 11

- Typed `DwmSetWindowAttribute` wrappers for:
  - Immersive dark title bars.
  - Window corner preference.
  - Border, caption, and caption-text colors.
  - Visible frame-border thickness.
  - System backdrop type, including Mica-style system backdrops on supported
    builds.
- Guard attributes by the documented Windows 11 build that introduced them.
- Standard framed windows with a normal maximize button receive Snap Layouts
  from Windows automatically; this is not a missing LibDWT feature.
- WinUI 3 controls and Windows App SDK title bars are outside the scope of a
  classic Win32 widget toolkit.

## Widget and Control Gaps

### Application, Widget, Control, Window, and Frame

**Priority: P0-P1**

- **Partial:** DPI-awareness configuration, per-window DPI querying,
  `WM_DPICHANGED`, child after-parent handling, and DPI-aware geometry are
  added. Fonts recreate automatically; other resources have pre-layout hooks
  and resized-copy helpers.
- **Added:** Typed pointer, touch, gesture, detailed pen data, capture helpers,
  history, and cancellation coverage is added.
- **Partial:** Typed theme, system-color, and system-settings notifications are
  added, including high-contrast and client-area-animation state. Dedicated
  text-scale values and more specific accessibility-setting classifications
  remain.
- **Partial:** UI Automation fragment and logical-item infrastructure,
  Splitter `RangeValue`, ScrolledContainer `Scroll`, and
  TableTree/VirtualTree/TabView item patterns are added; additional specialist
  patterns remain.
- **Remaining:** Typed DWM appearance attributes and composition-change events.
- **Remaining:** Optional drag/drop modernization with richer `IDataObject` formats and shell
  items.

### Button

**Priority: P1; available at the Windows 7 minimum**

- **Added:** Command-link and default-command-link styles through `Button::Seed`.
- **Added:** Command-link note get/set.
- **Partial:** Split-button configuration and `BCN_DROPDOWN`; explicit
  drop-down-state helpers remain.
- **Added:** Elevation shield.
- **Partial:** Button image-list setter with alignment; getter remains.
- **Remaining:** Text-margin getter and richer hot/focus notifications.

### CheckBox and RadioButton

**Priority: P2**

- First-class three-state values, including indeterminate state and automatic
  three-state styles.
- Button image-list alignment.
- Consistent state-change callbacks that report the new checked state.

### ComboBox

**Priority: P2**

- Cue-banner get/set.
- Minimum visible item count.
- Dropped width, dropped state/rectangle, top index, horizontal extent, item
  height, edit selection, and locale wrappers.
- Optional shell/autocomplete integration.
- `ComboBoxEx` remains an entirely missing control for image-bearing items.

### TextBox

**Priority: P2**

- Cue-banner readback and explicit show-while-focused behavior.
- Hide balloon tip, margins, tab stops, formatting rectangle, password
  character query, and undo-state APIs.
- Better IME/composition event exposure.

### RichTextBox

**Priority: P2-P3**

- Zoom, paragraph formatting, streaming callbacks, OLE objects, protected
  ranges, redo/undo metadata, and Text Object Model access.
- Modern Rich Edit options such as spell checking, typography, math, and
  accessibility need a separate capability-based audit because installed Rich
  Edit behavior is not cleanly tied to one Windows release.

### DateTime and MonthCalendar

**Priority: P1-P2**

- **Added:** Nullable/no-date mode through `DTS_SHOWNONE`, `getValue()`, and
  `setNone()`.
- **Added:** Date range get/set.
- **Added:** Ideal size, close-month-calendar, picker information,
  month-calendar styles, direct month-calendar handle/font wrappers, and
  format-query/format callback notifications.
- **Added:** A standalone `MonthCalendar` widget with value, multiselect range,
  maximum selection count, min/max range, today, month range, sizing, colors,
  Unicode format, hit testing, current view, month delta, calendar count, grid
  info, and selection/view-change events.
- **Remaining:** Specialist standalone `MonthCalendar` calendar-ID and day-state
  APIs.

### Header

**Priority: P2**

- **Added:** Checkbox items and checked state.
- **Added:** Split/drop-down item affordances and drop-down/overflow
  notifications.
- **Added:** Filter-bar timeout, editing, clearing, and filter-change/button
  notifications.
- **Added:** Focus item, order arrays, sort arrows, image lists, drag images,
  item/drop-down/overflow rectangles, hit testing, hot divider, drag tracking,
  item text/data fixes, and Unicode-format wrappers.
- **Added:** `CustomDraw` aspect coverage for native header custom drawing.

### Table (ListView)

**Priority: P1**

- **Added:** View and tile-view get/set APIs.
- **Added:** Empty-text/empty-markup callback support.
- **Added:** Footer information, item and rectangle retrieval plus link events.
- **Added:** Generic group info, rectangle, count, move/removal, metrics,
  state, selection, and focus APIs.
- **Added:** `LVITEMINDEX` iteration and state APIs for owner-data lists
  combined with grouping, including item-index rectangles.
- **Added:** Insert marks/colors, work areas, hot cursor/item, hover time,
  outline color, selected column, background image, view rectangle, origin,
  visibility, top index, count-per-page, and edit-control access.
- **Added:** Complete column support through the existing Columns aspect and
  owned image-list getters.
- **Added:** Begin-drag, item-activation, key, item-changing/item-changed,
  label-edit, and footer-link events.

### Tree and VirtualTree

**Priority: P1**

- **Added:** Generic extended-style get/set plus dedicated multiselect and
  double-buffering helpers. Auto-horizontal scrolling, fade expandos, rich
  tooltips, asynchronous image drawing, no-single-collapse, and no-indent-state
  can be selected through the generic style API.
- **Added:** Partial, exclusion, and dimmed checkbox styles and item states,
  configurable through `Tree::Seed` or at runtime.
- **Added:** Multiselect-aware selected-item enumeration and counts for native
  Tree controls.
- **Added:** Item-changing/item-changed, info-tip, label-edit, async-draw, drag,
  right-drag, and key notifications.
- **Added:** Drag images, insertion marks/colors, line colors, indentation,
  scroll timing, autoscroll settings, tooltips, sorting, full item and part
  rectangles, and accessibility item-ID mapping.
- **Partial:** `VirtualTree` translates applicable item states, geometry, drag
  images, insertion marks, sorting, accessibility IDs, and notifications to
  stable virtual handles. True native-style multiselect storage and enumeration
  remain.

### ProgressBar

**Priority: P1**

- **Added:** Marquee mode and speed.
- **Added:** Normal, error, and paused states.
- **Added:** Foreground and background colors.
- **Added:** Cached step value; `getStep()` no longer mutates the control.

### Slider (Trackbar)

**Priority: P1-P2**

- **Added:** Full 32-bit min/max range messages instead of the packed 16-bit
  range API.
- **Added:** Selection range, line/page size, thumb length,
  channel/thumb rectangles, and tick value/position/count arrays.
- **Added:** Tooltip control and placement, buddy getter, and Unicode-format
  setting.
- **Added:** Reversed, down-is-left, transparent-background,
  selection-range-visible, fixed-thumb-length, and notify-before-move styles.
- **Added:** `TRBN_THUMBPOSCHANGING` for validation before movement.

### Spinner (Up-Down)

**Priority: P2**

- Numeric base, acceleration table, range getter, and explicit invalid-value
  reporting from `UDM_GETPOS32`.
- Wrap, horizontal, hot-track, and Unicode-format options.

### StatusBar

**Priority: P3**

- Simple mode, text retrieval, icon retrieval, borders, public part rectangles,
  and Unicode-format control.

### Link

**Priority: P2**

- Per-link ID, text, and state get/set for controls containing multiple links.
- Enabled, focused, visited, hot-track, and default-color states.

### ToolTip

**Priority: P1**

- **Added:** Title and title icon.
- **Remaining:** Balloon and close-button styles.
- **Partial:** `TTN_LINKCLICK` callback added; per-tool parsed-link flags remain.
- **Partial:** Margins, colors, and pop/update are added. Tracking
  position/activation, current tool, enumeration, bubble size, and rectangle
  adjustment remain.
- **Added:** Typed window-theme support.
- **Remaining:** Per-tool absolute, transparent, centered, and parse-link flags.

### ToolBar

**Priority: P2**

- Pressed image list.
- Metrics, padding, button/bitmap size, max/ideal size, and text-row limits.
- Hot item, anchor highlight, insertion mark/color, and button movement.
- Complete button info access for text, image, command, state, style, and data.
- List gap, indent, drawing flags, string queries, accelerator mapping,
  save/restore state, and accessibility object access.

### Rebar

**Priority: P2**

- Complete band data: image, background, header/ideal/integral size, data,
  chevron, location, margins, and child-size constraints.
- Extended styles, borders, band/bar rectangles, row count/height, colors,
  image list, hit testing, drag, maximize, and minimize.
- Chevron-pushed, autosize, child-size, and layout-change events.

### TabView

**Priority: P3**

`TabView` is substantially custom-drawn, so native tab-control parity is less
valuable than DPI and UI Automation work. Remaining native gaps include
highlight state, extended styles, minimum width, padding, item size, row count,
adjust-rectangle, deselect-all, focus item, image-list access, and Unicode
format.

### Notification

**Priority: P1**

- **Added:** Call `NIM_SETVERSION` with `NOTIFYICON_VERSION_4` and handle its
  updated callback semantics.
- **Remaining:** GUID-based identity, real-time display, quiet-time respect,
  large icons, and sound suppression.
- **Partial:** `Shell_NotifyIconGetRect`, `NIM_SETFOCUS`, keyboard selection,
  and popup-open/popup-close events are added. Balloon-show events remain.
- **Partial:** Basic icon redisplay after Explorer/taskbar recreation exists;
  stronger restoration and error handling remain.

### Taskbar

**Priority: P1**

LibDWT already wraps overlay icons and custom tab thumbnails. Missing
`ITaskbarList3` coverage includes:

- **Added:** Progress state and progress value.
- **Remaining:** Thumbnail toolbar buttons and button updates.
- **Remaining:** Thumbnail tooltip and clipping rectangle.
- **Remaining:** Tab properties.
- **Remaining:** AppUserModelID support and optional Jump List/recent-document
  integration.

### LoadDialog, SaveDialog, and FolderDialog

**Priority: P0**

**Partial:** These now use an `IFileDialog` backend while preserving the simple
path-returning API. `FolderDialog::setRoot()` retains a legacy rooted fallback
because `IFileDialog` does not provide an equivalent hard-root restriction.
Public shell-item results, shell-item places and initial folders,
multiple-result APIs, events, and customization hooks are available. Higher-level
typed custom-control helpers and live validation of library and virtual-folder
behavior remain.

### MessageBox

**Priority: P0**

**Added:** `MessageBox` remains available for simple prompts and the separate
`TaskDialog` abstraction handles richer Windows 7+ dialogs.

### Menu

**Priority: P2-P3**

- Better owner-draw/theming support, menu-item bitmaps, default items, item
  state/data queries, and keyboard cue handling.
- UI Automation and high-contrast behavior matter more than adding wrappers for
  every legacy menu function.

### Composite, Container, Grid, TableTree, Splitter, SplitterContainer, and ScrolledContainer

**Priority: P0-P2**

These have no direct common-control equivalent. Their principal Windows 7-11
gaps are per-monitor DPI, pointer input, keyboard operation, high contrast, and
UI Automation rather than missing SDK messages.

- **Partial:** Framework DPI, child resource callbacks, automatic fonts, and
  basic pointer events are available.
- **Partial:** `Grid`, `ScrolledContainer`, `SplitterContainer`, and `Splitter`
  expose fragment navigation; Splitter and ScrolledContainer expose semantic
  patterns.
- **Added:** Item-level Selection/SelectionItem, ExpandCollapse, and Invoke
  patterns for `TableTree`, `VirtualTree`, and `TabView`.
- **Added:** Keyboard behavior was audited and `ScrolledContainer` now supports
  focusable arrow, page, home, and end scrolling.
- **Partial:** Theme, system-color, and settings changes trigger typed callbacks,
  relayout, and repaint with high-contrast state available to controls.
- **Remaining:** Visual high-contrast and text-scaling audits, live UIA client
  validation, and specialist patterns.

### ModalDialog and ModelessDialog

**Priority: P1-P2**

There is no newer native dialog-window class to wrap. Their material gaps are
the framework-wide DPI, pointer, theme, accessibility, and Windows 11 DWM
features. Richer prompts are now available through the separate `TaskDialog`
abstraction.

### Label, GroupBox, ColorDialog, and FontDialog

**Priority: P3**

No material Windows 7-11-specific native feature gap was found. They still
benefit from framework-wide DPI, theme, and accessibility improvements.

### MDIChild, MDIFrame, and MDIParent

**Priority: P3**

MDI has no meaningful modern SDK expansion. Maintenance should focus on DPI,
theme, and accessibility compatibility rather than new MDI-specific APIs.

## Entirely Missing Native Controls

Ordered by likely usefulness:

1. `ComboBoxEx`.
2. Native `ListBox`.
3. Hot-key control.
4. IP-address control.
5. Pager control.
6. Animation control.
7. Property sheet/wizard abstraction.

No longer entirely missing:

- **Added:** `TaskDialog` abstraction.
- **Added:** Standalone `MonthCalendar` abstraction.
- **Partial:** Modern `IFileDialog` abstraction. The path-based load, save, and
  folder APIs use it; shell-item results, events, and customization hooks are
  available. Typed custom-control helpers and live virtual-folder validation
  remain.

The final four are mature or niche controls and should not displace DPI,
accessibility, shell-dialog, Table, or Tree work.

## Recommended Implementation Order

1. **Partial:** Per-Monitor V2 foundation, manifests, thread contexts, system
   parameters, automatic fonts/image lists, and general resource hooks are
   added, with headless DPI contract tests. Continue with remaining
   direct-resource policies and live multi-monitor transition tests.
2. **Partial:** UI Automation fragments, logical item providers, structure
   events, Selection, ExpandCollapse, Invoke, RangeValue, and Scroll are added.
   Logical-item contracts, keyboard behavior, and typed appearance/settings
   notifications are covered. Continue with live client validation,
   text-scaling and visual high-contrast audits, and specialist patterns.
3. **Partial:** `IFileDialog` and `TaskDialog` wrappers are added. Continue with
   typed file-dialog custom-control helpers, live virtual-folder validation, and
   typed task-dialog live updates.
4. **Added:** Pointer, touch, gesture, pen details, history, capture, and
   cancellation coverage is complete against the original P1 audit.
5. **Partial:** Native Table and Tree common-controls v6 coverage is complete
   against the original P1 audit. Continue with live VirtualTree UI and
   accessibility validation.
6. **Partial:** Taskbar progress, tray v4, ProgressBar, Button, and ToolTip work
   is added. Continue thumbnail buttons and the remaining tray/Button/ToolTip
   items.
7. **Partial:** DateTime and MonthCalendar additions are present, and Slider and
   Header are complete against the audit. Continue with ToolBar and Rebar.
8. **Remaining:** Windows 11 DWM appearance options with build guards.
9. **Remaining:** Lower-priority completeness work and specialist Rich Edit
   features.

## Suggested Next Work

The highest-value remaining sequence after this update is:

1. Add live UIA client and multi-monitor DPI transition tests, then complete
   visual high-contrast and text-scaling audits.
2. Add higher-level typed file-dialog custom-control helpers and live
   library/virtual-folder validation.
3. Add taskbar thumbnail buttons and complete tray identity/options.
4. Add ToolTip/Button completion, then ToolBar/Rebar.
5. Add guarded Windows 11 DWM appearance APIs.

## Primary References

- [Common control reference](https://learn.microsoft.com/en-us/windows/win32/controls/control-library)
- [Tree-view control reference](https://learn.microsoft.com/en-us/windows/win32/controls/tree-view-control-reference)
- [List-view control reference](https://learn.microsoft.com/en-us/windows/win32/controls/list-view-control-reference)
- [Header control reference](https://learn.microsoft.com/en-us/windows/win32/controls/header-control-reference)
- [Progress-bar control reference](https://learn.microsoft.com/en-us/windows/win32/controls/progress-bar-control-reference)
- [IFileDialog](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-ifiledialog)
- [TaskDialogIndirect](https://learn.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-taskdialogindirect)
- [ITaskbarList3](https://learn.microsoft.com/en-us/windows/win32/api/shobjidl_core/nn-shobjidl_core-itaskbarlist3)
- [High-DPI desktop application development](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows)
- [WM_POINTERDOWN](https://learn.microsoft.com/en-us/windows/win32/inputmsg/wm-pointerdown)
- [UI Automation provider overview](https://learn.microsoft.com/en-us/windows/win32/winauto/uiauto-providersoverview)
- [DwmSetWindowAttribute](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmsetwindowattribute)
- [DWMWINDOWATTRIBUTE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute)
- [Windows 11 rounded corners](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/ui/apply-rounded-corners)
