# Testing

CGDB utilizes the Catch2 (C++ Automated Test Cases in a Header) framework for
its unit and integration testing.

## Building Tests

To build the test executable you must pass the option `--enable-tests` to the
configuration script.

## Running Tests

To run the test executable simply execute `<build destination>/tests/tests`. The
default run configuration will execute all unit and integraton tests.
