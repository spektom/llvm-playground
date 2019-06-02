FROM ubuntu:19.04

ENV TERM=xterm \
    LC_ALL=C \
    DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
      && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        curl \
        g++ \
        python \
        xz-utils \
      && apt-get clean \
      && curl -sSL http://releases.llvm.org/8.0.0/llvm-8.0.0.src.tar.xz | tar -xJC /tmp \
      && mkdir /tmp/llvm-build \
      && cd /tmp/llvm-build \
      && cmake -DLLVM_INCLUDE_TESTS=NO -DLLVM_INCLUDE_EXAMPLES=NO /tmp/llvm-8.0.0.src \
      && cmake --build . -j 8 \
      && cmake -DCMAKE_INSTALL_PREFIX=/usr/local -P cmake_install.cmake \
      && rm -rf /tmp/llvm-* \
      && groupadd -g 1000 dev \
      && useradd -m -u 1000 -g dev dev

USER dev
CMD ["bash"]
