#include "random.h"

char *random(void)
{
	const int MAX_LENGTH = 11;
	int i;
	// generate random number as integer
	get_random_bytes(&i, sizeof(i));
	printk("%d", i);

	// count the number of digits (including '-')
	int t = i, c = 0;
	if (t < 0) {
		t = -t;
		c = 1;
	}
	while (t > 0) {
		t = t / 10;
		++c;
	}

	// allocate string
	char *str = kzalloc(MAX_LENGTH, GFP_KERNEL);
	int index = 0;

	// pad space at the start
	int remaining = MAX_LENGTH - c, j = 0;
	while (j++ < remaining) {
		str[index++] = ' ';
	}

	// add '-' (if applicable)
	if (i < 0) {
		str[index] = '-';
		i = i * -1;
		++index;
	}

	// extract the rest of the digits (these digits are in the wrong direction)
	char *temp = kzalloc(c + 1, GFP_KERNEL);
	int temp_index = 0;
	while (i > 0) {
		temp[temp_index] = (char)(i % 10 + '0');
		i = i / 10;
		++temp_index;
	}

	// revert the digits to the right direction
	int k = temp_index - 1;
	while(k >= 0) {
		str[index++] = temp[k--];
	}

	printk("Final random number: %s", str);
	return str;
}



