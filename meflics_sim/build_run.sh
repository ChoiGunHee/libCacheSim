#!/bin/bash

# Define the build directory
BUILD_DIR="build"

# Remove the build directory if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing $BUILD_DIR directory..."
    rm -rf "$BUILD_DIR"
fi

# Create a new build directory
echo "Creating $BUILD_DIR directory..."
mkdir "$BUILD_DIR"

# Navigate into the build directory
cd "$BUILD_DIR" || exit

# Run cmake
echo "Running cmake..."
cmake ..

# Run make with parallel jobs
echo "Running make with -j..."
make -j

echo "Build and installation completed."

