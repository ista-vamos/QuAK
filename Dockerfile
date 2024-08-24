FROM ubuntu:24.04 AS base

RUN set -e

ENV DEBIAN_FRONTEND=noninteractive
ENV GIT_SSL_NO_VERIFY=1

# Install packages
RUN apt-get -y update
RUN apt-get install -y --no-install-recommends\
        python3\
        python3-pip\
        g++\
        gcc\
        make\
        time\
        cmake\
        git


COPY . /opt/quak
WORKDIR /opt/quak/
RUN git clean -xdf

RUN mkdir -p build build-no-scc

WORKDIR /opt/quak/build
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_IPO=ON -DENABLE_SCC_SEARCH_OPT=ON
RUN make -j2

WORKDIR /opt/quak/build-no-scc
RUN cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_IPO=ON -DENABLE_SCC_SEARCH_OPT=OFF
RUN make -j2

WORKDIR /opt/quak/experiments
