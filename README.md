# cxx_coroutines_examples
Some examples with C++20 coroutines feature. Was verified on Ubuntu 20.04

## Prerequisites

```
sudo apt-get install clang libc++-dev libc++abi-dev
```

## Compile all examples

```
mkdir build
cd build
cmake ..
make
```

## Compile one example

On Clang-9 and below:
```
clang++ -stdlib=libc++ -std=c++2a -Wall resumable_type.cpp -o resumable_type
```

On Clang-10 and onwards:
```
clang++ -std=c++20 -stdlib=libc++ -fcoroutines-ts -Wall resumable_type.cpp -o resumable_type
```
