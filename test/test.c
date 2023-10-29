#include <unistd.h>
#include <stdio.h>

int main()
{
	char buf[4];

	printf("len: %zu", read(0, buf, 4));
	printf("len: %zu", read(0, buf, 4));
}