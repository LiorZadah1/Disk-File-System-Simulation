///Lior Zadah - Final Project
#include <iostream>
#include <vector>
#include <map>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

#define DISK_SIZE 256

// ============================================================================
void decToBinary(int n, char &c)
{
    // array to store binary number
    int binaryNum[8];

    // counter for binary array
    int i = 0;
    while (n > 0)
    {
        // storing remainder in binary array
        binaryNum[i] = n % 2;
        n = n / 2;
        i++;
    }

    // printing binary array in reverse order
    for (int j = i - 1; j >= 0; j--)
    {
        if (binaryNum[j] == 1)
            c = c | 1u << j;
    }
}

// ============================================================================

class FsFile {
    int file_size;
    int block_in_use;
    int index_block;
    int block_size;

public:
    FsFile(int _block_size) {
        file_size = 0;
        block_in_use = 0;
        block_size = _block_size;
        index_block = -1;
    }

    int getfile_size(){
        return file_size;
    }

    int getindex_block(){
        return index_block;
    }

    void setIndexBlock(int n) {
        index_block = n;
    }

    int getblock_in_use(){
        return block_in_use;
    }
    void setBlockInUse(int n) {
        block_in_use = n;
    }

    void setMoreBlocksInUse(int i) {
        block_in_use += i;
    }

    void setMoreFileSize(int i) {
        file_size += i;
    }
};

// ============================================================================

class FileDescriptor {
    string file_name;
    FsFile* fs_file;
    bool inUse;

public:

    FileDescriptor(string FileName, FsFile* fsi) {
        file_name = FileName;
        fs_file = fsi;
        inUse = true;
    }

    string getFileName() {
        return this -> file_name;
    }
    bool getInUse(){
        return this -> inUse;
    }
    FsFile* getFsFile(){
        return this -> fs_file;
    }
    void setInUse(bool Use) { // to change the inUse to be true! use in open&close file
        this->inUse = Use;
    }

};
// all above is good.
#define DISK_SIM_FILE "DISK_SIM_FILE.txt"

// ============================================================================

class fsDisk {
    FILE *sim_disk_fd;
    bool is_formated;
    int BitVectorSize;
    int *BitVector;
    int block_size;
    int free_Blocks;
    int maximumSize;
    // BitVector - "bit" (int) vector, indicate which block in the disk is free
    //              or not.  (i.e. if BitVector[0] == 1 , means that the
    //             first block is occupied.
    vector <FileDescriptor*> MainDir;
    map <int, FileDescriptor*> OpenFileDescriptors; //first - int is the key. second - is the FD


    // (5) MainDir --
    // Structure that links the file name to its FsFile

    // (6) OpenFileDescriptors --
    //  when you open a file,
    // the operating system creates an entry to represent that file
    // This entry number is the file descriptor.
    int FDSearch() {
        int i = 0;
        while(i < DISK_SIZE){ //looking for free space at the open files
            auto aut = OpenFileDescriptors.find(i);
            if(aut == OpenFileDescriptors.end())
                break;
            i++;
        }
        if(i == DISK_SIZE)// if we arrived to the end we didn't found space..
            return -1;
        return i;
    }
    int OpenFDSearch(string fileName){ //here i return the FD of the file name i got
        int retValue = 0;
        for(auto &a: OpenFileDescriptors){
            if(a.second->getFileName() == fileName)
              retValue =  a.first;
                break;
        }
        return retValue;
    }
    //looking for the file by his name in the MAinDir!
    auto MainDirSearch(string name){
        auto aut = MainDir.begin();
        for(; aut != MainDir.end(); aut++){
            if((*aut.base()) -> getFileName() == name)
                break;
        }
        return aut;
    }
    int freeBlockSearch(){//search for free block using the bitVector! when 0 its empty
        int i;
        for(i = 0; i < BitVectorSize; i++)
            if(BitVector[i] == 0) {
                BitVector[i] = 1;
                free_Blocks--;
                break;
            }
        if(i == BitVectorSize) // if we don't have free block
            i = -1;

        return i;
    }

    // ------------------------------------------------------------------------
public:
    fsDisk() { // the contractor - init the dataBase.
        sim_disk_fd = fopen(DISK_SIM_FILE , "w+");
        assert(sim_disk_fd);

        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd);
            assert(ret_val == 1);
        }

        fflush(sim_disk_fd);
        is_formated = false;
    }

    ~fsDisk(){
        //close all files in the openFiles  and delete all file in MainDir
        auto aut = OpenFileDescriptors.begin();
        while (OpenFileDescriptors.size() > 0) {
            aut = OpenFileDescriptors.begin();
            CloseFile(aut -> first);
        }
        //delete all files
        for(int i = 0; i < MainDir.size(); i++){
            delete MainDir[i];
        }
        delete BitVector;//delete bit vector
        fclose(sim_disk_fd);
    }

    // ------------------------------------------------------------------------
    void listAll() { // asssaf func
        int i;

        for( i = 0; i < MainDir.size(); i++){
            cout << "Index: " << i << ": FileName: " << MainDir[i] -> getFileName() << " , isInUse: " << MainDir[i] -> getInUse() << endl;

        }
        printf("freeBlocks = %d\n", free_Blocks);
        char bufy;
        cout << "Disk content: '";
        for (i = 0; i < DISK_SIZE; i++)
        {
            cout << "(";
            int ret_val = fseek(sim_disk_fd, i, SEEK_SET);
            ret_val = fread(&bufy, 1, 1, sim_disk_fd);
            cout << bufy;
            cout << ")";
        }
        cout << "'" << endl;
    }

    // ------------------------------------------------------------------------
    void fsFormat( int blockSize = 4 ) {
        //format disk by the block size and add elements that I will use later
        this -> block_size = blockSize;
        this -> BitVectorSize = DISK_SIZE / block_size;
        this -> BitVector = new int[BitVectorSize];
        free_Blocks = BitVectorSize; //tells us how many free blocks i have
        maximumSize = blockSize * block_size;

        for (int i = 0; i < this->BitVectorSize; i++) // init bit vector
            this->BitVector[i] = 0;

        for (int i=0; i < DISK_SIZE ; i++) { // init disk fd
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, this->sim_disk_fd);
            assert(ret_val == 1);
        }
        for(auto & aut : MainDir){
            delete aut -> getFsFile();
            delete aut;
        }
        OpenFileDescriptors.clear();
        MainDir.clear();
        this->is_formated = true;
    }

// ------------------------------------------------------------------------
    // First, need to check if the fileName is already exist,
    // Then if not, open a new one
    // Returens -1 when there is a problem. if the file name exist.
    int CreateFile(string fileName) { // need to add freeBlocks++??
        int freeSpace = 0, free_index = 0;
        // need to check if formated?
        if(!is_formated){
            printf("the file is not formated\n");
            return -1;
        }

        auto aut = MainDir.begin(); // find in MainDir
        for( ;aut != MainDir.end(); aut++) {
            if (fileName == (*aut.base())->getFileName()) {
                printf("File name already exist!\n");
                return -1;
            }
        }
        freeSpace = FDSearch();// looking for free FD to use
        if(freeSpace == -1) {
            printf("Don't have free FD\n");
            return -1;
        }

        //open new FD and push it to the main dir and insert to the OpenFiles
        FileDescriptor* newFile = new FileDescriptor(fileName, new FsFile(block_size));
        MainDir.push_back(newFile);
        OpenFileDescriptors.insert({freeSpace, newFile});
        return freeSpace;
    }

    // ------------------------------------------------------------------------
    int OpenFile(string fileName) {
        int freeSpace = 0;
        //check if the file in the mainDir and not at the OpenFiles.
        auto aut = MainDirSearch(fileName); // find in MainDir
        if (aut == MainDir.end()) {
            printf("File name is not exist in MainDir!\n");
            return -1;
        }
        //check if the file is in the OpenFileDescriptors.

          if((*aut.base())->getInUse()) { //the file is already open
              printf("File name is already exist in OpenFileDescriptors!");
              return -1;
          }
        //looking for free space
        freeSpace = FDSearch();
        if(freeSpace == -1) {
            printf("Don't have free FD\n");
            return -1;
        }
        (*aut.base()) -> setInUse(true);
        this->OpenFileDescriptors.insert({freeSpace, (*aut.base())});
        return freeSpace;
    }

    // ------------------------------------------------------------------------
    string CloseFile(int fd) {
        auto aut = OpenFileDescriptors.find(fd); // find the specific fileDescriptor.
        if(aut == OpenFileDescriptors.end()) // if the file isn't open
            return "-1";
        aut -> second ->setInUse(false); //inUse = -1
        string fileName = aut ->second -> getFileName(); //get the FD fileName to return it
        this->OpenFileDescriptors.erase(aut); // and delete it.
        return fileName;
    }
    // ------------------------------------------------------------------------
    int WriteToFile(int fd, char *buf, int len ) { // here we're going to wrei to file
        //start whit couple of checks if the file is formatted & open.
        int toTrans = 0;///the remain data i need to transfer.
        int offset = 0, tempPointer = 0; ///here to tell me the offset and a pointer to the write part
        if(!is_formated) {
            printf("The file isn't formatted\n");
            return -1;
        }
        auto aut = OpenFileDescriptors.find(fd);// check the file is open.
        if(aut == OpenFileDescriptors.end()){
            printf("The file is closed!\n");
            return -1;
        }
        //Now check if the size of file has enough space to write.
        //checking wat is the actual size of the curent file.
        int curFileSize = aut -> second -> getFsFile() -> getfile_size();
        if(maximumSize < curFileSize + len){ // check its not to big.
            printf("There is not enough space to write on this file\n");
            return -1;
        }
        /*
         * check the options of the size i can write to the block / disk.
         * if i'm on the start of the index i have the ability to write by the size of the block.
         * if the size of the file % size of block is 0 that means i dont have free space at all.???????
         * else - the remain space i have its the block size - file % size.
         */

        int blockIndex = aut -> second -> getFsFile() -> getindex_block();
        int remainSpaceInBlock = 0, temp_free_block = 0; // how much free space we have inside the block????
        // Now I am checking if there is enough space to write on the disk
        if(blockIndex == -1) {
            if ((free_Blocks * block_size) + remainSpaceInBlock + block_size < len)
                return -1;
        }
//        else
//            if ((free_Blocks * block_size) + remainSpaceInBlock < len)
//                return -1;

        if (blockIndex == -1) { //The first time write
            temp_free_block = freeBlockSearch();
            aut -> second -> getFsFile() ->setIndexBlock(temp_free_block);
        }
        if((aut -> second -> getFsFile() -> getblock_in_use() * block_size) - curFileSize == 0) //the space in the block is over --- ????
            remainSpaceInBlock = block_size;
        else
            remainSpaceInBlock = block_size - (curFileSize % block_size);
        //Now lets deal whit each case
        //If there is place so i can write inside the block -> let's begin :)

        if(curFileSize % block_size > 0){
            //the offset & fseek will send us to the first place we can write down!
            offset = aut -> second -> getFsFile() -> getblock_in_use() + (block_size *  aut -> second -> getFsFile() -> getindex_block()) - 1;
            if (fseek (sim_disk_fd, offset, SEEK_SET) == -1)
                return -1;
            //the
            toTrans = getc(sim_disk_fd) - 48;
            int remainChars = block_size - (curFileSize % block_size);//how much more we need to write

            // if I can write all of my chars inside this curr block
            ///it means i have enough space inside to write everything!!
            if(remainChars >= len){ //
                offset = (curFileSize % block_size) + (toTrans * block_size); /// NEED TO CHECK
                if (fseek (sim_disk_fd, offset, SEEK_SET) == -1)
                    return -1;
                if (fwrite(buf, sizeof(char), len, sim_disk_fd) != len)
                    return -1;
                aut -> second -> getFsFile() ->setMoreFileSize(len);
                return 1;//returning the length of the chars i wroth.
            }

            // Now, lets write done to the remain letters to the file. starting by transfer them to buffer
            // and copy this buffer to the disk. then, update the file amount.
            char newBuffer[remainChars];
            for(int i = 0; i < len; i++)
                newBuffer[i] = buf[i + tempPointer];
            offset = (curFileSize % block_size) + (block_size * toTrans);
            if (fseek (sim_disk_fd, offset, SEEK_SET) == -1)
                return -1;
            if (fwrite(newBuffer, sizeof(char), remainChars, sim_disk_fd) != remainChars)
                return -1;
            aut -> second -> getFsFile() ->setMoreFileSize(remainChars);
            tempPointer += remainChars;
        }

        // until now we filled up the last block :)
        //Now were going to write the data to the block while we need to write full ones.
        len -= tempPointer;
        while(len > 0){
            int new_free_block = freeBlockSearch(); // start whit find new block
            offset = aut -> second -> getFsFile() -> getblock_in_use() + (block_size * aut -> second -> getFsFile() -> getindex_block());
            if (fseek (sim_disk_fd, offset, SEEK_SET) == -1)
                return -1;
            putc(new_free_block + '0', sim_disk_fd);        //write down the new data block im using

            aut -> second -> getFsFile() ->setMoreBlocksInUse(1);  //add the block i'm using now and mark it
            char finalBuffer[block_size]; //writing to the block part
            int charsWritten = 0;
            for(int i = 0; i < block_size && buf[tempPointer + i] != '\0'; i++){
                finalBuffer[i] = buf[tempPointer + i];
                charsWritten++;
            }

            offset = block_size * new_free_block;//forward to the free block
            if (fseek (sim_disk_fd, offset, SEEK_SET) == -1)
                return -1;
            if (fwrite(finalBuffer, sizeof(char), charsWritten, sim_disk_fd) != charsWritten)
                return -1;
            aut -> second -> getFsFile() -> setMoreFileSize(charsWritten);
            len -= charsWritten;//decrease the length i need to write
            tempPointer += charsWritten;//take forward the pointer
        }
        return 1;
    }
    // ------------------------------------------------------------------------
    int DelFile( string FileName ) {
        //start whit format checking and find the file in the MainDir
        //if not found return -1
        if(!is_formated){
            printf("The file isn't Formated.\n");
            return -1;
        }
        auto aut = MainDirSearch(FileName);
        if(aut == MainDir.end()){
            printf("The file did not found in MainDir.\n");
            return -1;
        }
        //see if the file is in use - if its open i cant delete it -ASSAF SAID
        if((*aut.base()) -> getInUse()){
            printf("The file is in open.\n");
            return -1;
        }
        //get ind block
        int mainBlockIndex = (*aut.base()) -> getFsFile() -> getindex_block();
        int intTempForIndBlock = 0;
        char tempForIndBlock;//that's will be the int to know which index is the index block
        int TempBlocks = (*aut.base()) -> getFsFile() -> getblock_in_use();
        for(int i = 0; i < TempBlocks; i++){
            int offset = block_size * mainBlockIndex;// that's were we want to go and find the data block's by the index block.
            if (fseek(sim_disk_fd, offset + i, SEEK_SET) < 0) {//goto the rigth place
                printf("fseek was failed1 %d\n",i);
                return -1;
            }
            //intTempForIndBlock = tempForIndBlock - 48;
            intTempForIndBlock = getc(sim_disk_fd) - '0';

            for (int f = 0 ; f < block_size ; f++) {//at this block put '\0'
                if (fseek(sim_disk_fd, (intTempForIndBlock * block_size) + f, SEEK_SET) < 0) {
                    printf("fseek was failed2 %d\n",i);
                    return -1;
                }
                fwrite("\0", 1, 1, sim_disk_fd);
            }
            free_Blocks++;
            BitVector[intTempForIndBlock] = 0;//mark as free block
        }
        if(mainBlockIndex >= 0) {///skip to the index block to get him empty///
            if (fseek(sim_disk_fd, mainBlockIndex * block_size, SEEK_SET) < 0) {
                printf("fseek was failed3\n");
                return -1;
            }
        }
        for (int f = 0 ; f < block_size ; f++) {//write \0 in every sell to delete it
            putc('\0', sim_disk_fd);
        }
        free_Blocks++;
        BitVector[mainBlockIndex] = 0;

        ///receave an fd and delete all files!!!
        int returnVal = OpenFDSearch(FileName);//reciave the FD of this file Name to return it
        delete((*aut.base()) -> getFsFile());
        delete(*aut.base());
        this->MainDir.erase(aut);
        return returnVal; // the file name FD
    }
    // ------------------------------------------------------------------------
    int ReadFromFile(int fd, char *buf, int len) {
        //check if the file is formatted & open
        if(!is_formated) {
            printf("The file isn't formatted\n");
            return -1;
        }
        auto aut = OpenFileDescriptors.find(fd);// check the file is open.
        if(aut == OpenFileDescriptors.end()){
            printf("The file is closed!\n");
            return -1;
        }
        //collect the index block of this fd adress. if its -1 that is worng and we return -1.
        //get the block in use so we can over all of them
        int mainIndexBlock = aut -> second -> getFsFile() -> getindex_block();
        if(mainIndexBlock == -1) {
            printf("I have no index block -> nothing to read...\n");
            return -1;
        }
        int dataBlocks = aut -> second -> getFsFile() -> getblock_in_use();
        //if the size of the file is smaller from what we need to read -> read the file.
        if(aut -> second -> getFsFile() -> getfile_size() < len)
            len = aut -> second -> getFsFile() -> getfile_size();
        int pointToIndexInBuf = 0, readenChars = 0;
        int blocks_moving = 0, chars_moving;
        //to copy the data i'm working whit 2 loops. to find the block and block moving.
        while(blocks_moving < dataBlocks){
            int offset = (mainIndexBlock * block_size) + blocks_moving; // skiiping to the correct index
            if (fseek(sim_disk_fd, offset, SEEK_SET) < 0) {
                printf("fseek was failed\n");
                return -1;
            }
            char c = getc(sim_disk_fd); //get the int of the specific block!
            offset =  c - 48; //ascii DIFF -- make the char int
            offset *= block_size;
            chars_moving = 0; // that int will by our pointer inside the block
            while(chars_moving < block_size){
                if(readenChars == len){ //check if we copied the needed length already
                    break;
                }
                if(fseek(sim_disk_fd, offset, SEEK_SET) == -1){//goto the data block and get the char
                    printf("fseek was failed");
                    return -1;
                }
                buf[pointToIndexInBuf] = getc(sim_disk_fd);//get the char after the fseek puts us in the place
                readenChars++, pointToIndexInBuf++, offset++, chars_moving++;
            }
            if(readenChars == len){ //if now we arrived the end finish the string
                buf[pointToIndexInBuf] = '\0';
                break;
            }
            blocks_moving++;
        }
        return 1;
    }

};
//----------------------------------------------------------------------------------------------------------

int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) {
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                fs->fsFormat(blockSize);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;
            default:
                break;
        }
    }
}
