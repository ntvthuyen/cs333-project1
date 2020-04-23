#include "random.h"

int main(){
	int a = random_integer();
	unsigned c = random_unsigned_integer();
	double b = random_double();
	printf("%d %d \n",a,c);
	printf("%f",b);
	return 0;
}

