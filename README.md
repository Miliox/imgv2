# Image Viewer V2

Simple Image Viewer Enhanced Edition

## Features

* Minimalist (No title bar)
* Window Menu options
  * File > Open File
  * Edit > Flip Horizontal/Vertical
* Double click to fill desktop
* Responsive window resizing
* Support multiple images open simultaneously
* Support all formats that SDL_IMG does

## Requirements

* Meson
* C++
* Objective-C (macOS)
* SDL2

### MacOS

```bash
# install required c++/objc compilers
xcode-select --install

# install required tools and libraries
brew install meson ninja sdl2 sdl2_image sdl2_mixer sdl2_ttf
```

## Build

```bash
mkdir builddir

meson setup builddir

meson compile -C builddir
```

## Run

```bash
# Command line help
imgv2 --help

# Launch with dialog to open images
imgv2

# Launch to open a given image
imgv2 some_image.png

# Launch to open multiple images
imgv2 image1.jpg image2.png image3.svg ...
```

## License

[MIT](LICENSE.md)
