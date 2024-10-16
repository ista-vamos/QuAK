FROM ubuntu:24.04 AS base

RUN set -e

ENV DEBIAN_FRONTEND=noninteractive
ENV GIT_SSL_NO_VERIFY=1

# Install packages
RUN apt-get -y update
RUN apt-get install -y --no-install-recommends\
        python3\
        python3-numpy\
        g++\
        gcc\
        make\
        time\
        cmake\
        git

#python3-pip

# IMPORTANT: This dockerfile must be build from the top-level directory
# of quak, otherwise the following command will fail.
COPY .. /opt/quak
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

RUN bash ./gen-aut.sh

#pip install --break-system-packages numpy
