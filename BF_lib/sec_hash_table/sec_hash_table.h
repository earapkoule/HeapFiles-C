#ifndef SEC_HASH_TABLE_H
#define SEC_HASH_TABLE_H

#include "record_struct.h"
#include "hash_table.h"

typedef struct SHT_info {
	int fileDesc;	/* αναγνωριστικός αριθμός ανοίγματος αρχείου από το επίπεδο block */
	char *attrName;	/* το όνομα του πεδίου που ςίναι κλειδί για το συγκεκριμένο αρχείο */
	int attrLength;	/* το μέγεθος του πεδίου που είναι κλειδί για το συγκεκριμένο αρχείο */
	long int numBuckets;	/* το πλήθος των "κάδων" του αρχείου κατακερματισμού */
	char *fileName;	/* όνομα αρχείου με πρωτεύον ευρετήριο στο id */
} SHT_info;

int SHT_CreateSecondaryIndex(
	char *sfileName,	/* όνομα αρχείου */
	char* attrName,	/* όνομα πεδίου-κλειδιού */
	int attrLength,	/* μήκος πεδίου-κλειδιού */
	int buckets,	/* αριθμός κάδων κατακερματισμού*/
	char* fileName	/* όνομα αρχείου πρωτεύοντος ευρετηρίου*/
	);

SHT_info* SHT_OpenSecondaryIndex(
	char *sfileName	/* όνομα αρχείου */
	);

int SHT_CloseSecondaryIndex(
	SHT_info* header_info	/* δομή που προσδιορίζει το αρχείο */
	);

int SHT_SecondaryInsertEntry(
	SHT_info header_info,	/* επικεφαλίδα του αρχείου*/
	SecondaryRecord record 	/* δομή που προσδιορίζει την εγγραφή */
	);

int SHT_SecondaryGetAllEntries(
	SHT_info header_info_sht,	/* επικεφαλίδα του αρχείου δευτερεύοντος ευρετηρίου*/
	HT_info header_info_ht,	/* επικεφαλίδα του αρχείου πρωτεύοντος ευρετηρίου*/
	void *value	/* τιμή του πεδίου-κλειδιού προς αναζήτηση */
	);

int HashStatistics(
	char* filename 	/* όνομα του αρχείου που ενδιαφέρει */
	);

#endif // SEC_HASH_TABLE_H