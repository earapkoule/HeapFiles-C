#ifndef HEAP_FILE_H
#define HEAP_FILE_H

#include "record_struct.h"

typedef struct HP_info {
  int fileDesc;		/* αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */
  char attrType;	/* ο τύπος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο, 'c' ή 'i' */
  char *attrName;	/* το όνομα του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
  int attrLength;	/* το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
} HP_info;

int HP_CreateFile(
	char *fileName, /* όνομα αρχείου */
	char attrType,	/* τύπος πεδίου-κλειδιού: 'c', 'i' */
	char *attrName, /* όνομα πεδίου-κλειδιού */
	int attrLength	/* μήκος πεδίου-κλειδιού */
	);

HP_info* HP_OpenFile(
	char *fileName	/* όνομα αρχείου */
	);

int HP_CloseFile(
	HP_info* header_info	/* δομή που προσδιορίζει το αρχείο */
	);

int HP_InsertEntry(
	HP_info header_info,/* επικεφαλίδα του αρχείου */
	Record record		/* δομή που προσδιορίζει την εγγραφή */
	);

int HP_DeleteEntry(
	HP_info header_info,/* επικεφαλίδα του αρχείου */
	void *value			/* τιμή του πεδίου-κλειδιού προς διαγραφή */
	);

int HP_GetAllEntries(
	HP_info header_info,/* επικεφαλίδα του αρχείου */
	void *value         /* τιμή του πεδίου-κλειδιού προς αναζήτηση */
	);

#endif // HEAP_FILE_H