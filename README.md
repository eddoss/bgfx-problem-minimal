# Build

`cd "to project root"`

`cmake -S . -B ./build -DCMAKE_PREFIX_PATH=path_to_installed_bgfx`

`cmake --build ./build`

# Run

`./build/bin/bgfx-minimal`

Application will create `image.png` in project root