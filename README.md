# gif_engine

This project is an example project generated with [cmake-init][1] with the
purpose of showing off how to add fuzz testing to a project using a superbuild
structure.

# Building and installing

See the [BUILDING](BUILDING.md) document.

## Fuzzing

To fuzz the project configure the `fuzz` directory instead of the project root.
This directory contains a superbuild lists file, which connects the library and
fuzzer projects. The superbuild lists file expects you to pass the
`CMAKE_C_COMPILER` variable.

### CI

The [CI workflow](.github/workflows/ci.yml#L36) shows exactly how to build and
run the fuzzer executable. It also shows how to use persistently stored corpus
data to drive the fuzzing process.

[1]: https://github.com/friendlyanon/cmake-init
