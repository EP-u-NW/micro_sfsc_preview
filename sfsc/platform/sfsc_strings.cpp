/*
 * sfsc_strings.c
 *
 *  Created on: 05.12.2020
 *      Author: Eric
 */


#include "sfsc_strings.h"

#ifndef HAS_STRING_H

void* memcpy(void* to, const void* from,sfsc_size size){
	for(sfsc_size i=0;i<size;i++){
		to[i]=from[i];
	}
	return to;
}

sfsc_size strlen(const char* buf){
	for(sfsc_size i=0;;i++){
		if(buf[0]==NULL){
			return i;
		}
	}
}

void* memset(void* block,int c,sfsc_size size){
	unsigned char value=(unsigned char)c;
	for(sfsc_size i=0;i<size;i++){
			block[i]=value;
		}
		return block;
}
#endif
