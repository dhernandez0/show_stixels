# show_stixels

Code to generate stixels image from .stixels format, for more details check [Stixels World on the GPU](https://github.com/dhernandez0/stixels/)

## How to compile and test

Simply use CMake and target the output directory as "build". In command line this would be (from the project root folder):

```
mkdir build
cd build
cmake ..
make
```

## How to use it

Type: `./show_stixels dir max_disparity`

The argument `max_disparity` is the maximum disparity of the given disparity images.

`dir` is the name of the directory which needs this format:

```
dir
---- left (images taken from the left camera)
---- right (right camera)
---- disparities (disparity maps)
---- stixels (.stixels files)
---- stixlesim (results will be here)
```

An example is provided, to run it type: `./stixels ./example 128`
