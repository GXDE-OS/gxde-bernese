# Project Instructions for AI Agents
## Introduction
Welcome to `gxde-bernese`, GXDE's compatibility library that replaces direct Qt Private API with those public ones.

### Background Informaton
GXDE used linuxdeepin's components heavily. Early Deepin source code (particularly during the DTK2 era) relied heavily on Qt private headers. The Qt private headers does NOT guarantee API stability, hence when Qt upgrades, those components might be also break.

This issue becomes particularly acute now that we are supporting rolling-release distributions like Arch Linux: Arch Linux continuously updates its Qt version, leading to more frequent breakages.

Every time a breaking change occurred, we had to rebuild all programs that included Qt private headers. Consequently, the new idea was to eliminate the applications' dependencies on Qt private APIs entirely—and that is how this library came into being.

## Coding Instruction
- Keep every class at or below 600 lines. Split a file into modules before it exceeds this limit. This is NOT a hard limit that this rule may be ignored ONLY IF THIS IS NECESSARY, I MEAN NECESSARY!!
- Your code should be as concise as possible—to ensure it remains human-readable and maintainable. You should include comments that clearly explain the intent of the code.
- DOXYGEN COMMENTS ARE REQUIRED FOR EACH FUNCTION!!
- Comments must be written in English; please use simple English so that non-native English speakers can understand them. If you need to update the documentation, do so in the respective language of each document.
- The goal of the code is to be human-readable and maintainable; use this as the foundation to complete your task.
- Please refer to `CODE_OF_CONDUCT.md` and `CONTRIBUTING.md`; all requirements applicable to human contributors and AI policies apply to you as well.
- Your comment should NOT contain Markdown elements.

### Format Instruction
Your coding style should be Google Styled C++ with the following instructions:

#### Namespace
```C++
// Don't do...
namespace bernese::wayland::internal {
}  // namespace bernese::wayland::internal

// Instead, do...
namespace bernese {
namespace wayland {
namespace internal {
}  // namespace internal
}  // namespace wayland
}  // namespace bernese
```

#### Line breaks
Keep lines within 80 columns so the code is easy to read on small HiDPI screens.

Use 2 spaces for each block indentation level. A wrapped line uses a fixed
4-space continuation indent relative to the current level. Do not align
continuation lines with an opening parenthesis. Nested continuations add
another 4 spaces; when a nested call ends, return to the outer level.

Keep these settings in .clang-format:
```yaml
IndentWidth: 2
ContinuationIndentWidth: 4
AlignAfterOpenBracket: DontAlign
ColumnLimit: 80
```

```C++
if (first_condition &&
    second_condition) {
  HandleCondition();
}

void Example() {
  auto* resources = xcb_randr_get_screen_resources_current_reply(connection,
      xcb_randr_get_screen_resources_current(connection, root), nullptr);
}

if (CheckConditions(first_argument,
        second_argument) &&
    other_condition) {
  HandleCondition();
}
```

#### Overall Style
```C++
// This will be awful for it looks literally like a brick wall:
#ifndef BACKDROP_ORIGIN_CONNECTION_CONNECTION_H_
#define BACKDROP_ORIGIN_CONNECTION_CONNECTION_H_
#include "xcb/xcb.h"
namespace bernese::wayland::internal {
// Owns the separate process-lifetime XWayland work-area connection.
struct Connection {
  int screen = 0;
  xcb_connection_t* value = xcb_connect(nullptr, &screen);
};
}  // namespace bernese::wayland::internal
#endif  // BACKDROP_ORIGIN_CONNECTION_CONNECTION_H_

// And this will be easier to read:
#ifndef BACKDROP_ORIGIN_CONNECTION_CONNECTION_H_
#define BACKDROP_ORIGIN_CONNECTION_CONNECTION_H_

#include "xcb/xcb.h"

namespace bernese {
namespace wayland {
namespace internal {

// Owns the separate process-lifetime XWayland work-area connection.
struct Connection {
  int screen = 0;
  xcb_connection_t* value = xcb_connect(nullptr, &screen);
};

}  // namespace internal
}  // namespace wayland
}  // namespace bernese

#endif  // BACKDROP_ORIGIN_CONNECTION_CONNECTION_H_
```
