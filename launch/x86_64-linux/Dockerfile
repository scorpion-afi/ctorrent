
FROM x86_64-linux

# we need root access to deploy our built package
USER 0

RUN apt-get update && apt-get install -y gdb && rm -rf /var/lib/apt/lists/*

# we have boost libraries installed within /usr/local/libraries, so to simplify
# launching process we have to let ld to know where to look for libraries
RUN ldconfig /usr/local/libraries

# aux script to deploy oru build package
COPY reload.sh /usr/local/bin/