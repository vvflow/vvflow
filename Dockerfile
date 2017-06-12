FROM ubuntu:xenial
# RUN echo "deb http://deb.debian.org/debian jessie contrib" >> /etc/apt/sources.list
RUN apt-get update && apt-get install -y \
    build-essential \
    git \
    libfl-dev \
    libhdf5-dev \
    liblapack-dev \
    libmatheval-dev \
    make \
    pkg-config \
    cmake \
    cmake-curses-gui \
    libgflags-dev \
    && apt-get clean

CMD /bin/bash
