# GXDE Bernese Project Guide

## Project Overview

GXDE Bernese is a GPL-3.0 licensed compatibility library for GXDE. It
replaces application dependencies on unstable Qt private APIs with stable
implementations based on public APIs. This reduces breakage and rebuilds when
Qt changes, especially on rolling-release distributions.

The project is a C++20 shared library built with CMake 3.16 or newer. It can be
built against either Qt 5 or Qt 6 (Qt 6 is the default) and also uses X11, XCB,
XCB RandR, and Wayland client protocols. The library produces
`libgxde-bernese-qt5` or `libgxde-bernese-qt6`, depending on configuration.
There is no standalone application to run.

## Repository Layout

- `src/`: Library implementation and public headers.
  - `image_utils/`: Image processing helpers.
  - `text_layout_utils/`: Text measurement and layout helpers.
  - `file_dialog_utils/`: File dialog integration helpers.
  - `x11_info/`: X11 display and connection information.
  - `window_utils/`: Window geometry, stacking, and general utilities.
  - `window_effects/`: Window effects, including Wayland implementations.
  - `backdrop_origin/`: Backdrop origin support.
  - `paint_utils/`, `platform_utils/`, `tab_bar_utils/`: Additional utility
    areas; inspect their contents before making changes.
- `src/window_effects/protocols/`: Wayland XML protocol definitions. CMake
  generates client headers and protocol C sources with `wayland-scanner`.
- `tests/`: QtTest-based tests, public API checks, and header self-containment
  checks.
- `cmake/`: Installed CMake package and pkg-config templates.
- `debian/`, `arch/`: Debian and Arch Linux packaging.
- `docs/`: Generated Doxygen documentation; `docs/man/` is installed.
- `tools/check_style.py`: Repository-wide formatting, lint, and naming checks.
- `AGENTS.md`: Detailed contributor and AI coding instructions.
- `CONTRIBUTING.md`: Contribution, issue, merge request, and AI policies.

## Dependencies and Build Configuration

Required build-time dependencies include:

- CMake 3.16 or newer and a C++20 compiler
- Qt 5 or Qt 6 development packages for Core, Gui, Widgets, and Test
- X11 development files
- pkg-config modules `xcb`, `xcb-randr`, and `wayland-client`
- `wayland-scanner`
- Python 3 for the style-check target
- `clang-format-19`, `clang-tidy-19`, and `cpplint` for full style checks

Configure and build a Qt 6 development tree with tests enabled:

```sh
cmake -S . -B build -DBERNESE_QT_MAJOR_VERSION=6 -DBUILD_TESTING=ON
cmake --build build
```

For Qt 5, use a separate build directory so the cached Qt selection does not
conflict with the Qt 6 build:

```sh
cmake -S . -B build-qt5 -DBERNESE_QT_MAJOR_VERSION=5 -DBUILD_TESTING=ON
cmake --build build-qt5
```

`BERNESE_QT_MAJOR_VERSION` accepts only `5` or `6`. Compilation commands are
exported automatically to `compile_commands.json` in the build directory.

To install into a chosen staging prefix:

```sh
cmake --install build --prefix /desired/prefix
```

The installed CMake package is `GxdeBerneseQt5` or `GxdeBerneseQt6`; its
imported target is `GxdeBernese::Qt5` or `GxdeBernese::Qt6`. A corresponding
versioned pkg-config file is also generated.

## Testing and Quality Checks

Build first, then run the configured test suite:

```sh
ctest --test-dir build --output-on-failure
```

Run the complete style suite after building:

```sh
cmake --build build --target check-style
```

The equivalent direct invocation is:

```sh
python3 tools/check_style.py --build-dir build
```

The test setup includes:

- `bernese-helpers`: Main QtTest helper suite, run with the offscreen Qt
  platform plugin.
- `window-geometry`: Geometry tests with offscreen Qt and scale factor 2.
- `window-geometry-multiscreen`: Additional cross-screen coverage for Qt 6.10
  and newer, using `tests/screens.json`.
- `public-api-baseline`: Qt 6 exported-symbol comparison against
  `tests/public_api_qt6.txt`.
- `no-qt-private-api`: Verifies that the built library does not retain Qt
  private API dependencies.
- Public-header compile checks: Each public header must work as the first
  include in its own translation unit.

When changing behavior, add or update focused QtTest coverage. Preserve the
public API baseline unless an API change is intentional, and update the
baseline only after reviewing the exported-symbol diff. Keep all code free of
Qt private API usage; this is the central purpose of the project.

## Development Conventions

### C++ Style

- Follow Google-style C++ with the repository overrides in `.clang-format`.
- Use 2-space indentation, an 80-column limit, and 4-space continuation
  indentation. Do not align continuations with opening parentheses.
- Use left pointer alignment (`Type* value`). Control statements use braces;
  do not put short `if` statements or loops on one line.
- Expand nested namespaces instead of using `namespace a::b::c` syntax. Close
  each namespace with its own comment.
- Keep header guards, includes, namespaces, and declarations visually separated
  by blank lines as shown in `AGENTS.md`.
- Follow `.clang-tidy`: classes, structs, functions, and methods use
  `CamelCase`; namespaces, parameters, variables, and members use
  `lower_case`; private members end in `_`; global/static constants use a `k`
  prefix and `CamelCase`.
- Keep classes at or below 600 lines unless splitting is genuinely impractical.
  Prefer concise, human-readable, maintainable code.
- Every function requires a Doxygen comment. Write comments in simple English,
  explain intent, and do not use Markdown formatting inside code comments.

### Architecture and Public API

- Public headers are listed explicitly in `src/CMakeLists.txt`; update that
  list when adding an installed header.
- Generated export declarations come from `generated/bernese/export.h`.
  Symbol visibility is hidden by default, so public API symbols must be
  exported deliberately.
- Keep Wayland protocol XML as the source of truth; do not manually edit
  generated protocol sources in the build directory.
- Keep build artifacts in build directories. Generated distribution headers
  under `build/include/` and generated protocol files are not source files.
- Use the existing Qt-version abstraction (`QT_PACKAGE` and
  `BERNESE_QT_MAJOR_VERSION`) rather than hard-coding a Qt major version.

### Documentation and Contributions

- Project license: GNU General Public License v3.0 or later.
- Use English for commit messages, issues, and merge requests.
- Commit messages should use simple English and prefixes such as `chore:`,
  `feat:`, `add:`, or `create:`.
- Follow `CONTRIBUTING.md`: avoid duplicate or multi-topic issues, keep merge
  requests focused, link an issue when required, and ensure all tests pass.
- AI-assisted changes are allowed only when the contributor understands and
  takes responsibility for the implementation and its rationale.
- Update Doxygen comments with code changes. Treat the checked-in `docs/`
  output as generated documentation rather than hand-written source.
