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
