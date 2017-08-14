
# the minimal started image we're basing on
FROM debian

RUN apt-get update && apt-get install -y \
    cmake \
    pkg-config \
    build-essential \
    && rm -rf /var/lib/apt/lists/*

# as prebuilt boost libraries are used, it has to be some
# way to find them during compilation/linkage
COPY boost-crystax-*.pc /usr/lib/pkgconfig/

# provide an uid (-u) as your main user on the host to be
# able to deal with files generated within the container 
# by the user created at this line;
# a primary group has to be created/specified, look at 
# the documentation for the RUN command
RUN groupadd -f -g 1000 dev && \
    useradd -g 1000 -m -N -s /bin/bash -u 1000 dev
    
WORKDIR /home/dev

# build boost from sources and install result in /usr/local
ADD boost_1_59_0.tar.gz ./
RUN mkdir boost_build && cd boost_1_59_0 && \
./bootstrap.sh && \
./b2 --build-dir=../boost_build \
variant=debug \
link=shared \
threading=multi \
install; echo "boost build process returned non-zero code, ignore that and keep going on :-)" && \
cd ../ && rm -rf ./boost_build ./boost_1_59_0

# set a default user you'll log under when you're
# going to run this image, /bin/bash will be executed
# unless otherwise specified
USER 1000

ENV TARGET_NAME x86_64-linux
