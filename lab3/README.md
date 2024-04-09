Source code of scheduler is available in all .cpp and .h files

To use terminal to make file and generate executable file 'mmu', 

1. 'cd' to this directory
2. Type 'make' and then enter
3. Executable file 'mmu' could be found in this directory



To test 'mmu', you should

1. 'mv mmu test', move executable 'mmu' file to 'test' directory
2. 'cd test', cd to test directory
4. create a new folder 'output_std' for standard output and a new folder 'output' for your output in 'test' directory
5. run './runit.sh output_std ./sched_std' for standard output
6. run './runit.sh output ./sched' for your output
7. run './gradeit output_std output' to compare output results



You can also use 'mmu_generator' to generate your test case under test directory

Detailed command is available using './mmu_generator -h'
If you want to export test case to file, please use './mmu_generator <params> >> filename'