#include <iostream>
#include "bigInt.h"
void testBigAndBig() {
    BigInt a , b;
    char op;
    std::cin >> a >> op >> b;
    BigInt c{};
    switch (op) {
        case '+':
            c = a + b;
            a += b;
            assert(a == c);
            std::cout << c << std::endl;
            break;
        case '-':
            c = a - b;
            a -= b;
            assert(a == c);
            std::cout << c << std::endl;
            break;
        case '*':
            c = a * b;
            a *= b;
            assert(a == c);
            std::cout << c << std::endl;
            break;
        case '/':
            if (b == BigInt(0)) {
                std::cout << "Error: Division by zero" << std::endl;
            } else {
                c = a / b;
                a /= b;
                assert(a == c);
                std::cout << c << std::endl;
            }
        break;
        case '%':
            if (b == BigInt(0)) {
                std::cout << "Error: Division by zero" << std::endl;
            } else {
                c = a % b;
                a %= b;
                assert(a == c);
                std::cout << c << std::endl;
            }
        break;
    }
}
void testBigAndSmall() {
    BigInt a;
    int b;
    char op;
    std::cin >> a >> op >> b;
    BigInt c{};
    switch (op) {
        case '+':
            c = a + BigInt(b);
            a += BigInt(b);
            assert(a == c);
            std::cout << c << std::endl;
            break;
        case '-':
            c = a - BigInt(b);
            a -= BigInt(b);
            assert(a == c);
            std::cout << c << std::endl;
            break;
        case '*':
            c = a * b;
            a *= b;
            assert(a == c);
            std::cout << c << std::endl;
            break;
        case '/':
            if (b == 0) {
                std::cout << "Error: Division by zero" << std::endl;
            } else {
                c = a / b;
                a /= b;
                assert(a == c);
                std::cout << c << std::endl;
            }
        break;
        case '%':
            if (b == 0) {
                std::cout << "Error: Division by zero" << std::endl;
            } else {
                c = a % b;
                a %= b;
                assert(a == c);
                std::cout << c << std::endl;
            }
        break;
    }
}

int main() {
    char op;
    std::cin >> op;
    switch (op) {
        case '1':
            testBigAndBig();
            break;
        case '2':
            testBigAndSmall();
            break;
    }
    return 0;
}