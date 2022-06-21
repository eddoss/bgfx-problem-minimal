# Problem

```
This example draws a white square and then a green one. 
I need the framebuffer to be completely cleared with a specific color before the green square is drawn. 
How to completely clear the framebuffer before submiting green square drawing?
```

`What I expect:`

![expect.png](expect.png)

`What I actually get:`

![get.png](get.png)

# Build

```
cd "to project root"
cmake -S . -B ./build -DCMAKE_PREFIX_PATH=path_to_installed_bgfx
cmake --build ./build
```
# Run

```
./build/bin/bgfx-minimal
Application will create `image.png` in project root
```