#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BF.h"
#include "sec_hash_table.h"
#include "hash_table.h"
#include "record_struct.h"
#include "error_messages.h"

int SHT_CreateSecondaryIndex(char *sfileName, char* attrName, int attrLength, int buckets, char* fileName) {
	return OK;
}

SHT_info* SHT_OpenSecondaryIndex(char *sfileName) {
	return NULL;
}

int SHT_CloseSecondaryIndex(SHT_info* header_info) {
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