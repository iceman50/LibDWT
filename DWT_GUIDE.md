# LibDWT programming guide

This is the practical guide to the DWT version shipped in this repository. DWT (the DC++ Widget Toolkit) is a C++17, Win32-native GUI toolkit. It wraps Windows HWND controls and messages without hiding Win32 concepts: styles are still `WS_*`, `BS_*`, `LVS_*`, and similar flags; colors are `COLORREF`; notifications are `WM_COMMAND` and `WM_NOTIFY`; and every widget exposes its native handle when the wrapper is not enough.

The most important ideas are:

- A `Widget` wraps an `HWND` and routes Windows messages to C++ callbacks.
- A widget is normally constructed with `WidgetCreator<T>` and configured through `T::Seed`.
- A parent owns the native lifetime of its child windows. Most widget wrappers delete themselves when Windows destroys their HWND.
- Behaviors such as captions, painting, keyboard input, selection, and closing are supplied by reusable **aspects**.
- A `Window` can contain controls but does not automatically lay them out. `Grid` is the normal general-purpose layout container.
- DWT intentionally leaves an escape hatch through `handle()`, `sendMessage()`, `addCallback()`, and `onRaw()`.

## Contents

- [Building and linking](#building-and-linking)
- [A complete first application](#a-complete-first-application)
- [The object and window model](#the-object-and-window-model)
- [Windows, composites, and containers](#windows-composites-and-containers)
- [Grid layout in depth](#grid-layout-in-depth)
- [Events, callbacks, and aspects](#events-callbacks-and-aspects)
- [`onRaw` and direct Win32 messages](#onraw-and-direct-win32-messages)
- [Painting, canvases, brushes, pens, and fonts](#painting-canvases-brushes-pens-and-fonts)
- [Widget catalog](#widget-catalog)
- [Menus, dialogs, notifications, and the taskbar](#menus-dialogs-notifications-and-the-taskbar)
- [The application loop, async work, and threads](#the-application-loop-async-work-and-threads)
- [DPI, themes, and accessibility](#dpi-themes-and-accessibility)
- [Resources and utility APIs](#resources-and-utility-apis)
- [Writing a custom control](#writing-a-custom-control)
- [Common failure modes](#common-failure-modes)
- [Examples and source reference](#examples-and-source-reference)

## Building and linking

The shipped projects build DWT as a static library (`dwt.lib` with MSVC or `libdwt.a` with MinGW-w64). The repository's supported build configurations use C++17, Unicode, and the Windows GUI subsystem.

### Default build: MinGW-w64 x64 Release

From the repository root:

```powershell
.\scripts\build-interactive.ps1
```

The default build compiles and tests the MinGW-w64 x64 Release configuration
and stages its artifacts under `Builds/MinGW-w64/Release`. Add `-SkipTests` to
build without running the validation programs, or add `-Interactive` to choose
a different toolchain, configuration, clean mode, and test mode from menus.

### Build this repository with MSVC

From the repository root:

```powershell
.\scripts\build.ps1 -Configurations Release -Platforms x64
```

The MSVC script builds the library, examples, and tests. Add `-SkipTests` to build without running the validation programs. The solution is also available at `projects/msvc/LibDWT.sln`.

### Build this repository with MinGW-w64

From the repository root:

```powershell
mingw32-make -C projects\mingw-w64 build ARCH=x64 CONFIG=release
```

Use `ARCH=x86` or `CONFIG=debug` for the other configurations. The MinGW build places the library under `projects/mingw-w64/build/<arch>/<config>/lib` and programs under the adjacent `bin` directory.

### Add DWT to another program

Use the existing example projects as the authoritative template. In summary:

- Add `dwt/include` to the compiler include path.
- Compile as C++17 and define `UNICODE`, `_UNICODE`, `NOMINMAX`, and the target-version macros used by the repository (`WINVER=0x0601`, `_WIN32_WINNT=0x0601`, `_WIN32_IE=0x0A00`).
- Link DWT plus `Comctl32`, `Gdi32`, `Shlwapi`, `Ole32`, `OleAut32`, `Uuid`, `Shell32`, `Comdlg32`, `Advapi32`, `UxTheme`, and `Dwmapi`.
- Build a Windows-subsystem executable. DWT supplies `WinMain`; the application supplies `dwtMain`.
- Embed a Common Controls v6 manifest. The repository template is [projects/windows-common.manifest](projects/windows-common.manifest). Without it, native controls can have old rendering and reduced feature support.
- Include the individual headers you use; there is no single all-widgets umbrella header.

`DWT_DEBUG_WIDGETS` prints widget creation/destruction counts in a diagnostic build and is useful for finding leaked HWND wrappers. `DWT_SHARED` changes startup and message-loop ownership as described later.

The normal executable entry point is:

```cpp
int dwtMain(dwt::Application& app);
```

DWT initializes common controls, per-monitor DPI awareness, and OLE before calling it. See [Shared-library mode](#shared-library-mode) if DWT is hosted in a DLL instead.

## A complete first application

This example creates a top-level window, puts a grid in its client area, places controls in grid cells, handles a button click, and shuts down the message loop correctly.

```cpp
#include <dwt/Application.h>
#include <dwt/WidgetCreator.h>
#include <dwt/widgets/Button.h>
#include <dwt/widgets/Grid.h>
#include <dwt/widgets/Label.h>
#include <dwt/widgets/TextBox.h>
#include <dwt/widgets/Window.h>

int dwtMain(dwt::Application& app) {
    using namespace dwt;

    Window::Seed windowSeed(_T("DWT hello"));
    windowSeed.location = dwt::Rectangle(100, 100, 640, 360);
    auto* window = WidgetCreator<Window>::create(windowSeed);

    auto* grid = WidgetCreator<Grid>::create(window, Grid::Seed(3, 2));
    grid->setSpacing(8);

    grid->row(0).mode = GridInfo::AUTO;
    grid->row(1).mode = GridInfo::AUTO;
    grid->row(2).mode = GridInfo::FILL;
    grid->row(2).align = GridInfo::STRETCH;

    grid->column(0).mode = GridInfo::AUTO;
    grid->column(1).mode = GridInfo::FILL;
    grid->column(0).align = GridInfo::STRETCH;
    grid->column(1).align = GridInfo::STRETCH;

    auto* prompt = WidgetCreator<Label>::create(
        grid, Label::Seed(_T("Your name:")));
    auto* input = WidgetCreator<TextBox>::create(
        grid, TextBox::Seed());
    auto* greet = WidgetCreator<Button>::create(
        grid, Button::Seed(_T("Greet")));
    auto* output = WidgetCreator<Label>::create(
        grid, Label::Seed(_T("Enter a name, then press Greet.")));

    grid->setWidget(prompt, 0, 0);
    grid->setWidget(input, 0, 1);
    grid->setWidget(greet, 1, 0, 1, 2);
    grid->setWidget(output, 2, 0, 1, 2);

    greet->onClicked([input, output] {
        output->setText(_T("Hello, ") + input->getText() + _T("!"));
    });

    auto layout = [window, grid] {
        grid->resize(dwt::Rectangle(window->getClientSize()));
    };
    window->onSized([layout](const SizedEvent&) { layout(); });

    // Closing the last window does not implicitly post WM_QUIT.
    window->onDestroy([] { ::PostQuitMessage(0); });

    // Most child Seed defaults contain WS_CHILD but not WS_VISIBLE.
    prompt->setVisible(true);
    input->setVisible(true);
    greet->setVisible(true);
    output->setVisible(true);
    grid->setVisible(true);

    layout();
    window->setVisible(true);
    input->setFocus();
    app.run();
    return 0;
}
```

There are four details worth remembering from this example:

1. `WidgetCreator<T>::create` allocates the wrapper, creates the HWND, and returns the widget's `ObjectType` (normally `T*`).
2. The child must be created with the grid as its parent before it can be assigned with `setWidget`.
3. The top-level `Window` does not fill its child automatically, so its resize handler sizes the grid.
4. Child controls normally need `setVisible(true)`. Alternatively, add `WS_VISIBLE` to each seed's `style` before creation.

## The object and window model

### Core hierarchy

The practical inheritance hierarchy is:

```text
Widget                         HWND, message callbacks, DPI, accessibility
└── Control                    common control aspects and accelerators
    ├── native controls        Button, TextBox, Table, Tree, Slider, ...
    └── Composite              can contain multiple children
        ├── Frame              top-level behavior
        │   ├── Window
        │   ├── MDIFrame
        │   └── dialog types
        └── Container          layout-oriented child window
            ├── Grid
            └── SplitterContainer
```

Some utility objects, including `Menu`, `Notification`, and common dialogs, are not ordinary self-deleting HWND widgets.

### Seeds and creation

Every creatable widget defines a `Seed`. It ultimately contains the base fields:

| Field | Meaning |
|---|---|
| `caption` | Initial window/control text passed to `CreateWindowEx`. |
| `style` | Normal Win32 styles such as `WS_CHILD`, `BS_COMMANDLINK`, or `LVS_REPORT`. |
| `exStyle` | Extended styles such as `WS_EX_CLIENTEDGE` or `WS_EX_CONTROLPARENT`. |
| `location` | Initial `Rectangle`; many layouts replace it immediately. |
| `menuHandle` | Menu handle for a top-level window or control ID where Win32 expects one. |

Derived seeds add useful typed configuration such as a `FontPtr`, row/column counts, image settings, or control-specific flags. Configure the seed before calling `create`:

```cpp
dwt::Table::Seed seed;
seed.style |= WS_VISIBLE;
seed.lvStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
seed.font = uiFont;
auto* table = dwt::WidgetCreator<dwt::Table>::create(parent, seed);
```

Do not replace a control's complete `style` casually. Native child controls require `WS_CHILD`, and many DWT seeds add notification, tab-stop, clipping, or control-specific flags that the wrapper expects. Prefer `seed.style |= flag`, `addRemoveStyle`, or a typed setter.

There are four factory forms:

```cpp
WidgetCreator<T>::create(seed);                  // top-level/no explicit parent
WidgetCreator<T>::create(parent, seed);          // child
WidgetCreator<T>::create(parent);                // child with default seed
WidgetCreator<T>::attach(parent, existingHwnd);  // subclass an existing HWND
```

`attach` replaces the existing window procedure with DWT's procedure. Use it only when ownership and destruction of the attached HWND are understood; some DWT controls use it for native subwindows created by another common control.

### Widget lifetime and ownership

Normal DWT widget wrappers are allocated with `new`. When their HWND receives `WM_NCDESTROY`, the DWT window procedure calls `kill()`, whose default implementation is `delete this`. Consequently:

- Treat `Window*`, `Button*`, and other widget pointers as **non-owning** handles.
- Do not place a normal factory-created widget in `std::unique_ptr` and do not `delete` it yourself.
- Closing or destroying a parent causes Windows to destroy its child HWNDs; their DWT wrappers then delete themselves.
- A pointer becomes invalid as soon as its HWND is destroyed. Clear cached pointers in `onDestroy` when another longer-lived object stores them.
- `close(false)` sends `WM_CLOSE` synchronously. `close(true)` posts it and returns immediately.
- `onClosing([] { return true; })` allows the default close. Returning `false` vetoes it.

`ModalDialog` is the exception: it overrides `kill()` and is intended to be represented by a C++ object, commonly a derived object on the stack. Its `create()` creates the HWND, `show()` runs the modal loop and destroys the HWND, and `endDialog(value)` ends that loop.

Graphics resources have a different lifetime model. `BitmapPtr`, `BrushPtr`, `FontPtr`, `IconPtr`, `ImageListPtr`, `PenPtr`, and `RegionPtr` are intrusive reference-counted pointers. `MenuPtr` and `NotificationPtr` are `std::unique_ptr` aliases.

### Native interoperation

Every `Widget` supplies the main Win32 escape hatches:

```cpp
HWND hwnd = widget->handle();
LRESULT value = widget->sendMessage(message, wParam, lParam);
widget->postMessage(message, wParam, lParam);
widget->addRemoveStyle(style, true);
widget->addRemoveExStyle(exStyle, false);
bool hasIt = widget->hasStyle(style);
```

Use `dwt::hwnd_cast<T*>(hwnd)` to recover a DWT wrapper from an HWND registered with DWT. It returns `nullptr` if the property is missing or the dynamic type does not match.

### Text, geometry, and coordinates

`dwt::tstring` is `std::wstring` in Unicode builds and `std::string` otherwise. The shipped projects are Unicode builds; use `_T("text")` for literals or construct `tstring` explicitly.

`Point` stores `x` and `y`. `Rectangle` stores a top-left `pos` and a `size`; its four-number constructor is `(x, y, width, height)`, not `(left, top, right, bottom)`.

```cpp
dwt::Point size = widget->getClientSize();
dwt::Rectangle client(size);               // (0, 0, width, height)
dwt::Rectangle bounds = widget->getWindowRect();
widget->resize(dwt::Rectangle(10, 10, 200, 30));
```

Child positions passed to `resize` are relative to the parent's client area. A top-level window's rectangle is in screen coordinates. `ScreenCoordinate` and `ClientCoordinate` make explicit conversions available for mouse and hit-test APIs.

## Windows, composites, and containers

### `Window` and `Frame`

`Window` is the normal top-level application window. Its default seed uses `WS_OVERLAPPEDWINDOW`, but not `WS_VISIBLE`; set its bounds and show it after the interface is ready.

Through `Frame`, it supports:

- `setSmallIcon` and `setLargeIcon`.
- `setMinimizeBox` and `setMaximizeBox`.
- `maximize`, `minimize`, and `restore` through the `MinMax` aspect.
- `setActive`, `isActive`, and `onActivate` through the `Activate` aspect.
- `animateSlide`, `animateBlend`, and `animateCollapse`.

`Window::onForeground()` reports whether the window, or an owned dialog, is the current foreground window.

A typical main window also registers:

```cpp
window->onSized([window, content](const dwt::SizedEvent&) {
    content->resize(dwt::Rectangle(window->getClientSize()));
});
window->onDestroy([] { ::PostQuitMessage(0); });
```

### `Composite`

`Composite` is a multi-child `Control`. It adds:

- Caption operations (`setText`, `getText`).
- `addChild(seed)`, typed `getChildren<T>()`, and `removeChild`.
- Background-erasure handling.
- `WS_CLIPCHILDREN` to reduce drawing over child windows.

It is primarily a base class. Its default `layout()` does nothing. `Window` derives from `Composite`, which is why a window does not automatically fill or arrange its children.

### `Container`

`Container` derives from `Composite` and is the simplest layout container. Whenever its window position changes, it calls `layout()`. The default implementation resizes the **first child `Control`** to fill the complete client area. Additional children are not arranged by the default implementation.

Use a plain `Container` for a single content child. Use `Grid`, `SplitterContainer`, `TabView`, or a custom `layout()` for multiple children.

### Single-child composites

The `Child` aspect is used when a control owns one content child:

- `GroupBox::addChild(seed)` replaces any existing child, then lays the new child inside the titled frame and padding.
- `ScrolledContainer::addChild(seed)` supplies one scrollable content surface.

By contrast, `Children` is a multi-child aspect used by `Composite` and its descendants.

### Nested layout is the normal pattern

Complex screens should be made from small nested containers. For example:

```text
Window
└── Grid (toolbar / content / status rows)
    ├── Rebar
    │   └── ToolBar
    ├── SplitterContainer
    │   ├── Grid (navigation pane)
    │   ├── Splitter
    │   └── TabView (document pane)
    └── StatusBar
```

This avoids one enormous coordinate-based resize handler and gives each container one clear responsibility.

## Grid layout in depth

`Grid` is DWT's primary layout engine. It divides its client area into rows and columns, calculates track sizes, and positions each assigned child. Rows and columns are zero-indexed.

### Creating tracks

The seed fixes the initial track counts:

```cpp
auto* grid = dwt::WidgetCreator<dwt::Grid>::create(
    parent, dwt::Grid::Seed(3, 2)); // 3 rows, 2 columns
```

Tracks can also be added and removed dynamically:

```cpp
size_t row = grid->addRow(dwt::GridInfo(0, dwt::GridInfo::FILL,
                                        dwt::GridInfo::STRETCH));
size_t col = grid->addColumn();
grid->removeRow(row);
grid->removeColumn(col);
```

Keep at least one column while the grid contains automatically managed children. Auto-placement divides by the column count.

### Track sizing modes

Each row and column is a `GridInfo` with `size`, `mode`, and `align`.

| Mode | Sizing behavior |
|---|---|
| `GridInfo::STATIC` | Uses `size` as a fixed 96-DPI logical size, scaled through the grid's current DPI. |
| `GridInfo::AUTO` | Uses the largest relevant child preferred size for the track. Only children with `WS_VISIBLE` contribute. |
| `GridInfo::FILL` | Starts with its `size`, then shares remaining client space equally with the other `FILL` tracks. Unlike `STATIC`, that baseline is not DPI-scaled by the grid. |

There are no weights or percentages. Three `FILL` columns receive equal shares of remaining space. For unequal regions, combine fixed and fill tracks, nest grids, or use a `SplitterContainer`.

If the fixed and automatic tracks need more room than the current client size, the grid does not proportionally shrink them; controls can overflow or clip. Put intrinsically large content in `ScrolledContainer` or establish an appropriate minimum window size.

Typical configuration:

```cpp
grid->row(0).mode = dwt::GridInfo::AUTO;   // toolbar-sized
grid->row(1).mode = dwt::GridInfo::FILL;   // remaining content
grid->row(1).align = dwt::GridInfo::STRETCH;
grid->row(2).mode = dwt::GridInfo::STATIC;
grid->row(2).size = 24;                    // scaled from 96 DPI

grid->column(0).mode = dwt::GridInfo::AUTO;
grid->column(1).mode = dwt::GridInfo::FILL;
```

### Alignment

Alignment belongs to a track, not an individual child:

| Alignment | Horizontal column meaning | Vertical row meaning |
|---|---|---|
| `TOP_LEFT` | Left-align at preferred width. | Top-align at preferred height. |
| `BOTTOM_RIGHT` | Right-align at preferred width. | Bottom-align at preferred height. |
| `CENTER` | Center at preferred width. | Center at preferred height. |
| `STRETCH` | Use the complete allocated width. | Use the complete allocated height. |

The defaults established by `Grid::create` are `CENTER` for rows and `STRETCH` for columns. Thus controls normally stretch horizontally but retain their preferred height.

For a spanning child, the grid uses the alignment of its starting row and starting column.

### Assigning children and spans

The explicit form is recommended:

```cpp
grid->setWidget(label, 0, 0);
grid->setWidget(editor, 0, 1);
grid->setWidget(table, 1, 0, 1, 2); // one row high, two columns wide
```

The signature is:

```cpp
setWidget(control, row, column, rowSpan = 1, columnSpan = 1);
```

The control must already be a direct child of the grid. `setWidget` records layout metadata; it does not reparent the HWND. Use valid, nonzero spans whose ending row and column are inside the grid. An out-of-range assignment is skipped during layout.

If `setWidget` is omitted, the grid discovers direct child controls and assigns them in row-major creation order to the first unoccupied cell. Explicit placement is clearer, especially after dynamic insertions or spans.

The one-argument overload has a different meaning:

```cpp
grid->setWidget(control);
```

It marks the child as known but **not managed**: the grid leaves its current rectangle unchanged. This is useful for an overlay or internally positioned helper window, not for ordinary auto-placement.

### Spacing and preferred size

`setSpacing(n)` puts `n` pixels between adjacent tracks; the default is 3. In the current implementation this value is not DPI-scaled automatically, so pass `grid->scale(n)` if the spacing is intended as a logical 96-DPI measurement.

`getPreferredSize()` returns the sum of the calculated track requirements plus inter-track spacing. Native controls override `getPreferredSize()` when they can measure meaningful content. A custom widget should override it if it will live in an `AUTO` row or column.

### When layout runs

Because `Grid` derives from `Container`, moving or resizing the grid triggers `layout()`. Changing track metadata, assignments, spacing, or child visibility does not itself guarantee an immediate relayout. After a dynamic change, call:

```cpp
grid->layout();
```

or resize/redraw the appropriate containing surface.

### Removing tracks

`removeRow` and `removeColumn` close controls whose recorded starting cell is in the removed track and shift later starting indices. Removing by `Control*` finds the track containing that control's starting cell.

`clearRows()` and `clearColumns()` clear only the track arrays; they do not close all child windows or reset every placement record. They are low-level rebuilding operations. Re-establish valid tracks and assignments before the next layout; use `removeChild`/`close` when the intent is to destroy controls.

### A form layout pattern

```cpp
auto* form = dwt::WidgetCreator<dwt::Grid>::create(
    parent, dwt::Grid::Seed(4, 2));

form->setSpacing(form->scale(6));
for(size_t row = 0; row < 3; ++row) {
    form->row(row).mode = dwt::GridInfo::AUTO;
}
form->row(3).mode = dwt::GridInfo::FILL;
form->row(3).align = dwt::GridInfo::STRETCH;
form->column(0).mode = dwt::GridInfo::AUTO;
form->column(1).mode = dwt::GridInfo::FILL;

auto* nameLabel = dwt::WidgetCreator<dwt::Label>::create(
    form, dwt::Label::Seed(_T("Name")));
auto* nameEdit = dwt::WidgetCreator<dwt::TextBox>::create(
    form, dwt::TextBox::Seed());
auto* notesLabel = dwt::WidgetCreator<dwt::Label>::create(
    form, dwt::Label::Seed(_T("Notes")));

dwt::TextBox::Seed notesSeed;
notesSeed.lines = 6;
notesSeed.style |= ES_MULTILINE | ES_WANTRETURN | WS_VSCROLL;
auto* notes = dwt::WidgetCreator<dwt::TextBox>::create(form, notesSeed);

form->setWidget(nameLabel, 0, 0);
form->setWidget(nameEdit, 0, 1);
form->setWidget(notesLabel, 1, 0);
form->setWidget(notes, 1, 1, 3, 1);
```

## Events, callbacks, and aspects

### Typed events first

Prefer the most specific typed event a widget exposes:

```cpp
button->onClicked([] { /* command */ });
combo->onSelectionChanged([combo] { /* read combo->getSelected() */ });
editor->onUpdated([] { /* text is changing */ });
window->onSized([](const dwt::SizedEvent& event) { /* layout */ });
table->onItemChanged([](const NMLISTVIEW& change) { /* inspect change */ });
```

Typed handlers translate `WPARAM` and `LPARAM`, choose the correct notification code, and apply the correct handled/default-processing convention.

Common translated event values are defined in `Events.h`:

| Event value | Important fields |
|---|---|
| `SizedEvent` | New client `size` and maximized/minimized/restored flags. |
| `MouseEvent` | Screen-coordinate `pos`, modifier state, and the pressed left/right/middle/X button. |
| `PointerEvent` | Pointer ID/type, screen position, flags, wheel delta, primary/contact/canceled state, raw pointer/touch/pen information, and history when the OS supplies it. |
| `TouchEvent` / `TouchPoint` | Validity plus all touch contacts, screen/contact rectangles, down/up/move/primary/palm/pen flags, and raw `TOUCHINPUT`. |
| `GestureEvent` | Validity, gesture type, screen position, flags, IDs, arguments, extra data, and raw `GESTUREINFO`. |
| `GestureNotifyEvent` | Validity, screen position, flags, instance ID, and raw notification structure. |
| `DpiChangedEvent` / `DpiResourceEvent` | Old/new DPI, suggested bounds, and old-to-new resource scaling helpers. |
| `SystemSettingsEvent` | Setting action/section plus high-contrast, animation, and accessibility-change information. |

### What an aspect is

An aspect is a small CRTP mixin that adds a related set of methods. The compiler exposes only capabilities inherited by that widget. Most widgets derive from `Control`, which already supplies the common aspects; data controls add collection, columns, selection, or custom-draw aspects as appropriate.

The complete aspect inventory is:

| Aspect | Main operations |
|---|---|
| `Activate` | `setActive`, `isActive`, `onActivate`. |
| `Border` | Simple, sunken, smooth-sunken, and raised border styles. |
| `Caption` | `setText`, `getText`, `length`, `onTextChanging`. |
| `Child` | Add or obtain one content child; adding another closes the old one. |
| `Children` | `addChild`, typed iteration through `getChildren<T>`, `removeChild`. |
| `Clickable` | Click, right-click, and double-click events using a widget's native notification codes. |
| `Closeable` | `close` and vetoable `onClosing`. |
| `Collection` | `size`, `empty`, `erase`, and `clear` over a widget-specific item type. |
| `Colorable` | `setColor(text, background)`; normally implemented through `WM_CTLCOLOR`. |
| `Columns` | Add/insert/erase columns; get/set definitions, widths, and display order. |
| `Command` | Command and system-command callbacks, plus `sendCommand`. |
| `ContextMenu` | `onContextMenu` with a `ScreenCoordinate`; return whether handled. |
| `CustomDraw` | Typed `NM_CUSTOMDRAW` callback returning `CDRF_*`/native results. |
| `Data` | Store and retrieve per-item `LPARAM` values; `findData` where supported. |
| `Dialog` | Dialog item access, item text, and dialog message routing. |
| `DragDrop` | Enable `WM_DROPFILES` and receive paths plus the drop point. |
| `Enabled` | Notification when `WM_ENABLE` changes enabled state. The base `Widget` performs `setEnabled`/`getEnabled`. |
| `EraseBackground` | Handle `WM_ERASEBKGND` with a `Canvas`, or suppress erasure. |
| `FileFilter` | Filters and modern file-dialog configuration shared by load/save dialogs. |
| `Fonts` | Reference-counted fonts, text measurement, and automatic DPI font recreation. |
| `Help` | Inherited help IDs and `WM_HELP` callbacks. |
| `Keyboard` | Focus, key/character/system-key handlers, and keyboard-state helpers. Return `true` from key callbacks to consume the key. |
| `MinMax` | `maximize`, `minimize`, and `restore` for frames/MDI children. |
| `Mouse` | Mouse buttons/move/wheel/leave, pointer capture, pointer, touch, and gesture events. Boolean callbacks indicate handled state. |
| `Painting` | `onPainting(PaintCanvas&)` and `onPrinting(Canvas&)`. |
| `Raw` | Handle an exact `Message` as untyped `WPARAM`/`LPARAM`. |
| `Scrollable` | Read scroll information, test the vertical end, and handle horizontal/vertical scroll codes. |
| `Selection` | Single-selection get/set, count, existence, and change notification. |
| `Sizable` | `resize`, centered owned windows, iconic/zoomed queries, and position/size/move events. |
| `Timer` | `setTimer`; return `true` to repeat or `false` to stop. Passing zero milliseconds kills the ID. |
| `Update` | Widget-specific value/text update notification. |
| `Visible` | Show/hide, visibility notification, redraw control, and rectangle invalidation. |

### Base callback API

Typed aspects are built on `Widget` callbacks:

```cpp
using Callback = std::function<bool(const MSG&, LRESULT&)>;

auto token = widget->addCallback(dwt::Message(WM_APP + 1), callback);
widget->clearCallback(dwt::Message(WM_APP + 1), token);

widget->setCallback(message, callback); // replaces callbacks for this Message
widget->clearCallbacks(message);        // removes all for this Message
```

Return `true` when the callback has handled the message and set `result` as required. Return `false` to observe the message while allowing the dispatcher/native control to process it. All callbacks registered for the matching `Message` are invoked in registration order; handled state is accumulated rather than short-circuiting the list.

`onCreate` observes `WM_CREATE`; `onDestroy` observes `WM_DESTROY`. Do not access the widget after native destruction completes.

### Message routing

DWT deliberately routes child notifications to the child wrapper that generated them:

- `WM_NOTIFY` is routed to the wrapper for `NMHDR::hwndFrom`.
- Control-originated `WM_COMMAND`, `WM_HSCROLL`, and `WM_VSCROLL` are routed to the HWND in `lParam`.
- Menu and accelerator `WM_COMMAND` messages, whose `lParam` is zero, stay with the receiving window.
- `WM_CTLCOLOR*` messages are routed to the affected child and normalized to DWT's `WM_CTLCOLOR` message key.

Therefore, register table notifications on the `Table`, button notifications on the `Button`, and menu command IDs on the containing window.

## `onRaw` and direct Win32 messages

`onRaw` is the last-resort handler for a message that has no suitable typed API:

```cpp
void onRaw(std::function<LRESULT(WPARAM, LPARAM)> callback,
           const dwt::Message& message);
```

The callback's return value becomes the native `LRESULT`, and the message is always marked handled. That last point is crucial: use `onRaw` when you intend to replace default processing. To inspect a message without swallowing it, use `addCallback` and return `false`.

### A private application message

```cpp
constexpr UINT WM_REFRESH_DATA = WM_APP + 1;

window->onRaw([reload](WPARAM reason, LPARAM) -> LRESULT {
    reload(static_cast<unsigned>(reason));
    return 0;
}, dwt::Message(WM_REFRESH_DATA));

window->postMessage(WM_REFRESH_DATA, 7, 0);
```

### A notification code

`Message` can match a plain message or a message plus its identifying code:

```cpp
table->onRaw([](WPARAM, LPARAM value) -> LRESULT {
    auto* info = reinterpret_cast<NMLVDISPINFO*>(value);
    if(info && (info->item.mask & LVIF_TEXT)) {
        // Supply callback text here.
    }
    return 0;
}, dwt::Message(WM_NOTIFY, LVN_GETDISPINFO));
```

Constructing `Message(WM_NOTIFY, code)` matches `NMHDR::code`. `Message(WM_COMMAND, value)` matches a control notification code for control-originated commands or the low-word command ID for menu/accelerator commands. `Message(WM_TIMER, id)` matches a timer ID, and system-command IDs are normalized with the standard `0xfff0` mask.

For complete access to `MSG::hwnd` and `MSG::message`, or to choose handled state dynamically, use `addCallback`:

```cpp
widget->addCallback(dwt::Message(WM_NCHITTEST),
    [](const MSG& msg, LRESULT& result) -> bool {
        // Inspect msg.hwnd, msg.wParam, and msg.lParam.
        // Set result and return true only when overriding the hit test.
        return false;
    });
```

### Raw-handler rules of thumb

- Prefer a typed widget event or aspect when one exists.
- Consult the Microsoft documentation for the exact `WPARAM`, `LPARAM`, and required `LRESULT` contract.
- Register the handler on the DWT widget to which message routing sends the notification.
- Keep any pointed-to notification structure within the callback; Windows usually owns it only for the duration of the message.
- Avoid capturing a widget pointer whose HWND may close before a posted message arrives.
- If default processing is still required, use `addCallback(... return false)` or call the appropriate native procedure deliberately; `onRaw` itself consumes the message.

## Painting, canvases, brushes, pens, and fonts

DWT drawing is a thin RAII wrapper over GDI. The `noEraseBackground()` calls in the examples apply to a `Window`, `Composite`, or custom control that inherits the `EraseBackground` aspect; a plain `Control` does not add that aspect by default.

### Canvas types

| Type | Use |
|---|---|
| `PaintCanvas` | Painting inside `WM_PAINT`. Its constructor calls `BeginPaint`; its destructor calls `EndPaint`. Obtain it through `onPainting`. |
| `UpdateCanvas` | Client-area drawing or measurement outside `WM_PAINT`; wraps `GetDC`/`ReleaseDC`. |
| `WindowUpdateCanvas` | Like `UpdateCanvas`, but uses `GetWindowDC` for the whole window. |
| `FreeCanvas` | Wraps a borrowed `HDC` without acquiring or releasing it. Useful in custom-draw notifications. |
| `CompatibleCanvas` | Owns a memory DC created with `CreateCompatibleDC`; select a suitable bitmap before drawing pixels. |
| `BufferedCanvas<CanvasType>` | Replaces the selected canvas with a compatible bitmap-backed DC and copies a requested rectangle to the source with `blast`. |

Never call `BeginPaint` again inside an `onPainting` callback; the supplied `PaintCanvas` already owns the paint cycle. Drawing performed with `UpdateCanvas` is not persistent—store state and call `redraw()` so the next `WM_PAINT` reproduces it.

### Basic custom painting

```cpp
surface->noEraseBackground();
surface->onPainting([surface, uiFont](dwt::PaintCanvas& canvas) {
    const dwt::Rectangle client(surface->getClientSize());
    canvas.fill(client, dwt::Brush(RGB(248, 250, 252)));

    dwt::Rectangle panel(
        surface->scale(16), surface->scale(16),
        client.width() - surface->scale(32),
        client.height() - surface->scale(32));

    dwt::Brush panelBrush(RGB(221, 237, 255));
    dwt::Pen border(RGB(0, 102, 204), dwt::Pen::Solid,
                    surface->scale(2));

    canvas.fill(panel, panelBrush);
    auto selectedPen = canvas.select(border);
    canvas.line(panel);

    if(uiFont) {
        auto selectedFont = canvas.select(*uiFont);
        canvas.setTextColor(RGB(24, 42, 64));
        auto transparent = canvas.setBkMode(true);
        dwt::Rectangle text = panel;
        canvas.drawText(_T("Custom DWT surface"), text,
                        DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
});
```

`canvas.select(resource)` returns a scope guard that restores the previous GDI object. Keep that returned object alive for the entire drawing interval. Likewise, `setBkMode(true)` returns a guard that restores the previous background mode.

Useful `Canvas` operations include:

- Lines: `moveTo`, `lineTo`, and `line` overloads.
- Shapes: `polygon`, `ellipse`, and `rectangle` using the selected pen and brush.
- Filling: `fill(Rectangle, Brush)` and `fill(Region, Brush)`.
- Text: `drawText`, `extTextOut`, `getTextExtent`, `getTextMetrics`, text/background colors, and alignment.
- Pixels and regions: `setPixel`, `getPixel`, flood fill, region inversion.
- Icons: `drawIcon`.
- Low-level access: `handle()` returns the HDC.

`drawText` takes a non-const `Rectangle&` because Win32 may update it, notably with `DT_CALCRECT`.

### Brushes, pens, and colors

`Brush` can wrap an existing `HBRUSH`, a DWT system color, or an RGB `COLORREF`:

```cpp
dwt::Brush background(dwt::Brush::Window); // borrowed GetSysColorBrush
dwt::Brush accent(RGB(0, 120, 215));        // owned solid brush
dwt::Brush borrowed(nativeBrush, false);    // DWT will not delete the handle
```

`Pen` can wrap an `HPEN` or create a solid/dash/dot/dash-dot/null/inside-frame pen:

```cpp
dwt::Pen pen(RGB(40, 40, 40), dwt::Pen::Dash, 1);
```

Use the Win32 `RGB(r, g, b)` macro to construct `COLORREF`. `Color::darken`, `lighten`, `alphaBlend`, and `predefined` provide small color helpers.

### Fonts

Create a `FontPtr` from `LOGFONT`, an existing `HFONT`, or a predefined stock font:

```cpp
LOGFONT lf = {};
lf.lfHeight = -::MulDiv(9, static_cast<int>(window->getDpi()), 72);
lf.lfQuality = CLEARTYPE_QUALITY;
_tcscpy_s(lf.lfFaceName, LF_FACESIZE, _T("Segoe UI"));
dwt::FontPtr uiFont(new dwt::Font(lf));

button->setFont(uiFont);
```

`setFont` retains the resource and registers automatic DPI recreation. `getTextSize` measures with the control's current font. `makeBold()` creates a related bold font.

### Flicker and buffering

For an entirely custom surface, suppress background erasure only if `WM_PAINT` paints every exposed pixel:

```cpp
surface->noEraseBackground();
```

For expensive drawing inside an existing paint callback, a buffer can wrap the supplied HDC:

```cpp
surface->onPainting([surface](dwt::PaintCanvas& paint) {
    const auto dirty = paint.getPaintRect();
    dwt::BufferedCanvas<dwt::FreeCanvas> buffer(paint.handle());
    buffer.fill(dwt::Rectangle(surface->getClientSize()),
                dwt::Brush(dwt::Brush::Window));
    // Draw the rest into buffer.
    buffer.blast(dirty);
});
```

The buffer destructor does not call `blast`; copying is explicit.

### Native custom draw

Common controls that implement `CustomDraw` expose `onCustomDraw` with the correct native structure type—for example `NMLVCUSTOMDRAW` for `Table` and `NMTVCUSTOMDRAW` for `Tree`. Examine `dwDrawStage`, set fields or draw using `FreeCanvas(data.nmcd.hdc)`, and return the proper `CDRF_*` flags. Multiple DWT custom-draw handlers can coexist; the aspect preserves an earlier non-default result.

## Widget catalog

All common `Control` operations—visibility, enabled state, focus, bounds, fonts, colors, input, painting, raw messages, help, timers, closing, and drag/drop—remain available unless the widget has a specialized implementation.

### Text, buttons, and input

| Widget | Purpose and primary API |
|---|---|
| [`Button`](dwt/include/dwt/widgets/Button.h) | Push, command-link, split, default, icon, and bitmap buttons. Use `onClicked`, `setNote`, `setImage`, `setImageList`, elevation shield, split/drop-down state, and hot/focus/drop-down events. |
| [`CheckBox`](dwt/include/dwt/widgets/CheckBox.h) | Button-derived check box with `getChecked` and `setChecked`. |
| [`RadioButton`](dwt/include/dwt/widgets/RadioButton.h) | Check-box-derived radio button. Group behavior follows Win32 styles and tab/group ordering. |
| [`TextBox`](dwt/include/dwt/widgets/TextBox.h) | Native edit control. Text/caret/selection, line access, find/replace, read-only/password/number/case styles, cue text, scrollbars, popup menu, and `onUpdated`. Set `Seed::lines` and edit styles for multiline use. |
| [`RichTextBox`](dwt/include/dwt/widgets/RichTextBox.h) | Rich Edit control with RTF insertion, stable appending, link/format support through native messages, search highlighting, selection/position helpers, and rich text/color behavior. |
| [`ComboBox`](dwt/include/dwt/widgets/ComboBox.h) | Editable drop-down list. `addValue`, `insertValue`, item data/collection operations, find, selection, and access to attached list/edit subcontrols. |
| [`Link`](dwt/include/dwt/widgets/Link.h) | SysLink wrapper. Seed can treat the caption as a URL; `setLink` updates an indexed link. |
| [`DateTime`](dwt/include/dwt/widgets/DateTime.h) | Date/time picker with optional-none state, ranges, formatting callbacks, month-calendar access, colors, and ideal-size queries. |
| [`MonthCalendar`](dwt/include/dwt/widgets/MonthCalendar.h) | Single/multiple date selection, ranges, today value, view, calendar grid information, colors, hit testing, and selection/view events. |
| [`Slider`](dwt/include/dwt/widgets/Slider.h) | Trackbar with range/position, selection range, ticks, line/page increments, buddy windows, tooltip, orientation, reversal, and pre-move notification. |
| [`Spinner`](dwt/include/dwt/widgets/Spinner.h) | Up-down control with range/value, optional edit buddy, and delta/position update callback. |

### Display and data controls

| Widget | Purpose and primary API |
|---|---|
| [`Label`](dwt/include/dwt/widgets/Label.h) | Text or icon static control with preferred-size measurement and click/double-click support. |
| [`ProgressBar`](dwt/include/dwt/widgets/ProgressBar.h) | Range, position, step, marquee, state (`normal`, error, paused), foreground/background colors, orientation, and custom draw. |
| [`Header`](dwt/include/dwt/widgets/Header.h) | Standalone header items, ordering, widths, check boxes, sort arrows, images, filters, drag/track/drop-down/overflow notifications, focus, and hit testing. |
| [`Column`](dwt/include/dwt/widgets/Column.h) | Column value object: header, width, and left/right/center alignment. `SIZE_TO_CONTENT` and `SIZE_TO_HEADER` are the auto-width sentinels. |
| [`Table`](dwt/include/dwt/widgets/Table.h) | List-view wrapper supporting report/icon/list/tile views, rows and columns, per-item data, selection/checks, sorting, image lists, groups, virtual callback text, footers, work areas, label editing, tooltips, and detailed notifications. |
| [`Tree`](dwt/include/dwt/widgets/Tree.h) | Tree view with insertion/navigation, item data/text, selection and multiselect, checks/extended check states, images, edit labels, drag, custom draw, accessibility mapping, and optional multi-column header. |
| [`VirtualTree`](dwt/include/dwt/widgets/VirtualTree.h) | Tree-derived virtualized hierarchy that retains logical items while displaying the expanded visible subset; use the familiar tree insert/expand/select/check surface. |
| [`TableTree`](dwt/include/dwt/widgets/TableTree.h) | Table-based hierarchical rows keyed by `LPARAM`; add parent/child relations, expand/collapse, and normally serve display text through `LVN_GETDISPINFO`. |

### Containers, navigation, and chrome

| Widget | Purpose and primary API |
|---|---|
| [`Composite`](dwt/include/dwt/widgets/Composite.h) | Multi-child base with caption, child iteration, and erase-background support; no automatic layout. |
| [`Container`](dwt/include/dwt/widgets/Container.h) | Composite that fills its first child by default. |
| [`Grid`](dwt/include/dwt/widgets/Grid.h) | Row/column layout with static, automatic, and fill tracks, alignment, spacing, spans, preferred size, and dynamic tracks. |
| [`GroupBox`](dwt/include/dwt/widgets/GroupBox.h) | Titled single-child frame that accounts for title and padding in preferred size/layout. |
| [`ScrolledContainer`](dwt/include/dwt/widgets/ScrolledContainer.h) | Single-child viewport with horizontal/vertical scrollbars, keyboard scrolling, preferred content size, and an accessibility scroll provider. |
| [`SplitterContainer`](dwt/include/dwt/widgets/SplitterContainer.h) | Multiple child panes separated by generated `Splitter` controls. Configure orientation/start position, move splitters by relative position, or maximize one pane. |
| [`Splitter`](dwt/include/dwt/widgets/Splitter.h) | Draggable divider created/managed by a splitter container; supplies range-value accessibility. |
| [`TabView`](dwt/include/dwt/widgets/TabView.h) | Container pages as tabs. Add/remove/activate/mark pages, icons, context menus, keyboard cycling, close behavior, owner drawing, and taskbar-tab integration. A page's caption supplies its tab title. |
| [`Rebar`](dwt/include/dwt/widgets/Rebar.h) | Hosts toolbar or other child bands; add/remove bands and call `refresh` after contents change. |
| [`ToolBar`](dwt/include/dwt/widgets/ToolBar.h) | Named buttons, image lists, callbacks/drop-downs, visibility/enabled/checked state, persistent layout, customization, hit testing, and tooltips. |
| [`StatusBar`](dwt/include/dwt/widgets/StatusBar.h) | Multiple sized parts with text, icons, tooltips, help, click handlers, or embedded controls. One configured part can fill remaining width. |
| [`ToolTip`](dwt/include/dwt/widgets/ToolTip.h) | Tooltip window for one or more widgets, static or callback text, delay/width/title/margins/colors/theme, activation, and link events. |

### Top-level and MDI windows

| Widget | Purpose and primary API |
|---|---|
| [`Frame`](dwt/include/dwt/widgets/Frame.h) | Base for top-level frames: activation, minimize/maximize/restore, icons, frame buttons, and animation. |
| [`Window`](dwt/include/dwt/widgets/Window.h) | Normal top-level application window. |
| [`ModelessDialog`](dwt/include/dwt/widgets/ModelessDialog.h) | Self-destroying dialog-style frame that remains open while the main message loop runs. |
| [`ModalDialog`](dwt/include/dwt/widgets/ModalDialog.h) | Non-self-deleting modal frame intended for subclassing/C++ scope ownership; configure, create, then `show`, and finish with `endDialog`. |
| [`MDIFrame`](dwt/include/dwt/widgets/MDIFrame.h) | Top-level MDI frame. Its seed accepts the Window menu and first child command ID; it owns an `MDIParent`. |
| [`MDIParent`](dwt/include/dwt/widgets/MDIParent.h) | MDI client with child activation, next/previous, cascade/tile/arrange, minimize/close all, menu switching, and background color/image. |
| [`MDIChild`](dwt/include/dwt/widgets/MDIChild.h) | MDI document child; derive a class, call `createMDIChild`, and use `activate` plus frame/control aspects. |

## Menus, dialogs, notifications, and the taskbar

### Menus and accelerators

Create a `Menu` through `WidgetCreator<Menu>`. A non-popup seed represents a menu bar; `appendPopup` creates owned submenus, while `appendItem` installs a callback and returns an ID.

```cpp
dwt::Menu::Seed menuSeed(false);
menuSeed.popup = false;
menuSeed.commandMessages = true;
auto menuBar = dwt::WidgetCreator<dwt::Menu>::create(window, menuSeed);
auto* file = menuBar->appendPopup(_T("&File"));
file->appendItem(_T("E&xit\tAlt+F4"), [window] { window->close(); });
menuBar->setMenu();
```

For a derived frame whose C++ object exists before its HWND, the alternative is to create the menu first and pass `menuBar->handle()` in the frame seed's `menuHandle`, as the MDI example does.

Menus support owner drawing, icons, titles/sidebar, checks, enabled/default state, text updates, removal, and popup opening at a `ScreenCoordinate`.

`Control::addAccel(fVirt, key, callback)` adds an accelerator before or after creation; call `initAccels()` if accelerators were added after creation.

### Simple and task dialogs

- `MessageBox(parent).show(...)` wraps the native message box and returns a typed result.
- `TaskDialog` is a fluent wrapper for title, instruction, content/footer, expanded/verification text, common/custom/radio buttons, command links, progress, icons, flags, width, events, and a structured `Result`.
- `ColorDialog::open` updates `ColorParams` only on acceptance.
- `FontDialog::open` updates `LOGFONT` and color on acceptance; `Options` controls strikeout, underline, color, and preview background.

### File and folder dialogs

`LoadDialog` and `SaveDialog` share the `FileFilter` aspect:

```cpp
dwt::LoadDialog dialog(window);
dialog.setTitle(_T("Open document"))
      .addFilter(_T("Text files"), _T("*.txt;*.log"))
      .addFilter(_T("All files"), _T("*.*"))
      .setDefaultExtension(_T("txt"));

dwt::tstring path;
if(dialog.open(path)) {
    // Use path.
}
```

The modern dialog surface also supports client GUID persistence, initial folders, custom places, raw options, event callbacks, custom controls, filesystem forcing, shell-item results, and multiple selection (`LoadDialog::openMultiple`/`openShellItems`).

`FolderDialog` selects folders and exposes the same modern events, custom controls, places, initial folder, and shell-item support, with a rooted legacy fallback when required.

### Tray notifications

`Notification` represents a notification-area icon and balloon queue. Create it through its `NotificationPtr` factory type, retain the unique pointer, set its visible state, and register click/context/double-click/balloon callbacks. `setGuid` gives the icon stable identity. `getLastNotifyError` diagnoses shell failures.

### Taskbar integration

`Taskbar` supplies:

- Process/window application IDs.
- Jump lists with named categories and user tasks.
- Window progress state/value and overlay icon.
- Thumbnail toolbars, tooltip, and thumbnail clip.
- Registered tab previews, active ordering, tab properties, and per-tab thumbnail controls.

`TabView` inherits `Taskbar`; call `tabView->initTaskbar(window)` before using its taskbar-tab integration. COM/OLE is initialized automatically by DWT's normal executable entry point.

## The application loop, async work, and threads

`Application` is a singleton passed to `dwtMain` and available as `Application::instance()`.

### Message-loop APIs

- `run()` blocks until `WM_QUIT` is processed.
- `dispatch()` processes one queued native message or one asynchronous callback and reports whether more work was immediately done/allowed.
- `sleep()` waits for messages or registered wait handles.
- `processMessages()` drains currently pending work without blocking. It can invoke arbitrary callbacks and is therefore reentrant.
- `wake()` wakes the GUI thread.
- `addFilter`/`removeFilter` install pre-translation message filters.
- `addWaitEvent` integrates up to the Windows wait-object limit with the UI loop; remove the event before its handle becomes invalid.

Call `processMessages()` during a long GUI-thread operation only when the operation is designed for reentrancy: the user can click controls, close windows, and trigger nested code before it returns. Prefer moving long work to a worker thread.

### Returning work to the UI thread

`Application::callAsync` is safe to enqueue from another thread. Callbacks are processed on the GUI thread in FIFO order:

```cpp
HWND target = window->handle();
std::thread([target] {
    auto value = loadExpensiveData();
    dwt::Application::instance().callAsync([target, value] {
        if(auto* live = dwt::hwnd_cast<dwt::Window*>(target)) {
            live->setText(value);
        }
    });
}).detach();
```

`widget->callAsync(callback)` is a convenience that checks whether the captured HWND is still a window before calling the function. It does not make arbitrary widget access thread-safe; do not read or mutate HWND-backed controls on the worker thread.

### Startup data

`getCommandLine().getParams()` provides argv-like parameters; `getParamsRaw()` exposes the raw process command line. `getModulePath()` returns the executable directory and `getModuleFileName()` the complete executable path. `getCmdShow()` exposes the initial show command.

### Shared-library mode

When compiling with `DWT_SHARED`:

- The host/library integration must call `Application::init()` and `Application::uninit()`.
- DWT does not supply or run the host's message loop.
- `callAsync` executes synchronously because DWT cannot inject its queue into an unknown host pump.
- DWT accelerators are not handled by the normal DWT loop.

Use the host's thread-marshalling and accelerator mechanisms in this mode.

## DPI, themes, and accessibility

### DPI-aware layout and resources

DWT enables per-monitor DPI awareness during normal initialization. `Widget` provides:

```cpp
unsigned dpi = widget->getDpi();
int px = widget->scale(12);                // 12 logical pixels at 96 DPI
dwt::Point size = widget->scale({16, 16});
int metric = widget->getSystemMetric(SM_CXVSCROLL);
widget->adjustWindowRect(nativeRect, hasMenu);
```

Use `scale` for custom margins, hit targets, pen widths, manually positioned controls, and icon dimensions. Grid `STATIC` track sizes are already scaled; grid spacing is not.

`onDpiResourcesChanged` runs when DPI changes and before post-message layout. Recreate cached icons, image lists, bitmaps, and drawing resources there. Fonts set through the `Fonts` aspect are recreated automatically. `onDpiChanged` receives old/new DPI and the suggested bounds; top-level windows are moved to those suggested bounds before callbacks run.

### Theme and settings changes

Register:

```cpp
widget->onThemeChanged(callback);
widget->onSystemColorsChanged(callback);
widget->onSystemSettingsChanged(
    [](const dwt::SystemSettingsEvent& event) {
        if(event.accessibilityChanged()) { /* refresh policy */ }
    });
```

After these appearance messages, `Widget` calls `layout()` and invalidates the window. `Widget::isHighContrast()` reports current high-contrast mode.

`Theme` wraps UxTheme: load a class for a widget, test it as a boolean, draw themed background/text, format rectangles, query colors/part sizes, and detect partially transparent backgrounds. A loaded theme can follow `WM_THEMECHANGED` automatically.

### Accessibility

Standard Win32 controls already expose native accessibility providers. `Widget::enableAccessibility` is intended for custom HWND surfaces and custom logical item models:

```cpp
custom->enableAccessibility(dwt::accessibility::Custom);
custom->setAccessibleName(_T("Waveform"));
custom->setAccessibleHelpText(_T("Shows the current audio waveform"));
custom->setAccessibleKeyboardFocusable(true);
```

For richer custom controls, set:

- `RangeValueProvider` for numeric get/set/min/max/change behavior.
- `ScrollProvider` for scroll actions, percentages, view sizes, and scrollable axes.
- `ItemProvider` for logical item hierarchy, names, bounds, selection, expansion, invocation, and focus.

Call `raiseAccessibleEvent`, `raiseAccessibleItemEvent`, or `raiseAccessibleStructureChanged` when state changes. Several DWT custom controls—including grid, tabs, splitters, scrolled containers, and virtual hierarchies—configure appropriate providers internally.

## Resources and utility APIs

### GDI and image resources

All resource wrappers expose `handle()` and accept an ownership flag when wrapping a native handle.

| Resource | Main use |
|---|---|
| [`Bitmap`](dwt/include/dwt/resources/Bitmap.h) | Load from HBITMAP, resource ID, or file; query size; create a resized copy. |
| [`Brush`](dwt/include/dwt/resources/Brush.h) | Wrap/create solid or system brushes for fills and selected GDI drawing. |
| [`Pen`](dwt/include/dwt/resources/Pen.h) | Wrap/create pens with common line styles and width. |
| [`Font`](dwt/include/dwt/resources/Font.h) | Wrap/create fonts, inspect `LOGFONT`, and make a bold copy. |
| [`Icon`](dwt/include/dwt/resources/Icon.h) | Load from HICON, resource, or file; query size; render a resized owned copy. |
| [`ImageList`](dwt/include/dwt/resources/ImageList.h) | Add icons/bitmaps, obtain images/icons, resize the list, and manage background color. |
| [`Region`](dwt/include/dwt/resources/Region.h) | Rectangle or polygon region and transformed copies. |

Passing `owned = false` means DWT will not destroy the native handle. Never mark a stock/system/shared handle as owned.

### Other top-level helpers

| API | Purpose |
|---|---|
| `Clipboard::setData` | Put a `tstring` on the clipboard using a control as owner. |
| `Cursor::getWaitCursor` | Scope guard that installs the wait cursor and restores the previous cursor. |
| `LibraryLoader` | RAII dynamic-library loader with name/ordinal procedure lookup. |
| `CommandLine` | Split and raw command-line access through `Application`. |
| `Point`, `Rectangle` | Native-interoperable geometry, arithmetic, containment, and monitor visibility. |
| `ScreenCoordinate`, `ClientCoordinate` | Explicit coordinate-space conversions. |
| `DWTException`, `Win32Exception` | Toolkit and translated last-error exceptions. |
| `Texts` | Toolkit-localized standard command labels such as undo, cut, copy, paste, delete, resize, and close. |
| `Atom`, `GlobalAtom` | RAII wrappers for local/global Windows atom-table entries. |
| `Dispatcher` family | Registers DWT/custom window classes and chains to `DefWindowProc`, MDI procedures, or a superclassed native procedure. Most application code chooses a dispatcher only when implementing a custom control. |
| `Message` | Comparable/hashable message key with special normalization for command, notify, timer, system-command, menu-command, and control-color messages. |
| `Version.h` | `DWT_VERSION_*`, `DWT_VERSION_STRING`, and `DWT_MAKE_VERSION` compile-time version macros. |

### Utilities

| Header/API | Purpose |
|---|---|
| `util::HoldRedraw` | Scope guard around `WM_SETREDRAW` for a batch update. Redraw after the guard if necessary. |
| `util::HoldResize` | Batch child positioning through deferred window positioning, with a normal fallback. |
| `util::DateTime` / `TimeSpan` | SYSTEMTIME/date arithmetic, formatting, comparison, now/minimum, and Unix timestamp conversion. |
| `util::RegKey` | Registry key open/create, enumeration, deletion, and DWORD/string/binary reads/writes. |
| `util::UnicodeConverter` | Explicit conversion among UTF-8/UTF-7/ANSI/OEM/etc. and wide/current-build strings. |
| `util::StringUtils` | Menu-ampersand escaping and length cutting. |
| `util::tstream` | `TCHAR`-width stream and file-stream aliases. |
| `util::check` | Debug-only `dwtassert` and `dwtWin32Assert`; checks compile away outside `_DEBUG`. |
| `util::GDI` | Image-list icon merging and legacy DPI factor helper. Prefer per-widget `scale` for new layout code. |
| `util::win32::FileDialog` | Low-level modern file-dialog results, events, options, custom controls, and shell items used by the public dialogs. |
| `util::win32::Dpi` | Lower-level DPI-awareness, scaling, metrics, system-parameter, and window-rectangle fallbacks. |
| `util::win32::Version` | Runtime Windows-version feature check. |

## Writing a custom control

A custom DWT control normally derives from `Control`, declares `ThisType`, `ObjectType`, and a `Seed`, forwards construction to a dispatcher, and calls `Control::create`.

```cpp
#include <dwt/CanvasClasses.h>
#include <dwt/WidgetCreator.h>
#include <dwt/aspects/EraseBackground.h>
#include <dwt/resources/Brush.h>
#include <dwt/widgets/Control.h>

class ColorPanel :
    public dwt::Control,
    public dwt::aspects::EraseBackground<ColorPanel> {
public:
    using ThisType = ColorPanel;
    using ObjectType = ColorPanel*;

    struct Seed : dwt::Control::Seed {
        using WidgetType = ThisType;

        explicit Seed(COLORREF color_ = RGB(240, 240, 240))
            : dwt::Control::Seed(
                  WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN),
              color(color_) {}

        COLORREF color;
    };

    explicit ColorPanel(dwt::Widget* parent)
        : dwt::Control(parent, dwt::NormalDispatcher::getDefault()) {}

    void create(const Seed& seed = Seed()) {
        color = seed.color;
        dwt::Control::create(seed);
        enableAccessibility(dwt::accessibility::Custom);
        setAccessibleName(_T("Color panel"));
        noEraseBackground();
        onPainting([this](dwt::PaintCanvas& canvas) {
            canvas.fill(dwt::Rectangle(getClientSize()), dwt::Brush(color));
        });
    }

    dwt::Point getPreferredSize() override {
        return scale(dwt::Point(160, 100));
    }

    void setPanelColor(COLORREF value) {
        color = value;
        redraw();
    }

private:
    COLORREF color = RGB(240, 240, 240);
};
```

Create it exactly like a built-in widget:

```cpp
auto* panel = dwt::WidgetCreator<ColorPanel>::create(parent, ColorPanel::Seed());
```

`NormalDispatcher::getDefault()` gives a DWT-owned window class. To wrap/superclass a native Windows control, declare its `windowClass` and use `ChainingDispatcher::superClass<ThisType>()`, as the built-in native controls do.

For a production custom control:

- Override `getPreferredSize` for useful `AUTO` grid behavior.
- Override `layout` if it owns children.
- Use typed aspects and `addCallback`; override `handleMessage` only when message processing must surround the base implementation.
- Scale logical geometry and recreate cached resources on DPI changes.
- Paint the complete invalid region if background erasure is suppressed.
- Enable accessibility and expose range, scroll, or item providers when native providers cannot describe the control.
- Preserve the self-deleting HWND lifetime model unless there is a deliberate reason, as with `ModalDialog`.

## Common failure modes

### The window or controls are invisible

Top-level `Window` and most child-control seeds do not include `WS_VISIBLE`. Call `setVisible(true)` after setup or add `WS_VISIBLE` to the seed. A hidden control does not contribute to a grid's `AUTO` preferred size.

### Closing the main window leaves the process running

`Application::run()` exits on `WM_QUIT`, not merely because no window remains. Register `window->onDestroy([] { ::PostQuitMessage(0); });` on the last/main window.

### The content does not resize with the window

`Window` is a `Composite`, not a `Container`; its default `layout()` is empty. Resize the root container in `onSized`, or derive a window and override `layout()`.

### A grid child is missing or not moving

Verify that the control's parent is the grid, row/column indices and spans are valid, tracks exist, and the child is visible. The one-argument `setWidget(control)` intentionally disables grid resizing for that child. Call `layout()` after dynamic metadata changes.

### A callback crashes after a close

Normal widget wrappers delete themselves during `WM_NCDESTROY`. Do not retain owning smart pointers to them, do not delete them, and do not dereference cached raw pointers after close. Prefer callbacks scoped to the widget itself, clear longer-lived references in `onDestroy`, and use `Widget::callAsync` for widget-bound queued work.

### A native control stopped doing its normal work

`onRaw` always reports handled. If the goal was only to observe a message, replace it with `addCallback` and return `false`. For typed key/mouse handlers, return `false` when native processing should continue.

### Painting flickers or disappears

Persistent visuals belong in `onPainting`, backed by application state. `UpdateCanvas` drawing is temporary. Suppress `WM_ERASEBKGND` only when the paint callback covers the entire surface, and use a `BufferedCanvas` for costly multi-step drawing.

### Controls look old or newer messages/styles do not work

Embed the Common Controls v6 manifest and initialize through the normal DWT entry point. Also ensure the repository's Windows target macros are used before Windows headers.

### Creation throws `Win32Exception("Unable to create widget")`

The most common cause is replacing a child control's style and losing `WS_CHILD`. Start from the widget's default `Seed` and OR in additional styles.

### Text types do not compile

The shipped build is Unicode. Use `dwt::tstring`, `_T(...)`, and the conversion helpers for external UTF-8 data. Avoid mixing `std::string` with `std::wstring` by accident.

## Examples and source reference

The guide describes the supported public surface, while the headers remain the exact signature and native-flag reference.

- [examples/MultiControlExample/main.cpp](examples/MultiControlExample/main.cpp) is the broad control gallery: grids, nested containers, every major widget, dialogs, resources, notifications, taskbar integration, DPI, and events.
- [examples/CustomDrawExample/main.cpp](examples/CustomDrawExample/main.cpp) demonstrates brushes, pens, canvases, custom draw stages, themes, raw callbacks, and owner-painted controls.
- [examples/MDIExample/main.cpp](examples/MDIExample/main.cpp) demonstrates MDI frames/children, manual layout, custom controls, menus, accelerators, dialogs, and `onPainting`.
- [examples/RichTextCapabilities/main.cpp](examples/RichTextCapabilities/main.cpp) focuses on Rich Edit text, formatting, links, selection, find, and raw `EN_LINK` handling.
- [examples/FrameworkValidation/main.cpp](examples/FrameworkValidation/main.cpp) demonstrates grids, splitters, scrolling, virtual controls, DPI callbacks, pointer/touch/gesture input, and accessibility providers.
- [examples/UIDesignerExample/main.cpp](examples/UIDesignerExample/main.cpp) demonstrates an interactive grid-based form designer with all grid-placeable DWT widgets, reliable delete/reselection, row and column spans, axis modes/sizes/alignment, widget-specific generated layout code, and live preview windows.
- [tests/FrameworkTests/main.cpp](tests/FrameworkTests/main.cpp) provides small executable examples for lifecycle, layout, message, hierarchy, resource, and compatibility behavior.
- Public headers are under [dwt/include/dwt](dwt/include/dwt), implementation is under [dwt/src](dwt/src), and build definitions are under [projects](projects).

When a typed DWT method does not expose a newly added Windows feature, use the same progression DWT itself uses: typed aspect or widget API first, then `sendMessage`, then `addCallback`, and finally `onRaw` when intentionally replacing native processing.
