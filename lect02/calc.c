#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
	if (argc != 4) {
		return 1;
	}

	char *num1_str = argv[1];
	char *op_str = argv[2];
	char *num2_str = argv[3];

	if (strlen(op_str) != 1) {
		return 2;
	}

	char op = op_str[0];

	for(int i = 0; num1_str[i]; i++) {
		if (!isdigit(num1_str[i]) && num1_str[i] != '.') {
			return 1;
		}
	}
	for(int i = 0; num2_str[i]; i++) {
		if (!isdigit(num2_str[i]) && num2_str[i] != '.') {
			return 1;
		}
	}

	double num1 = atof(num1_str);
	double num2 = atof(num2_str);
	double result;

	switch(op) {
		case '+':
			result = num1 + num2;
			break;
		case '*':
			result = num1 * num2;
			break;
	}

	if (result == (int)result) {
		printf("%d\n", (int)result);
	}else {
		printf("%.2f\n", result);
	}

	return 0;
}
