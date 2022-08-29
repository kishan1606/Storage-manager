Storage manager

--------------------------------------
Running the Program:

Step 1 - Go to Project root (assign1) using Terminal.
Step 2 - $ make clean (to delete old compiled .o files)
Step 3 - $ make (to compile all project files) 
Step 4 - $ make run_test1 (to run "test_assign1_1.c" file)

To run additional Test Cases:

Step 1 - Repeat Step 1 to 3 from previous steps.
Step 2 - $ make test2 (to compile Custom test file "test_assign1_2.c")
Step 3 - $ make run_test2 (to run "test_assign1_2.c" file)

--------------------------------------

Function Description:
--------------------------------------
1.File related function

a) createPageFile():
	- This function generates a page file with the same name as the passed parameter.
	- To create a file, we use the C function fopen(). We use the 'w+' mode, which creates a new file and allows us to read and write to it.
	- if everything went well, we return RC_OK and if the file could not be created, we return RC_FILE_NOT_FOUND.

b) openPageFile():
	- To open the file, we use the C function fopen() and the 'r' mode to open it in read-only mode.
	- We also set the values for curPagePos and totalPageNum in struct FileHandle.
	- We use the linux fstat() C function, which returns various file information. Using the fstat() function, we can get the file size.
	- We return RC_OK if everything goes well and RC_FILE_NOT_FOUND if file could not be open.

c) closeFile():
	- Checking if the storage manager or the file pointer has been initialized. if initialized then set The page file pointer to NULL.

d) destroyFile():
	- We check to see if the file is in memory. 
	- if file is in memory use the remove() C function to remove it and If the file isn't there, the error RC_FILE_NOT_FOUND is returned.

--------------------------------------
2.Read Function

a) readBlock()
	- We check to see if the page number is correct. The number of pages on each page should be greater than 0 but less than the total number of pages.
	- Check to see if the page file pointer is available.
	- We use fseek() to navigate to the given location using the valid file pointer.
	- If fseek() succeeds, the data from the page number specified in the paramter is read and stored in the memPage parameter.

b) getBlockPos()
	- This function retrieves the current page position from FileHandle's and returns it.

c) readFirstBlock()
	- The readBlock() function is called with the pageNum argument set to 0.

d) readPreviousBlock()
	- The readBlock() function is called with the pageNum argument equal to the current page position - 1.
	- If the block is empty then the current page position is set to the previous block or else return output.

e) readCurrentBlock:
	- The readBlock() function is called with the pageNum argument equal to the current page position.
	- If the block is empty then the current page position is set to the current block or else return output.

f) readNextBlock:
	- The readBlock() function is called with the pageNum argument equal to the current page position + 1.
	- If the block is empty then the current page position is set to the next block or else return output.

g) readLastBlock:
	- The readBlock() function is called with the pageNum argument equal to the total number of pages - 1.

--------------------------------------
3.write Function

writeBlock()
	- We check to see if the page number is correct. The number of pages on each page should be greater than 0 but less than the total number of pages.
	- Check to see if the page file pointer is available.
	- We use fseek() to navigate to the given location using the valid file pointer.
	- If fseek() succeeds, we use the fwrite() C function to write the data to the correct location and store it in the memPage parameter.
	- Set the current page position to the file stream's pointer position.
	- Increment the totalNumPage Parameter by 1.
	- We return RC_OK if the current block is successfully written. Otherwise, RC_WRITE_FAILED is returned.

writeCurrentBlock()
	- The getBlockPos method is used to obtain the current page position.
	- The writeBlock() function is called with the parameter pageNum equal to the current page position.
	
appendEmptyBlock() 
	- We make an empty block with the same size as PAGE_SIZE.
	- The file stream's pointer is moved to the last page.
	- Since we just added a new page, write the empty block data and update the total number of pages by one.

ensureCapacity()
	- Check that the number of pages required exceeds the total number of pages, indicating that more pages are required.
	- Calculate the number of pages needed and then add that many empty blocks.
	- The appendEmptyBlock() function is used in loop to add empty blocks.