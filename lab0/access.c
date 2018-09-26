#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "md5.h"

int check_pin(char *name, int test) {
	char *salt = "SodiumChloride";
	char *expect = "\x79\xa6\x8f\x6c\xc5\xd2\x29\x6f\x1c\x15\x11\xa0\x41\x14\xfa\x50";
	MD5_CTX ctx;
	char temp[100];
	int i;

	if (test > 9999) { return 0; }

	MD5_Init(&ctx);
	MD5_Update(&ctx, salt, strlen(salt));
	MD5_Update(&ctx, name, strlen(name));
	snprintf(temp, 99, "%u", test);
	MD5_Update(&ctx, temp, strlen(temp));
	memset(temp, 0, 100);
	MD5_Final(temp, &ctx);

	return !strcmp(expect, temp);
}

int main(void) {

	char name[20];
	int pin;
	int ok;

	printf("%s\n", "Please enter your name:");
	scanf("%19s", name);
	printf("%s, %s\n", name, "please enter your PIN:");
	scanf("%u", &pin);
	ok = check_pin(name, pin);

	if (ok) {
		printf("%s\n", "Success.");
	} else {
		printf("%s\n", "Incorrect.");
	}

	return 0;
}

