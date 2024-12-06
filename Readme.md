# Mandelbrot GPU
 An OpenGL OpenCL GPU implementation of the Mandelbrot fractal, with some interesting coloring. Interoperability is used to manipulate textures using OpenCL and render them on the framebuffer using OpenGL.\

<img src="mandelbrot_screenshot.png" width="512">

## Build
Use CMake to build the solution. Everything should work by default.

## Use
Basic controls:
- W or S: zoom (scale)
- A or D: offset horizontally
- E or Q: offset vertically
- R: reset parameters
- F: enable/disable filtering
- P: play/pause animation
- ] or [: increase/decrease animation speed

## License
>The MIT License (MIT)

>Copyright (c) 2024 Panagiotis Vrettis

>Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

>The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

>THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
