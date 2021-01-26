#!/bin/bash

#Exit-code checking function
check()
{
	if [ $? -eq 0 ]
	then
		echo "Correct exit code 0"
	else
		echo "Exit code error"
	fi
}

#Log test
#./lab4b --log=log.txt
#check

#Other tests using log as output
#./lab4b --scale=C --log=log.txt
#check
#./lab4b --scale=F --log=log.txt
#check
#./lab4b --period=5 --log=log.txt
#check


{
echo "START";
sleep 3;
echo "STOP";
sleep 3;
echo "OFF";
sleep 3;
} | ./lab4b --log=log.txt
grep "START" log.txt > out.txt
check
grep "STOP" log.txt > out.txt
check
grep "OFF" log.txt > out.txt
check
grep "SHUTDOWN" log.txt > out.txt
check

{
echo "STOP";
sleep 3;
echo "START";
sleep 3;
echo "OFF";
sleep 3;
} | ./lab4b --log=log.txt
grep "STOP" log.txt > out.txt
check
grep "START" log.txt > out.txt
check
grep "OFF" log.txt > out.txt
check
grep "SHUTDOWN" log.txt > out.txt
check

{
echo "STOP";
sleep 3;
echo "START";
sleep 3;
echo "STOP";
sleep 3;
echo "START";
sleep 3;
echo "OFF";
sleep 3;
} | ./lab4b --log=log.txt
grep "STOP" log.txt > out.txt
check
COUNT=$(wc -l < out.txt)
if [ $COUNT -eq 2 ]
then
	echo "Correct count"
else
	echo "Count error"
fi
grep "START" log.txt > out.txt
check
COUNTt=$(wc -l < out.txt)
if [ $COUNT -eq 2 ]
then
	echo "Correct count"
else
	echo "Count error"
fi
grep "OFF" log.txt > out.txt
check
grep "SHUTDOWN" log.txt > out.txt
check

{ echo "SCALE=C"; sleep 3; echo "OFF"; sleep 3;} | ./lab4b --log=log.txt
grep "SCALE=C" log.txt > out.txt
check

{ echo "SCALE=F"; sleep 3; echo "OFF"; sleep 3;} | ./lab4b --scale=C --log=log.txt
grep "SCALE=F" log.txt > out.txt
check

{ echo "PERIOD=10"; sleep 3; echo "OFF"; sleep 3;} | ./lab4b --log=log.txt
grep "PERIOD=10" log.txt > out.txt
check

{ echo "LOG test"; sleep 3; echo "OFF"; sleep 3;} | ./lab4b --log=log.txt
grep "LOG test" log.txt > out.txt
check

#Delete log files
rm -rf log.txt out.txt
