Matthew Reigada

This library creates an alternative malloc library implementation.
Behavior for this library is undefined for simultaneous use with
other malloc implementations and simultaneous usage may result in
memory leaks or unintended process memory retention.  This malloc
utilizes a BST sorted by address location to merge and allocate
free memory chunks.