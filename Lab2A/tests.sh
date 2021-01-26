#!/bin/bash

#Variables
ADD=./lab2_add
TH=--threads
IT=--iterations
Y=--yield
S=--sync

LIST=./lab2_list

#Output to file
exec &>> lab2_add.csv

#Perform Tests

#Single Thread, No Yield (lab2_add-3.png)
$ADD $TH 1 $IT 100
$ADD $TH 1 $IT 1000
$ADD $TH 1 $IT 10000
$ADD $TH 1 $IT 100000

#Multi-Thread, No Yield (lab2_add-2.png)
$ADD $TH 2 $IT 100
$ADD $TH 2 $IT 1000
$ADD $TH 2 $IT 10000
$ADD $TH 2 $IT 100000

#$ADD $TH 4 $IT 100
#$ADD $TH 4 $IT 1000
#$ADD $TH 4 $IT 10000
#$ADD $TH 4 $IT 100000

$ADD $TH 8 $IT 100
$ADD $TH 8 $IT 1000
$ADD $TH 8 $IT 10000
$ADD $TH 8 $IT 100000

#$ADD $TH 12 $IT 100
#$ADD $TH 12 $IT 1000
#$ADD $TH 12 $IT 10000
#$ADD $TH 12 $IT 100000

#$ADD $TH 16 $IT 100
#$ADD $TH 16 $IT 1000
#$ADD $TH 16 $IT 10000
#$ADD $TH 16 $IT 100000

#With Yield (lab2_add-1.png)
$ADD $TH 2 $IT 10 $Y
$ADD $TH 2 $IT 20 $Y
$ADD $TH 2 $IT 40 $Y
$ADD $TH 2 $IT 80 $Y

$ADD $TH 2 $IT 100 $Y
$ADD $TH 2 $IT 1000 $Y
$ADD $TH 2 $IT 10000 $Y
$ADD $TH 2 $IT 100000 $Y

$ADD $TH 4 $IT 10 $Y
$ADD $TH 4 $IT 20 $Y
$ADD $TH 4 $IT 40 $Y
$ADD $TH 4 $IT 80 $Y

$ADD $TH 4 $IT 100 $Y
$ADD $TH 4 $IT 1000 $Y
$ADD $TH 4 $IT 10000 $Y
$ADD $TH 4 $IT 100000 $Y

$ADD $TH 8 $IT 10 $Y
$ADD $TH 8 $IT 20 $Y
$ADD $TH 8 $IT 40 $Y
$ADD $TH 8 $IT 80 $Y

$ADD $TH 8 $IT 100 $Y
$ADD $TH 8 $IT 1000 $Y
$ADD $TH 8 $IT 10000 $Y
$ADD $TH 8 $IT 100000 $Y

$ADD $TH 12 $IT 10 $Y
$ADD $TH 12 $IT 20 $Y
$ADD $TH 12 $IT 40 $Y
$ADD $TH 12 $IT 80 $Y

$ADD $TH 12 $IT 100 $Y
$ADD $TH 12 $IT 1000 $Y
$ADD $TH 12 $IT 10000 $Y
$ADD $TH 12 $IT 100000 $Y

#With Yield, Sync (lab2_add-4.png)
$ADD $TH 2 $IT 10000 $Y $S m
$ADD $TH 2 $IT 1000 $Y $S s
$ADD $TH 2 $IT 10000 $Y $S c

$ADD $TH 4 $IT 10000 $Y $S m
$ADD $TH 4 $IT 1000 $Y $S s
$ADD $TH 4 $IT 10000 $Y $S c

$ADD $TH 8 $IT 10000 $Y $S m
$ADD $TH 8 $IT 1000 $Y $S s
$ADD $TH 8 $IT 10000 $Y $S c

$ADD $TH 12 $IT 10000 $Y $S m
$ADD $TH 12 $IT 1000 $Y $S s
$ADD $TH 12 $IT 10000 $Y $S c

#With Sync, No Yield (lab2_add-5.png)
$ADD $TH 1 $IT 10000 $S m
$ADD $TH 1 $IT 10000 $S s
$ADD $TH 1 $IT 10000 $S c

$ADD $TH 2 $IT 10000 $S m
$ADD $TH 2 $IT 10000 $S s
$ADD $TH 2 $IT 10000 $S c

$ADD $TH 4 $IT 10000 $S m
$ADD $TH 4 $IT 10000 $S s
$ADD $TH 4 $IT 10000 $S c

$ADD $TH 8 $IT 10000 $S m
$ADD $TH 8 $IT 10000 $S s
$ADD $TH 8 $IT 10000 $S c

$ADD $TH 12 $IT 10000 $S m
$ADD $TH 12 $IT 10000 $S s
$ADD $TH 12 $IT 10000 $S c



exec &>> lab2_list.csv

#Single Thread, No Yield (lab2_list-1.png)
$LIST $TH 1 $IT 10
$LIST $TH 1 $IT 100
$LIST $TH 1 $IT 1000
$LIST $TH 1 $IT 10000
$LIST $TH 1 $IT 20000

#Multi-Thread, No Yield (lab2_list-2.png)
$LIST $TH 2 $IT 1
$LIST $TH 2 $IT 10
$LIST $TH 2 $IT 100
$LIST $TH 2 $IT 1000

$LIST $TH 4 $IT 1
$LIST $TH 4 $IT 10
$LIST $TH 4 $IT 100
$LIST $TH 4 $IT 1000

$LIST $TH 8 $IT 1
$LIST $TH 8 $IT 10
$LIST $TH 8 $IT 100
$LIST $TH 8 $IT 1000

$LIST $TH 12 $IT 1
$LIST $TH 12 $IT 10
$LIST $TH 12 $IT 100
$LIST $TH 12 $IT 1000

#Multi-Thread, Yield (lab2_list-2.png)
$LIST $TH 2 $IT 1 $Y i
$LIST $TH 2 $IT 2 $Y i
$LIST $TH 2 $IT 4 $Y i
$LIST $TH 2 $IT 8 $Y i
$LIST $TH 2 $IT 16 $Y i
$LIST $TH 2 $IT 32 $Y i

$LIST $TH 2 $IT 1 $Y d
$LIST $TH 2 $IT 2 $Y d
$LIST $TH 2 $IT 4 $Y d
$LIST $TH 2 $IT 8 $Y d
$LIST $TH 2 $IT 16 $Y d
$LIST $TH 2 $IT 32 $Y d

$LIST $TH 2 $IT 1 $Y il
$LIST $TH 2 $IT 2 $Y il
$LIST $TH 2 $IT 4 $Y il
$LIST $TH 2 $IT 8 $Y il
$LIST $TH 2 $IT 16 $Y il
$LIST $TH 2 $IT 32 $Y il

$LIST $TH 2 $IT 1 $Y dl
$LIST $TH 2 $IT 2 $Y dl
$LIST $TH 2 $IT 4 $Y dl
$LIST $TH 2 $IT 8 $Y dl
$LIST $TH 2 $IT 16 $Y dl
$LIST $TH 2 $IT 32 $Y dl


$LIST $TH 4 $IT 1 $Y i
$LIST $TH 4 $IT 2 $Y i
$LIST $TH 4 $IT 4 $Y i
$LIST $TH 4 $IT 8 $Y i
$LIST $TH 4 $IT 16 $Y i
$LIST $TH 4 $IT 32 $Y i

$LIST $TH 4 $IT 1 $Y d
$LIST $TH 4 $IT 2 $Y d
$LIST $TH 4 $IT 4 $Y d
$LIST $TH 4 $IT 8 $Y d
$LIST $TH 4 $IT 16 $Y d
$LIST $TH 4 $IT 32 $Y d

$LIST $TH 4 $IT 1 $Y il
$LIST $TH 4 $IT 2 $Y il
$LIST $TH 4 $IT 4 $Y il
$LIST $TH 4 $IT 8 $Y il
$LIST $TH 4 $IT 16 $Y il
$LIST $TH 4 $IT 32 $Y il

$LIST $TH 4 $IT 1 $Y dl
$LIST $TH 4 $IT 2 $Y dl
$LIST $TH 4 $IT 4 $Y dl
$LIST $TH 4 $IT 8 $Y dl
$LIST $TH 4 $IT 16 $Y dl
$LIST $TH 4 $IT 32 $Y dl


$LIST $TH 8 $IT 1 $Y i
$LIST $TH 8 $IT 2 $Y i
$LIST $TH 8 $IT 4 $Y i
$LIST $TH 8 $IT 8 $Y i
$LIST $TH 8 $IT 16 $Y i
$LIST $TH 8 $IT 32 $Y i

$LIST $TH 8 $IT 1 $Y d
$LIST $TH 8 $IT 2 $Y d
$LIST $TH 8 $IT 4 $Y d
$LIST $TH 8 $IT 8 $Y d
$LIST $TH 8 $IT 16 $Y d
$LIST $TH 8 $IT 32 $Y d

$LIST $TH 8 $IT 1 $Y il
$LIST $TH 8 $IT 2 $Y il
$LIST $TH 8 $IT 4 $Y il
$LIST $TH 8 $IT 8 $Y il
$LIST $TH 8 $IT 16 $Y il
$LIST $TH 8 $IT 32 $Y il

$LIST $TH 8 $IT 1 $Y dl
$LIST $TH 8 $IT 2 $Y dl
$LIST $TH 8 $IT 4 $Y dl
$LIST $TH 8 $IT 8 $Y dl
$LIST $TH 8 $IT 16 $Y dl
$LIST $TH 8 $IT 32 $Y dl


$LIST $TH 12 $IT 1 $Y i
$LIST $TH 12 $IT 2 $Y i
$LIST $TH 12 $IT 4 $Y i
$LIST $TH 12 $IT 8 $Y i
$LIST $TH 12 $IT 16 $Y i
$LIST $TH 12 $IT 32 $Y i

$LIST $TH 12 $IT 1 $Y d
$LIST $TH 12 $IT 2 $Y d
$LIST $TH 12 $IT 4 $Y d
$LIST $TH 12 $IT 8 $Y d
$LIST $TH 12 $IT 16 $Y d
$LIST $TH 12 $IT 32 $Y d

$LIST $TH 12 $IT 1 $Y il
$LIST $TH 12 $IT 2 $Y il
$LIST $TH 12 $IT 4 $Y il
$LIST $TH 12 $IT 8 $Y il
$LIST $TH 12 $IT 16 $Y il
$LIST $TH 12 $IT 32 $Y il

$LIST $TH 12 $IT 1 $Y dl
$LIST $TH 12 $IT 2 $Y dl
$LIST $TH 12 $IT 4 $Y dl
$LIST $TH 12 $IT 8 $Y dl
$LIST $TH 12 $IT 16 $Y dl
$LIST $TH 12 $IT 32 $Y dl

#12 Threads, 32 Iterations, Yield, Sync (lab2_list-3.png)
$LIST $TH 12 $IT 32 $Y i $S s
$LIST $TH 12 $IT 32 $Y i $S m

$LIST $TH 12 $IT 32 $Y d $S s
$LIST $TH 12 $IT 32 $Y d $S m

$LIST $TH 12 $IT 32 $Y il $S s
$LIST $TH 12 $IT 32 $Y il $S m

$LIST $TH 12 $IT 32 $Y dl $S s
$LIST $TH 12 $IT 32 $Y dl $S m

#1000 Iterations, Sync (lab2_list-4.png)
$LIST $TH 1 $IT 1000 $S s
$LIST $TH 2 $IT 1000 $S s
$LIST $TH 4 $IT 1000 $S s
$LIST $TH 8 $IT 1000 $S s
$LIST $TH 12 $IT 1000 $S s
$LIST $TH 16 $IT 1000 $S s
$LIST $TH 24 $IT 1000 $S s

$LIST $TH 1 $IT 1000 $S m
$LIST $TH 2 $IT 1000 $S m
$LIST $TH 4 $IT 1000 $S m
$LIST $TH 8 $IT 1000 $S m
$LIST $TH 12 $IT 1000 $S m
$LIST $TH 16 $IT 1000 $S m
$LIST $TH 24 $IT 1000 $S m
