#!/bin/bash

#Variables
LIST=./lab2_list
TH=--threads
IT=--iterations
Y=--yield
S=--sync
L=--lists

rm -rf lab2b_list.csv

#Output to file
exec &>> lab2b_list.csv

#Perform Tests

#Number of operations per second (throughput?)
#Mutex (lab2b_1.png)
$LIST $TH 1 $IT 1000 $S m
$LIST $TH 2 $IT 1000 $S m
$LIST $TH 4 $IT 1000 $S m
$LIST $TH 8 $IT 1000 $S m
$LIST $TH 12 $IT 1000 $S m
$LIST $TH 16 $IT 1000 $S m
$LIST $TH 24 $IT 1000 $S m

#Spin-Lock (lab2b_1.png)
$LIST $TH 1 $IT 1000 $S s
$LIST $TH 2 $IT 1000 $S s
$LIST $TH 4 $IT 1000 $S s
$LIST $TH 8 $IT 1000 $S s
$LIST $TH 12 $IT 1000 $S s
$LIST $TH 16 $IT 1000 $S s
$LIST $TH 24 $IT 1000 $S s


#Wait-for-lock time and average time per operation vs # competing threads
#Mutex (lab2b_2.png)

##Already Done##
#$LIST $TH 1 $IT 1000 $S m
#$LIST $TH 2 $IT 1000 $S m
#$LIST $TH 4 $IT 1000 $S m
#$LIST $TH 8 $IT 1000 $S m
#$LIST $TH 12 $IT 1000 $S m
#$LIST $TH 16 $IT 1000 $S m
#$LIST $TH 24 $IT 1000 $S m

#No sync, Multiple Lists, Yield ID
$LIST $TH 1 $IT 1 $Y id $L 4
$LIST $TH 4 $IT 1 $Y id $L 4
$LIST $TH 8 $IT 1 $Y id $L 4
$LIST $TH 12 $IT 1 $Y id $L 4
$LIST $TH 16 $IT 1 $Y id $L 4

$LIST $TH 1 $IT 2 $Y id $L 4
$LIST $TH 4 $IT 2 $Y id $L 4
$LIST $TH 8 $IT 2 $Y id $L 4
$LIST $TH 12 $IT 2 $Y id $L 4
$LIST $TH 16 $IT 2 $Y id $L 4

$LIST $TH 1 $IT 4 $Y id $L 4
$LIST $TH 4 $IT 4 $Y id $L 4
$LIST $TH 8 $IT 4 $Y id $L 4
$LIST $TH 12 $IT 4 $Y id $L 4
$LIST $TH 16 $IT 4 $Y id $L 4

$LIST $TH 1 $IT 8 $Y id $L 4
$LIST $TH 4 $IT 8 $Y id $L 4
$LIST $TH 8 $IT 8 $Y id $L 4
$LIST $TH 12 $IT 8 $Y id $L 4
$LIST $TH 16 $IT 8 $Y id $L 4

$LIST $TH 1 $IT 16 $Y id $L 4
$LIST $TH 4 $IT 16 $Y id $L 4
$LIST $TH 8 $IT 16 $Y id $L 4
$LIST $TH 12 $IT 16 $Y id $L 4
$LIST $TH 16 $IT 16 $Y id $L 4

#Sync s, Multiple Lists, Yield ID  (lab2b_3.png)
$LIST $TH 1 $IT 10 $Y id $L 4 $S s
$LIST $TH 4 $IT 10 $Y id $L 4 $S s
$LIST $TH 8 $IT 10 $Y id $L 4 $S s
$LIST $TH 12 $IT 10 $Y id $L 4 $S s
$LIST $TH 16 $IT 10 $Y id $L 4 $S s

$LIST $TH 1 $IT 20 $Y id $L 4 $S s
$LIST $TH 4 $IT 20 $Y id $L 4 $S s
$LIST $TH 8 $IT 20 $Y id $L 4 $S s
$LIST $TH 12 $IT 20 $Y id $L 4 $S s
$LIST $TH 16 $IT 20 $Y id $L 4 $S s

$LIST $TH 1 $IT 40 $Y id $L 4 $S s
$LIST $TH 4 $IT 40 $Y id $L 4 $S s
$LIST $TH 8 $IT 40 $Y id $L 4 $S s
$LIST $TH 12 $IT 40 $Y id $L 4 $S s
$LIST $TH 16 $IT 40 $Y id $L 4 $S s

$LIST $TH 1 $IT 80 $Y id $L 4 $S s
$LIST $TH 4 $IT 80 $Y id $L 4 $S s
$LIST $TH 8 $IT 80 $Y id $L 4 $S s
$LIST $TH 12 $IT 80 $Y id $L 4 $S s
$LIST $TH 16 $IT 80 $Y id $L 4 $S s

#Sync m, Multiple Lists, Yield ID (lab2b_3.png)
$LIST $TH 1 $IT 10 $Y id $L 4 $S m
$LIST $TH 4 $IT 10 $Y id $L 4 $S m
$LIST $TH 8 $IT 10 $Y id $L 4 $S m
$LIST $TH 12 $IT 10 $Y id $L 4 $S m
$LIST $TH 16 $IT 10 $Y id $L 4 $S m

$LIST $TH 1 $IT 20 $Y id $L 4 $S m
$LIST $TH 4 $IT 20 $Y id $L 4 $S m
$LIST $TH 8 $IT 20 $Y id $L 4 $S m
$LIST $TH 12 $IT 20 $Y id $L 4 $S m
$LIST $TH 16 $IT 20 $Y id $L 4 $S m

$LIST $TH 1 $IT 40 $Y id $L 4 $S m
$LIST $TH 4 $IT 40 $Y id $L 4 $S m
$LIST $TH 8 $IT 40 $Y id $L 4 $S m
$LIST $TH 12 $IT 40 $Y id $L 4 $S m
$LIST $TH 16 $IT 40 $Y id $L 4 $S m

$LIST $TH 1 $IT 80 $Y id $L 4 $S m
$LIST $TH 4 $IT 80 $Y id $L 4 $S m
$LIST $TH 8 $IT 80 $Y id $L 4 $S m
$LIST $TH 12 $IT 80 $Y id $L 4 $S m
$LIST $TH 16 $IT 80 $Y id $L 4 $S m

#Total operations per sec vs # of threads

#Sync s, Multiple Lists, No yield  (lab2b_5.png)

##Already Done##
#$LIST $TH 1 $IT 1000 $L 1 $S s
#$LIST $TH 2 $IT 1000 $L 1 $S s
#$LIST $TH 4 $IT 1000 $L 1 $S s
#$LIST $TH 8 $IT 1000 $L 1 $S s
#$LIST $TH 12 $IT 1000 $L 1 $S s

$LIST $TH 1 $IT 1000 $L 4 $S s
$LIST $TH 2 $IT 1000 $L 4 $S s
$LIST $TH 4 $IT 1000 $L 4 $S s
$LIST $TH 8 $IT 1000 $L 4 $S s
$LIST $TH 12 $IT 1000 $L 4 $S s

$LIST $TH 1 $IT 1000 $L 8 $S s
$LIST $TH 2 $IT 1000 $L 8 $S s
$LIST $TH 4 $IT 1000 $L 8 $S s
$LIST $TH 8 $IT 1000 $L 8 $S s
$LIST $TH 12 $IT 1000 $L 8 $S s

$LIST $TH 1 $IT 1000 $L 16 $S s
$LIST $TH 2 $IT 1000 $L 16 $S s
$LIST $TH 4 $IT 1000 $L 16 $S s
$LIST $TH 8 $IT 1000 $L 16 $S s
$LIST $TH 12 $IT 1000 $L 16 $S s


#Sync m, Multiple Lists, No yield (lab2b_4.png)

##Already Done##
#$LIST $TH 1 $IT 1000 $L 1 $S m
#$LIST $TH 2 $IT 1000 $L 1 $S m
#$LIST $TH 4 $IT 1000 $L 1 $S m
#$LIST $TH 8 $IT 1000 $L 1 $S m
#$LIST $TH 12 $IT 1000 $L 1 $S m

$LIST $TH 1 $IT 1000 $L 4 $S m
$LIST $TH 2 $IT 1000 $L 4 $S m
$LIST $TH 4 $IT 1000 $L 4 $S m
$LIST $TH 8 $IT 1000 $L 4 $S m
$LIST $TH 12 $IT 1000 $L 4 $S m

$LIST $TH 1 $IT 1000 $L 8 $S m
$LIST $TH 2 $IT 1000 $L 8 $S m
$LIST $TH 4 $IT 1000 $L 8 $S m
$LIST $TH 8 $IT 1000 $L 8 $S m
$LIST $TH 12 $IT 1000 $L 8 $S m

$LIST $TH 1 $IT 1000 $L 16 $S m
$LIST $TH 2 $IT 1000 $L 16 $S m
$LIST $TH 4 $IT 1000 $L 16 $S m
$LIST $TH 8 $IT 1000 $L 16 $S m
$LIST $TH 12 $IT 1000 $L 16 $S m