# Instructions for running tests.

In addition to testing for correct output, the automated testing system will
check your program and its source code. To successfully pass the test, you must
make sure that your program meets the requirements below. To do this,
run the commands below locally and make sure that they do not output any errors.

* **Style tests.** To check how much the beauty of your code matches
  for example, you can test your code using the _clang-format_ utility.
  The ```materials/linters``` folder contains the ```.clang-format``` file, which contains
  the necessary settings for the style test. This configuration file extends its action to all files that lie with it in the directory
  or in the directories below. So in order for these settings to apply to your source code files,
  copy ```.clang-format``` to the ```src``` folder. \
  \
  To run the style check, run the following command: \
  ```clang-format -n src/sourcefile_name.c``` \
  \
  To download clang-format, enter one of the following commands in the terminal: \
  ```brew install clang-format``` \
  or if you have root rights (for Ubuntu / Linux Mint / Debian) \
  ```sudo apt install clang-format```

  Required version of clang-format: \
  **Mac** 18.1.8 \
  **Linux** 18.1.8


 * **Static code analysis.** Sometimes (or not quite sometimes) it happens that
   a correctly compiled C program runs completely incorrectly or terminates
   with an error trying to access the wrong memory area. To prevent this from happening
   errors at the stage of writing the program, use special utilities that analyze
   check your source code for potential errors. Our autotest system uses
   ```cppcheck``` for this. 

   You can test your source code: \
   ```cppcheck --enable=all --suppress=missingIncludeSystem src/soursefile_name.c``` \
   You can also check all the source code files in the directory at once: \
   ```cppcheck --enable=all --suppress=missingIncludeSystem src/```

   Required version of cppcheck: \
   **Mac** 2.13 \
   **Linux** 2.13
   
