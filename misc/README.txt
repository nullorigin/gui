misc/debuggers/
  Helper files for popular debuggers.
  With the .natvis file, types like Vector<> will be displayed nicely in Visual Studio debugger.

misc/fonts/
  Fonts loading/merging instructions (e.g. How to handle glyph ranges, how to merge icons fonts).
  Command line tool "binary_to_compressed_c" to create compressed arrays to embed data in source code.
  Suggested fonts and links.

misc/single_file/
  Single-file header stub.
  We use this to validate compiling all *.cpp files in a same compilation unit.
  Users of that technique (also called "Unity builds") can generally provide this themselves,
  so we don't really recommend you use this in your projects.
