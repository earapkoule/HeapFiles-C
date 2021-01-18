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
	char *hash_info_block;
    int num_of_records;
    int fileDesc = header_info.fileDesc;
    CALL_BF(BF_ReadBlock(fileDesc, 0, (void**) &hash_info_block));
    int buckets = header_info.numBuckets;

    int hash_value = record.id % buckets;
    int hash_block_num = (hash_value / 128) + 1; // first block is used for metadata storing
    int hash_bucket_num = hash_value % 128;

    char *hash_block;
    CALL_BF(BF_ReadBlock(fileDesc, hash_block_num, (void**) &hash_block));
    int records_num;

    char *block_of_records;

    // HT_info *ht_header_info = HT_OpenIndex(header_info.filename);

    if (hash_block[hash_bucket_num] == -1) {
        CALL_BF(BF_AllocateBlock(fileDesc));
        CALL_BF(BF_ReadBlock(fileDesc, BF_GetBlockCounter(fileDesc)-1, (void**) &block_of_records));
        records_num = 1;
        memcpy(block_of_records, &records_num, sizeof(int));
        int next_block = -1;
        memcpy(block_of_records+sizeof(int), &next_block, sizeof(int)); // setting the last sizeof(int) bytes to -1, to indicate that there is no next block
        memcpy(block_of_records+(2*sizeof(int)), &record, sizeof(SecondaryRecord));

        int blocks_num = BF_GetBlockCounter(fileDesc)-1;
        hash_block[hash_bucket_num] = blocks_num ;

        CALL_BF(BF_WriteBlock(fileDesc, blocks_num));
        CALL_BF(BF_WriteBlock(fileDesc, hash_block_num));
    } else {
        int currentBlock=hash_block[hash_bucket_num];
        CALL_BF(BF_ReadBlock(fileDesc, hash_block[hash_bucket_num], (void**) &block_of_records));
        int block_num = block_of_records[sizeof(int)];

        while (block_num != -1) {
            CALL_BF(BF_ReadBlock(fileDesc, block_num, (void**) &block_of_records));
            currentBlock=block_num;
            block_num = block_of_records[sizeof(int)];
        }

        int max_records_in_block = (BLOCK_SIZE - (2 * sizeof(int))) / sizeof(SecondaryRecord);
        int records_num = block_of_records[0];
        
        if (records_num == max_records_in_block) {
            int numberOfAllBlocks = BF_GetBlockCounter(fileDesc);

            memcpy(block_of_records + sizeof(int), &numberOfAllBlocks, sizeof(int));
            CALL_BF(BF_WriteBlock(fileDesc, currentBlock));

            CALL_BF(BF_AllocateBlock(fileDesc));
            CALL_BF(BF_ReadBlock(fileDesc, (BF_GetBlockCounter(fileDesc)-1), (void**) &block_of_records));
            int one=1;
            int minus=-1;
            memcpy(block_of_records, &one, sizeof(int));
            memcpy(block_of_records+sizeof(int), &minus, sizeof(int));

            memcpy(block_of_records+(2*sizeof(int)), &record, sizeof(SecondaryRecord));

            CALL_BF(BF_WriteBlock(fileDesc, (BF_GetBlockCounter(fileDesc)-1)));
        } else {
            int numRecords = block_of_records[0] + 1;
            memcpy(block_of_records, &numRecords, sizeof(int));
            memcpy(block_of_records+(2*sizeof(int))+((numRecords-1)*sizeof(SecondaryRecord)), &record, sizeof(SecondaryRecord));

            CALL_BF(BF_WriteBlock(fileDesc, currentBlock));
        }
    }
    return OK;
}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value) {
	// char *block;
 //    int num_of_records;
 //    int buckets;
 //    Record *record;

 //    int *int_value;
 //    char *char_value;
 //    int fileDesc = header_info.fileDesc;
 //    int num_of_blocks = BF_GetBlockCounter(fileDesc);

 //    CALL_BF(BF_ReadBlock(fileDesc, 0, (void **)&block));
 //    buckets = header_info.numBuckets;
 //    int num_hash_blocks = (buckets % 128 == 0) ? (buckets / 128) : ((buckets / 128) + 1);

 //    if (value == NULL) {
 //        for (int index = num_hash_blocks + 1; index < num_of_blocks; index++)
 //        {
 //            CALL_BF(BF_ReadBlock(fileDesc, index, (void **)&block));
 //            memcpy(&num_of_records, block, sizeof(int));
 //            record = (Record *)(block + 2*sizeof(int));
 //            for (int i = 0; i < num_of_records; i++)
 //            {
 //                printf("Id: %d Name: %s Surname: %s Address: %s\n", record[i].id, record[i].name, record[i].surname, record[i].address);
 //            }
 //        }
 //    } else {
 //        int_value = (int *)value;

 //        int hash_value = *int_value % buckets;
 //        int hash_block_num = (hash_value / 128) + 1;
 //        int hash_bucket_num = hash_value % 128;
 //        CALL_BF(BF_ReadBlock(fileDesc, hash_block_num, (void **) &block));

 //        while(1) {
 //            int num_of_records_in_block = block[0];
 //            for (int i = 0; i < num_of_records_in_block; i++) {
 //                record = (Record *)(block + 2*sizeof(int) + (i*sizeof(Record)));
 //                if ((record->id) == *int_value) {
 //                    printf("Id: %d Name: %s Surname: %s Address: %s\n", record->id, record->name, record->surname, record->address);
 //                    return OK;
 //                }
 //            }
 //            if(block[sizeof(int)] == -1) {
 //                printf("Not found.");
 //                return OK;
 //            }
 //            CALL_BF(BF_ReadBlock(fileDesc, block[sizeof(int)], (void **) &block));
 //        }
 //    }
 //    return OK;
}

int HashStatistics(char* filename) {
	return OK;
}

int FindBlockIdOfHashTable(HT_info header_info, int *value) {
    char *block;
    int num_of_records;
    int buckets;
    Record *record;

    int fileDesc = header_info.fileDesc;
    int num_of_blocks = BF_GetBlockCounter(fileDesc);

    CALL_BF(BF_ReadBlock(fileDesc, 0, (void **)&block));
    buckets = header_info.numBuckets;
    int num_hash_blocks = (buckets % 128 == 0) ? (buckets / 128) : ((buckets / 128) + 1);

    int hash_value = *value % buckets;
    int hash_block_num = (hash_value / 128) + 1;
    int hash_bucket_num = hash_value % 128;
    CALL_BF(BF_ReadBlock(fileDesc, hash_block_num, (void **) &block));

    while(1) {

        int num_of_records_in_block = block[0];

        for (int i = 0; i < num_of_records_in_block; i++) {
            record = (Record *)(block + 2*sizeof(int) + (i*sizeof(Record)));
            if ((record->id) == *value) {
                return block[sizeof(int)];
            }
        }
        if(block[sizeof(int)] == -1) {
            return -1;
        }
        CALL_BF(BF_ReadBlock(fileDesc, block[sizeof(int)], (void **) &block));
    }

    return -1;
}