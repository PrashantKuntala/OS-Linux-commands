/* Following program prints file contents and file attributes. similar to the cat command.

	Authors : Charvi Dhingra and Prashant Kuntala.

	usage : type make to compile the program and then to execute type the command as below.

	command : ./cat <filesystem> <directory Path>
	example : ./cat fsy /hello/hi.txt
		
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "ext2_fs.h"
#include <string.h>
#include <time.h>

/* #define's used for i_mode flag*/
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



/*super block variables*/
__u32 noOfInodes; 
__u32 noOfInodesPerGroup;
__u32 noOfInodesPerBlock;
__u32 inodeSize;
__u32 blockSize;
__u32 noOfBlocks;
__u32 noOfBlocksPerGroup;
__u32 noOfBlockGroups;

/* group descriptor table variables */
__u32 inodeTableBlockNo;
__u32 inodeBitmapBlockNo;
__u32 blockBitmapBlockNo;

/*date and time formatting*/
static const char DTformat[] = "%b %d %G %R";

/* function to compute the file format, user permissions and other flags*/
void calculateFlags(unsigned int imode,char *myFlags)
{
	/*File Format Flags*/
 	if((EXT2_S_IFREG & imode) > 0){
    	myFlags[0]='-';
 	}else if((EXT2_S_IFDIR & imode) > 0){
    	myFlags[0]='d';
 	}else if((EXT2_S_IFLNK & imode) > 0){
    	myFlags[0]='l';
 	}else{
 		myFlags[0]='-';
 	}

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


/* reads the super block of file system and populates the super block globalvariables. */
int readSB(int fd, struct ext2_super_block *superBlock){
	lseek(fd,1024,SEEK_CUR);/*seek boot block*/
 	/*Read the Super Block*/
 	if(read(fd, superBlock, sizeof(struct ext2_super_block))){/*Super Block Read*/
		noOfBlocks=superBlock->s_blocks_count;
		blockSize=1024 << superBlock->s_log_block_size;
		noOfInodes=superBlock->s_inodes_count;
		noOfInodesPerGroup=superBlock->s_inodes_per_group;
		noOfBlocksPerGroup=superBlock->s_blocks_per_group;

		if(noOfBlocks % noOfBlocksPerGroup != 0){
    			noOfBlockGroups = noOfBlocks/noOfBlocksPerGroup + 1;
 		}else{
  				noOfBlockGroups = noOfBlocks/noOfBlocksPerGroup;
		}
		noOfInodesPerBlock = blockSize/sizeof(struct ext2_inode);
		inodeSize=superBlock->s_inode_size;
 		
 		/*Seek the unused space after Super Block*/
 		lseek(fd,1024,SEEK_CUR); 
 		lseek(fd,1024,SEEK_CUR);
 		return 1;
	}
	return 0;
}

/* directorySearch function searches for a given directory in a block and returns 0 if its not found and returns the inode number if it is found. */
__u32 directorySearch(int fd, int inode_blockNo, char *dir){ //prash
	struct ext2_dir_entry_2 dirEntry;
	int rec_len=1;
	char *buff;
	int i;
	__u32 startAddressOfBlock = blockSize*inode_blockNo; /*start Address of an inode block*/
	__u32 endAddressOfBlock=blockSize+startAddressOfBlock;/*End address of an inode block*/
	__u32 startAddressOfEntry=startAddressOfBlock;

	while(rec_len){ /*loop through  the entries*/
    	lseek(fd, startAddressOfEntry, SEEK_SET);/*Move the start address of an entry*/
   	 	long bytesRead = read(fd,&dirEntry,sizeof(struct ext2_dir_entry_2));/*Read the entry*/
   	 	rec_len= dirEntry.rec_len;/*get rec_len of the entry*/
		char c_object[255];
		strncpy(c_object,dirEntry.name,dirEntry.name_len);
		c_object[dirEntry.name_len]='\0';
		if(strcmp(c_object,dir) == 0){ /*object found*/
			return dirEntry.inode; /*returns the inode number if an object is found*/
		}
    	if(endAddressOfBlock-startAddressOfEntry == rec_len)
			rec_len=0;
		startAddressOfEntry+=rec_len;/*end address of current directory entry*/
	}
return 0; /*0 if not found*/
}


/* search function searches the given inode_no and token and return the inode number if found else it would return 0. */
__u32 search(int ext2fd, __u32 inode_no, char *token){
	struct ext2_dir_entry_2 dir_entry; /*Access directory Entries*/
	
	__u32 inode_bloc_no = inodeTableBlockNo + (noOfBlocksPerGroup * ((inode_no-1)/noOfInodesPerGroup));
 
    __u32 inode_no_in_groupBlock = (inode_no-1) % noOfInodesPerGroup;
    inode_bloc_no += inode_no_in_groupBlock/noOfInodesPerBlock;

    struct ext2_inode inode;
    lseek(ext2fd, blockSize*inode_bloc_no + inodeSize*(inode_no_in_groupBlock % noOfInodesPerBlock), SEEK_SET);
    read(ext2fd, &inode, sizeof(inode));

    int i;
	for(i = 0; i < 12; i++) { 
		if(inode.i_block[i] != 0) {
			__u32 inode_no_Found=directorySearch(ext2fd,inode.i_block[i],token);
			if( inode_no_Found !=0 ){
				return inode_no_Found;
			}
		}
	}
return 0; 
}


/* converts the time_t into readable date and time format */
void convertTime(time_t tim, char *buffer){
    struct tm time;
    (void) localtime_r(&tim,&time);
	strftime(buffer,80, DTformat, &time);
}

/* displayBlocks displays the blocks of the indirection */
void displayBlocks(__u32 block_num,int ext2fd,__u8 indirectionType, __u32 filesize)
{
char buff[filesize];

	__u32 blockStartAddress=block_num*blockSize;
	__u32 nextBlackStartAddress= (block_num+1)*blockSize;
	__u32 cur_block_no=1;
	__u8 indir= indirectionType-1;
	while(cur_block_no>0 && (nextBlackStartAddress-blockStartAddress)>3){
		lseek(ext2fd, blockStartAddress, SEEK_SET);

/* read the contnet of the block */
read(ext2fd, buff, sizeof(char) * filesize);
buff[filesize]='\0';
        read(ext2fd, &cur_block_no, sizeof(__u32));
        blockStartAddress += sizeof(__u32);
        if(indir == 0){
        	// printf("%d", cur_block_no);
        }else{	
        	displayBlocks(cur_block_no,ext2fd,indir,filesize);
        }

	}

printf("%s",buff);
printf("\n");
}

void DisplayData(__u32 inode_no, int ext2fd) {
	int i;
	/*read the ext2_inode Stucture for a given inode_no*/
	__u32 inode_bloc_no = inodeTableBlockNo + (noOfBlocksPerGroup * ((inode_no-1)/noOfInodesPerGroup));
    /*Get the inode number in that specific  group block*/
    __u32 inode_no_in_groupBlock = (inode_no-1) % noOfInodesPerGroup;
    inode_bloc_no += inode_no_in_groupBlock/noOfInodesPerBlock;
	int buff[10];

    struct ext2_inode inode; /*Read inode structure from inode number*/
    lseek(ext2fd, blockSize*inode_bloc_no + inodeSize*(inode_no_in_groupBlock % noOfInodesPerBlock), SEEK_SET);
    read(ext2fd, &inode, sizeof(inode));
	
	printf("\nDisplaying the Meta data of the Searched Object\n");
	char permissions[11];
	calculateFlags(inode.i_mode,permissions);
	printf("Object Permissions:%s\n",permissions);
	printf("Owner Uid:%u\n",inode.i_uid);
	printf("Object Size:%u\n",inode.i_size);
	char formattedDate[80];
	convertTime(inode.i_atime,formattedDate);
	printf("Access Time:%s\n",formattedDate);
	convertTime(inode.i_ctime,formattedDate);
	printf("Creation Time:%s\n",formattedDate);
	convertTime(inode.i_mtime,formattedDate);
	printf("Modification Time:%s\n",formattedDate);
	convertTime(inode.i_dtime,formattedDate);
	printf("Deletion Time:%s\n",formattedDate);
	printf("Group Uid:%u\n",inode.i_gid);
	printf("Links Count:%u\n",inode.i_links_count);
	printf("Blocks Count%u\n",inode.i_blocks);
	if(inode.i_size - inode.i_blocks*512){
		printf("Sparse File \n");
		int i;
		for (i=0;i<15;i++){ 
			
			if(inode.i_block[i] != 0) {
				printf("\nDirect Blocks\n");
				if(i<12){ /*Direct Blocks*/
					printf("%d\n ", inode.i_block[i]);
					displayBlocks(inode.i_block[i], ext2fd, 1,inode.i_size);					
				}else if(i==12){ /*Single indirect*/
					printf("\nSingle Indirect Blocks\n");
					printf("from indirect blocks\n");
					displayBlocks(inode.i_block[i], ext2fd, 1,inode.i_size);
				}else if(i==13){ /*Double indirect*/
					printf("Double Indirect Blocks\n");
					displayBlocks(inode.i_block[i], ext2fd, 2,inode.i_size);
				}else if(i==14){ /*Triple Indirect*/
					printf("Triple Indirect Blocks\n");
					displayBlocks(inode.i_block[i], ext2fd, 3,inode.i_size);
				}
			}
		}
	}else{
		printf("Non Sparse File \n");
	}
}

__u32 TLSearch(int ext2fd,int level,int inode_no,char tokens[][255], int isDeletedFileSearch,int noOfTokens){
	/*read the ext2_inode Stucture for a given inode_no*/
	__u32 inode_bloc_no = inodeTableBlockNo + (noOfBlocksPerGroup * ((inode_no-1)/noOfInodesPerGroup));
    /*Get the inode number in that specific  group block*/
    __u32 inode_no_in_groupBlock = (inode_no-1) % noOfInodesPerGroup;
    inode_bloc_no += inode_no_in_groupBlock/noOfInodesPerBlock;

    struct ext2_inode inode; /*Read inode structure from inode number*/
    lseek(ext2fd, blockSize*inode_bloc_no + inodeSize*(inode_no_in_groupBlock % noOfInodesPerBlock), SEEK_SET);
    read(ext2fd, &inode, sizeof(inode));
   
    __u32 new_inode_no;
    int i;
	for(i = 0; i < 12; i++) {  /*loop through the direct blocks*/
		if(inode.i_block[i] != 0) {
			if(tokens[level] != "") {
				if(isDeletedFileSearch == 0){
					new_inode_no = search(ext2fd, inode_no, tokens[level]);
				}else{ 
				}
				if(new_inode_no!=0){ /*found an inode*/
					if(level == noOfTokens-1){ /*last token*/
						return new_inode_no;
					}else if(level < noOfTokens){ 
						return TLSearch(ext2fd,++level,new_inode_no,tokens,isDeletedFileSearch,noOfTokens);
					}	
				}else{
					printf("Message: No Search Found. Sorry\n");
					exit(1);
				}
			}
		}
	}
}


void main(int argc, char *argv[]){
	printf("\n\n");
	int ext2fd=open(argv[1],O_RDONLY); /*File descriptor for EXT2 File System*/
	int isDeletedFileSearch=0; /*Search deleted files? 1 if true*/
	int level=0; /*used for searching a file/directory: path token number*/
	int root_inode_no=2; /*Root Inode Number is always 2*/
	
	char tokens[10][255];/* path tokens */
	const char s[2] = "/";/* delimiter */	
	char *token = strtok(argv[2], s); /*tokenize the given path*/
   	int noOfTokens=0; /*Count of the tokens*/
   	while( token != NULL ) /* walk through other tokens */
   	{
    	strcpy(tokens[noOfTokens++],token);
      	token = strtok(NULL, s);
   	}
/* stupid thought trying to create a file using system calls !
char *filename = argv[2];
	printf("file name %s \n",fds);
int fds = open(filename, O_WRONLY | O_CREAT, 0777);
  char i;
  char j = '\0';
	printf("file created %d \n",fds);
  for (i = 48; i < 57 ; i++) {
    write(fds, &i, sizeof(char));
   }
write(fds,&j,sizeof(char));
  close(fds);
*/
	struct ext2_super_block superBlock; /*super block Access */
 	struct ext2_group_desc gtDesc; /*Group Descriptor Access*/

   	if(ext2fd < 0){ /*File Open Failure*/
		printf("File System Corrupted");
	}else{/*Start reading the File System*/

		if(readSB(ext2fd, &superBlock)==1){/*Magic Number Found in Super Block*/

			/*Group Descriptor Block*/
			int gDesc_bloc_num=1; /*First Group Descriptor Block*/
			lseek(ext2fd, blockSize*gDesc_bloc_num, SEEK_SET);/*seek through the first group descriptor block*/
			read(ext2fd,&gtDesc,sizeof(gtDesc));/*read the group descriptor block*/
			inodeTableBlockNo=gtDesc.bg_inode_table;
			inodeBitmapBlockNo=gtDesc.bg_inode_bitmap;			
			blockBitmapBlockNo=gtDesc.bg_block_bitmap;
			//printf("value of ext2fd is %d\n\n",ext2fd);
			/*search the inode table and get the inode number for each token*/
			__u32 found_inode_no= TLSearch(ext2fd,level,root_inode_no,tokens,isDeletedFileSearch,noOfTokens);
		printf("----done");
			DisplayData(found_inode_no,ext2fd);
		}else{
			printf("Un able to read File system\n");
			exit(-1);
		}
	}
}
