# ctorrent
This project is about the library to distribute calculations. ctorrent stands as 'calculation torrent' which implies that
calculations are performed on a huge amount of hardware devices (computation units) connected over Internet.
The project is a cross-platform and implies an ability to be built for several platforms (now only x_86_64_linux and arm_android are supported).

Distributed calculations are performed by using the next topology:
 - there are a lot of computation units (distinct hardware devices: PCs, smartphones, IoT,...) with deployed a special
 software based on the libctorrent_server library, such a software is called xdu (eXecute and Dispatch Unit). These
 xdux are responsible for execution (calculation) of tasks and/or dispatching those tasks between each other (e.g. if
 some xdu is overloaded or it's gone, etc.). [the dispatching isn't supported yet]

 - if someone needs to distribute calculation of the TASK it has to write a software which splits this TASK into
 several small tasks and pass those tasks to the libctorrent_client library, it's the library responsibility
 to find xdus, distribute those tasks between them, wait for results and return back those results to a caller
 (in the same order as tasks were provided or in a random order).

 - there are may be a lot of clients' devices with such a software deployed.

The project consists of the ctorrent library and several auxiliary programs:
 ctorrent library - a library which implements the distribute calculation (for a client and a server sides)
 task_solver - an example of a client, which needs a distributed calculation:
                 -calculate a simple hash for a string;
                 -invert a string
 xdu - an example of a server, which handles clients' and xdus' requests.

The libctorrent_client library itself doesn't split the TASK it just supposes that such a spiting gets performed by a user of
the library. TASK can be split by several ways(protocols), library supports some of them:
 - TASK gets split into several tasks, where a task is a set of a data and a method to work with that data, the
   data is a POD-type object and the method is just a source code. (Yes the source code, this source code gets compiled
   on the server side for a specific hardware into a dynamically loaded library and then gets loaded to be executed.)
 - TASK gets split into several tasks, where a task is a set of a data and a method to work with that data, the
   data is a POD-type object and the method is a compiled code as a dynamic library. [not implemented yet]
 -... (hope to add a support for the new protocols)

The main goal of this project is to improve skills in c++, stl, boost, multithreading and network programming, cmake and docker
and of course it's interesting :)

Tasks are executed(computed) on the server side concurrently within several threads. Yes it's unsafe, 'cause one task may affect
execution of other task (intentionally or unintentionally) or even corrupt whole server(xdu), but don't forget it's rather a
research project than a real one, so don't worry :) (it's interesting is it possible to encapsulate such a task so it'd be
safe to execute it in a distinct thread, not in a distinct process, preventing somehow harmful systemcalls and applying some memory
protections).

The project uses the boost.serialization library to exchange objects across the network.

As a C++ support for Android isn't full and there's no boost support, crystax is used. crystax is a set of toolchains(with full libstdc++)
and boost for Android (https://www.crystax.net/).

Project gets built via docker's images(containers), so they have to be built before, look at docker/<arch>/how_to.txt. As you have docker's
images built you may build the project by using a deploy_build.sh script which builds for all supported platforms.
