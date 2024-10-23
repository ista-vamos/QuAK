FROM ubuntu:24.04 AS base

RUN set -e

ENV DEBIAN_FRONTEND=noninteractive
ENV GIT_SSL_NO_VERIFY=1

# Install packages
RUN apt-get -y update
RUN apt-get install -y --no-install-recommends\
        g++\
        gcc\
        make\
        cmake
#        git

#python3-pip

COPY . /opt/quak
WORKDIR /opt/quak/
#RUN git clean -xdf

RUN mkdir -p build

WORKDIR /opt/quak/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_IPO=ON -DENABLE_SCC_SEARCH_OPT=ON
RUN make -j2

