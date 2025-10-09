#include <iostream>
using namespace std;

int main() {
    // Это однострочный комментарий
    int x = 42;
    float pi = 3.14159;
    string message = "Hello, World!";

    /* Это многострочный
       комментарий */

    if (x > 0) {
        cout << message << endl;
        x++;
        pi -= 1.5e-3;
    }

    for (int i = 0; i < 10; ++i) {
        x *= 2;
    }

    return 0
}