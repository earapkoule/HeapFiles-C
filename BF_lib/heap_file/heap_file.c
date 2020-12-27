#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "heap_file.h"
#include "record_struct.h"
#include "error_messages.h"

#define MAX_RECORDS (BLOCK_SIZE - sizeof(int)) / sizeof(Record)

/* The first block of a heap file looks like this:
 * --------------------------------------------------
 * | % | attrType | attrLength | attrName | \0 | 0 | 0 | ... |
 * --------------------------------------------------
 * In index 0: char '%' identifies a heap file
 * In index 1: char attrType ( 'c' or 'i' ) is the type of the key
 * In index 2: attrLength
 * In indexes from 2 + sizeof(int) to 2 + sizeof(int) + attrLength + 1: the attrName character by character and char '\0' as well
 * In indexes from 2 + sizeof(int) + attrLength + 1 + to BLOCK_SIZE: filled with 0
 */

int HP_CreateFile(char *fileName, char attrType, char *attrName, int attrLength) {
  char *block; // Initialize it to type char, so that block + 1 means "move 1 index"

  BF_Init();

  CALL_BF(BF_CreateFile(fileName)); // Creating a file
  int fileDesc = BF_OpenFile(fileName); // Opening existing file
  CALL_BF(BF_AllocateBlock(fileDesc)); // Allocating a block, the first one in this case
  CALL_BF(BF_ReadBlock(fileDesc, 0, (void**) &block));
  char identifier = '%';
  memcpy(block, &identifier, sizeof(char)); // Used to identify the heap file
  memcpy(block + 1, &attrType, sizeof(char));
  memcpy(block + 2, &attrLength, sizeof(int)); 
  memcpy(block + (2 + sizeof(int)), attrName, attrLength + 1);
  CALL_BF(BF_WriteBlock(fileDesc, 0));
  CALL_BF(BF_CloseFile(fileDesc));
  return OK;
}

HP_info* HP_OpenFile(char *fileName) {
  char *block;

  HP_info *header_info = malloc(sizeof(HP_info));
  int fileDesc = BF_OpenFile(fileName); // Opening existing file
  CALL_OR_RETURN_NULL(BF_ReadBlock(fileDesc, 0, (void**) &block));
  if ( block[0] != '%' ) { // Check if it is a HeapFile
    free(header_info); // Free memory in case the file does not open
    return NULL;
  } else {
    header_info->fileDesc = fileDesc;
    memcpy(&header_info->attrType, block + 1, 1);
    memcpy(&header_info->attrLength, block + 2, sizeof(int));
    memcpy(&header_info->attrName, block + (2 + sizeof(int)), header_info->attrLength);
  }
  return header_info;
}

int HP_CloseFile(HP_info* header_info) {
  CALL_BF(BF_CloseFile(header_info->fileDesc));
  free(header_info); // Free memory before closing the file
  return OK;
}

/* Records cannot be stored in the first block. It is used only for the heap file info.
 * Record blocks look like:
 * ------------------------------------------------
 * | num of records (int) | Record | Record | ... |
 * ------------------------------------------------
 */

// Inserts the record to the end of the file, no matter if there are available indexes in the middle of the file
int HP_InsertEntry(HP_info header_info, Record record) {
  char *block;
  int num_of_records;

  int fileDesc = header_info.fileDesc;

  int num_of_blocks = BF_GetBlockCounter(fileDesc);

  if (num_of_blocks == 1) { // There is only the block that contains heaap file info
    CALL_BF(BF_AllocateBlock(fileDesc));
    CALL_BF(BF_ReadBlock(fileDesc, 1, (void**) &block)); // Blocks' numbering starts on 0
    
    num_of_records = 1;
    memcpy(block, &num_of_records, sizeof(int));
    memcpy(block + sizeof(int), &record, sizeof(Record));
    CALL_BF(BF_WriteBlock(fileDesc, 1));
  } else {
    CALL_BF(BF_ReadBlock(fileDesc, num_of_blocks - 1, (void**) &block));
    memcpy(&num_of_records, block, sizeof(int));
    if (num_of_records < MAX_RECORDS) { // There is space for a record in the same block
      num_of_records += 1;
      memcpy(block, &num_of_records, sizeof(int)); // Increase the number of records
      memcpy(block + sizeof(int) + ((num_of_records - 1) * sizeof(Record)), &record, sizeof(Record)); // Add the new record
    } else { // There is not enough space for a record in this block
      CALL_BF(BF_AllocateBlock(fileDesc)); // Allocate a new block
      num_of_blocks = BF_GetBlockCounter(fileDesc);
      CALL_BF(BF_ReadBlock(fileDesc, num_of_blocks - 1, (void**) &block));
      
      num_of_records = 1;
      memcpy(block, &num_of_records, sizeof(int));
      memcpy(block + sizeof(int), &record, sizeof(Record));

    }
    CALL_BF(BF_WriteBlock(fileDesc, num_of_blocks - 1));
  }
  return OK;
}

int HP_DeleteEntry(HP_info header_info, void *value) {
  return OK;
}

int HP_GetAllEntries(HP_info header_info, void *value) {
  char *block;
  int num_of_records;
  Record *record;
  
  int *int_value;
  int fileDesc = header_info.fileDesc;
  int num_of_blocks = BF_GetBlockCounter(fileDesc);
  if ( value == NULL ) {
    for ( int index = 1; index < num_of_blocks; index++ ) {
      CALL_BF(BF_ReadBlock(fileDesc, index, (void**) &block));
      memcpy(&num_of_records, block, sizeof(int));
      record = (Record*) (block + sizeof(int));
      for ( int i = 0; i < num_of_records; i++ ) {
        printf("Id: %d Name: %s Surname: %s Address: %s\n", record[i].id, record[i].name, record[i].surname, record[i].address);
      }
    }
  } else if ( !strcmp(&header_info.attrName, "id") ) {
    int_value = (int*) value;
    for ( int index = 1; index < num_of_blocks; index++ ) {
      CALL_BF(BF_ReadBlock(fileDesc, index, (void**) &block));
      memcpy(&num_of_records, block, sizeof(int));
      record = (Record*) (block + 4);
      for ( int i = 0; i < num_of_records; i++ ) {
        if ( record[i].id == *int_value ) {
          printf("Id: %d Name: %s Surname: %s Address: %s\n", record[i].id, record[i].name, record[i].surname, record[i].address);
        }
      }
    }
  } else if ( !strcmp(&header_info.attrName, "name") ) {
    value = (char*) value;
    for ( int index = 1; index < num_of_blocks; index++ ) {
      CALL_BF(BF_ReadBlock(fileDesc, index, (void**) &block));
      memcpy(&num_of_records, block, sizeof(int));
      record = (Record*) (block + sizeof(int));
      for ( int i = 0; i < num_of_records; i++ ) {
        if ( !strcmp(record[i].name, value) ) {
          printf("Id: %d Name: %s Surname: %s Address: %s\n", record[i].id, record[i].name, record[i].surname, record[i].address);
        }
      }
    }
  } else if ( !strcmp(&header_info.attrName, "surname") ) {
    value = (char*) value;
    for ( int index = 1; index < num_of_blocks; index++ ) {
      CALL_BF(BF_ReadBlock(fileDesc, index, (void**) &block));
      memcpy(&num_of_records, block, sizeof(int));
      record = (Record*) (block + sizeof(int));
      for ( int i = 0; i < num_of_records; i++ ) {
        if ( !strcmp(record[i].surname, value) ) {
          printf("Id: %d Name: %s Surname: %s Address: %s\n", record[i].id, record[i].name, record[i].surname, record[i].address);
        }
      }
    }
  } else if ( !strcmp(header_info.attrName, "address") ) {
    value = (char*) value;
    for ( int index = 1; index < num_of_blocks; index++ ) {
      CALL_BF(BF_ReadBlock(fileDesc, index, (void**) &block));
      memcpy(&num_of_records, block, sizeof(int));
      record = (Record*) (block + sizeof(int));
      for ( int i = 0; i < num_of_records; i++ ) {
        if ( !strcmp(record[i].address, value) ) {
          printf("Id: %d Name: %s Surname: %s Address: %s\n", record[i].id, record[i].name, record[i].surname, record[i].address);
        }
      }
    }
  }
  return OK;
}
