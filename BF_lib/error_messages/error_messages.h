#ifndef ERROR_MESSAGES_H
#define ERROR_MESSAGES_H

#include "BF.h"

#define OK 0
#define ERROR -1

#define CALL_BF(call)       					\
{                           					\
  if (call < BFE_OK) {     						\
    if(BF_Errno == BFE_NOMEM) {					\
	    BF_PrintError(							\
	    	"NOMEM"								\
	    );					    				\
	    return ERROR;        					\
    }											\
    if(BF_Errno == BFE_CANNOTOPENFILE) {		\
    	BF_PrintError(							\
    		"CANNOTOPENFILE"					\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_CANNOTCLOSEFILE) {		\
    	BF_PrintError(							\
    		"CANNOTCLOSEFILE"					\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_CANNOTCREATEFILE) {		\
    	BF_PrintError(							\
    		"CANNOTCREATEFILE"					\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_INCOMPLETEREAD) {		\
    	BF_PrintError(							\
    		"INCOMPLETEREAD"					\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_INCOMPLETEWRITE) {		\
    	BF_PrintError(							\
    		"INCOMPLETEWRITE"					\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_FILEEXISTS) {			\
    	BF_PrintError(							\
    		"FILEEXISTS"						\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_NOBUF) {					\
    	BF_PrintError(							\
    		"NOBUF"								\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_LISTERROR) {				\
    	BF_PrintError(							\
    		"LISTERROR"							\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_FILEOPEN) {				\
    	BF_PrintError(							\
    		"FILEOPEN"							\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_FD) {					\
    	BF_PrintError(							\
    		"FD"								\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_FILENOTEXISTS) {			\
    	BF_PrintError(							\
    		"FILENOTEXISTS"						\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_FTABFULL) {				\
    	BF_PrintError(							\
    		"FTABFULL"							\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_HEADOVERFLOW) {			\
    	BF_PrintError(							\
    		"HEADOVERFLOW"						\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_BLOCKFIXED) {			\
    	BF_PrintError(							\
    		"BLOCKFIXED"						\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_BLOCKUNFIXED) {			\
    	BF_PrintError(							\
    		"BLOCKUNFIXED"						\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_EOF) {					\
    	BF_PrintError(							\
    		"EOF"								\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_FILEHASFIXEDBLOCKS) {	\
    	BF_PrintError(							\
    		"FILEHASFIXEDBLOCKS"				\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_BLOCKFREE) {				\
    	BF_PrintError(							\
    		"BLOCKFREE"							\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_BLOCKINBUF) {			\
    	BF_PrintError(							\
    		"BLOCKINBUF"						\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_BLOCKNOTINBUF) {			\
    	BF_PrintError(							\
    		"BLOCKNOTINBUF"						\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_INVALIDBLOCK) {			\
    	BF_PrintError(							\
    		"INVALIDBLOCK"						\
    	);					    				\
    	return ERROR;        					\
    }											\
    if(BF_Errno == BFE_CANNOTDESTROYFILE) {		\
    	BF_PrintError(							\
    		"CANNOTDESTROYFILE"					\
    	);					    				\
    	return ERROR;        					\
    }											\
  }                         					\
}

#endif // ERROR_MESSAGES_H