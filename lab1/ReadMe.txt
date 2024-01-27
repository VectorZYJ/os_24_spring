Source code of linker is available in 'linker.cpp'

To use terminal to make file and generate executable file 'linker', 

1. 'cd' to this directory
2. Type 'make' and then enter
3. Executable file 'linker' could be found in this directory

To test 'linker', you should

1. 'mv linker test', move executable 'linker' file to 'test' directory
2. 'cd test', cd to test directory
3. 'python lab1gen.py', run generation program for test input data, this will generate 20 input files in 'test' directory
4. create a new folder 'output_std' for standard output and a new folder 'output' for your output in 'test' directory
5. run './runit.sh output_std ./linker_std' for standard output
6. run './runit.sh output ./linker' for your output
7. run './gradeit output_std output' to compare output results