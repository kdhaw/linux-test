#define _USE_MATH_DEFINES
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <math.h>

#define N 4

void sinx_taylor(int num_elements, int terms, double* x, double* result) {
        for (int i = 0; i < num_elements; i++) {
                double value = x[i];
                double number = x[i] * x[i] * x[i];
                double denom = 6.0; //3!
                int sign = -1;

                for (int j = 1; j < terms; j++) {
                        value += (double)sign * number / denom;
                        number *= x[i] * x[i];
                        denom *= (2.*(double)j + 2.) * (2.*(double)j +3.);
                        sign *= -1;
                }

                result[i] = value;
        }
}

int main() {
        double x[N] = {0, M_PI / 6., M_PI / 3., 0.134};
        double res[N];
        int fd[2];

        if (pipe(fd) == -1) {
        perror("pipe error");
        exit(1);
	}

    	for (int i = 0; i < N; i++) {
        	pid_t pid = fork();

        	if (pid < 0) {
            	perror("fork error");
            	exit(1);
        	} else if (pid == 0) { 
            	close(fd[0]); 
            	double result;
            	double x_val[1] = { x[i] };
            	double res_single[1];
            	sinx_taylor(1, 5, x_val, res_single);
            	result = res_single[0];

            	write(fd[1], &result, sizeof(double));
            	close(fd[1]);
            	exit(0);
        	}
    	}

    	close(fd[1]);
    	for (int i = 0; i < N; i++) {
        	read(fd[0], &res[i], sizeof(double));
        	wait(NULL);
    	}
    	close(fd[0]);

    	for (int i = 0; i < N; i++) {
		printf("sin(%.3f) by Taylor = %.6f | sin(x) = %.6f\n", 
				x[i], res[i], sin(x[i]));
	}

    	return 0;
}
