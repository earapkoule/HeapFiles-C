#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "heap_file.h"
#include "error_messages.h"

#define FILENAME "file"
#define MAX_FILES 100
#define MAX_BLOCKS 500
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
    char filename[5];
    HP_info *hp_pointer;
    strcpy(filename, FILENAME);

    HP_CreateFile(filename, 'c', "id", strlen("id"));
    hp_pointer = HP_OpenFile(filename);
    Record record;
    srand(12569874);
    int r;
    printf("Insert Entries\n");
    for (int id = 0; id < RECORDS_NUM; ++id)
    {
        record.id = id;
        r = rand() % 12;
        memcpy(record.name, names[r], strlen(names[r]) + 1);
        r = rand() % 12;
        memcpy(record.surname, surnames[r], strlen(surnames[r]) + 1);
        r = rand() % 10;
        memcpy(record.address, addresses[r], strlen(addresses[r]) + 1);

        HP_InsertEntry(*hp_pointer, record);
    }

    printf("RUN PrintAllEntries\n");
    int id = rand() % RECORDS_NUM;
    HP_GetAllEntries(*hp_pointer,NULL);

    // printf("Delete Entry with id = %d\n", id);
    //CALL_OR_DIE(HP_DeleteEntry(indexDesc, id));
    printf("Print Entry with id = %d\n", id);
    HP_GetAllEntries(*hp_pointer,&id); // must print something like : Entry doesn't exist or nothing at all

    HP_CloseFile(hp_pointer);
    return 0;
}