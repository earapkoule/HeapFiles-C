#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "hash_table.h"
#include "record_struct.h"
#include "error_messages.h"

/* The first block of a hash file looks like this:
 * --------------------------------------------------
 * | % | attrType | attrLength | attrName | numBuckets| \0 | 0 | 0 | ... |
 * --------------------------------------------------
 * In index 0: char '$' identifies a hash file
 * In index 1: char attrType ( 'c' or 'i' ) is the type of the key
 * In index 2: attrLength
 * In indexes from 2 + sizeof(int) to 2 + sizeof(int) + attrLength + 1: the attrName character by character and char '\0' as well
 * In indexes from 2 + sizeof(int) + attrLength + 1 + to BLOCK_SIZE: filled with 0
 */

int HT_CreateIndex(char *fileName, char attrType, char *attrName, int attrLength, int buckets)
{
    char *block; // Initialize it to type char, so that block + 1 means "move 1 index"

    BF_Init();

    CALL_BF(BF_CreateFile(fileName)); // Creating a file
    int fileDesc = BF_OpenFile(fileName); // Opening existing file
    CALL_BF(BF_AllocateBlock(fileDesc)); // Allocating a block, the first one in this case
    CALL_BF(BF_ReadBlock(fileDesc, 0, (void **)&block));
    char identifier = '$';
    memcpy(block, &identifier, sizeof(char)); // Used to identify the hash file
    memcpy(block + 1, &attrType, sizeof(char));
    memcpy(block + 2, &attrLength, sizeof(int));
    memcpy(block + (2 + sizeof(int)), attrName, attrLength + 1);
    memcpy(block + (2 + sizeof(int)) + (attrLength + 1), &buckets, sizeof(int));

    CALL_BF(BF_WriteBlock(fileDesc, 0));

    int num_hash_blocks = (buckets % 128 == 0) ? (buckets / 128) : ((buckets / 128) + 1);

    char *hash_block;
    for (int i = 0; i < num_hash_blocks; i++)
    {
        CALL_BF(BF_AllocateBlock(fileDesc));
        CALL_BF(BF_ReadBlock(fileDesc, i + 1, (void **) &hash_block));
        for (int j = 0; j < BLOCK_SIZE / sizeof(int); j++)
        {
            int minus = -1;
            memcpy(hash_block + (j * sizeof(int)), &minus, sizeof(int));
        }
        CALL_BF(BF_WriteBlock(fileDesc, i + 1));
    }
    CALL_BF(BF_CloseFile(fileDesc));
    return OK;
}

HT_info *HT_OpenIndex(char *fileName)
{
    char *block;
    
    int fileDesc = BF_OpenFile(fileName); // Opening existing file
    CALL_OR_RETURN_NULL(BF_ReadBlock(fileDesc, 0, (void **)&block));
    if (block[0] != '$')
    {                      // Check if it is a HashTable
        return NULL;
    }
    HT_info *header_info = malloc(sizeof(HT_info));
    header_info->fileDesc = fileDesc;
    memcpy(&(header_info->attrType), block + 1, 1);
    memcpy(&(header_info->attrLength), block + 2, sizeof(int));

    header_info->attrName = malloc(header_info->attrLength + 1);

    memcpy(header_info->attrName, block + (2 + sizeof(int)), (header_info->attrLength + 1));
    memcpy(&(header_info->numBuckets), block + (2 + sizeof(int)) + (header_info->attrLength + 1), sizeof(int));

    return header_info;
}

int HT_CloseIndex(HT_info *header_info)
{
    CALL_BF(BF_CloseFile(header_info->fileDesc));
    free(header_info->attrName);
    free(header_info); // Free memory before closing the file
    return OK;
}

int HT_InsertEntry(HT_info header_info, Record record)
{
    printf("inInsert\n");fflush(stdin);
    char *hash_info_block;
    int num_of_records;
    int fileDesc = header_info.fileDesc;
    CALL_BF(BF_ReadBlock(fileDesc, 0, (void**) &hash_info_block));
    int buckets = header_info.numBuckets;

    int hash_value = record.id % buckets;
    int hash_block_num = (hash_value / 128) + 1;//first block is used for metadata storing
    int hash_bucket_num = hash_value % 128;

    char *hash_block;
    CALL_BF(BF_ReadBlock(fileDesc, hash_block_num, (void**) &hash_block));
    int records_num;

    char *block_of_records;
    if (hash_block[hash_bucket_num] == -1)
    {
        CALL_BF(BF_AllocateBlock(fileDesc));
        CALL_BF(BF_ReadBlock(fileDesc, BF_GetBlockCounter(fileDesc)-1, (void**) &block_of_records));
        records_num = 1;
        memcpy(block_of_records, &records_num, sizeof(int));
        int next_block = -1;
        memcpy(block_of_records+sizeof(int), &next_block, sizeof(int)); /*setting the last 4bytes to -1, to indicate that there is no next block*/
        memcpy(block_of_records+(2*sizeof(int)), &record, sizeof(Record));

        int blocks_num = BF_GetBlockCounter(fileDesc)-1;
        hash_block[hash_bucket_num] = blocks_num ;

        CALL_BF(BF_WriteBlock(fileDesc, blocks_num));
        CALL_BF(BF_WriteBlock(fileDesc, hash_block_num));
    }
    else
    {
        int currentBlock=hash_block[hash_bucket_num];
        CALL_BF(BF_ReadBlock(fileDesc, hash_block[hash_bucket_num], (void**) &block_of_records));
        int block_num = block_of_records[1];
        printf("%d size of int ",block_num);

        while (block_num != -1)
        {
            CALL_BF(BF_ReadBlock(fileDesc, block_num, (void**) &block_of_records));
            currentBlock=block_num;
            block_num = block_of_records[1];
        }

        int max_records_in_block = (BLOCK_SIZE - (2 * sizeof(int))) / sizeof(Record);
        int records_num = block_of_records[0];
        if (records_num == max_records_in_block)
        {
            int numberOfAllBlocks = BF_GetBlockCounter(fileDesc)-1;

            memcpy(block_of_records + sizeof(int), &numberOfAllBlocks, sizeof(int));
            CALL_BF(BF_WriteBlock(fileDesc, block_num));

            CALL_BF(BF_AllocateBlock(fileDesc));
            CALL_BF(BF_ReadBlock(fileDesc, (BF_GetBlockCounter(fileDesc)-1), (void**) &block_of_records));
            int one=1;
            int minus=-1;
            memcpy(block_of_records, &one, sizeof(int));
            memcpy(block_of_records+sizeof(int), &minus, sizeof(int));

            memcpy(block_of_records+(2*sizeof(int)), &record, sizeof(Record));

            CALL_BF(BF_WriteBlock(fileDesc, (BF_GetBlockCounter(fileDesc)-1)));
        }
        else
        {
            int numRecords = block_of_records[0] + 1;
            memcpy(block_of_records, &numRecords, sizeof(int));
            memcpy(block_of_records+(2*sizeof(int))+((num_of_records-1)*sizeof(Record)), &record, sizeof(Record));

            CALL_BF(BF_WriteBlock(fileDesc, currentBlock));
        }
    }


    //CALL_BF(BF_WriteBlock(fileDesc, hash_block_num));

    return OK;
}

int HT_DeleteEntry(HT_info header_info, void *value)
{
    return ERROR;
}

int HT_GetAllEntries(HT_info header_info, void *value)
{
    char *block;
    int num_of_records;
    int buckets;
    Record *record;

    int *int_value;
    int fileDesc = header_info.fileDesc;
    int num_of_blocks = BF_GetBlockCounter(fileDesc);

    CALL_BF(BF_ReadBlock(fileDesc, 0, (void **)&block));
    buckets = block[5];
    int num_hash_blocks = (buckets % 128 == 0) ? (buckets / 128) : ((buckets / 128) + 1);

    if (value == NULL)
    {
        for (int index = num_hash_blocks + 1; index < num_of_blocks; index++)
        {
            CALL_BF(BF_ReadBlock(fileDesc, index, (void **)&block));
            memcpy(&num_of_records, block, sizeof(int));
            record = (Record *)(block + sizeof(int));
            for (int i = 0; i < num_of_records; i++)
            {
                printf("Id: %d Name: %s Surname: %s Address: %s\n", record[i].id, record[i].name, record[i].surname, record[i].address);
            }
        }
    }
    else
    {
        int_value = (int *)value;

        int hash_value = *int_value % buckets;
        int hash_block_num = (hash_value / 128) + 1;
        int hash_bucket_num = hash_value % 128;
        CALL_BF(BF_ReadBlock(fileDesc, hash_block_num, (void **)&block));
        memcpy(&num_of_records, block, sizeof(int));
        record = (Record *)(block + 4);
        for (int i = 0; i < num_of_records; i++)
        {
            if (record[i].id == *int_value)
            {
                printf("Id: %d Name: %s Surname: %s Address: %s\n", record[i].id, record[i].name, record[i].surname, record[i].address);
            }
        }
    }
    return OK;
}

int HashStatistics(char *filename)
{
 return ERROR;
}
