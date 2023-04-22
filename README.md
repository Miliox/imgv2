# Image Viewer V2

Simple Image Viewer Enhanced Edition

## Features

* Minimalist (No title bar)
* Window Menu option: "File > Open File"
* Double click to fill desktop
* Responsive window resizing
* Support multiple images open simultaneously
* Support all formats that SDL_IMG does

## Requirements

* Meson
* C++
* Objective-C (macOS)
* SDL2

## Build

```bash
meson setup --wipe builddir
meson compile   -C builddir
```

## Run

```bash
# Launch with dialog to open images
imgv2

# Launch to open a given image
imgv2 some_image.png

# Launch to open multiple images
imgv2 image1.jpg image2.png image3.svg ...
```

## License

[MIT](LICENSE.md)
