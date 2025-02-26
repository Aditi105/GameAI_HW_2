# HW2 Steering Behaviors

This project demonstrates different parameter implementations using SFML libraries. Each source file is set up to compile into its own executable so you can easily test and compare behaviors.

## Overview

Part A Files: 
  These files (e.g., 'part1.cpp', 'part2a.cpp', etc.) contain parameters that are not pleasing or do not satisfy all conditions.

Part B Files:
  These files (e.g., 'part1b.cpp', 'part2b.cpp', etc.) include the fixed parameters with a more pleasing and correct implementation.

ChatGPT assisted with concept understanding and error fixing during the development of this project.

## Dependencies

- **SFML Libraries:**  
  The project uses [SFML](https://www.sfml-dev.org/) (Simple and Fast Multimedia Library).  
  Ensure that SFML is installed on your system. The Makefile includes the appropriate library paths for macOS (Darwin) and Linux.

## Compilation

A Makefile is provided to compile all C++ source files in the `src` directory. Each source file is compiled separately into its own executable. To compile all files, run:

```bash
make
