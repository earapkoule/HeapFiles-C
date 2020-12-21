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

    CALL_BF(BF_CreateFile(fileName));              // Creating a file
    int fileDesc = CALL_BF(BF_OpenFile(fileName)); // Opening existing file
    CALL_BF(BF_AllocateBlock(fileDesc));           // Allocating a block, the first one in this case
    CALL_BF(BF_ReadBlock(fileDesc, 0, (void **)&block));
    char identifier = '$';
    memcpy(block, &identifier, sizeof(char)); // Used to identify the hash file
    memcpy(block + 1, &attrType, sizeof(char));
    memcpy(block + 2, &attrLength, sizeof(int));
    memcpy(block + (2 + sizeof(int)), attrName, attrLength + 1);
    memcpy(block + (2 + sizeof(int)) + (attrLength + 1), buckets, sizeof(int));
    CALL_BF(BF_WriteBlock(fileDesc, 0));

    int num_hash_blocks = (buckets % 128 == 0) ? (buckets / 128) : ((buckets / 128) + 1);

    char *hash_block;
    for (int i = 0; i < num_hash_blocks; i++)
    {
        CALL_BF(BF_AllocateBlock(file_desc));
        CALL_BF((BF_ReadBlock(fileDesc, num_hash_blocks + 1, (void **)&hash_block)));
        for (i = 0; i < BLOCK_SIZE / sizeof(int); i++)
        {
            //hash_block_data[i] = -1;
        }
        CALL_BF(BF_WriteBlock(fileDesc, num_hash_blocks + 1));
    }
    CALL_BF(BF_CloseFile(fileDesc));

    return OK;
}
HT_info *HT_OpenIndex(char *fileName)
{
    char *block;
    HT_info *header_info = malloc(sizeof(HT_info));

    int fileDesc = CALL_OR_RETURN_NULL(BF_OpenFile(fileName)); // Opening existing file
    CALL_OR_RETURN_NULL(BF_ReadBlock(fileDesc, 0, (void **)&block));
    if (strcmp(block[0], '$'))
    {                      // Check if it is a HeapFile
        free(header_info); // Free memory in case the file does not open
        return NULL;
    }
    else
    {
        header_info->fileDesc = fileDesc;
        memcpy(header_info->attrType, block + 1, 1);
        memcpy(header_info->attrLength, block + 2, sizeof(int));
        memcpy(header_info->attrName, block + (2 + sizeof(int)), header_info->attrLength);
        memcpy(header_info->numBuckets, block + (2 + sizeof(int)) + (header_info->attrLength + 1), sizeof(int));
    }
    return header_info;
}

int HT_CloseIndex(HT_info *header_info)
{
    CALL_BF(BF_CloseFile(header_info->fileDesc));
    free(header_info); // Free memory before closing the file
    return OK;
}

int HT_InsertEntry(HT_info header_info, Record record)
{
    char *hash_block;
    int num_of_records;

    int fileDesc = header_info.fileDesc;
}

int HT_DeleteEntry(HT_info header_info, void *value)
{
    return -1;
}

int HT_GetAllEntries(HT_info header_info, void *value)
{
    char *block;
    int num_of_records;
    int buckets;
    Record *record;

    int *int_value;
    int fileDesc = header_info.fileDesc;
    int num_of_blocks = CALL_BF(BF_GetBlockCounter(fileDesc));
    if (value == NULL)
    {
        for (int index = 1; index < num_of_blocks; index++)
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
        CALL_BF(BF_ReadBlock(fileDesc, 0, (void **)&block));
        buckets=block[5];

        
        int hash_value = *int_value % buckets;
        int hash_block_num = (hash_value / 128) + 1;
        int hash_bucket_num = hash_value % 128;
        CALL_BF(BF_ReadBlock(fileDesc, hash_block_num, (void **)&block));

        //??
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
}

int HashStatistics(char *filename)
{
    return -1;
}