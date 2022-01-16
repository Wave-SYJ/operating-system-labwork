# Projects for an Operating Systems Class

This repository holds a number of projects that can be used in an 
operating systems class aimed at upper-level undergraduates, from either
 **Southeast University**, or **Efrei Paris**.

Also available are some tests to see if your code works. A specific 
testing script, found in each project directory, can be used to run the 
tests against your code.

For example, in the initial utilities project, the relatively simple `seucat` program that you create can be tested by running the `test-seucat.sh` script. This could be accomplished by the following commands:

```
prompt> cd projects/initial-utilities/seucat
prompt> emacs -nw seucat.c 
prompt> gcc -o seucat seucat.c -Wall
prompt> sudo chmod 777 test-seucat.sh
prompt> ./test-seucat.sh
test 1: passed
test 2: passed
test 3: passed
test 4: passed
test 5: passed
test 6: passed
test 7: passed
prompt> 
```

## Syllabus of OS Labs

| **Chapter**                 | **#** | **Project**                 |
| --------------------------- | ----- | --------------------------- |
| Introduction                | 1     | <u>Unix Utilities</u>       |
| Operating System Structures | 2     | <u>Xv6 Syscall (part 1)</u> |
| Processes                   | 3     | <u>Unix Shell</u>           |
| Process Synchronization     | 6     | <u>Xv6 Syscall (part 2)</u> |
| Deadlocks                   | 7     | Map Reduce                  |
