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

  CALL_BF(BF_CreateFile(fileName)); // Creating a file
  int fileDesc = CALL_BF(BF_OpenFile(fileName)); // Opening existing file
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

  int fileDesc = CALL_OR_RETURN_NULL(BF_OpenFile(fileName)); // Opening existing file
  CALL_OR_RETURN_NULL(BF_ReadBlock(fileDesc, 0, (void**) &block));
  if ( !strcmp(block[0], '%') ) { // Check if it is a HeapFile
    free(header_info); // Free memory in case the file does not open
    return NULL;
  } else {
    header_info->fileDesc = fileDesc;
    memcpy(header_info->attrType, block + 1, 1); 
    memcpy(header_info->attrLength, block + 2, sizeof(int));
    memcpy(header_info->attrName, block + (2 + sizeof(int)), header_info->attrLength);
  }
  return HP_info;
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
  char *fileΝame = header_info.fileName;
  CALL_BF(BF_OpenFile(fileΝame));

  int num_of_blocks = CALL_BF(BF_GetBlockCounter(fileDesc));

  if (num_of_blocks == 1) { // There is only the block that contains heaap file info
    CALL_BF(BF_AllocateBlock(fileDesc));
    CALL_BF(BF_ReadBlock(fileDesc, 1, (void**) &block); // Blocks' numbering starts on 0
    
    num_of_records = 1;
    memcpy(block, &num_of_records, sizeof(int));
    memcpy(block + sizeof(int), &record, sizeof(Record));
  } else {
    CALL_BF(BF_ReadBlock(fileDesc, num_of_blocks - 1, (void**) &block);
    memcpy(&num_of_records, block, sizeof(int));
    if (num_of_records < MAX_RECORDS) { // There is space for a record in the same block
      num_of_records += 1;
      memcpy(block, &num_of_records, sizeof(int)); // Increase the number of records
      block = block + (num_of_records * sizeof(Record)); // Move the index to the end of the block
      memcpy(block, &record, sizeof(Record)); // Add the new record
    } else { // There is not enough space for a record in this block
      CALL_BF(BF_AllocateBlock(fileDesc)); // Allocate a new block
      num_of_blocks = CALL_BF(BF_GetBlockCounter(fileDesc));
      CALL_BF(BF_ReadBlock(fileDesc, num_of_blocks - 1, (void**) &block);
      
      num_of_records = 1;
      memcpy(block, &num_of_records, sizeof(int));
      memcpy(block + sizeof(int), &record, sizeof(Record));
    }
  }
  return OK;
}

// Deletes the record that 
int HP_DeleteEntry(HP_info header_info, void *value) {
  return OK;
}

int HP_GetAllEntries(HP_info header_info, void *value) {
  return OK;
}




HP_ErrorCode HP_PrintAllEntries(int fileDesc, char *attrName, void* value) {
  int blocks_num, index1, index2;
  BF_Block *block;
  BF_Block_Init(&block);
  Record *record;
  char *block_data;
  int records_num;
  int *int_value;

  CALL_BF(BF_GetBlockCounter(fileDesc, &blocks_num));
  if(value == NULL){
    for(index1 = 1; index1 < blocks_num; index1++){
      CALL_BF(BF_GetBlock(fileDesc, index1, block));
      block_data = BF_Block_GetData(block);
      memcpy(&records_num, block_data, 4);
      record = (Record*)(block_data + 4);
      for(index2 = 0; index2 < records_num; index2++){
        printf("Id: %d Name: %s Surname: %s City: %s\n", record[index2].id, record[index2].name, record[index2].surname, record[index2].city);
      }
      CALL_BF(BF_UnpinBlock(block));
    }
  }else if(!strcmp(attrName, "id")){
    int_value = (int*)value;
    for(index1 = 1; index1 < blocks_num; index1++){
      CALL_BF(BF_GetBlock(fileDesc, index1, block));
      block_data = BF_Block_GetData(block);
      memcpy(&records_num, block_data, 4);
      record = (Record*)(block_data + 4);
      for(index2 = 0; index2 < records_num; index2++){
        if(record[index2].id == *int_value){
          printf("Id: %d Name: %s Surname: %s City: %s\n", record[index2].id, record[index2].name, record[index2].surname, record[index2].city);
        }
      }
      CALL_BF(BF_UnpinBlock(block));
    }
  }else if(!strcmp(attrName, "name")){
    value = (char*)value;
    for(index1 = 1; index1 < blocks_num; index1++){
      CALL_BF(BF_GetBlock(fileDesc, index1, block));
      block_data = BF_Block_GetData(block);
      memcpy(&records_num, block_data, 4);
      record = (Record*)(block_data + 4);
      for(index2 = 0; index2 < records_num; index2++){
        if(!strcmp(record[index2].name, value)){
          printf("Id: %d Name: %s Surname: %s City: %s\n", record[index2].id, record[index2].name, record[index2].surname, record[index2].city);
        }
      }
      CALL_BF(BF_UnpinBlock(block));
    }
  }else if(!strcmp(attrName, "surname")){
    value = (char*)value;
    for(index1 = 1; index1 < blocks_num; index1++){
      CALL_BF(BF_GetBlock(fileDesc, index1, block));
      block_data = BF_Block_GetData(block);
      memcpy(&records_num, block_data, 4);
      record = (Record*)(block_data + 4);
      for(index2 = 0; index2 < records_num; index2++){
        if(!strcmp(record[index2].surname, value)){
          printf("Id: %d Name: %s Surname: %s City: %s\n", record[index2].id, record[index2].name, record[index2].surname, record[index2].city);
        }
      }
      CALL_BF(BF_UnpinBlock(block));
    }
  }else if(!strcmp(attrName, "city")){
    value = (char*)value;
    for(index1 = 1; index1 < blocks_num; index1++){
      CALL_BF(BF_GetBlock(fileDesc, index1, block));
      block_data = BF_Block_GetData(block);
      memcpy(&records_num, block_data, 4);
      record = (Record*)(block_data + 4);
      for(index2 = 0; index2 < records_num; index2++){
        if(!strcmp(record[index2].city, value)){
          printf("Id: %d Name: %s Surname: %s City: %s\n", record[index2].id, record[index2].name, record[index2].surname, record[index2].city);
        }
      }
      CALL_BF(BF_UnpinBlock(block));
    }
  }
  BF_Block_Destroy(&block);
  return HP_OK;
}

HP_ErrorCode HP_GetEntry(int fileDesc, int rowId, Record *record) {
  int blocks_num, records_num, max_records; //max_records = Max number of records in a block;
  char *block_data;
  BF_Block *block;
  BF_Block_Init(&block);

  max_records = (BF_BLOCK_SIZE - 4) / sizeof(Record); //The first 4 Bytes are used to store an int indicating the current number of records of the block
  if(rowId <= 0){ //Check if rowId is valid
    return HP_ERROR;
  }
  CALL_BF(BF_GetBlockCounter(fileDesc, &blocks_num));
  if(((rowId - 1) / max_records) + 1 >= blocks_num){ //Check if rowId is in range
    BF_Block_Destroy(&block);
    return HP_ERROR;
  }
  CALL_BF(BF_GetBlock(fileDesc, ((rowId-1)/max_records)+1, block));
  block_data = BF_Block_GetData(block);
  memcpy(&records_num, block_data, 4); //Take the first 4 bytes
  if((rowId - 1) % max_records >= records_num){ //Check if rowId is valid in block
      CALL_BF(BF_UnpinBlock(block));
      BF_Block_Destroy(&block);
      return HP_ERROR;
  }
  memcpy(record, ((block_data + 4) + ((rowId - 1) % max_records) * sizeof(Record)), sizeof(Record)); //Take the record that is written in the rowId position
  CALL_BF(BF_UnpinBlock(block));
  BF_Block_Destroy(&block);
  return HP_OK;

}
