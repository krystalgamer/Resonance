#include "xor.h"

const unsigned char functionStart[] = "\x09\xC0\x09\xC0\x09\xC0";

const unsigned char functionEnd[] = "\x09\xC9\x09\xC9\x09\xC9";

char text[50];

unsigned char buffer[4096];

BOOL getModuleHandle(DWORD processId, const char* name, DWORD* baseAddr, DWORD* moduleSize){
	
	
	HANDLE moduleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);
	
	MODULEENTRY32 moduleEntry;
	moduleEntry.dwSize = sizeof moduleEntry;
	
	
	
	
	if(moduleSnap == INVALID_HANDLE_VALUE)
		return FALSE;
		
	
	Module32First(moduleSnap, &moduleEntry);
	
	
	do{
		
		if(!strcmp(name, moduleEntry.szModule)){
			
			(*baseAddr) = (DWORD)moduleEntry.modBaseAddr;
			(*moduleSize) = moduleEntry.modBaseSize;
			CloseHandle(moduleSnap);
			return TRUE;
		}
		
	}while(Module32Next(moduleSnap, &moduleEntry));
	
	return FALSE;
}

void freeLinkedList(void **entry, DWORD sizeOfEntry){
	
	if(!(*(DWORD*)entry))
		return;
	
	freeLinkedList((void*)(*(DWORD*)(entry)) + sizeOfEntry-4, sizeOfEntry); //next level casting lol
	free((void*)*(DWORD*)entry);
	*(DWORD**)entry = NULL;
}

void removeLastEntry(void **entry, uint32_t sizeOfEntry){
	
	if(!(*(uint32_t*)entry))
		return;//should never reach here
	
	uint32_t *curNext = (uint32_t*)(*(uint32_t*)entry + sizeOfEntry-4);
	if((void*)*curNext == NULL){
		
		free(*entry);//no leaks
		*entry = NULL;
		return;
	}
	
	removeLastEntry((void*)((*(uint32_t*)entry)+sizeOfEntry-4), sizeOfEntry);
	
}




BOOLEAN FindPattern(PROCESS_INFORMATION* processInfo, char* pattern, char* mask, void* address){
	
	DWORD scan1 = 0,scan2 = 0;
	BOOLEAN found = TRUE;
	BYTE* memPosition = (BYTE*)0x69;
	
	
	for(scan1 = 0; scan1 < 0x70; scan1++){
		
		found = TRUE;
		for(scan2 = 0; scan2<strlen(mask); scan2++){
			
			found &= ((BYTE)mask[scan2] == '?' || memPosition[scan1 + scan2] == (BYTE)pattern[scan2]);//Casting is needed if not it fuckups
			if(!found)
				break;
		}
		
		if(found){
			(*(DWORD*)address) = (DWORD)memPosition + scan1; //Treat as 32bit value
			return TRUE;
		}
		
	}
	return FALSE;
}