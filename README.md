# StringArt
Convert images to string art.

It uses [Vivid](https://github.com/Aviii06/Vivid) for rendering and GUI.

# Dependencies
## ImageMagick
Download link: https://imagemagick.org/script/download.php

### Using package managers:
**Using brew**
```bash
brew install imagemagick
```
**Using apt**
```bash
sudo apt update
sudo apt install imagemagick
```
**Using chocolatey**
```bash
choco install imagemagick.app
```

# Build Instructions
```
git clone git@github.com:Aviii06/StringArt.git
cd StringArt
git submodule update --init --recursive
mkdir build && cd build
cmake .. -G Ninja
ninja -j8
./StringArt /path/to/file
```
