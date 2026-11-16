# Contributin to AgataSelector

If you find a bug or want to contribute to the project, you will help us a lot!
Here are some rules to make our lives easier.

## Check for compilation errors and run tests

Before committing check if there are any compilation errors and run the included tests:

```
 cmake -B build -DENABLE_TESTING=ON -S .
 cmake --build build
 cd build/Tests
 ctest --output-on-failure -j6
```

The CI tests will run after your commit, check for a green marker for a successful pipeline. If you see a red mark and cannot solve the problem you can inform the other maintainers.

## Check the code format

If you have clang installed, you can use it automatically so that your changes are within the expected formatting in the .clang-format file.
To do so, run the following commands from your build directory:

```
 cmake -DUSE_CLANG_FORMAT=On .
 make clangformat
```

The updated source files will be ready to be committed.