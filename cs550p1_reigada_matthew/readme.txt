Matthew Reigada

This library creates an alternative malloc library implementation.
Behavior for this library is undefined for simultaneous use with
standard malloc implementations and simultaneous usage may result in
memory leaks or unintended process memory retention.  This malloc
utilizes a BST sorted by address location to merge and allocate
free memory chunks.  For convenience, instructor test files and
makefiles have been provided, these may be removed if unnecessary.
All work by student is in malloc.c and readme.txt.