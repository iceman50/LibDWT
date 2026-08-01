# LibDWT Visual UI Designer Example

This example is a small, deliberately inspectable foundation for a future
LibDWT form designer. It builds its own interface from nested `dwt::Grid`
containers and keeps a simple model separate from the live controls.

The example demonstrates:

- a palette for all 27 concrete DWT child widgets that can be placed in a
  `Grid`: `Button`, `CheckBox`, `ComboBox`, `Container`, `DateTime`, `Grid`,
  `GroupBox`, `Header`, `Label`, `Link`, `MonthCalendar`, `ProgressBar`,
  `RadioButton`, `Rebar`, `RichTextBox`, `ScrolledContainer`, `Slider`,
  `Spinner`, `SplitterContainer`, `StatusBar`, `Table`, `TableTree`, `TabView`,
  `TextBox`, `ToolBar`, `Tree`, and `VirtualTree`;
- selectable designer-item containers with a painted selection border;
- deleting a selected control and immediately selecting any surviving control;
- editing a control's caption, row, column, row span, and column span;
- customizing row and column `STATIC`, `FILL`, and `AUTO` modes, sizes, and all
  four `GridInfo` alignment modes;
- rejecting overlapping spans and spans outside the design grid;
- regenerating widget-specific `WidgetCreator<Grid>` C++ after each change;
- opening a functional preview window from the current model; and
- a `--self-test` workflow that creates every palette widget and exercises add,
  select, edit, span, grid mode/alignment, remove/reselect, generate, preview,
  resize, and close operations.

Top-level windows and dialogs, menus, notifications, and tooltip services are
not palette items because they are host-level or non-layout objects rather than
child controls that can legally occupy a grid cell. `Splitter` is managed
automatically by `SplitterContainer` and is represented through that widget.

Build it from `projects/msvc/LibDWT.sln`, or with MinGW-w64:

```powershell
mingw32-make -C projects/mingw-w64 build ARCH=x64 CONFIG=debug
```

Run the interactive example:

```powershell
projects/mingw-w64/build/x64/debug/bin/UIDesignerExample.exe
```

This is an example rather than a complete designer. Natural next layers are
drag-and-drop placement, undo/redo, serialization, per-widget style editors,
and generation of a complete window class rather than only its grid
construction.
