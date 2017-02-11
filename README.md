# OS-Linux-commands
An C implementation of Linux commands  :  ls  and cat 

/*Read me*/

Authors :  Prashant Kuntala and Chinky Dhingra

/* For the program that prints file contents and file attributes. similar to the cat command.

	usage : type make to compile the program and then to execute type the command as below.

	command : ./mycat <filesystem> <directory Path>
	example : ./mycat fsy /hello/hi.txt
		
*/

/* For the program that prints the superblock information and other information regarding the Filesystem 

	This program also prints the inode number , permissions , filesize, uid, gid , ilink count and time for each file and folder
	similar to the functionality of ls -il.

	usage : type make to compile the program and then to execute type the command as below.

	command : ./ls_il <filesystem> <directory Path>
	example : ./ls_il fsy /hello
		
*/

Also find the filesystem we worked on in the folder. "fsy" is the name of the test filesystem we used for development purpose.

You can mount the filesystem "fsy" with below commands
------------------------------------------------------

mkdir Test
sudo mount fsy ./Test

/* cd into the Test directory to view the contents of the filesystem. Create folders or files if needed. Now use one of the above complied commands on the filesystem */

to unmount the filesystem
-------------------------
 
 sudo umount fsy
 

 


