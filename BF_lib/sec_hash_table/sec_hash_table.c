#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "sec_hash_table.h"
#include "hash_table.h"
#include "record_struct.h"
#include "error_messages.h"

#define MAX_BUCKETS BLOCK_SIZE / sizeof(int)

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

    int num_hash_blocks = (buckets % MAX_BUCKETS == 0) ? (buckets / MAX_BUCKETS) : ((buckets / MAX_BUCKETS) + 1);

    char *sec_hash_block;
    for (int i = 0; i < num_hash_blocks; i++) {
        CALL_BF(BF_AllocateBlock(fileDesc));
        CALL_BF(BF_ReadBlock(fileDesc, i + 1, (void **) &sec_hash_block));
        for (int j = 0; j < MAX_BUCKETS; j++)
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

    char *surname = record.record.surname;
    int hash_value = strlen(surname) % buckets;
    int hash_block_num = (hash_value / MAX_BUCKETS) + 1; // first block is used for metadata storing
    int hash_bucket_num = hash_value % MAX_BUCKETS;

    char *hash_block;
    CALL_BF(BF_ReadBlock(fileDesc, hash_block_num, (void**) &hash_block));
    int records_num;

    char *block_of_records;

	SHTRecord sht_record;
	sht_record.blockId = record.blockId;
	memcpy(&(sht_record.surname), &(record.record.surname), 25 * sizeof(char));

    if (hash_block[hash_bucket_num] == -1) {
        CALL_BF(BF_AllocateBlock(fileDesc));
        CALL_BF(BF_ReadBlock(fileDesc, BF_GetBlockCounter(fileDesc)-1, (void**) &block_of_records));
        records_num = 1;
        memcpy(block_of_records, &records_num, sizeof(int));
        int next_block = -1;
        memcpy(block_of_records + sizeof(int), &next_block, sizeof(int)); // setting the last sizeof(int) bytes to -1, to indicate that there is no next block
        memcpy(block_of_records + (2*sizeof(int)), &sht_record, sizeof(SHTRecord));

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
            currentBlock = block_num;
            block_num = block_of_records[sizeof(int)];
        }

        int max_records_in_block = (BLOCK_SIZE - (2 * sizeof(int))) / sizeof(SHTRecord);
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

            memcpy(block_of_records+(2*sizeof(int)), &sht_record, sizeof(SHTRecord));

            CALL_BF(BF_WriteBlock(fileDesc, (BF_GetBlockCounter(fileDesc)-1)));
        } else {
            int numRecords = block_of_records[0] + 1;
            memcpy(block_of_records, &numRecords, sizeof(int));
            memcpy(block_of_records+(2*sizeof(int))+((numRecords-1)*sizeof(SHTRecord)), &sht_record, sizeof(SHTRecord));

            CALL_BF(BF_WriteBlock(fileDesc, currentBlock));
        }
    }
    return OK;
}

int SHT_SecondaryGetAllEntries(SHT_info header_info_sht, HT_info header_info_ht, void *value) {
    if (value == NULL) {
        return HT_GetAllEntries(header_info_ht, NULL);
    } else {
    	char *block;
    	SHTRecord *sht_record;

        char *char_value = (char *)value;

        int fileDesc_sht = header_info_sht.fileDesc;
		int buckets_sht = header_info_sht.numBuckets;

        int hash_value_sht = strlen(char_value) % buckets_sht;
        int hash_block_num_sht = (hash_value_sht / MAX_BUCKETS) + 1;

        CALL_BF(BF_ReadBlock(fileDesc_sht, hash_block_num_sht, (void **) &block));

        while(1) {
            int num_of_records_in_block = block[0];
            for (int i = 0; i < num_of_records_in_block; i++) {
                sht_record = (SHTRecord *)(block + 2*sizeof(int) + (i*sizeof(SHTRecord)));
                
                if (strcmp(&(sht_record->surname), char_value) == 0) {
                	char *block_ht;
                	Record *record;

                	int fileDesc_ht = header_info_ht.fileDesc;
                	CALL_BF(BF_ReadBlock(fileDesc_ht, sht_record->blockId, (void **) &block_ht));
                	int num_of_records_in_block_ht = block_ht[0];

                	int index_ht;
                	for(index_ht = 0; index_ht < num_of_records_in_block_ht; index_ht++) {
                		record = (Record *)(block + 2*sizeof(int) + (index_ht*sizeof(Record)));

                		if(strcmp(&(record->surname), char_value) == 0) {
                			printf("Id: %d Name: %s Surname: %s Address: %s\n", record->id, record->name, record->surname, record->address);
	                    	return OK;
                		}
                	}
                	if (index_ht == num_of_records_in_block_ht) {
		                printf("This blockId does not contain the asked surname.");
		                return OK;
		            }
                }
            }
            if (block[sizeof(int)] == -1) {
                printf("Not found.");
                return OK;
            }
            CALL_BF(BF_ReadBlock(fileDesc_sht, block[sizeof(int)], (void **) &block));
        }
    }
    return OK;
}

int HashStatistics(char* filename) {
	return OK;
}