#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "ext2_fs.h"

////////////////Globals///////////////////////
int imageFD;

//Superblock
struct ext2_super_block superBlock;
//Not necessary but make atrribute use easier
int numBlocks;
int numInodes;
int blockSize;
int numGroups;
int blocksPerGroup;
int iNodesPerGroup;

int freeBlockBitmapBlockNumber;
int freeInodeBitmapBlockNumber;
int firstInodeBlockNumber;

//////////////Functions//////////////////////

//"A function that takes in a block number and outputs its offset would be useful"
int getBlockOffset(int blockNumber){
	return 1024 + ((blockNumber-1)*blockSize);
}

void scanFreeBlockBitmap(int groupNum){
	//Get bitmap location
	int bitmapLoc = getBlockOffset(freeBlockBitmapBlockNumber);
	
	//Hold bitmap as string
	char* bitmapString = malloc(blocksPerGroup/8);
	
	//Read in bitmap
	pread(imageFD, bitmapString, blocksPerGroup/8, bitmapLoc);
	
	//Set starting block number
	int blockNumber = superBlock.s_first_data_block + groupNum * blocksPerGroup;
	
	//Iterate through bits in bitmap byte by byte (2D Array of bits)
	//Each char in bitmap string holds 8 bits
	int i;
	int j;
	int bitSelector;
	for(i = 0, bitSelector = 1; i < blocksPerGroup/8; i++, bitSelector = 1){
		for(j = 0; j < 8; j++, blockNumber++, bitSelector <<= 1){
			//1 = used, 0 = free
			if(!(bitmapString[i] & bitSelector)){
				//Print required info
				fprintf(stdout, "BFREE,%d\n", blockNumber);
			}
		}
	}
	
	free(bitmapString);
}

void scanDirEntry(int parentInodeNum, int blockNum){
	//Structure to hold directory attributes
	struct ext2_dir_entry dir;
	
	//Get starting block position
	int dirLoc = getBlockOffset(blockNum);
	
	//Iterate through each directory entry
	int i;
	//int entryLen = 0;
	for(i = 0; i < blockSize; i += dir.rec_len){
		//Read directory attributes into structure
		pread(imageFD, &dir, sizeof(dir), dirLoc + i);
		
		if(dir.inode != 0){		//Check for valid entry
			//Extract required attributes
			int byteOffset = i;
			int fileInode = dir.inode;
			int entryLen = dir.rec_len;
			int nameLen = dir.name_len;
			
			char name[260] = "\'\0";
			strncat(name, dir.name, nameLen);
			strcat(name, "\'");
			
			//Print attributes
			fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,%s\n", parentInodeNum, byteOffset, fileInode, entryLen, nameLen, name);
		}
	}
}

void mmddyyhhmmss(char* timeBuffer, time_t epochTime){
	struct tm ts = *gmtime(&epochTime);
    strftime(timeBuffer, 80, "%m/%d/%y %H:%M:%S", &ts);	// mm/dd/yy hh:mm:ss, GMT
}

void indirect(char fileType, int startLevel, int iLevel, int iNodeNumber, int iBlockNum){
	//End condition
	//If next level of indirection doesn't exist, return
	if(iLevel == 0)	//Block not allocated
		return;
		
	//Find position of indirect block
	int iblockPos = getBlockOffset(iBlockNum);
		
	//Store array of iblocks
	uint32_t *iblockArray = malloc(blockSize);
	pread(imageFD, iblockArray, blockSize, iblockPos);
	
	//Iterate through iblocks
	unsigned int k;
	int logicalOffset;
	for(k = 0; k < (blockSize / sizeof(uint32_t)); k++){
		if(iblockArray[k] != 0){	//Only non-zero block pointers
			//Get logical offset from startLevel
			switch(startLevel){
				case 3:
					logicalOffset = 12 + 256 + 65536 + k;
					break;
				case 2:
					logicalOffset = 12 + 256 + k;
					break;
				case 1:
					logicalOffset = 12 + k;
					break;
				default:
					logicalOffset = 0;
			}
			
			//Print directory summary
			if(fileType == 'd' && iLevel == 1)
				scanDirEntry(iNodeNumber, iblockArray[k]);
			
			//Print
			fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", iNodeNumber, iLevel, logicalOffset, iBlockNum, iblockArray[k]);
			
			//Recurse
			indirect(fileType, startLevel, iLevel-1, iNodeNumber, iblockArray[k]);
		}
	}
	free(iblockArray);
}

void scanInodes(int groupNum, int iNodeNumber){
	//Create temporary i-Node structure
	struct ext2_inode inode;
	
	//Get inode location
	int iNodeLoc = getBlockOffset(firstInodeBlockNumber) + ( (iNodeNumber - (iNodesPerGroup*groupNum + 1) )*sizeof(inode));
	
	//Read in i-Node
	pread(imageFD, &inode, sizeof(inode), iNodeLoc);
	
	//Abort if mode or link count are 0
	if(inode.i_mode == 0 || inode.i_links_count == 0){
		return;
	}
	
	//Extract required info
	char fileType;
	switch(inode.i_mode & 0xF000){	//Upper 4 bits of mode
		case 0x8000:	//regular file
			fileType = 'f';
			break;
		case 0x4000:	//directory
			fileType = 'd';
			break;
		case 0xA000:	//symbolic link
			fileType = 's';
			break;
		default:
			fileType = '?';
			break;
	}
	
	int mode = inode.i_mode & 07777; //Octal lower 12 bits
	int owner = inode.i_uid;
	int group = inode.i_gid;
	int linkCount = inode.i_links_count;
	
	char creatTime[80], modTime[80], accessTime[80];
	mmddyyhhmmss(creatTime, inode.i_ctime);
	mmddyyhhmmss(modTime, inode.i_mtime);
	mmddyyhhmmss(accessTime, inode.i_atime);
	int fileSize = inode.i_size;
	int numBlocksOcupied = inode.i_blocks;
	
	//Print I-node attributes
	fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", iNodeNumber, fileType, mode, owner, group, linkCount, creatTime, modTime, accessTime, fileSize, numBlocksOcupied);
	
	if(fileType == 'f' || fileType == 'd' || (fileType == 's' && fileSize > 60)){	//For files and directories and symbolic links > 60 bytes
		//Print block addresses
		int i;
		for(i = 0; i < 15; i++){	//iterate through iblock array
			fprintf(stdout, ",%d", inode.i_block[i]);	//print each block number
		}
	}

	//Terminate
	fprintf(stdout, "\n");
	
	//Directories
	if(fileType == 'd'){
		int i;
		for(i = 0; i < 12; i ++){	//Scan every data block
			if(inode.i_block[i] != 0)
				scanDirEntry(iNodeNumber, inode.i_block[i]);
		}
	}
	
	//Indirect blocks
	if(fileType == 'f' || fileType == 'd'){
		int j;
		for(j = 12; j < 15; j ++){
			//logicalOffset = 0;
			indirect(fileType, j-11, j-11, iNodeNumber, inode.i_block[j]);
		}
	}
}

void scanInodeBitmap(int groupNum){
	//Get bitmap location
	int bitmapLoc = getBlockOffset(freeInodeBitmapBlockNumber);
	
	//Hold bitmap as string
	char* bitmapString = malloc(iNodesPerGroup/8);
	
	//Read in bitmap
	pread(imageFD, bitmapString, iNodesPerGroup/8, bitmapLoc);
	
	//Set starting I-node number
	int iNodeNumber = iNodesPerGroup*groupNum + 1;
	
	//Iterate through bits in bitmap byte by byte (2D Array of bits)
	//Each char in bitmap string holds 8 bits
	int i;
	int j;
	int bitSelector;
	for(i = 0, bitSelector = 1; i < iNodesPerGroup/8; i++, bitSelector = 1){
		for(j = 0; j < 8; j++, iNodeNumber++, bitSelector <<= 1){
			//1 = used, 0 = free
			if(!(bitmapString[i] & bitSelector)){ //Free
				//Print required info
				fprintf(stdout, "IFREE,%d\n", iNodeNumber);
			}
			else	//Allocated
				scanInodes(groupNum, iNodeNumber);
		}
	}
	free(bitmapString);
}

void scanGroups(int groupNum){
	
	//Account for not knowing length of last group
	int numBlocksInGroup;
	int numInodesInGroup;
	if(groupNum == numGroups-1){
		numBlocksInGroup = numBlocks - blocksPerGroup*(numGroups-1);
		numInodesInGroup = numInodes - iNodesPerGroup*(numGroups-1);;
	}
	else{
		numBlocksInGroup = blocksPerGroup;
		numInodesInGroup = iNodesPerGroup;
	}
	
	//Use block group descriptor (BGD) to gather other info
	struct ext2_group_desc BGD;
	
	//Starts at first block after superblock - 3rd block on 1KB block FS or 2nd block on >=2KB block FS
////Can be moved outside loop if necessary////
	int bgdLoc; //first block group descriptor location
	if(blockSize == 1024)
		bgdLoc = 2048;
	else
		bgdLoc = 1024;
	
	//BGD location for this particular group
	int thisGroupBGD = bgdLoc + (groupNum*32);	//Each BGD is 32 bits long
	
	//Read BGD
	pread(imageFD, &BGD, sizeof(BGD), thisGroupBGD);
	
	//Extract required info
	int numFreeBlocks = BGD.bg_free_blocks_count;
	int numFreeInodes = BGD.bg_free_inodes_count;
	freeBlockBitmapBlockNumber = BGD.bg_block_bitmap;
	freeInodeBitmapBlockNumber = BGD.bg_inode_bitmap;
	firstInodeBlockNumber = BGD.bg_inode_table;
	
	
	//Print required info
	fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", groupNum, numBlocksInGroup, numInodesInGroup, numFreeBlocks, numFreeInodes, freeBlockBitmapBlockNumber, freeInodeBitmapBlockNumber, firstInodeBlockNumber);
}

//"Superblock is located at an offset of 1024 bytes from the start of the device"
//Superblock is 1 block long
//"You will need to refer to superblock data several times: make it accessible! (save it in a structure)"
void getSuperBlock(){
	//Save superblock to structure
	pread(imageFD, &superBlock, sizeof(superBlock), 1024);
	
	//Extract required info
	numBlocks = superBlock.s_blocks_count;
	numInodes = superBlock.s_inodes_count;
	blockSize = 1024 << superBlock.s_log_block_size;
	int iNodeSize = superBlock.s_inode_size;
	blocksPerGroup = superBlock.s_blocks_per_group; //For all but last group
	iNodesPerGroup = superBlock.s_inodes_per_group; //For all but last group
	int firstNonreservedInode = superBlock.s_first_ino;
	numGroups = (numBlocks + (blocksPerGroup-1))/blocksPerGroup; //Division but always rounded up
	
	//Print required info
	fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n", numBlocks, numInodes, blockSize, iNodeSize, blocksPerGroup, iNodesPerGroup, firstNonreservedInode);
}

//////////////////Main//////////////////////
int main(int argc, char* argv[]){
	if( argc > 2 ) {
		fprintf(stderr, "Too many arguments supplied.\n");
		exit(1);
	}
	else if( argc < 2 ) {
		fprintf(stderr, "Image file expected as argument.\n");
		exit(1);
	}
	
	char* filetype = argv[1] + (strlen(argv[1])-4);
	//fprintf(stdout, "Filetype: %s", filetype);
	if(strcmp(filetype, ".img") != 0){
		fprintf(stderr, "The argument supplied is not a disk image.\n");
		exit(1);
	}
	
	//Open file
	imageFD = open(argv[1], O_RDONLY);
	if(imageFD == -1){
		fprintf(stderr, "Could not open image file: %s\n", strerror(errno));
		exit(1);
	}
	
	//Superblock
	getSuperBlock();
	
	//Groups
	int i;
	for(i = 0; i < numGroups; i++){
		scanGroups(i);
		scanFreeBlockBitmap(i);
		scanInodeBitmap(i);
	}
	
	exit(0);
}