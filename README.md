
# CAF Examples

This repository contain some examples that try to emulate real world
use cases while still trying to remain simple.

## Install

There are two ways to compile the examples. 

### Approach 1 (if you do not want to debug CAF)

First install CAF following the instructions from here

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

### Approach 2 (if you have a locally compiled version of CAF)

Set an environment variable `CAF_LOCAL_COMPILE_ROOT` that points to the CAF directory

At present I assume that you used `_debug` in CAF directory as the folder where you
compiled it. If it is something else then go to `cmake/FindCAF_local.cmake` and change
`_debug` to whatever you have chosen as the name of folder

```bash
mkdir _debug
cd _debug
cmake ..
cmake --build .
```

### Examples

The order of examples is *kind* of important (but not mandaotry to follow) if you are new to CAF.

Here is suggested order to try

* **hello-caf**

The customary hello world but with a little twist. Demonstrates how to construct function and
class based actors. The purpose of this example is for you to get started very quickly with the
simplest possible use case.

* **polymorphic-actors**

The example demonstrates how you can have different implementations of the same trait and 
thereby achiving polymorphism.