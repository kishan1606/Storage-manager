#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<unistd.h>
#include<string.h>
#include<math.h>

#include "storage_mgr.h"

FILE *File;

extern void initStorageManager (void) {
	// Initialising file pointer i.e. storage manager.
	File = NULL;
}

extern RC createPageFile (char *fileName) {
	// Opening file stream in read & write mode. 'w+' mode creates an empty file for both reading and writing.
	File = fopen(fileName, "w+");

	// Checking if file was successfully opened.
	if(File != NULL) {
		// Creating an empty page in memory.
		// calloc() allocates the memory and also initializes every byte in the allocated memory to 0.
        SM_PageHandle Page = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
   		// Writing empty page to file.
        if(fwrite(Page, sizeof(char), PAGE_SIZE,File) != PAGE_SIZE)
      	    printf("write failed \n");
       	else
       	    printf("write succeeded \n");
        // Closing file stream so that all the buffers are flushed.
   		fclose(File);
        // De-allocating the memory previously allocated to 'Page'.
  		// This is optional but always better to do for proper memory management.
        free(Page);
  		return RC_OK;
	}
	else {
	    return RC_FILE_NOT_FOUND;
	}
}

extern RC openPageFile (char *fileName, SM_FileHandle *fHandle) {
	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.
	File = fopen(fileName, "r");

	// Checking if file was successfully opened.
	if(File != NULL) {
	    // Updating file handle's filename and set the current position to the start of the page.
        fHandle->fileName = fileName;
        fHandle->curPagePos = 0;
        
		
		// Initializing a structure named "Info"
        struct stat Info;
           
        /* Using fstat() to get the file total size.
        fstat() is a system call that is used to determine information about a file based on its file descriptor.
        'st_size' member variable of the 'stat' structure gives the total size of the file in bytes.
         The fileno() function returns the file descriptor for the specified file stream.
        hence, using fileno(File) to get the file descriptor in the fstat() function
        */   
        if(fstat(fileno(File), &Info) < 0)          //getting the information of File and storing it in info
        	return RC_ERROR;
        fHandle->totalNumPages = Info.st_size/ PAGE_SIZE;   //setting the totalNumPages Parameter
        // Closing file stream so that all the buffers are flushed.
        fclose(File);
        return RC_OK;
	}
	 else {
	    return RC_FILE_NOT_FOUND;
	}
}

extern RC closePageFile (SM_FileHandle *fHandle) {
	// Checking if file pointer or the storage manager is intialized. If initialised, then close.
	if(File != NULL)
		File = NULL;	
	return RC_OK; 
}


extern RC destroyPageFile (char *fileName) {
	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.	
	File = fopen(fileName, "r");
	
	if(File == NULL){
		return RC_FILE_NOT_FOUND; 
	}
	
	// Closing file to have no memory leak
	fclose(File);

	// Deleting the given filename so that it is no longer accessible.	
	remove(fileName);
	return RC_OK;
}

extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	RC size_of_read;
	// Checking if the pageNumber parameter is less than Total number of pages and less than 0, then return respective error code
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        	return RC_READ_NON_EXISTING_PAGE;

	// Opening file stream in read mode. 'r' mode creates an empty file for reading only.	
	File = fopen(fHandle->fileName, "r");

	// Checking if file was successfully opened.
	if(File == NULL)
		return RC_FILE_NOT_FOUND;
	
	// Setting the cursor(pointer) position at the page we want to read. Position is calculated by Page Number x Page Size
	// And the seek is success if fseek() return 0
	fseek(File, (pageNum * PAGE_SIZE), SEEK_SET);
	//fread returns the size of memory read.
	size_of_read=fread(memPage, sizeof(char), PAGE_SIZE, File);
	if(size_of_read < PAGE_SIZE || size_of_read > PAGE_SIZE) {
		// if the size is greater then or less then PAGE_SIZE then throwing an error
		return RC_READ_NON_EXISTING_PAGE;
	}
	
	// Closing file stream so that all the buffers are flushed.     	
	fclose(File);
    return RC_OK;
}

extern int getBlockPos (SM_FileHandle *fHandle) {
	if(fHandle->mgmtInfo != NULL)
		// Returning the current page position retrieved from the file handle	
		return fHandle->curPagePos;
	else
		return RC_FILE_HANDLE_NOT_INIT;
}

extern RC readFirstBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	if(fHandle->mgmtInfo != NULL)
		// Re-directing (passing) to readBlock(...) function with pageNumber = 0 i.e. first block
    	return readBlock(0, fHandle, memPage);
	else
		return RC_FILE_HANDLE_NOT_INIT;
}

extern RC readPreviousBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	RC Output;
	int previousPageNumber = 0;
	if(fHandle->curPagePos >=1)	
		previousPageNumber = (fHandle->curPagePos / PAGE_SIZE) - 1;
	else
		return RC_READ_NON_EXISTING_PAGE;
	// Re-directing (passing) to readBlock(...) function with pageNumber = currentPagePosition - 1 i.e. previous block
	
	//Reading the Block by redirecting to ReadBlock()
	Output = readBlock(previousPageNumber, fHandle, memPage);
	if(Output==0){
	    fHandle->curPagePos=previousPageNumber;
	}
	return Output;
}

extern RC readCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {
	RC Output;
	
	// Calculating current page number by dividing page size by current page position
	int currentPageNumber = fHandle->curPagePos / PAGE_SIZE;
	
	//Reading the Block by redirecting to ReadBlock()
	Output = readBlock(currentPageNumber, fHandle, memPage);
    if(Output==0){
  	    fHandle->curPagePos=currentPageNumber;
    }
   	return Output;
}

extern RC readNextBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	RC Output;

	int nextPageNumber = (fHandle->curPagePos / PAGE_SIZE) + 1;
    // Re-directing (passing) to readBlock(...) function with pageNumber = currentPagePosition i.e. current block
   	
	//Reading the Block by redirecting to ReadBlock()
	Output = readBlock(nextPageNumber, fHandle, memPage);
    if(Output==0){
        fHandle->curPagePos=nextPageNumber;
    }
   	return Output;
}

extern RC readLastBlock (SM_FileHandle *fHandle, SM_PageHandle memPage){
	// Re-directing (passing) to readBlock(...) function with pageNumber = totalNumPages i.e. last block	
	return readBlock(fHandle->totalNumPages - 1, fHandle, memPage);
}

extern RC writeBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
	// Checking if the pageNumber parameter is less than Total number of pages and less than 0, then return respective error code
	if (pageNum > fHandle->totalNumPages || pageNum < 0)
        return RC_WRITE_FAILED;
	
	// Opening file stream in read & write mode. 'r+' mode opens the file for both reading and writing.	
	File = fopen(fHandle->fileName, "r+");
	
	// file not iniltialized
	if(fHandle == NULL)
	    return RC_FILE_HANDLE_NOT_INIT;
	// Checking if file was successfully opened.
	if(File == NULL)
		return RC_FILE_NOT_FOUND;

	// Setting the cursor(pointer) position of the file stream. The seek is successfull if fseek() return 0
	int isSeekSuccess = fseek(File, (pageNum * PAGE_SIZE), SEEK_SET);
	if(isSeekSuccess == 0) {
		// Writing content from memPage to File stream
		fwrite(memPage, sizeof(char), strlen(memPage), File);

		// Setting the current page position to the cursor(pointer) position of the file stream
		fHandle->curPagePos = pageNum;
		
		fHandle->totalNumPages = ftell(File) / PAGE_SIZE;
		//update the value of totalNumPages which increases by 1
		
		// Closing file stream so that all the buffers are flushed.     	
		fclose(File);
	}
	else {
		return RC_WRITE_FAILED;
	}
	return RC_OK;
}

extern RC writeCurrentBlock (SM_FileHandle *fHandle, SM_PageHandle memPage) {

	if(fHandle == NULL)
		return RC_FILE_HANDLE_NOT_INIT;

	// Calculating current page number by dividing page size by current page position	
	int cur_position = getBlockPos(fHandle);

	if(cur_position < 0)
		return RC_WRITE_FAILED;
	
	// Incrementing total number of pages since we are adding this content to a new location as in current empty block.
	fHandle->totalNumPages++;
	return writeBlock(cur_position, fHandle, memPage);
}


extern RC appendEmptyBlock (SM_FileHandle *fHandle) {
	// Creating an empty page of size PAGE_SIZE bytes
	SM_PageHandle emptyBlock = (SM_PageHandle)calloc(PAGE_SIZE, sizeof(char));
	
	// Moving the cursor (pointer) position at the end of the file stream.
	// And the seek is success if fseek() return 0
	int isSeekSuccess = fseek(File, 0, SEEK_END);
	
	if( isSeekSuccess == 0 ) {
		// Writing an empty page to the file
		fwrite(emptyBlock, sizeof(char), PAGE_SIZE, File);
	} else {
		free(emptyBlock);
		return RC_WRITE_FAILED;
	}
	
	// De-allocating the memory previously allocated to 'Page'.
	// This is optional but always better to do for proper memory management.
	free(emptyBlock);
	
	// Incrementing the total number of pages since we added an empty black.
	fHandle->totalNumPages++;
	return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
	// Opening file stream in append mode. 'a' mode opens the file to append the data at the end of file.
	File = fopen(fHandle->fileName, "a");
	
	if(File == NULL)
		return RC_FILE_NOT_FOUND;
	
	// Checking if numberOfPages is greater than totalNumPages.
	// If that is the case, then add empty pages untill numberofPages = totalNumPages
	for(int i=fHandle->totalNumPages;i<numberOfPages;i++){
    	appendEmptyBlock(fHandle);
    }
	
	// Closing file stream so that all the buffers are flushed. 
	fclose(File);
	return RC_OK;
}
