# GXDE Bernese

## Introduction
Welcome to `gxde-bernese`, GXDE's compatibility library that replaces direct Qt Private API with those public ones.

### Background Informaton
GXDE used linuxdeepin's components heavily. Early Deepin source code (particularly during the DTK2 era) relied heavily on Qt private headers. The Qt private headers does NOT guarantee API stability, hence when Qt upgrades, those components might be also break.

This issue becomes particularly acute now that we are supporting rolling-release distributions like Arch Linux: Arch Linux continuously updates its Qt version, leading to more frequent breakages.

Every time a breaking change occurred, we had to rebuild all programs that included Qt private headers. Consequently, the new idea was to eliminate the applications' dependencies on Qt private APIs entirely—and that is how this library came into being.

## Licensing
GXDE Bernese is licensed under *[GNU GENERAL PUBLIC LICENSE Version 3](./LICENSE)*. You may see the `LICENSE` file on the project repository.
