#!/bin/bash
set -e

# Change to the project directory where platformio.ini is located
cd src/farkle

echo "Running Native Tests (Game Logic)..."
pio test -e native

echo "Running Component Tests..."
pio test -e component_tests
