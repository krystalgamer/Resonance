#include "xor.h"
#include <windows.h>

unsigned char functionBegining[] = "\x55\x89\xE5";
unsigned char functionEnding[] = "\xC9\xC3";
extern unsigned char buffer[];
extern const unsigned char functionStart[];
extern const unsigned char functionEnd[];

DWORD getFunctionBegining( DWORD counterPos, DWORD modulePosition, HANDLE hProcess){
	
	
	static unsigned char tmp[20];//Should be nuff
	BYTE tmpCounter = 0, timesRead = 0;
	
	while(1){
		
		if(counterPos > 0){
			if(!memcmp(functionBegining, &buffer[counterPos], 3))
				return counterPos;
			counterPos--;
			}
		else
		{
			if(!ReadProcessMemory(hProcess, (void*)modulePosition - timesRead * 20, tmp, 20, NULL)){
				MessageBoxA(NULL, "ReadProcessMemory1 failed!", "FAILED", 0);
				return 0;
			}
			timesRead++;
			
			while(tmpCounter < 19){
				
				if(!memcmp(functionEnding, &tmp[tmpCounter], 2))
					return tmpCounter + (timesRead - 1) * 2;
			}
			
		}
	}
	
	return 0;//HOW?
}

DWORD getFunctionEnding(DWORD bytesRead, DWORD counterPos, DWORD modulePosition, HANDLE hProcess){
	
	
	static unsigned char tmp[20];//Should be nuff
	BYTE tmpCounter = 0, timesRead = 0;
	
	while(1){
		
		if(counterPos <= bytesRead - 2){
			if(!memcmp(functionEnding, &buffer[counterPos], 2))
				return counterPos+2;
			counterPos++;
			}
		else
		{
			if(!ReadProcessMemory(hProcess, (void*)modulePosition + timesRead * 20, tmp, 20, NULL)){
				MessageBoxA(NULL, "ReadProcessMemory1 failed!", "FAILED", 0);
				return 0;
			}
			timesRead++;
			
			while(tmpCounter < 20){
				
				if(!memcmp(functionEnding, &tmp[tmpCounter], 2))
					return tmpCounter + (timesRead - 1) * 2;
			}
			
		}
	}
	
	return 0;//HOW?
}

void getFunctionsToEncrypt(HANDLE hProcess, DWORD modBaseAddr, DWORD modSize, PFUNCTION pFunction){
	
	static DWORD counter = 0, counter1 = 0, counter2 = 0, bytesRead = 0;
	BOOL functionStartDetected = FALSE, functionEndDetected = FALSE;
	
	
	counter = 0;
	while(counter < modSize){
		
		//Read the memory
		if(!ReadProcessMemory(hProcess, (void*)modBaseAddr+counter, buffer, 4096, &bytesRead)){
			MessageBoxA(NULL, "ReadProcessMemory failed!", "FAILED", 0);
			return;
		}
		
		//Analyze the memory
		counter1 = counter2 = functionStartDetected = functionEndDetected = FALSE;
		
		while(counter1 < bytesRead){
			
			if(!functionStartDetected){
				counter2 += (buffer[counter1] == functionStart[counter2]) ? 1 : -(counter2); //Add or go back to 0
				
				functionStartDetected = (counter2 == 6);	//Set the bool only if detected
				
				pFunction->startAddr = (counter2 == 6) ? modBaseAddr+counter+
				getFunctionBegining( counter1, modBaseAddr+counter, hProcess)				: 0;
				
				counter2 = functionStartDetected ? 0 : counter2;//reset if the function was detected
			}
			else if(!functionEndDetected){
				counter2 += (buffer[counter1] == functionEnd[counter2]) ? 1 : -(counter2); //Add or go back to 0
				functionEndDetected = (counter2 == 6);	
				
				pFunction->size = (counter2 == 6) ? 
				modBaseAddr+counter+getFunctionEnding(bytesRead, counter1+1, modBaseAddr+counter,hProcess) - pFunction->startAddr : 0; //+1 is needed becaause it's incorrectly placed
				
				counter2 = functionEndDetected ? 0 : counter2;
				
			}
			
			if(functionStartDetected && functionEndDetected)
				return;
			
			counter1++;
		
		}
		
		counter += bytesRead;
		bytesRead = 0; //not needed tbh
	}
}

bool isValidExe(void *exe){
	
	PIMAGE_DOS_HEADER dosHeader = exe;
	if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
		return false;
	
	PIMAGE_NT_HEADERS32 ntHeader = (PIMAGE_NT_HEADERS32)((DWORD)dosHeader->e_lfanew + (DWORD)dosHeader);
	if(ntHeader->Signature != IMAGE_NT_SIGNATURE)
		return false;
	
	if(ntHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386)
		return false;
	
	return true;
}

void scanSections(void *exe){
	
	//skip to the start of section table
	PIMAGE_DOS_HEADER dosHeader = exe;
	PIMAGE_NT_HEADERS32 ntHeader = (PIMAGE_NT_HEADERS32)((DWORD)dosHeader + (DWORD)dosHeader->e_lfanew);
	PIMAGE_SECTION_HEADER sectionTable = (PIMAGE_SECTION_HEADER)((DWORD)dosHeader + dosHeader->e_lfanew + offsetof(IMAGE_NT_HEADERS32, FileHeader) 
	+ sizeof(IMAGE_FILE_HEADER) + ntHeader->FileHeader.SizeOfOptionalHeader);
	
	for(int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
		printf("Name of section %s\n", sectionTable[i].Name);
	
}

int main(){
	
	PROCESS_INFORMATION processInfo;
	STARTUPINFO startupInfo;
	DWORD bytesWritten;
	
	
	//opens the file
	HANDLE hFile = CreateFile("test.exe", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE){
		puts("Couldn't open the file test.exe");
		return 1;
	}
	
	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	if(hFileMapping == NULL){
		CloseHandle(hFile);
		puts("Couldn't create file mapping");
	}
	
	PVOID mappedFile = MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);
	if(!mappedFile){
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		puts("Couldn't MapViewOfFile");
	}
	
	//checks if it's a 32bit exe
	if(!isValidExe(mappedFile)){
		UnmapViewOfFile(mappedFile);
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		puts("Not a valid EXE");
		return 2;
	}
	
	scanSections(mappedFile);
	
	return 0;
	ZeroMemory(&startupInfo, sizeof(startupInfo));
	startupInfo.cb = sizeof startupInfo ; //Only compulsory field
	
	CreateProcess("test.exe", NULL, NULL, NULL, FALSE, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, NULL, NULL, &startupInfo, &processInfo);
	
	DWORD moduleSize = 0, baseAddr = 0;

	ResumeThread(processInfo.hThread);
	getchar();
	
	
	//Suspend so we can scan it in peace
	SuspendThread(processInfo.hThread);
	getModuleHandle(processInfo.dwProcessId, "test.exe", &baseAddr, &moduleSize);
	
	
	FUNCTION function;
	function.startAddr = function.size = 0;
	getFunctionsToEncrypt(processInfo.hProcess, baseAddr, moduleSize, &function);

	srand(time(NULL));
	if(interptFunction(processInfo.hProcess, function))
		MessageBoxA(NULL,"YEAH", "YEAH", 0);
	else
		MessageBoxA(NULL,"NO", "NO", 0);
	getchar();
	
	//Resume it
	ResumeThread(processInfo.hThread);
	getchar();
	
	return 0;
}




