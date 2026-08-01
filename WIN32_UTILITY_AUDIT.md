# Win32 Utility API Audit

**Audit date:** 2026-08-01
**Platform baseline:** Windows 7 (`_WIN32_WINNT=0x0601`)
**Scope:** Existing LibDWT geometry, resources, canvases, widgets, controls,
dialogs, and shell abstractions

## Selection Criteria

This pass favors APIs that are broadly reusable, have stable Windows 7+
behavior, preserve LibDWT ownership rules, and can be exposed without leaking
temporary buffers or application-specific policy. It deliberately avoids
adding a wrapper for every Win32 message.

The MSVC framework tests now embed the common-controls v6 manifest. Modern
common-control messages may legitimately be unavailable to applications that
do not opt into version 6; LibDWT applications should continue embedding
`projects/windows-common.manifest`.

## Foundation and GDI

| Surface | Added in this pass | Useful remaining candidates |
| --- | --- | --- |
| `Point` / coordinates | Existing typed client/screen coordinate classes retained | Optional size-specific type if ABI compatibility permits |
| `Rectangle` | Empty, normalize, offset, inflate, intersection, and union operations | Subtraction into multiple rectangles is better represented by `Region` |
| `Bitmap` | `BITMAP` metadata readback; resource-loading flags now honored | DIB-section creation and pixel access need a dedicated owned buffer API |
| `Brush` | `LOGBRUSH` construction and readback | Pattern/DIB constructors should make lifetime rules explicit |
| `Pen` | `LOGPEN` construction and readback | `ExtCreatePen` needs a typed variable-length style description |
| `Font` | Existing `LOGFONT` round-trip is sufficient | Font fallback/enumeration belongs in a separate text API |
| `Icon` | Existing sizing and immutable resize are sufficient | Multi-resolution ICO enumeration is specialist functionality |
| `ImageList` | Existing ownership, sizing, icon, merge, color, and resize coverage is strong | Drag-image helpers and overlay slots are reasonable later additions |
| `Region` | Bounds/type, empty, contains/intersects/equality, offset, combine, and reference-form transform | Region-data enumeration only if callers need rectangle decomposition |
| `Canvas` | Scoped `SaveDC`/`RestoreDC`, current position, polyline/Bezier/rounded rectangle/frame, rectangle inversion, clipping, `BitBlt`, and `StretchBlt` | Map modes, world transforms, paths, gradients, alpha blending, and print-document lifecycle |
| `Color`, `Theme`, `GDI` utilities | Existing color and DPI helpers retained | HSL conversion and theme-part helpers are useful but not core Win32 parity |

LibDWT does not currently model palettes, enhanced metafiles, or printer
documents. Palette APIs are low priority on modern true-color displays;
enhanced-metafile and print lifecycle support would be useful, but each needs a
separate ownership-oriented abstraction rather than additions to `Canvas`.

## Core Window and Application Surfaces

| Surface | Audit result |
| --- | --- |
| `Application` | Message pumping, initialization, module paths, common-control initialization, and DPI setup are already covered. Thread-message registration and accelerator-table ownership are possible later utilities. |
| `Widget` | Added client/screen point and rectangle conversion, invalidation/validation/update queries, minimized/maximized state, and `WINDOWPLACEMENT` round-trip. Existing style, DPI, redraw, parent, z-order, callback, input, theme, and accessibility coverage is strong. |
| `Control`, `CommonControl` | No separate raw utility gap; shared behavior belongs on `Widget` or aspects. |
| `Window`, `Frame` | Existing close/layout/menu/state aspects cover common work. Typed DWM attributes and monitor information remain high-value future work. |
| `Clipboard` | Existing text and data ownership should be audited separately before adding format enumeration or delayed rendering. |
| `Taskbar`, `Notification` | Existing overlays, previews, progress, thumbnail buttons, AppUserModelID, Jump Lists, and tray-v4 coverage are strong. Remaining work is primarily live Explorer validation and recovery behavior. |

## Native Controls

| Control | Result of this pass | Useful remaining candidates |
| --- | --- | --- |
| `Button` | No change; image lists, margins, notes, shields, split buttons, state, and notifications are already covered | Per-state imagery beyond the native image-list contract |
| `CheckBox` / `RadioButton` | Added typed unchecked/checked/indeterminate state, automatic three-state seed, and state-change callback | None material for the native button contract |
| `ComboBox` | Added drop-down visibility/geometry/width, top index, horizontal extent, item height, edit selection, locale, minimum-visible count, extended UI, and `COMBOBOXINFO` | Cue-banner readback and shell autocomplete; `ComboBoxEx` should be a separate control |
| `TextBox` | Added margins, formatting rectangle, tab stops, undo helpers, cue readback/focus behavior, password-character readback, and balloon dismissal | Typed IME/composition events and shell autocomplete |
| `RichTextBox` | Existing rich text surface left unchanged | Zoom, streaming, paragraph formats, redo metadata, OLE, and TOM require a dedicated capability pass |
| `ProgressBar` | Existing range, step, marquee, state, and color APIs are sufficient | Pause/error animation policy is application-level |
| `Slider` | Existing full range, tick, selection, tooltip, buddy, style, and notification coverage is strong | No material general-purpose gap found |
| `Spinner` | Added range readback, validated value retrieval, radix, acceleration table, and Unicode-format access | Convenience seed flags for wrap/horizontal/hot-track |
| `StatusBar` | Added text/icon/part-rectangle readback, part count, simple mode/text, minimum height, and Unicode format | Border metrics and owner-draw data helpers |
| `ToolTip` | Added per-tool flags/removal, width and color readback, tool count, bubble sizing, rectangle adjustment, and tracking | Tool enumeration/current-tool wrappers and rectangular (non-HWND) tools |
| `ToolBar` | Added pressed images, padding, button/bitmap sizing, maximum/ideal size, item rectangles, hot item, anchor highlighting, button movement, text rows, and extended styles | Full `TBBUTTONINFO`, insertion marks, string/accelerator mapping, and save/restore state |
| `Rebar` | Added band lookup/geometry, bar/row metrics, colors, extended-style query, image list ownership, hit testing, show/move/minimize/maximize, and insertion error reporting | Typed complete band-info get/set plus chevron/autosize notifications; `RB_SETEXTENDEDSTYLE` is intentionally omitted because Windows documents it as unimplemented |
| `Header` | Existing order, focus, filters, images, geometry, hit testing, checkbox, sort, Unicode, and notifications are strong | No material general-purpose gap found |
| `Table` / `TableTree` | Existing ListView surface and hierarchy translation are extensive | Specialist accessibility/live owner-data validation rather than more raw messages |
| `Tree` / `VirtualTree` | Existing native state, geometry, sorting, drag, multiselect, checkbox, and notification coverage is extensive | Specialist live UIA and async-data validation |
| `DateTime` / `MonthCalendar` | Existing value/range/view/grid/font/color/event coverage is strong | Calendar IDs and day-state callbacks |
| `Link` | Basic links work | Typed per-link ID/text/state get/set is the main remaining native gap |
| `TabView` | Custom-drawn abstraction is adequate | Native tab parity is lower value than its DPI/accessibility behavior |

## Custom Containers and Layout

`Composite`, `Container`, `Grid`, `ScrolledContainer`, `Splitter`,
`SplitterContainer`, and `TableTree` do not map directly to a missing Win32
message family. Their useful work is layout correctness, keyboard behavior,
DPI, high contrast, and UI Automation. Those cross-cutting foundations already
exist; visual high-contrast and live accessibility validation remain more
valuable than adding raw Win32 calls.

`GroupBox`, `Label`, `Column`, `Frame`, `MDIChild`, `MDIFrame`, `MDIParent`,
and the modal/modeless dialog bases have no important low-risk utility gap from
this audit. `Menu` would benefit from typed item-info/default-item/bitmap
helpers, but owner-draw and bitmap ownership need a separate design to avoid
unsafe handle lifetimes.

## Dialogs and Shell UI

`LoadDialog`, `SaveDialog`, and `FolderDialog` already use `IFileDialog` and
expose shell items, places, events, and customization. `ColorDialog`,
`FontDialog`, `MessageBox`, and `TaskDialog` cover their common contracts.
Remaining work is specialist live validation or richer typed customization,
not small Win32 utility functions.

## Missing Native Control Types

The audit still identifies `ComboBoxEx`, a native `ListBox`, hot-key, IP-address,
pager, animation, and property-sheet/wizard abstractions as absent. They were
not added here because each needs a real widget contract, creation seed,
ownership model, notifications, tests, and an example; they are not safe to
treat as a handful of utility methods.

## Recommended Next Passes

1. Typed Windows 10/11 DWM appearance attributes with runtime build guards.
2. Complete `ToolBar`/`Rebar` information structures and notifications.
3. Per-link `Link` state and `Menu` item-information ownership.
4. Rich Edit capability and streaming pass.
5. Add new native widgets in this order: `ListBox`, `ComboBoxEx`, hot-key,
   IP-address, then the more specialist controls.

## Primary Windows References

- [Graphics object information (`GetObject`)](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getobject)
- [Clipping regions (`SelectClipRgn`)](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-selectcliprgn)
- [Combo-box control messages](https://learn.microsoft.com/en-us/windows/win32/controls/bumper-combobox-control-reference-messages)
- [Edit-control text operations](https://learn.microsoft.com/en-us/windows/win32/controls/edit-controls-text-operations)
- [Up-down control reference](https://learn.microsoft.com/en-us/windows/win32/controls/up-down-control-reference)
- [Tooltip controls](https://learn.microsoft.com/en-us/windows/win32/controls/tooltip-controls)
- [Status bars](https://learn.microsoft.com/en-us/windows/win32/controls/status-bars)
