# LibDWT

LibDWT is a Windows GUI toolkit repository built on the original base of DC++'s implementation of DWT.

This repository contains the DWT library sources, example applications, build projects for MSVC and MinGW-w64, supporting scripts, and validation tests.

## Origin

The bundled DWT sources trace back to the DC++ implementation of DWT. The upstream DWT material in this repository also notes that DWT was originally based on SmartWin++ before diverging significantly.

## Repository Layout

- `dwt/include` and `dwt/src`: core library headers and implementation files.
- `examples`: sample applications demonstrating the toolkit.
- `tests`: framework and Windows SDK validation material.
- `projects/msvc`: Visual Studio solution and build assets.
- `projects/mingw-w64`: MinGW-w64 makefiles and build assets.
- `scripts`: helper scripts for building, packaging, and maintenance.

## Building

This repository includes build infrastructure for both:

- MSVC via the Visual Studio solution under `projects/msvc`
- MinGW-w64 via the makefiles under `projects/mingw-w64`

Prebuilt output folders are also present under `Builds` for debug and release configurations.

## License

This project is licensed under the GNU General Public License v2. See [LICENSE](LICENSE) for the full license text.

## Attribution

This README was created with assistance from OpenAI Codex 5.5 and GitHub Copilot.

OpenAI Codex 5.5 and GitHub Copilot were used side-by-side throughout the development process to help create LibDWT as it exists today.

They were also used for error checking, debugging, and documentation work, and to help create and refine the repository's build scripts and project files.