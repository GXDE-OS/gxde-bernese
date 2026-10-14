# GXDE Bernese

## 简介
`gxde-bernese`是GXDE的兼容库，当前我们的目标是使用Qt稳定的公开替换GXDE程序中使用Qt Private API的部分，`gxde-bernese`的目标是为这一过程提供可复用的一些趁手函数。

### 背景
GXDE大量使用linuxdeepin的源码。早期的Deepin（尤其是DTK2时期）大量使用Qt私有函数，而这些私有函数并不保证API稳定。当仓库里的Qt版本更新时，这些依赖私有函数的组件就可能崩溃。

随着我们现在要支持类似Archlinux这样滚动发行的发行版，这个问题就变得更加严重了：Archlinux更新仓库内的Qt版本更加频繁，所以break也会更多。

每次Qt版本更新，我们需要重新构建这些使用Qt私有函数的程序，于是，我们计划移除GXDE代码库中使用Qt私有函数的部分，转而使用API稳定的公开函数。

## 许可证
GXDE Bernese在 *[GNU GENERAL PUBLIC LICENSE Version 3](./LICENSE)* 协议下获得许可，您可以在`LICENSE`文件中查阅许可证。
