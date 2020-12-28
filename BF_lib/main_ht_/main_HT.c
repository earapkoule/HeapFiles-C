#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "hash_table.h"

#define FILENAME "file"
#define MAX_FILES 100
#define MAX_BLOCKS 500
#define BUCKETS_NUM 5
#define RECORDS_NUM 100
const char *names[] = {
    "Yannis",
    "Christofos",
    "Sofia",
    "Marianna",
    "Vagelis",
    "Maria",
    "Iosif",
    "Dionisis",
    "Konstantina",
    "Theofilos",
    "Giorgos",
    "Dimitris"};

const char *surnames[] = {
    "Ioannidis",
    "Svingos",
    "Karvounari",
    "Rezkalla",
    "Nikolopoulos",
    "Berreta",
    "Koronis",
    "Gaitanis",
    "Oikonomou",
    "Mailis",
    "Michas",
    "Halatsis"};

const char *addresses[] = {
    "Athens",
    "San Francisco",
    "Los Angeles",
    "Amsterdam",
    "London",
    "New York",
    "Tokyo",
    "Hong Kong",
    "Munich",
    "Miami"};

int main(int argc, char **argv)
{
    int bfs[MAX_FILES];
    int i, j;
    char filename[5];
    void *block;
    int blkCnt;
    HT_info *ht_pointer;
    strcpy(filename, FILENAME);

    HT_CreateIndex(filename, 'c', "id", strlen("id"), BUCKETS_NUM);
    ht_pointer = HT_OpenIndex(filename);
    Record record;
    srand(12569874);
    int r;
    printf("Insert Entries\n");fflush(stdin);
    for (int id = 0; id < RECORDS_NUM; ++id)
    {printf("Insert Entries1\n");fflush(stdin);
        record.id = id;
        printf("Insert Entries2\n");fflush(stdin);
        r = rand() % 12;
        printf("Insert Entries3\n");fflush(stdin);
        memcpy(record.name, names[r], strlen(names[r]) + 1);
        printf("Insert Entries4\n");fflush(stdin);
        r = rand() % 12;
        printf("Insert Entries5\n");fflush(stdin);
        memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
        printf("Insert Entries6\n");fflush(stdin);
        r = rand() % 10;
        printf("Insert Entries7\n");fflush(stdin);
        memcpy(record.address, addresses[r], strlen(addresses[r]) + 1);
printf("Insert Entries8\n");fflush(stdin);
        HT_InsertEntry(*ht_pointer, record);
        printf("Insert Entries9\n");fflush(stdin);
    }

    printf("RUN PrintAllEntries\n");
    int id = rand() % RECORDS_NUM;
    HT_GetAllEntries(*ht_pointer,NULL);

    printf("Delete Entry with id = %d\n", id);
    //CALL_OR_DIE(HT_DeleteEntry(indexDesc, id));
    printf("Print Entry with id = %d\n", id);
    HT_GetAllEntries(*ht_pointer,&id); // must print something like : Entry doesn't exist or nothing at all

    HT_CloseIndex(ht_pointer);
    return 0;
}
