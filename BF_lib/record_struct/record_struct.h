#ifndef RECORD_STRUCT_H
#define RECORD_STRUCT_H

typedef struct Record {
	int id;
	char name[15];
	char surname[25];
	char address[50];
} Record;

#endif // RECORD_STRUCT_H