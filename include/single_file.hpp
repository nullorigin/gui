// gui: single-file wrapper include
// We use this to validate compiling all *.cpp files in a same compilation unit.
// Users of that technique (also called "Unity builds") can generally provide
// this themselves, so we don't really recommend you use this in your projects.

// Do this:
//    #define IMPLEMENTATION
// Before you include this file in *one* C++ file to create the implementation.
// Using this in your project will leak the contents of internal.hpp and
// Vec2 operators in this compilation unit.

#ifdef IMPLEMENTATION
#define DEFINE_MATH_OPERATORS
#endif

#include "gui.hpp"
#ifdef IMPLEMENTATION
#include "demo.cpp"
#include "draw.cpp"
#include "gui.cpp"
#include "tables.cpp"
#include "widgets.cpp"
#endif
