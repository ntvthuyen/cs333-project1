#include "random.h"
#include <stdlib.h>

int random_integer(){
	time_t t;	
	// Init random number generator
	srand((unsigned) time(&t));		
	return (int)rand();
}

unsigned int random_unsigned_integer(){
	time_t t;
	// Init random number generator
	
	srand48((long) time(&t));

	return mrand48();
}

double random_double(){
	time_t t;
	// Init random number generator
	
	srand48((long) time(&t));
	

	return drand48();
}


