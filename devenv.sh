#!/bin/bash -eu

docker build -t llvm-devenv .
exec docker run --rm -ti -v $(pwd):/project -w /project llvm-devenv
