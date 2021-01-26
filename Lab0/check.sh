#!/bin/bash

##Tests Using stdin

#Test stdin to stdout (default behavior)
echo "Correct input has been passed to output" | ./lab0



#Test stdin to output file
echo "This text originated in stdin and should appear in my output" | ./lab0 --output output.txt


#Check stdin == output.txt
if [ "This text originated in stdin and should appear in my output" == "$(cat output.txt)" ]
then
	echo "Correct stdin == Output"
else
	echo "Input does not match output"
fi


#Check exit success
if [ $? -eq 0 ]
then
	echo "Correct exit code 0"
else
	echo "Exit code error"
fi




##Tests Using Input File

#Create input file
echo "This text originated in my input and should appear in my output" > input.txt


#Run with input.txt as input, output.txt as output
./lab0 --input input.txt --output output.txt


#Check input.txt == output.txt
if [ "$(cat input.txt)" == "$(cat output.txt)" ]
then
	echo "Correct Input == Output"
else
	echo "Input does not match output"
fi


#Check exit success
if [ $? -eq 0 ]
then
	echo "Correct exit code 0"
else
	echo "Exit code error"
fi



#Run with nonexistant file as input
./lab0 --input nofile.txt  2> error.txt


#Check for file not found error, exit code 2
if [ $? -eq 2 ]
then
	echo "Correct file not found exit"
else
	echo "Incorrect file not found exit"
fi



#Change permission of output.txt to read-only
chmod u-w output.txt


#Run with input.txt as input, output.txt as output
./lab0 --input input.txt --output output.txt 2> error.txt


#Check for write error, exit code 3
if [ $? -eq 3 ]
then
	echo "Correct file not writable exit"
else
	echo "Incorrect file not writable exit"
fi




##Tests for other options

#Run with segfault catch flags
./lab0 --segfault --catch 2> error.txt


#Check for segfault catch, exit code 4
if [ $? -eq 4 ]
then
	echo "Correct Segfault catch success"
else
	echo "Segfault catch error"
fi



#Test with unrecognized option
./lab0 --bogus 2> error.txt

#Check incorrect option error, exit 1
if [ $? -eq 1 ]
then
	echo "Correct exit code 1"
else
	echo "Exit code error"
fi



#Delete output, error files
$(rm -f input.txt output.txt error.txt)
