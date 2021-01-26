#!/usr/bin/env python3

#import libraries
import sys
import csv

#Block Consistency Audits
def invalidReservedBlockCheck():
	for block in blockList:
		if block < 0 or block > highestBlock:
			for dupBlock in blockList[block]:
				#Invalid
				print("INVALID " + str(dupBlock[0]) + "BLOCK " + str(block) + " IN INODE " + str(dupBlock[1]) + " AT OFFSET " + str(dupBlock[2]))
				clean = False
		elif block < firstValidBlock:
			for dupBlock in blockList[block]:
				#Reserved
				print("RESERVED " + str(dupBlock[0]) + "BLOCK " + str(block) + " IN INODE " + str(dupBlock[1]) + " AT OFFSET " + str(dupBlock[2]))
				clean = False
		else:
			#Legal block
			if block in legalBlocks:	#Duplicates
				legalBlocks[block].append(blockList[block])
			else:
				legalBlocks[block] = blockList[block]
	
def freeBlockListCheck():
	for block in range(firstValidBlock, highestBlock):
		#Unallocated block missing from free list
		if block not in freeBlockList and block not in legalBlocks:
			print("UNREFERENCED BLOCK " + str(block))
			clean = False
			
		#Allocated block in free list
		elif block in freeBlockList and block in legalBlocks:
			print("ALLOCATED BLOCK " + str(block) + " ON FREELIST")
			clean = False
			
def duplicateBlockCheck():
	for block in legalBlocks:
		if len(legalBlocks[block]) > 1:
			for dupBlock in legalBlocks[block]:
				print("DUPLICATE " + dupBlock[0] + "BLOCK " + str(block) + " IN INODE " + str(dupBlock[1]) + " AT OFFSET " + str(dupBlock[2]))
				clean = False



#I-node Allocation Audits
def freeInodeListCheck():
	for inode in inodeLinks:
		if inode in freeInodeList:
			print("ALLOCATED INODE " + str(inode) + " ON FREELIST")
			clean = False
	
	unAllocInodeList = [x for x in list(range(1, highestInode + 1)) if x not in inodeLinks and x not in reservedInodeList]	#all inodes - allocated inodes
	
	for inode in unAllocInodeList:
		if inode not in freeInodeList:
			print("UNALLOCATED INODE " + str(inode) + " NOT ON FREELIST")
			clean = False

			
#Directory Consistency Audits
def linkCheck():
	for link in inodeLinks:
		if link in iNodeDirLinks:
			if inodeLinks[link] != iNodeDirLinks[link]:		#Link counts don't match
				print("INODE " + str(link) + " HAS " + str(iNodeDirLinks[link]) + " LINKS BUT LINKCOUNT IS " + str(inodeLinks[link]))
				clean = False
		else:	#Unreferenced I-node
			print("INODE " + str(link) + " HAS " + str(0) + " LINKS BUT LINKCOUNT IS " + str(inodeLinks[link]))
			clean = False

def unAllocDirInodeCheck():
	for inode in dirInodes:
		if inode in freeInodeList and inode not in reservedInodeList:
			for x in dirInodes[inode]:
				print("DIRECTORY INODE " + str(x[1]) + " NAME " + x[0] + " UNALLOCATED INODE " + str(inode))
				clean = False
	
def invalidDirInodeCheck():
	for inode in dirInodes:
		if inode < 1 or inode > highestInode:
			for x in dirInodes[inode]:
				print("DIRECTORY INODE " + str(x[1]) + " NAME " + x[0] + " INVALID INODE " + str(inode))
				clean = False	
			
def checkDots():
	for inode in dirInodes:
		for x in dirInodes[inode]:
			if x[0] == "'.'" and inode != x[1]:
				print("DIRECTORY INODE " + str(x[1]) + " NAME " + x[0] + " LINK TO INODE " + str(inode) + " SHOULD BE " + str(x[1]))
				clean = False
	
			if x[0] == "'..'":
				#Root case
				if x[1] == 2 and inode != 2:
					print("DIRECTORY INODE " + str(2) + " NAME " + x[0] + " LINK TO INODE " + str(inode) + " SHOULD BE " + str(2))
					clean = False
				elif inode != dirParInodes[x[1]]:
					print("DIRECTORY INODE " + str(x[1]) + " NAME " + x[0] + " LINK TO INODE " + str(inode) + " SHOULD BE " + str(x[1]))
					clean = False
	for parent in dirParInodes:
		if parent in dirInodes:
			for x in dirInodes[parent]:
				if x[0] != "'.'" and x[0] != "'..'" and x[1] != dirParInodes[parent]:
					print("DIRECTORY INODE " + str(parent) + " NAME " + "'..'" + " LINK TO INODE " + str(dirParInodes[parent]) + " SHOULD BE " + str(x[1]))
					#print("DIRECTORY INODE " + str(x[1]) + " NAME " + x[0] + " LINK TO INODE " + str(dirParInodes[parent]) + " SHOULD BE " + str(x[1]))
					clean = False
#Main
def main():

	#Globals
	global clean
	clean = True
	global highestBlock
	highestBlock = 0
	global highestInode
	highestInode = 0
	global firstValidBlock
	
	global freeBlockList
	freeBlockList = []
	global freeInodeList
	freeInodeList = []
	
	global blockList
	blockList = {} 		#List of all blocks pointed to by inodes (blockNum:[Level, Inode, Offset])
	global legalBlocks
	legalBlocks = {}	#List of all legal blocks (blockNum:[Level, Inode, Offset])
	
	global inodeLinks
	inodeLinks = {}		#inode:numLinks
	global iNodeDirLinks
	iNodeDirLinks = {}	#inode:numlinks

	global dirInodes
	dirInodes = {}		#"name"'s Inode: name, current directory inode(parent of filename)
	global dirParInodes
	dirParInodes = {}	# '..' parent inode : Inode of '..'
	global reservedInodeList
	reservedInodeList = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
	
	
	

	#Check for proper arguments
	if len(sys.argv) > 2:
		print("Too many arguments supplied.\n", file = sys.stderr)
		sys.exit(1)
	
	elif len(sys.argv) < 2:
		print("CSV file expected as argument.\n", file = sys.stderr)
		sys.exit(1)
		
	filetype = sys.argv[1][(len(sys.argv[1])-4):]
	if filetype != ".csv":
		print("The argument supplied is not a CSV file.\n", file = sys.stderr)
		sys.exit(1)
	
	#Open file
	try:
		with open(sys.argv[1]) as csv_file:
			csv_reader = csv.reader(csv_file, delimiter=',')
			#Read/store data
			for row in csv_reader:
				if row[0] == "BFREE":
					#Store free blocks
					freeBlockList.append(int(row[1]))
				elif row[0] == "IFREE":
					#Store free inodes
					freeInodeList.append(int(row[1]))
				elif row[0] == "GROUP":
					#Save to determine reserved blocks
					numInodes = int(row[3])
					firstInodes = int(row[8])
				elif row[0] == "SUPERBLOCK":
					highestBlock = int(row[1])
					highestInode = int(row[2])
					blockSize = int(row[3])
					inodeSize = int(row[4])
				elif row[0] == "INODE":
					#Store link counts (and allocated inodes) for later
					inodeLinks[int(row[1])] = int(row[6])
					#Don't scan symbolic links < 60
					if row[2] != 's' or int(row[10]) > 60:
						#Check if blocks valid while reading them
						for i in range(12,27):
							if int(row[i]) != 0:
								inodePointer = i - 11
								
								#13, 14, 15 are indirect blocks
								if inodePointer < 13:
									offset = 0
									level = ""
								elif inodePointer == 13:
									offset = 12
									level = "INDIRECT "
								elif inodePointer == 14:
									offset = 12 + 256
									level = "DOUBLE INDIRECT "
								else: #inodePointer == 15
									offset = 12 + 256 + 65536
									level = "TRIPLE INDIRECT "
								
								#Save for later
								if int(row[i]) in blockList:	#Duplicates
									blockList[int(row[i])].append([level, int(row[1]), offset])
								else:
									blockList[int(row[i])] = [ [level, int(row[1]), offset] ]	
				elif row[0] == "INDIRECT":
					if int(row[2]) == 1:
						offset = 12
						level = "INDIRECT "
					elif int(row[2]) == 2:
						offset = 12 + 256
						level = "DOUBLE INDIRECT "
					elif int(row[2]) == 3:
						offset = 12 + 256 + 65536
						level = "TRIPLE INDIRECT "
						
					#Save for later
					if int(row[5]) in blockList:	#Duplicates
						blockList[int(row[5])].append([level, int(row[1]), offset])
					else:
						blockList[int(row[5])] = [ [level, int(row[1]), offset] ]
				elif row[0] == "DIRENT":
					#Count directory references to inodes
					if int(row[3]) in list(iNodeDirLinks.keys()):	#Duplicates
						iNodeDirLinks[int(row[3])] += 1
					else:
						iNodeDirLinks[int(row[3])] = 1
						
					#Store directory I-node info for later
					if int(row[3]) in dirInodes:	#Duplicates
						dirInodes[int(row[3])].append([row[6], int(row[1])])
					else:
						dirInodes[int(row[3])] = [ [row[6], int(row[1])] ]
					if row[6] == "'..'":
						dirParInodes[int(row[1])] = int(row[3])
						
	except IOError:
		print("CSV could not be opened.\n", file = sys.stderr)
		sys.exit(1)
					
	#After gathering all necessary data
	
	#Calculate first valid block
	firstValidBlock = int(((inodeSize*numInodes)/blockSize) + firstInodes)

	#Run checks
	invalidReservedBlockCheck()
	freeBlockListCheck()
	duplicateBlockCheck()
	freeInodeListCheck()
	linkCheck()
	unAllocDirInodeCheck()
	invalidDirInodeCheck()
	checkDots()
	
	#Exit
	if clean:
		sys.exit(0)
	else:
		sys.exit(2)

if __name__ == "__main__": 
  main()