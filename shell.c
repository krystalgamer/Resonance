
void funcaoCopiada();

void copyFunction(){

	unsigned int* counter = (unsigned int*)0x12345678;
	
	if(*counter == 0xFF){
		*counter = 0;
		
	}
	
	funcaoCopiada();
}

int main(){
	
	return 0;
}