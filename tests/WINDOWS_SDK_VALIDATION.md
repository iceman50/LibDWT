# Windows SDK Validation

This checklist covers live validation that cannot be proven by the default
headless `FrameworkTests` run. The default test executable remains suitable for
CI; the live checks below are opt-in so they can use the current desktop session
without making ordinary builds fragile.

## Automated Checks

Run:

```powershell
FrameworkTests.exe --live-validation
```

Expected result:

- The default headless framework tests pass.
- The live validation checks create only hidden windows.
- Synthetic `WM_DPICHANGED` dispatch calls DPI callbacks, calls resource
  callbacks, and applies suggested bounds without crashing.
- Synthetic `WM_SETTINGCHANGE` dispatch reaches typed settings callbacks and
  classifies accessibility changes.
- The UI Automation provider can be enabled and returned through the
  `WM_GETOBJECT`/`UiaRootObjectId` path without crashing.
- Single-monitor systems can run the automated checks. Real monitor-transition
  validation still requires the manual pass below.

## Manual Multi-Monitor DPI Pass

Use a Per-Monitor V2 manifest build on a machine with at least two monitors set
to different scale factors.

- Open a representative sample with native controls and custom controls.
- Move the top-level window between monitors.
- Confirm `WM_DPICHANGED` applies the suggested bounds and layout remains
  coherent after each move.
- Confirm assigned fonts and owned image lists resize once per transition.
- Confirm owner-drawn or custom cached bitmaps/icons are either recreated through
  `onDpiResourcesChanged` or documented as a remaining policy gap.
- Repeat while the window is restored, maximized, and minimized/restored.

## Manual UI Automation Pass

Use Inspect, Accessibility Insights, or another UIA client.

- Verify custom containers appear in the fragment tree.
- Verify `TableTree`, `VirtualTree`, and `TabView` expose logical children with
  names, bounds, focus, selection, expansion, and invocation where applicable.
- Verify `Splitter` exposes `RangeValue` and `ScrolledContainer` exposes
  `Scroll`.
- Verify native child controls are reachable through provider bridging.
- Exercise selection, expansion, invocation, and structure changes while the
  UIA client is attached; no stale-element crashes or missing-provider crashes
  should occur.

## Manual High-Contrast And Text-Scaling Pass

Run representative custom-drawn examples before and after changing Windows
contrast and text-size settings.

- Toggle high contrast and confirm custom controls repaint without stale colors.
- Confirm focus rectangles, selection states, disabled states, and hover states
  remain visible.
- Increase system text size and confirm text does not clip or overlap in custom
  controls.
- Confirm `WM_SETTINGCHANGE`, `WM_SYSCOLORCHANGE`, and `WM_THEMECHANGED` paths
  relayout/repaint affected widgets without crashes.

Record failures against the relevant audit item in `WINDOWS_SDK_FEATURE_AUDIT.md`
before implementing fixes, so validation findings remain traceable.
