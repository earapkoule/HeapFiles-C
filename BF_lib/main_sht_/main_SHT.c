#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "hash_table.h"
#include "sec_hash_table.h"
#define FILENAME "file"
#define SFILENAME "sfile"
#define MAX_FILES 100
#define MAX_BLOCKS 500
#define BUCKETS_NUM 5
#define RECORDS_NUM 100

int main(int argc, char **argv)
{
    FILE* pf;
    pf = fopen(argv[1], "r");
    const char s[2] = ",";
	char line[256];
	char* token = NULL;
   	int len;
    int bfs[MAX_FILES];
    int i, j;
    int counterColumn;
    char filename[5];
    char sfilename[5];
    void *block;
    int blkCnt;
    HT_info *ht_pointer;
    SHT_info *sht_pointer;
    Record recordTable[5];
    int tableCounter=0;
    strcpy(filename, FILENAME);
    strcpy(sfilename, SFILENAME);
    SecondaryRecord tempRecord;

    HT_CreateIndex(filename, 'c', "id", strlen("id"), BUCKETS_NUM);
    ht_pointer = HT_OpenIndex(filename);
    
    SHT_CreateSecondaryIndex(sfilename,"surname", strlen("surname"), BUCKETS_NUM, filename);
    sht_pointer=SHT_OpenSecondaryIndex(sfilename);

    if (pf != NULL)
	{
        printf("Opening File.");
        fflush(stdin);
        while (fgets(line, sizeof(line), pf))
		{	
            counterColumn=0;
            
            token = strtok(line, s);
            if(token[0]=='{')
            {
                token++;
            }
            while ((token != NULL) && (token != '\n'))
			{
                len = strlen(token);
				if (token[len - 1] == '\n')
				{
					token[len - 2] = '\0';
				}
                if(counterColumn==0)
                {
                  tempRecord.record.id=atoi(token);      
                }
                else if (counterColumn==1)
                {
                   strcpy(tempRecord.record.name, token);     
                }
                else if (counterColumn==2)
                {
                   strcpy(tempRecord.record.surname, token); 
                }
                else if (counterColumn==3)
                {
                    strcpy(tempRecord.record.address, token);
                }
                token = strtok(NULL, s);
                counterColumn++;
            }
            if(tableCounter<5)
            {
                recordTable[tableCounter]=tempRecord.record;
                tableCounter++;
            }
            if(HT_InsertEntry(*ht_pointer, tempRecord.record)== BFE_OK);
            {
                SHT_SecondaryInsertEntry(*sht_pointer,tempRecord);
            }
        }
        for (int k=0;k<5;k++)
        {
            printf("PrintingEntries From HashTable\n");

            HT_GetAllEntries(*ht_pointer, recordTable[k].id);
        }
        for (int k=0;k<5;k++)
        {
            printf("PrintingEntries From SecondaryHashTable\n");

            SHT_SecondaryGetAllEntries(*sht_pointer,*ht_pointer,recordTable[k].surname);
        }
    }
    else
	{
		printf("File doesn't exist.");
	}
    HT_CloseIndex(ht_pointer);
    SHT_CloseSecondaryIndex(sht_pointer);
    return 0;
}