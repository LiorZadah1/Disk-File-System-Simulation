Final Project - Disk File System Simulation.
Authored by Lior Zadah
318162930
==Description==
In this program, a file system is simulated in a small computer system with a small disk and a single folder.
In this program all the actions that an operating system does on the disk are implemented.

functions:
/my additions\
- FDSerach - finds an free space at open file Des
- openFDSerach - return the FD of the file name if he is open
- MainDirSearch - looking for an file name inside the main dir
- freeBlockSearch - looking for an free space by the bit vector + uptate
/the original\
-constractor & distractor - builds and delete all of the data base - clean the main dir and openFiles
-fsFormat - init the database by the block size that given, init disk, delete and clear data base.
-createFile - finds an free FD, check if formated, and looking for the name in the main dir
-openFile - finds the file by its name & open it for use.
-closeFile - close the file and remove him from the openFiles
-WriteToFile - this func will write to the disc: check if there is an index block or not, allocate new memory blocks and save it to the file.
-DelFile - here were going to delete the file from the disk whit all of his data . moving on his index block and go to there and delete it, after that delete the index block.
-readFromFile - here  were going to read from the file, using the offset were going to find the specipic place we need to start reading and moving by the index inside the buffer. when we know that readen chars = len we stop.

==Program Files==
FinalProject.cpp - implementation to all classes and main.

==How to compile?==
    compile: g++ main.cpp -o ex
    run: ./ex

==Input==
Commands numbered 0-8 to perform actions on the disk.

==Output==
Performing the actions as typed by the user.