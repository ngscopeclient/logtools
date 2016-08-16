# logtools
Tiny console/file logging library used across many of my projects.

Originally developed for openfpga and now moved to a separate repo for easier re-use. Re-licensed from LGPL to 3-clause BSD with 
the consent of all developers.

This project is not intended to be compiled and installed by itself since it's so simple; it's normally pulled into a larger 
project as a Git submodule and then built by that project's build system.

A simple leaf CMakeLists.txt is provided to ease integration with parent projects.
