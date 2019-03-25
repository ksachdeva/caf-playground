
# CAF Examples

This repository contain some examples that try to emulate real world
use cases while still trying to remain simple.

The examples are heavy commented so you can follow along the code.

## Install

There are two ways to compile the examples. 

### Approach 1 (if you do not want to debug CAF)

First install CAF following the instructions described at the CAF repository (https://github.com/actor-framework/actor-framework)

Typically you would end up installing it in /usr/local

For Windows you can use the version from vcpkg (https://github.com/Microsoft/vcpkg/tree/master/ports/caf) [at this point of time it is one version behind]

Finally compile this repository using

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
`_debug` to whatever you have chosen as the name of folder.

For example I compiled it using following steps

```bash
# compiling CAF (on OSX, should be same on Linux)
./configure --with-log-level=TRACE --build-type=Debug --build-dir=_debug
cd _debug
make -j4
```

Finally compile this repository using

```bash
mkdir _debug
cd _debug
cmake ..
cmake --build .
```

### Examples

The order of examples is *kind* of important (but not mandaotry to follow) if you are new to CAF.

Here is suggested order to try

* [**hello-caf**](samples/src/hello-caf.cc)

The customary hello world but with a little twist. Demonstrates how to construct function and
class based actors. The purpose of this example is for you to get started very quickly with the
simplest possible use case.

* [**polymorphic-actors**](samples/src/polymorphic-actors.cc)

This example demonstrates how you can have different implementations of the same trait and 
thereby achiving polymorphism.

* [**request-then**](samples/src/request-then.cc)

This example demonstrates the usage of 'then' when you use request. Usage of 'then' keeps the mailbox
of an actor active i.e. messages to its other behaviors can still be sent.

* [**request-await**](samples/src/request-await.cc)

This example demonstrates the usage of 'await' when you use request. Usage of 'await' informs the framework (CAF)
that it should not send messages to its other behaviors while it waiting for the request to finish.

* [**monitoring-actors**](samples/src/monitoring-actors.cc)

This example shows how one actor can terminate (logical unregistration) itself from the actor
system and other actor that can monitor such events.

* [**ref-of-cls-actors**](samples/src/ref-of-cls-actors.cc)

This example tries to construct a scenario where an actor would like to send its reference (at some time in future i.e. not
at its construction) to another actor. The other actor would then use the supplied reference to send messages. 

[This example is not working yet !]
