# README
A reference implementation for the Path Oblivious Range Tree algorithm.

All source files are in the `src/` directory.
- `Block`, `Bucket`: contains implementation for the Block and Bucket data structures as described
  in the paper.
- `UntrustedStorageInterface`: defines the interface that the Path ORange Tree implemenation uses to
  communicate with the untrusted cloud storage provider. A possible implementation is defined in
  `ServerStorage`.
- `RandForORangeInterface`: the Path ORange Tree implementation gets random leaf ids from here. A possible
  implementation is defined in `RandomForORange`. Can be overridden e.g. for testing purposes.
- `ORangeInterface`: The interface with which to use this Path ORange Tree implementation.
  `ORangeReadPathEviction` contains the reference implementation itself.
- `main`: Runs one of the above four tests.

To compile and run:

1. Uncomment out the desired test in `src/main.cpp`; comment out any other tests.
2. Run `make` in this directory.
3. Run the test with `./PORangeTree`.

C++17 is required. Known to compile with `gcc version 7.4.0 (Ubuntu 7.4.0-1ubuntu1~18.04.1)` and `gcc version 7.3.1 20180130 (Red Hat 7.3.1-2) (GCC)`.
