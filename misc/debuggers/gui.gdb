# GDB configuration to aid debugging experience

# To enable these customizations edit $HOME/.gdbinit (or ./.gdbinit if local gdbinit is enabled) and add:
#   add-auto-load-safe-path /path/to/gui.gdb
#   source /path/to/gui.gdb
#
# More Information at:
#   * https://sourceware.org/gdb/current/onlinedocs/gdb/gdbinit-man.html
#   * https://sourceware.org/gdb/current/onlinedocs/gdb/Init-File-in-the-Current-Directory.html#Init-File-in-the-Current-Directory

# Disable stepping into trivial functions
skip -rfunction Im(Vec2|Vec4|Strv|Vector|Span)::.+
