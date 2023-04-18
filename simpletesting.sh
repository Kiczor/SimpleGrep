#!/bin/bash

./simplegrep -d exampledir for > /dev/null
SIMPLEGREP=$(cat simplegrep.txt | wc -l)
ORIGINALGREP=$(grep -r for ./exampledir | wc -l)

#checking if same number of patterns found
if [ ! $SIMPLEGREP = $ORIGINALGREP ]
then
    echo "different number of patterns found"
else
    echo "OK; same number of patterns found"
fi

DIRRES=$(./simplegrep -d exampledir for | head -n 3)
DIRGOODRES=$'Searched files: 6\nFiles with pattern: 5\nPatterns number: 9'

if [ ! "$DIRGOODRES" = "$DIRRES" ]
then
    echo "wrong output"
else
    echo "OK"
fi

DIRRES=$(./simplegrep -d exampledir/anotherdir for | head -n 3)
DIRGOODRES=$'Searched files: 1\nFiles with pattern: 1\nPatterns number: 3'

if [ ! "$DIRGOODRES" = "$DIRRES" ]
then
    echo "wrong output"
else
    echo "OK"
fi

DIRRES=$(./simplegrep -d exampledir/seconddir for | head -n 3)
DIRGOODRES=$'Searched files: 3\nFiles with pattern: 2\nPatterns number: 3'

if [ ! "$DIRGOODRES" = "$DIRRES" ]
then
    echo "wrong output"
else
    echo "OK"
fi