### Program Description

This is an individual programming project for the first year of studies. The task is to implement operations on sparse multivariable polynomials and to write a calculator that performs calculations on such polynomials.

Basic polynomial operations have been implemented, including:

- Adding, subtracting, and multiplying polynomials
- Calculating the value of a polynomial for a given x (x_0 = x)
- Calculating the degree of a polynomial, as well as the degree of a polynomial with respect to a given variable
- Copying a polynomial
- Composing a polynomial
- The implementation is contained in two files: poly.h and poly.c.

The poly.h file contains the definitions of polynomial and monomial structures, as well as the definitions and implementations of basic functions.
The poly.c file contains implementations of more complex functions.

The second part of the task is to implement a calculator that operates on sparse multivariable polynomials.
The calculator performs the following commands:

- ZERO – places a polynomial identical to zero on top of the stack
- IS_COEFF – checks if the polynomial on top of the stack is a coefficient and prints 0 or 1 to standard output
- IS_ZERO – checks if the polynomial on top of the stack is identically zero and prints 0 or 1 to standard output
- CLONE – places a copy of the polynomial on top of the stack
- ADD – adds the two polynomials at the top of the stack, removes them, and places their sum on top of the stack
- MUL – multiplies the two polynomials at the top of the stack, removes them, and places their product on top of the stack
- NEG – negates the polynomial on top of the stack
- SUB – subtracts the polynomial below the top of the stack from the polynomial at the top of the stack, removes them, and places their difference on top of the stack
- IS_EQ – checks if the two polynomials at the top of the stack are equal and prints 0 or 1 to standard output
- DEG – prints the degree of the polynomial (−1 for the polynomial identically equal to zero) to standard output
- DEG_BY idx – prints the degree of the polynomial with respect to the variable with index idx (−1 for the polynomial identically equal to zero) to standard output
- AT x – evaluates the polynomial at point x, removes the polynomial at the top of the stack, and places the result of the operation on top of the stack
- PRINT – prints the polynomial at the top of the stack to standard output
- POP – removes the polynomial at the top of the stack
- COMPOSE – composes the polynomial p at the top of the stack with the polynomials q[k - 1], q[k - 2], …, q[0] removed from the stack in sequence. The result of the composition is placed on the stack.
The implementation of the calculator is contained in the calc.c file.




### Opis programu

Zadanie z indywidualnego projektu programistycznego na 1 roku studiów.
Zadanie polega na zaimplementowaniu operacji na wielomianach
rzadkich wielu zmiennych oraz napisaniu kalkulatora wykonującego obliczenia na takich wielomianach.

Zaimplementowane zostały podstawowe operacje na wielomianach w tym:
- dodawanie, odejmowanie i mnożenie wielomianów
- obliczanie wartości wielomianu dla danego x (x_0 = x)
- obliczanie stopień wielomianu, oraz stopień wielomianu ze względu na daną zmienną
- kopiowanie wielomianu
- składanie wielomianu

Implementacja znajduje się w dwóch plikach: poly.h oraz poly.c.

Plik poly.h zawiera definicje struktur wielomianu i jednomianu a także
definicje i implementacje podstawowych funkcji.

Plik poly.c zawiera implementacje bardziej skomplikowanych funkcji.

Druga część zadania polega na zaimplementowaniu kalkulatora operującego na wielomianach
rzadkich wielu zmiennych.

Kalkulator wykonuje następujące polecenia:

- ZERO – wstawia na wierzchołek stosu wielomian tożsamościowo równy zeru
- IS_COEFF – sprawdza, czy wielomian na wierzchołku stosu jest współczynnikiem – wypisuje na standardowe wyjście 0 lub 1
- IS_ZERO – sprawdza, czy wielomian na wierzchołku stosu jest tożsamościowo równy zeru – wypisuje na standardowe wyjście 0 lub 1
- CLONE – wstawia na stos kopię wielomianu z wierzchołka
- ADD – dodaje dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek stosu ich sumę
- MUL – mnoży dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek stosu ich iloczyn
- NEG – neguje wielomian na wierzchołku stosu
- SUB – odejmuje od wielomianu z wierzchołka wielomian pod wierzchołkiem, usuwa je i wstawia na wierzchołek stosu różnicę
- IS_EQ – sprawdza, czy dwa wielomiany na wierzchu stosu są równe – wypisuje na standardowe wyjście 0 lub 1
- DEG – wypisuje na standardowe wyjście stopień wielomianu (−1 dla wielomianu tożsamościowo równego zeru)
- DEG_BY idx – wypisuje na standardowe wyjście stopień wielomianu ze względu na zmienną o numerze idx (−1 dla wielomianu tożsamościowo równego zeru)
- AT x – wylicza wartość wielomianu w punkcie x, usuwa wielomian z wierzchołka i wstawia na stos wynik operacji
- PRINT – wypisuje na standardowe wyjście wielomian z wierzchołka stosu
- POP – usuwa wielomian z wierzchołka stosu
- COMPOSE - składa wielomian p z wierzchu stosu z wielomianami q[k - 1], q[k - 2], …, q[0] zdejmowanymi kolejno ze stosu. Wynik złożenia umieszcza na stosie

Implementacja kalkulatora znajduje się w pliku calc.c
