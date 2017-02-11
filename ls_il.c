/* Following program prints the superblock information and other information regarding the Filesystem 

	This program also prints the inode number , permissions , filesize, uid, gid , ilink count and time for each file and folder
	similar to the functionality of ls -il.

	Authors : Charvi Dhingra and Prashant Kuntala.

	usage : type make to compile the program and then to execute type the command as below.

	command : ./ls_il <filesystem> <directory Path>
	example : ./ls_il fsy /hello
		
*/

/*Including the headers*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include "ext2_fs.h"



/*super block  and group descriptor varibales used for populating information */
__u32 totalNoOfInodes; 
__u32 noOfInodesPerGroup;
__u32 noOfInodesPerBlock;
__u32 inodeSize;
__u16 magicSignature;	/* Magic signature */
__u32 freeBlockCount;	/* Free blocks count */
__u32 freeInodeCount;	/* Free inodes count */
__u32 noOfFirstUsefulBlock;
__u32 lastMountTime;
__u32 blockSize;
__u32 noOfBlocks;
__u32 noOfBlocksPerGroup;
__u32 noOfBlockGroups;
__u32 inodeTableBlockNo;
__u16 freeInodesCount; /* Free inodes count */
__u16 directoryCount;	/* Directories count */


/*i_mode flags */
#define EXT2_S_IFSOCK	0xC000	/*socket*/
#define EXT2_S_IFLNK	0xA000	/*symbolic link*/
#define EXT2_S_IFREG	0x8000	/*regular file*/
#define EXT2_S_IFBLK	0x6000	/*block device*/
#define EXT2_S_IFDIR	0x4000	/*directory*/
#define EXT2_S_IFCHR	0x2000	/*character device*/
#define EXT2_S_IFIFO    0x1000	/*fifo*/
#define EXT2_S_IRUSR	0x0100	/*user read*/
#define EXT2_S_IWUSR	0x0080	/*user write*/
#define EXT2_S_IXUSR	0x0040	/*user execute*/
#define EXT2_S_IRGRP	0x0020	/*group read*/
#define EXT2_S_IWGRP	0x0010	/*group write*/
#define EXT2_S_IXGRP	0x0008	/*group execute*/
#define EXT2_S_IROTH	0x0004	/*others read*/
#define EXT2_S_IWOTH	0x0002	/*others write*/
#define EXT2_S_IXOTH	0x0001	/*others execute*/


/* function to compute the file format, user permissions and other flags*/
void calculateFlags(unsigned int imode,char *myFlags){
 
 /*File Format Flag*/
 if((EXT2_S_IFREG & imode) > 0){
    myFlags[0]='-';
 }else if((EXT2_S_IFDIR & imode) > 0){
    myFlags[0]='d';
 }else if((EXT2_S_IFLNK & imode) > 0){
    myFlags[0]='l';
 }else{
 	myFlags[0]='-';
 }
 //printf("%c\n", myFlags[0]);
 /*User Flags*/
 if((EXT2_S_IRUSR & imode) > 0){
    myFlags[1]='r';
 }else{
    myFlags[1]='-';
 }
 if((EXT2_S_IWUSR & imode) > 0){
    myFlags[2]='w';
 }else{
    myFlags[2]='-';
 }
 if((EXT2_S_IXUSR & imode) > 0){
   myFlags[3]='x';
 }else{
    myFlags[3]='-';
 }

 /*Group Flags*/
 if((EXT2_S_IRGRP & imode) > 0){
    myFlags[4]='r';
 }else{
    myFlags[4]='-';
 }
 if((EXT2_S_IWGRP & imode) > 0){
    myFlags[5]='w';
 }else{
    myFlags[5]='-';
 }
 if((EXT2_S_IXGRP & imode) > 0){
    myFlags[6]='x';
 }else{
    myFlags[6]='-';
 }
 
 /*Others Flags*/
 if((EXT2_S_IROTH & imode) > 0){
    myFlags[7]='r';
 }else{
    myFlags[7]='-';
 }
 if((EXT2_S_IWOTH & imode) > 0){
    myFlags[8]='w';
 }else{
    myFlags[8]='-';
 }
 if((EXT2_S_IXOTH & imode) > 0){
   myFlags[9]='x';
 }else{
    myFlags[9]='-';
 }
	myFlags[10]='\0';
}

/*date and time formatting*/
static const char DTformat[] = "%b  %d  %G %R";

/* reads the super block of file system and populates the super block globalvariables. */
int readSB(int fd, struct ext2_super_block *superBlock)
{
	
	/*seek boot block*/
	lseek(fd,1024,SEEK_CUR);

 	/*Read the Super Block*/
 	if(read(fd, superBlock, sizeof(struct ext2_super_block)))
	{
		/*Super Block Read*/
		noOfBlocks = superBlock->s_blocks_count;
		blockSize = 1024 << superBlock->s_log_block_size;
		totalNoOfInodes = superBlock->s_inodes_count;
		noOfInodesPerGroup = superBlock->s_inodes_per_group;
		noOfBlocksPerGroup = superBlock->s_blocks_per_group;		
		noOfFirstUsefulBlock = superBlock->s_first_data_block;
		/* no of block groups calculation */ //something fishy with the formula?
		if(noOfBlocks % noOfBlocksPerGroup != 0)
		{
    			noOfBlockGroups = (noOfBlocks - noOfFirstUsefulBlock)/(noOfBlocksPerGroup);
 		}

		noOfInodesPerBlock = blockSize/sizeof(struct ext2_inode);
		inodeSize=superBlock->s_inode_size;
		magicSignature = superBlock->s_magic;		
		freeBlockCount = superBlock->s_free_blocks_count;
		freeInodeCount = superBlock->s_free_inodes_count;		
		lastMountTime = superBlock->s_mtime;
	

		//printing the time.
	    	 time_t mountTime=lastMountTime;
	     	struct tm timeinfo;
	    	 (void) localtime_r(&mountTime,&timeinfo);
	      	  char buffer[80];
 		//calculating and printing formated time.
   		 strftime(buffer, sizeof(buffer), DTformat, &timeinfo);

		printf("Last mount time : %s\n",buffer);
		printf("No of blockgroups %d\n",((noOfBlocks - noOfFirstUsefulBlock )/noOfBlocksPerGroup));		
 		printf("totalNoOfInodes : %d \n",totalNoOfInodes);
		printf("Filesystem size : %d\n",(noOfBlocks * blockSize));
 		printf("blockSize : %d \n",blockSize);
 		printf("NO of first useful block i.e first data block : %d \n",noOfFirstUsefulBlock);
		printf("freeBlockCount : %d \n",freeBlockCount);
		printf("freeInodeCount : %d \n",freeInodeCount);
		printf("noOfBlocksPerGroup : %d \n",noOfBlocksPerGroup);
 		printf("noOfInodesPerGroup : %d \n",noOfInodesPerGroup);
		printf("magicSignature  : %d \n",magicSignature);
		printf("No of Block groups : %d\n",noOfBlockGroups);		
 		printf("noOfBlocks  : %d \n",noOfBlocks);
 		printf("noOfInodesPerBlock : %d \n",noOfInodesPerBlock);
 		printf("inodeSize : %d \n\n",inodeSize);


 		/*Seek the unused space after Super Block*/
 		lseek(fd,1024,SEEK_CUR); 
 		lseek(fd,1024,SEEK_CUR);
 		return 1;
	}
	return 0;
}



void display(int fd, int inodeNo, char *name){
 	/*higlight ext2_inode structure using inodeNo*/
 	__u32 inodeBlockNumber = inodeTableBlockNo + (noOfBlocksPerGroup * ((inodeNo-1)/noOfInodesPerGroup));
    	/*Get the inode number in that specific  group block*/
    	__u32 inodeNumberInGrpBlk = (inodeNo-1) % noOfInodesPerGroup;    
   	 inodeBlockNumber += inodeNumberInGrpBlk/noOfInodesPerBlock;
    
    struct ext2_inode inode; 
	/*Read inode structure from inode number*/
    lseek(fd, blockSize*inodeBlockNumber + inodeSize*(inodeNumberInGrpBlk % noOfInodesPerBlock), SEEK_SET);
    read(fd, &inode, sizeof(inode));

    char result[11];
    calculateFlags(inode.i_mode,result);
	printf("%s\t ",result);
	printf("%d\t",inodeNo);
    printf("%d\t\t",inode.i_links_count);
    printf("%d\t",inode.i_size);
    printf("%u\t",inode.i_uid);
    printf("%u\t",inode.i_gid);  
//printing the time.
     time_t creationTime=inode.i_mtime;
     struct tm timeinfo;
     (void) localtime_r(&creationTime,&timeinfo);
  	 char buffer[80];
 //calculating and printing formated time.
    strftime(buffer, sizeof(buffer), DTformat, &timeinfo);
    printf("%s\t\t",buffer);
    printf("%s\t\n",name);
}



void Display(int fd,__u32 inodeNo){
    __u32 inodeBlockNumber = inodeTableBlockNo + (noOfBlocksPerGroup * ((inodeNo-1)/noOfInodesPerGroup));  
    __u32 inodeNumberInGrpBlk = (inodeNo-1) % noOfInodesPerGroup;
    
    inodeBlockNumber += inodeNumberInGrpBlk/noOfInodesPerBlock;
    
    struct ext2_inode inode; 
    lseek(fd, blockSize*inodeBlockNumber + inodeSize*(inodeNumberInGrpBlk % noOfInodesPerBlock), SEEK_SET);
	/*Read inode structure from inode number*/
    read(fd, &inode, sizeof(inode));

    printf("permisions \t inode \tilinkcount \tsize \tuid \tgid \ttime \t\t\t\tname \t\n");
   
    int k;
    for(k=0;k<12;k++){
		if(inode.i_block[k] != 0) {
			int rec_len=1;
			__u32 startAddressOfBlock = blockSize*inode.i_block[k]; /*start Address of an inode block*/
			__u32 endAddressOfBlock=blockSize+startAddressOfBlock;/*End address of an inode block*/
  			struct ext2_dir_entry_2 dirEntry;
			__u32 startAddressOfEntry=startAddressOfBlock;

			while(rec_len)
			{	
    				lseek(fd, startAddressOfEntry, SEEK_SET);/*Move the start address of an entry*/
   	 			long bytesRead = read(fd,&dirEntry,sizeof(struct ext2_dir_entry_2));/*Read the entry*/
   	 			rec_len= dirEntry.rec_len;
				//printf("REC Length ----------- %d\n",rec_len);
   	 			char name1[255];
			    	strncpy(name1, dirEntry.name, dirEntry.name_len);
				//printf("directory entry type ----------- %d\n",dirEntry.file_type);
			    	name1[dirEntry.name_len] = '\0';
    				display(fd, dirEntry.inode,name1);
	    	 		if(endAddressOfBlock-startAddressOfEntry == rec_len)
						rec_len=0;
					startAddressOfEntry+=rec_len;/*end address of current directory entry*/
			}	
		}
    }
}


__u32 HRsearch(int ext2fd,char *token, __u32 block_num){
	
	int rec_len=1;
	__u32 startAddressOfBlock = blockSize*block_num; /*start Address of an inode block*/
	__u32 endAddressOfBlock=blockSize+startAddressOfBlock;/*End address of an inode block*/
  	struct ext2_dir_entry_2 dirEntry;
	__u32 startAddressOfEntry=startAddressOfBlock;

	while(rec_len)
	{	
    		lseek(ext2fd, startAddressOfEntry, SEEK_SET);
   	 	long bytesRead = read(ext2fd,&dirEntry,sizeof(struct ext2_dir_entry_2));
   	 	rec_len= dirEntry.rec_len;
		if(dirEntry.file_type == 2){ /*Object is a directory */
			char name1[255];
            	strncpy(name1, dirEntry.name, dirEntry.name_len);
            	name1[dirEntry.name_len] = '\0';
            	if(strcmp(name1, token) == 0) {		
              	 return dirEntry.inode;
            	}
		}
    		if(endAddressOfBlock-startAddressOfEntry == rec_len)
			rec_len=0;
		startAddressOfEntry+=rec_len;
	}
	
return 0;
}

/* Search function to parse through the tokens.
 */

__u32 search(int ext2fd,char tokens[][255],__u32 inode_no,int level,int noOfTokens){
	
	 int o;
	struct ext2_inode inode; 	
	__u32 inodeBlockNumber = inodeTableBlockNo + (noOfBlocksPerGroup * ((inode_no-1)/noOfInodesPerGroup));
        __u32 inodeNumberInGrpBlk = (inode_no-1) % noOfInodesPerGroup;
    	inodeBlockNumber += inodeNumberInGrpBlk/noOfInodesPerBlock;
    
	lseek(ext2fd, blockSize*inodeBlockNumber + inodeSize*(inodeNumberInGrpBlk % noOfInodesPerBlock), SEEK_SET);
        read(ext2fd, &inode, sizeof(inode));
   
    for(o=0;o<12;o++){
    	if(inode.i_block[o]!=0){
    		__u32 newInode= HRsearch(ext2fd,tokens[level],inode.i_block[o]);
    		if(newInode!=0){
    			if(level == noOfTokens){ 
    				return newInode;
    			}else if(level < noOfTokens){
    				newInode = search(ext2fd,tokens,newInode,++level,noOfTokens);
    				return newInode;
    			}
    		}else if(newInode == 0){
    			printf("\nNo Search Found\n");
    			exit(-1);
    		}
    	}
    }
}

int main(int argc, char *argv[])
{
	int ext2fd=open(argv[1],O_RDONLY); /*File descriptor for EXT2 File System*/	
	int level=0; 
	int root_inode_no=2; /*Root Inode Number is always 2*/

	char tokens[10][255];
	const char s[2] = "/";
	char *token = strtok(argv[2], s); 
   	int noOfTokens=0; 

   	while( token != NULL )
   	{
    	strcpy(tokens[noOfTokens++],token);
      	token = strtok(NULL, s);
   	}
	
	/*super block Access */
	struct ext2_super_block superBlock; 
	/*Group Descriptor Access*/
 	struct ext2_group_desc grpDescTable; 

	/*File Open Failure*/	
	if(ext2fd < 0){ 
		printf("File System Might be Corrupted");
	}	
	else{
		
		if(readSB(ext2fd, &superBlock)==1){
			/*Group Descriptor Block*/
			int gDesc_bloc_num=1; 
			lseek(ext2fd, blockSize*gDesc_bloc_num, SEEK_SET);/*seek through the first group descriptor block*/
			read(ext2fd,&grpDescTable,sizeof(grpDescTable));/*read the group descriptor block*/
			
			inodeTableBlockNo=grpDescTable.bg_inode_table;
		
			
			/*Find the inode of the root directory*/
			int inode_no=2; /*for root inode*/
			struct ext2_inode inode; /*Read inode structure from inode number*/
			/*read the ext2_inode Stucture for a given inode_no*/
			/* the group block inode starts from inodes_start + blockspergrup * group# */
			__u32 inodeBlockNumber = inodeTableBlockNo + (noOfBlocksPerGroup * ((inode_no-1)/noOfInodesPerGroup));
    		/*Get the inode number in that specific  group block*/
		/* This will be the inodes offset in that block , no of inodes per blockgrp are fixed. */
    		__u32 inodeNumberInGrpBlk = (inode_no-1) % noOfInodesPerGroup;

    		inodeBlockNumber += inodeNumberInGrpBlk/noOfInodesPerBlock;
    		lseek(ext2fd, blockSize*inodeBlockNumber + inodeSize*(inodeNumberInGrpBlk % noOfInodesPerBlock), SEEK_SET);
    		read(ext2fd, &inode, sizeof(inode));

    		int k;
    		for(k=0;k<12;k++){/*loop through the direct blocks*/
    			if(inode.i_block[k]!=0){ // i_block is a member of inode struct that contains 
    				if(!strcmp(tokens[0],"")){
    					Display(ext2fd,2);
    					return 1;
    				}    				
    				__u32 topLevelInode_No=HRsearch(ext2fd,tokens[0],inode.i_block[k]);
    				if(noOfTokens==1 && topLevelInode_No > 0){						
						Display(ext2fd,topLevelInode_No);
						return 1;
    				}else if((noOfTokens!=1) && (topLevelInode_No > 0)){ 
						int level=1;
						__u32 result_inode = search(ext2fd,tokens,topLevelInode_No,level,noOfTokens-1);
						Display(ext2fd,result_inode);
						return 1;
    				}
    			}
    		}
		}else{
				printf("Unable to read !! ERROR !!\n");
				exit(-1);
			}
	}
}
