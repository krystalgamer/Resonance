#include <stdio.h>
#include <windows.h>

void useless();

/*void printCoisas(){

	asm("or %eax,%eax\n"
	"or %eax,%eax\n"
	"or %eax,%eax\n");
	
	printf("OLA\n");
	printf("SUP\n");
	
	asm("or %ecx,%ecx\n"
	"or %ecx,%ecx\n"
	"or %ecx,%ecx\n");
}*/

int main(){
	
	while(1){
		
		useless();
		fflush(stdout);
	}
	
	return 0;
}
