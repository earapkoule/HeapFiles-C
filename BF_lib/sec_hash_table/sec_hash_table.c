#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "sec_hash_table.h"
#include "hash_table.h"
#include "record_struct.h"
#include "error_messages.h"

/* The first block of a secondary hash table looks like this:
 * --------------------------------------------------
 * | & | attrLength | attrName | \0 | numBuckets | fileName | \0 | 0 | 0 | ... |
 * --------------------------------------------------
 * In index 0: char '&' identifies a secondary hash table
 * In index 1: attrLength
 * In index 1 + sizeof(int) to 1 + sizeof(int) + attrLength + 1: the attrName character by character and char '\0' as well
 * In indexes from 1 + sizeof(int) + attrLength + 1 to 1 + sizeof(int) + attrLength + 1 + sizeof(int): numBuckets
 * In indexes from 1 + sizeof(int) + attrLength + 1 + sizeof(int) to 1 + sizeof(int) + attrLength + 1 + sizeof(int) + strlen(fileName) + 1: fileName and char '\0'
 * In indexes from 1 + sizeof(int) + attrLength + 1 + sizeof(int) + strlen(fileName) + 1 to BLOCK_SIZE: filled with 0
 */

int SHT_CreateSecondaryIndex(char *sfileName, char* attrName, int attrLength, int buckets, char* fileName) {
	char *block; // Initialize it to type char, so that block + 1 means "move 1 index"

    BF_Init();

    CALL_BF(BF_CreateFile(sfileName)); // Creating a file
    int fileDesc = BF_OpenFile(sfileName); // Opening existing file
    CALL_BF(BF_AllocateBlock(fileDesc)); // Allocating a block, the first one in this case
    CALL_BF(BF_ReadBlock(fileDesc, 0, (void **)&block));
    char identifier = '&';
    memcpy(block, &identifier, sizeof(char)); // Used to identify a secondary hash file
    memcpy(block + 1, &attrLength, sizeof(int));
    memcpy(block + (1 + sizeof(int)), attrName, attrLength + 1);
    memcpy(block + (1 + sizeof(int)) + (attrLength + 1), &buckets, sizeof(int));
    memcpy(block + (1 + sizeof(int)) + (attrLength + 1) + sizeof(int), fileName, strlen(fileName) + 1);

    CALL_BF(BF_WriteBlock(fileDesc, 0));

    int max_num_buckets_in_block = BLOCK_SIZE / sizeof(int);
    int num_hash_blocks = (buckets % max_num_buckets_in_block == 0) ? (buckets / max_num_buckets_in_block) : ((buckets / max_num_buckets_in_block) + 1);

    char *sec_hash_block;
    for (int i = 0; i < num_hash_blocks; i++) {
        CALL_BF(BF_AllocateBlock(fileDesc));
        CALL_BF(BF_ReadBlock(fileDesc, i + 1, (void **) &sec_hash_block));
        for (int j = 0; j < max_num_buckets_in_block; j++)
        {
            int minus = -1;
            memcpy(sec_hash_block + (j * sizeof(int)), &minus, sizeof(int));
        }
        CALL_BF(BF_WriteBlock(fileDesc, i + 1));
    }
    CALL_BF(BF_CloseFile(fileDesc));
	return OK;
}

SHT_info* SHT_OpenSecondaryIndex(char *sfileName) {
	char *block;
    
    int fileDesc = BF_OpenFile(sfileName); // Opening existing file
    CALL_OR_RETURN_NULL(BF_ReadBlock(fileDesc, 0, (void **)&block));
    if (block[0] != '&') {	// Check if it is a secondary HashTable
        return NULL;
    }
    SHT_info *header_info = malloc(sizeof(SHT_info));
    header_info->fileDesc = fileDesc;
    memcpy(&(header_info->attrLength), block + 1, sizeof(int));

    header_info->attrName = malloc(header_info->attrLength + 1);

    memcpy(header_info->attrName, block + (1 + sizeof(int)), (header_info->attrLength + 1));
    memcpy(&(header_info->numBuckets), block + (1 + sizeof(int)) + (header_info->attrLength + 1), sizeof(int));

    header_info->fileName = malloc(header_info->fileName + 1);

	memcpy(header_info->fileName, block + (1 + sizeof(int)) + (header_info->attrLength + 1) + sizeof(int), (header_info->attrLength + 1));

    return header_info;
}

int SHT_CloseSecondaryIndex(SHT_info* header_info) {
	CALL_BF(BF_CloseFile(header_info->fileDesc));
    free(header_info->attrName);
    free(header_info->fileName);
    free(header_info); // Free memory before closing the file
	return OK;
}

int SHT_SecondaryInsertEntry(SHT_info header_info, SecondaryRecord record) {
	return OK;
}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value) {
	return OK;
}

int HashStatistics(char* filename) {
	return OK;
}