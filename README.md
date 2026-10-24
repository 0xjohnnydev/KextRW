# KextRW

A macOS kernel extension offering several features useful for security/vulnerability research against XNU.

The features provided by this kernel extension include:
* Virtual kernel read/write
* Physical read/write
* Getting the kernel base and slide
* Kernel memory allocation and freeing
* Kernel call primitive
* Address translation

The codebase is originally based on [IOKernelRW](https://github.com/Siguza/IOKernelRW), where you can find installation instructions, as they will be the same for this project. Any binary that wishes to create a userclient for this kernel extension must possess the `com.apple.security.alfie.kext-rw` entitlement. An easy-to-use test program can be found in the `tests/` folder.

This fork ports [AlfieCG's KextRW](https://github.com/alfiecg24/KextRW) to MacBook Neo (`Mac17,5`) on macOS 27.0.1 (`26A434`).

Building the project using `make all` will output a static `libkextrw` library and a header file in the `build/` directory, which you can then use to build projects on top of the primitives provided by the kernel extension. The `kextrw_test.c` file in the `tests/` directory offers an example of this use case.
