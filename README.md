# SimpleGrep
Program in C++ similar to using "grep -r" command.

To build use: "make simplegrep"

Usage: ./simplegrep \[parameters\] pattern
(pattern is obligatory)

Example:
 ./simplegrep -d "exampledir" -t 3 -l "logs.txt" -r "results.txt" "for"


 Very simple tests in "simpletesting.sh" script