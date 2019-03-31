# CAF Examples

This repository contains jupyter notebooks that illustrate various aspects
of C++ actor framework (CAF).

## Zero install setup [Running them using binder]

The advantage of writing examples using jupyter notebooks is that you can use both
markdown and code in the same document.

A service like mybinder.org provides functionality to run these notebooks (for free) in cloud and
hence there is no setup/install/compilation required.

Click on the below badge to go to mybinder.org and launch.

[![Binder](https://mybinder.org/badge_logo.svg)](https://mybinder.org/v2/gh/ksachdeva/caf-playground/master)

## Running locally [only need docker]

If you want to run them locally and/or contribute some notebooks/examples then it is also very easy.

Again you do not have install any library/cmake/compiler etc. All you need is docker and if you are on
linux or osx just use the `run-locally.sh` script provided in this repository.

If you are on windows run the command as :

```bash
docker run --rm -it -p 8888:8888 -v <full_path_notebooks_folder>:/home/jovyan/notebooks ksachdeva17/caf:latest
```

## Juppyter vs Jupyterlab

Irrespective of how you run them (binder or local) you have the option to use either jupyter or jupyerlab.

The default is jupyter as I have not yet figured out how to start in jupyterlab mode. I personally
prefer jupyterlab.

In order to use jupyterlab all you need is replace `tree` with `lab` in the browser url.

For e.g.

mybinder.org may launch url [it may be different for you as they spawn containers dynamically] as https://hub.mybinder.org/user/ksachdeva-caf-playground-hm5n1se0/tree, you simply replace it
with https://hub.mybinder.org/user/ksachdeva-caf-playground-hm5n1se0/lab.

## Only viewing the notebooks

Github natively supports viewing jupyter notebooks. Just click on the file however if you are interested in viewing using a better renderer then you can go to [https://nbviewer.jupyter.org/](https://nbviewer.jupyter.org/) and provide
the path to this repository.

### Examples

The order of examples is *kind* of important (but not mandaotry to follow) if you are new to CAF.

Here is a suggested order to try :

* [**hello-caf**](notebooks/hello-caf.ipynb)

The customary hello world but with a little twist. Demonstrates how to construct function and
class based actors. The purpose of this example is for you to get started very quickly with the
simplest possible use case.

* [**polymorphic-actors**](notebooks/polymorphic-actors.ipynb)

This example demonstrates how you can have different implementations of the same trait and 
thereby achiving polymorphism.

* [**request-then**](notebooks/request-then.ipynb)

This example demonstrates the usage of `then` when you use `request`. Usage of `then` keeps the mailbox
of an actor active i.e. messages to its other behaviors can still be sent.

* [**request-await**](notebooks/request-await.ipynb)

This example demonstrates the usage of `await` when you use `request`. Usage of `await` informs the framework (CAF)
that it should not send messages to its other behaviors while it waiting for the request to finish.

* [**monitoring-actors**](notebooks/monitoring-actors.ipynb)

This example shows how one actor can terminate (logical unregistration) itself from the actor
system and other actor that can monitor such events.

* [**ref-of-cls-actors**](notebooks/ref-of-cls-actors.ipynb)

This example tries to construct a scenario where an actor would like to send its reference (at some time in future i.e. not
at its construction) to another actor. The other actor would then use the supplied reference to send messages.

Please note that the scenario in this example is bit convoluted and normally you should not write your code like this. 

Please read the comments in the code for more information.
