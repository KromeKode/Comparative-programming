#include <math.h>
#include <stdio.h>

int main(void) {
    double a = 0.1;
    double b = 0.2;
    double sum = a + b;

    // %.17g prints enough digits to show the stored value; plain %g would round it to 0.3.
    printf("0.1 + 0.2 = %.17g\n", sum);
    printf("equal to 0.3? %s\n", sum == 0.3 ? "yes" : "no");
    printf("close to 0.3? %s\n", fabs(sum - 0.3) < 1e-9 ? "yes" : "no");
    return 0;
}
