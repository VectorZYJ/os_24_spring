Source code of IO scheduler is available in all .cpp and .h files

To use terminal to make file and generate executable file 'iosched', 

1. 'cd' to this directory
2. Type 'make' and then enter
3. Executable file 'iosched' could be found in this directory



To test 'iosched', you should

1. 'mv iosched test', move executable 'iosched' file to 'test' directory
2. 'cd test', cd to test directory
4. create a new folder 'output_std' for standard output and a new folder 'output' for your output in 'test' directory
5. run './runit.sh output_std ./sched_std' for standard output
6. run './runit.sh output ./sched' for your output
7. run './gradeit output_std output' to compare output results



You can also use 'iomake' to generate your test case under test directory

Detailed command is available using './iomake [-v] [-t maxtracks] [-i num_ios] [-L lambda] [-f interarrival_factor]'
If you want to export test case to file, please use './iomake [params] >> filename'