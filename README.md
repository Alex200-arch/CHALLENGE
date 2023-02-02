# Message Lite

## Dependence

#### Install Spdlog
```console
$ git clone https://github.com/gabime/spdlog.git
$ cd spdlog && mkdir build && cd build
$ cmake3 .. && make -j
```

#### Command line optiona parser
it is already in include/cxxopts.hpp

#### Csv file operator
it is already in include/csv.hpp

## Build Message Lite
```console
$ mkdir build && cd build
$ cmake3 .. && make
```

## Run Message Lite
```console
$ ./ms-lite -p 12345
$ less server.log
```
```console
$ ./mc-lite --name alice -p 123456a
$ less alice.log
```
```console
$ ./mc-lite --name bob -p 123456b
$ less bob.log
```