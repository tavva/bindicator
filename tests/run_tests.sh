#!/bin/bash

# Ensure we're in the tests directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Check if googletest is installed
if ! pkg-config --exists gtest; then
    echo "GoogleTest not found. Please install it first:"
    echo "  brew install googletest"
    exit 1
fi

# Clean any previous builds
make clean

# Build and run tests
make test

# Store the exit code
TEST_RESULT=$?

# Clean up
make clean

# Exit with the test result
exit $TEST_RESULT 