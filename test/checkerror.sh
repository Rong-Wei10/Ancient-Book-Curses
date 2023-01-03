#!/bin/bash
# This program checks error codes
# The first argument is the command to run
# The second is the error code we expect

# Run the program line
$1

# Check if the error code was what we expected
RV=$?
echo "Expecting error code $2, got code $RV"
if [ $RV -eq $2 ]
then
    echo "Success"
    exit 0
else
    echo "Failed"
    exit 1
fi
