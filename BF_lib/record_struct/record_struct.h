#ifndef RECORD_STRUCT_H
#define RECORD_STRUCT_H

typedef struct Record {
	int id;
	char name[15];
	char surname[25];
	char address[50];
} Record;

typedef struct SecondaryRecord {
	Record record;
	int blockId;	// To block στο οποίο έγινε η εεισαγωγή της εγγραφής στο πρωτεύον ευρετήριο
} SecondaryRecord;

#endif // RECORD_STRUCT_H