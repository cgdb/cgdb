# Testing

CGDB utilizes the Catch2 (C++ Automated Test Cases in a Header) framework for
its unit and integration testing.

## Building the Test Executable

To build the test executable you must pass the option `--enable-tests` to the
GNU configuration script.

## Running the Test Executable

To run the test executable simply execute `<build destination>/tests/tests`. The
default run configuration will execute all unit and integraton tests and output
the results to stdout. Integration tests that invoke curses will utilize the
terminal screen, therefore you can expect to see brief glimpses of rendering
CGDB interfaces during execution.

### Tags

CGDB currently supports the following tags:

- [unit] = unit tests
- [integration] = integration tests
- [curses] = tests that invoke curses

## Recording Coverage

The following instructions are for recording coverage with gcov:

1. To profile the source and record coverage, compile with the additional GCC
options `-fprofile-arcs -ftest-coverage`.

TO BE CONTINUED

## Caveats

- Currently some tests fail while executing in CGDB. This is likely due to a
  curses conflict and has not yet been investigated.
