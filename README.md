## Purpose

This repository contains code based on the examples given in the book [Modern C++ Programming with Test-Driven Development (Code Better, Sleep Better) by Jeff Langr](https://pragprog.com/book/lotdd/modern-c-programming-with-test-driven-development). The code is re-written as an excercise while reading the book, therefore the code is in no way my own and any usage of it falls under the original Copyright and Licensing as explained in the [original source](https://pragprog.com/titles/lotdd/source_code) files.
```
/***
 * Excerpted from "Modern C++ Programming with Test-Driven Development",
 * published by The Pragmatic Bookshelf.
 * Copyrights apply to this code. It may not be used to create training material, 
 * courses, books, articles, and the like. Contact us if you are in doubt.
 * We make no guarantees that this code is fit for any purpose. 
 * Visit http://www.pragmaticprogrammer.com/titles/lotdd for more book information.
***/
```

The code for each example (chapter) is bundled in a separate CMake projects. Each re-worked solution is written in an attempt to follow the book's advice while also trying to follow the [Google C++ Coding Style](https://google.github.io/styleguide/cppguide.html). 

To run the code a docker image with the required development environment is needed. My custom development environment is available in [this repository](https://github.com/gandrein/docker-xenial-guis-cpp-tdd-dev-env) which is also included here as a `git-submodule` in the [docker-env-submodule](./docker-env-submodule) folder. 

## Requirements

* `git >= 2.7.4`
* `CMake >= 2.8`
* `Googletest >= 1.8` framework
* `docker >= 17.09` ... if you use the above docker image
* [`nvidia-docker`](https://github.com/NVIDIA/nvidia-docker/wiki/Installation-(version-1.0)) version `1.0`, if you plan to use docker with GUIs and have a NVIDIA based machine.

### Using Docker

Read this [README.md](https://github.com/gandrein/docker-xenial-guis-cpp-tdd-dev-env/blob/master/README.md) to check how to make use of the above docker image.

## Book Source Code

If you wish to download the source code as presented in the book, download it from the original soruce [The Pragmatic Bookshelf source code](https://pragprog.com/titles/lotdd/source_code). 

Note that the original code won't compile on my custom docker image due to the way the book source code dependencies are build. Check my [other repository](https://github.com/gandrein/tdd-env-cpp-jeff-langr-book) for a docker image to run the book code as is. 

### The `./run_docker.sh` script

The [./run_docker.sh](./run_docker.sh) script is a modified version of the one available in the git-submodule which mounts the source code of this repository inside your docker container.

### Bash auto-completion for `./run_docker.sh`

When using `./run_docker.sh` in a bash shell to launch the container, source the [bash_docker_images_completion.sh](./docker-env-submodule/docker/configs/bash_docker_images_completion.sh) script available in the `docker-env-submodule/docker/configs` folder. Now you should be able to get the names of the available docker images on your system whenever you type 
```
./run_docker.sh <TAB><TAB>
```


