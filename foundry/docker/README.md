# Docker Build

This folder allows you to build Toccata in a container. The Dockerfile contains instructions to set up all build dependencies needed.

# Build Dev Environment

Building the development environment creates a container that has Toccata's prerequisites in place. Using this container, you can then compile and run the compiler.

To build the dev environment, use:

`docker build -t toccata-devenv -f Development.docker ../..`

The `../..` is very important. That's what allows the Docker container to use the sources in this tree.

This builds a local docker image and tags it as `toccata-devenv`.

The short form of this is spelled as:

`build-devenv`

## Using the Dev Environment

Having the image built is just the first step. Next, you'll want to log in and use it. For that, use:

`docker run -it --rm --name toccata-dev toccata-devenv`

This says to run interactively and attach to the terminal ("-it"), remove the container instance when you exit ("--rm"), name the container `toccata-dev` ("--name toccata-dev") and use the tagged container image we just built.

This also has a short form:

`enter-dev`

# Build Toccata Container

Building a Toccata container creates a container image with Toccata in binary form, ready to run. This is the image you might want to push to a Docker repository, or use as a base for Toccata applications.

`docker build -f Distribution.docker .`
