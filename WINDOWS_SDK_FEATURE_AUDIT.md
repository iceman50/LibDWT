# Windows SDK Feature Audit

Audit date: 2026-06-14
Last updated: 2026-06-15

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
| Per-monitor DPI | **Partial** | Per-Monitor V2 manifests, runtime awareness negotiation, per-widget DPI queries and scaling, DPI-aware metrics and window adjustment, suggested bounds, and typed `WM_DPICHANGED` events | Thread awareness APIs, DPI-aware system parameters, automatic font/image/icon/cache recreation, and DPI transition tests |
| UI Automation | **Partial** | Opt-in `IRawElementProviderSimple`, native host provider, names, help text, control type, enabled/focus/offscreen properties, event raising, and initial `Grid`/`ScrolledContainer`/`Splitter` opt-in | Fragment trees, control patterns, child/item providers, semantic selection/value state, structure notifications, and accessibility behavior audits |
| Modern file dialogs | **Partial** | `IFileOpenDialog`/`IFileSaveDialog` backends for load, save, and folder selection; filters, filesystem paths, long paths, custom places, client GUIDs, options, and `FOS_PICKFOLDERS` | Public `IShellItem` and multi-result APIs, virtual-folder results, `IFileDialogEvents`, and `IFileDialogCustomize` |
| Task dialogs | **Added** | `TaskDialogIndirect` wrapper with common/custom buttons, command links, radio buttons, verification, expanded/footer text, icons, progress modes, callbacks, and callback-based live updates | Convenience APIs for hyperlinks and typed live progress/text updates |

### P1 Work Completed So Far

| Area | Status | Added | Remaining |
| --- | --- | --- | --- |
| Pointer/input | **Partial** | Typed `WM_POINTER` down, update, up, enter, leave, and capture-changed events with pointer ID, type, position, and flags | `WM_TOUCH`, `WM_GESTURE`, pointer history, pressure/pen details, explicit capture helpers, and cancellation |
| Button | **Partial** | Command-link styles through seeds, note get/set, elevation shield, aligned image lists, split configuration, and `BCN_DROPDOWN` | Image-list getter, text-margin getter, explicit dropdown-state helpers, and hot/focus notifications |
| ProgressBar | **Added** | Marquee mode/speed, normal/error/paused states, colors, and non-mutating cached step retrieval | No remaining item from the original P1 audit |
| Table/ListView | **Partial** | View get/set, tile setters, empty markup, generic group info operations, footer rectangle, `LVITEMINDEX` state/iteration, insertion marks, top/page/edit accessors, and drag/activate/key/item-changing events | Tile readback, footer contents/events, remaining group metrics and focus/selection operations, work areas, hot/hover/color/background geometry APIs, complete columns/image lists, and label-edit events |
| Tree/VirtualTree | **Partial** | Checkboxes, generic extended styles, multiselect and selected-item enumeration, double buffering, and item-changing/item-changed events | Correct multiselect count integration, extended checkbox states, info-tip/label-edit/async-draw/drag events, drag images, insertion marks, timing/scroll/tooltips/sort/part rectangles, accessibility item IDs, and full `VirtualTree` parity |
| Taskbar | **Partial** | Progress state and value | Thumbnail toolbar buttons/updates, thumbnail tooltip/clipping, tab properties, AppUserModelID, and Jump Lists |
| Notification | **Partial** | `NOTIFYICON_VERSION_4`, keyboard selection, `Shell_NotifyIconGetRect`, `NIM_SETFOCUS`, popup lifecycle events, and basic Explorer recreation handling | GUID identity, real-time/quiet-time/large-icon/sound flags, balloon-show events, and stronger restoration/error handling |
| ToolTip | **Partial** | Title/icon, margins, colors, pop/update, window theme, and `TTN_LINKCLICK` | Balloon/close styles, tracking APIs, current-tool/enumeration/bubble sizing/rectangle adjustment, and per-tool flags |
| DateTime | **Partial** | Nullable values, ranges, ideal size, close-calendar, month-calendar styles, and picker information | Month-calendar handle/font wrappers, format-query callbacks, and standalone `MonthCalendar` |

## Priority Summary

### P0 - Framework Foundations

1. **Per-monitor DPI awareness and dynamic DPI changes - Partial**
   - **Added:** Per-Monitor V2 manifests and process DPI-awareness negotiation.
   - **Added:** Dispatch `WM_DPICHANGED` and expose old/new DPI plus the suggested window
     rectangle.
   - **Added:** Replace the process-global `LOGPIXELSX` scale cached by
     `util::dpiFactor()` with window/monitor-aware scaling.
   - **Remaining:** Recreate fonts, image lists, icons, layout metrics, and custom-control
     caches when DPI changes.
   - **Partial:** Wrap `GetDpiForWindow`, `GetSystemMetricsForDpi`, and
     `AdjustWindowRectExForDpi`; DPI-aware system parameters remain.
   - **Remaining:** Add thread DPI-awareness helpers.
   - Earliest relevant versions: Windows 8.1 for per-monitor DPI and Windows 10
     for the complete Per-Monitor V2 API.

2. **UI Automation for custom controls - Partial**
   - Native controls receive much of their accessibility behavior from Windows,
     but LibDWT's custom `Grid`, `TableTree`, `VirtualTree`, `TabView`,
     `Splitter`, and `ScrolledContainer` need UI Automation providers.
   - **Added:** Provider infrastructure, names, help text, control types,
     enabled/focus/offscreen state, native host providers, and event raising.
   - **Partial:** `Grid`, `ScrolledContainer`, and `Splitter` opt in to the base
     provider.
   - **Remaining:** Control patterns, fragment trees, values, selection state,
     child bounds, and structure-change notifications.
   - **Remaining:** Audit high-contrast, text scaling, keyboard-only operation, and
     `WM_THEMECHANGED`/`WM_SETTINGCHANGE` behavior.
   - Available at the Windows 7 minimum.

3. **Modern file and folder dialogs - Partial**
   - **Added:** Replace or supplement `OPENFILENAME` and `SHBrowseForFolder` with
     `IFileOpenDialog` and `IFileSaveDialog`.
   - **Added:** Filesystem path results, long paths, custom places, persisted
     client GUIDs, option flags, and folder picking through `FOS_PICKFOLDERS`.
   - **Remaining:** Public `IShellItem` results, libraries and virtual folders,
     event callbacks, custom controls, and public multiple-result APIs.
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

1. **Partial:** Pointer events added; touch, gesture, and richer pen data remain.
2. **Partial:** A substantial ListView/Table v6 subset is added.
3. **Partial:** Tree extended styles, multiselection, and item-change events are
   added; extended states and specialist events remain.
4. **Partial:** Taskbar progress is added; thumbnail-toolbar support remains.
5. **Added:** Notification icons now use `NOTIFYICON_VERSION_4`; related tray
   options remain.
6. **Partial:** Button, ProgressBar, ToolTip, and DateTime additions are in
   place; ProgressBar is complete against this audit.

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
  `WM_DPICHANGED`, and DPI-aware geometry are added. Automatic resource
  recreation remains.
- **Partial:** Basic typed pointer events are added. Touch, gesture, detailed
  pen data, capture helpers, history, and cancellation remain.
- **Remaining:** Theme, high-contrast, system-color, text-scale, and accessibility change
  notifications.
- **Partial:** UI Automation provider infrastructure is added; semantic
  providers and patterns remain.
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

### DateTime

**Priority: P1-P2**

- **Added:** Nullable/no-date mode through `DTS_SHOWNONE`, `getValue()`, and
  `setNone()`.
- **Added:** Date range get/set.
- **Partial:** Ideal size, close-month-calendar, picker information, and
  month-calendar styles are added; direct month-calendar handle/font wrappers
  remain.
- **Remaining:** Format-query and format callback notifications.
- **Remaining:** A standalone `MonthCalendar` widget, including multiselect, week
  numbers, range limits, current view, calendar identifiers, and grid info.

### Header

**Priority: P2**

- Checkbox items and checked state.
- Split/drop-down items and overflow notifications.
- Filter-bar editing, timeout, clear, and filter-change events.
- Focus item, order arrays, sort arrows, image lists, item rectangles, hot
  divider, drag tracking, and Unicode-format wrappers.

### Table (ListView)

**Priority: P1**

- **Partial:** View get/set and tile-view setters are added; complete tile
  readback remains.
- **Added:** Empty-text/empty-markup callback support.
- **Partial:** Footer rectangle retrieval; footer information, items, and link
  events remain.
- **Partial:** Generic group info get/set, rectangle, count, move, and removal
  are added. Dedicated group metrics, selection, and focus APIs remain; generic
  `LVGROUP` supports subtitle, task text, descriptions, subset text, state, and
  alignment.
- **Added:** `LVITEMINDEX` iteration and state APIs for owner-data lists
  combined with grouping.
- **Partial:** Insert marks, top index, count-per-page, and edit-control access
  are added. Work areas, hot cursor/item, hover time, outline color, selected
  column, background image, view rectangle, and origin remain.
- **Remaining:** Complete column and image-list accessors.
- **Partial:** Begin-drag, item-activation, key, and item-changing events are
  added. Label-edit and item-changed convenience events remain.

### Tree and VirtualTree

**Priority: P1**

- **Added:** Generic extended-style get/set plus dedicated multiselect and
  double-buffering helpers. Auto-horizontal scrolling, fade expandos, rich
  tooltips, asynchronous image drawing, no-single-collapse, and no-indent-state
  can be selected through the generic style API.
- **Remaining:** Extended checkbox variants: partial, exclusion, and dimmed states.
- **Partial:** Selected-item enumeration is added, so callers can obtain a count
  from the returned collection. The inherited `countSelected()` implementation
  still needs multiselect-aware integration.
- **Partial:** Item-changing/item-changed events are added. Info-tip, label-edit
  validation, async-draw, and drag notifications remain.
- **Remaining:** Drag images, insertion marks, colors, indent, scroll timing,
  autoscroll info, tooltips, sorting, item rectangles/parts, and accessibility
  item-ID mapping.
- **Remaining:** `VirtualTree` must explicitly mirror and test applicable
  item-state and selection additions.

### ProgressBar

**Priority: P1**

- **Added:** Marquee mode and speed.
- **Added:** Normal, error, and paused states.
- **Added:** Foreground and background colors.
- **Added:** Cached step value; `getStep()` no longer mutates the control.

### Slider (Trackbar)

**Priority: P1-P2**

- Full 32-bit min/max range messages instead of the packed 16-bit range API.
- Selection range, line/page size, thumb length, channel/thumb rectangles, and
  tick position/count arrays.
- Tooltip control and placement, buddy getter, and Unicode-format setting.
- Reversed, down-is-left, transparent-background, and notify-before-move
  styles.
- `TRBN_THUMBPOSCHANGING` for validation before movement.

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
Public shell-item results, events, customization, virtual-folder results, and
multiple-result APIs remain.

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

- **Partial:** Framework DPI and basic pointer events are available.
- **Partial:** `Grid`, `ScrolledContainer`, and `Splitter` opt in to the base UI
  Automation provider.
- **Remaining:** Semantic patterns and fragment trees for all custom controls,
  plus keyboard and high-contrast audits.

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

1. Standalone `MonthCalendar`.
2. `ComboBoxEx`.
3. Native `ListBox`.
4. Hot-key control.
5. IP-address control.
6. Pager control.
7. Animation control.
8. Property sheet/wizard abstraction.

No longer entirely missing:

- **Added:** `TaskDialog` abstraction.
- **Partial:** Modern `IFileDialog` abstraction. The path-based load, save, and
  folder APIs use it; shell-item results, events, and customization remain.

The final four are mature or niche controls and should not displace DPI,
accessibility, shell-dialog, Table, or Tree work.

## Recommended Implementation Order

1. **Partial:** Per-Monitor V2 foundation and manifests are added. Continue
   with resource recreation policies, system parameters, and DPI-change tests.
2. **Partial:** UI Automation foundation is added. Continue with semantic
   patterns and fragment trees for custom controls.
3. **Partial:** `IFileDialog` and `TaskDialog` wrappers are added. Continue with
   shell-item results, file-dialog events/customization, and typed task-dialog
   live updates.
4. **Partial:** Basic pointer events are added. Continue with touch, gestures,
   pen details, history, capture, and cancellation.
5. **In progress:** Continue Table and Tree common-controls v6 completion.
6. **Partial:** Taskbar progress, tray v4, ProgressBar, Button, and ToolTip work
   is added. Continue thumbnail buttons and the remaining tray/Button/ToolTip
   items.
7. **Partial:** DateTime additions are present. Add `MonthCalendar`, then work
   through Slider, Header, ToolBar, and Rebar.
8. **Remaining:** Windows 11 DWM appearance options with build guards.
9. **Remaining:** Lower-priority completeness work and specialist Rich Edit
   features.

## Suggested Next Work

The highest-value remaining sequence after this update is:

1. Complete custom-control UI Automation patterns and fragment navigation.
2. Add DPI resource recreation hooks and focused DPI transition tests.
3. Finish Table footer/group/geometry APIs and Tree specialist events/states.
4. Add `IFileDialogEvents`, shell-item results, and customization.
5. Add taskbar thumbnail buttons and complete tray identity/options.
6. Finish pointer/touch/gesture/pen input.
7. Add standalone `MonthCalendar`, then Slider and Header coverage.
8. Add guarded Windows 11 DWM appearance APIs.

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
