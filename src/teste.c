#include <stdio.h>


int main(int argc, char * argv[]){
	int a = 0;

	if(__atomic_test_and_set (&a, __ATOMIC_SEQ_CST))
		printf("O cu é lindo");


	printf("%d", a);
}
