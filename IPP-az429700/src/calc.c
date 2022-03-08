/** @file
Implementacja kalkulatora wykonującego operacje na wielomianach wielu zmiennych

@author Agata Załęska
@date 2021
*/

#define _GNU_SOURCE                     ///<zapewnienie prawidłowego działania getline

#include "poly.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#define OPEN_PARENTHESIS        '('         ///<znak nawias otwierający
#define CLOSE_PARENTHESIS       ')'         ///<znak nawias zamykający
#define MINUS                   '-'         ///<znak minus
#define COMMA                   ','         ///<znak przecinek
#define PLUS                    '+'         ///<znak plus
#define COMMENT_PREF            '#'         ///<znak hash - początek komentarza
#define SPACE                   ' '         ///<znak spacja
#define UNDERSCORE              '_'         ///<znak podkreślnik
#define ZERO_CHAR               '0'         ///<znak 0
#define NINE_CHAR               '9'         ///<znak 9
#define NEWLINE                 '\n'        ///<znak końca linii
#define AT_COM                  "AT"        ///<znak komenda AT
#define DEG_COM                 "DEG_BY"    ///<znak komenda DEG_BY
#define COMPOSE_COM             "COMPOSE"   ///<znak komenda COMPOSE

#define A_CHAR                  'a'      ///<małe 'a'
#define A_CHAR_CAPITAL          'A'      ///<wielkie 'A'
#define Z_CHAR                  'z'      ///<małe 'z'
#define Z_CHAR_CAPITAL          'Z'      ///<wielkie 'Z'

#define MAX_INSTRUCION_LEN       9      ///<maksymalna długość instrukcji
#define DECIMAL_BASE             10     ///<baza systemu dziesiętnego

///definicje określające stan wczytywania wielomianu (do funkcji CorrectPoly)
#define READING_COEFF           1001    ///<stan: wczytywanie współczynnika
#define READING_EXP             1002    ///<stan: wczytywane wykładnika
#define END_OF_MONO             1003    ///<stan: koniec jednomianu

#define INITIAL_ARR_SIZE        5       ///<początkowa długość tablicy zmiennej długości

///operacje arytmetyczne dwuargumentowe
enum binary_operation {
    ADD, MUL, SUB
};

///To jest struktura implementująca stos wielomianów.
struct stack{
    Poly v; ///<wielomian na wierzchu stosu
    struct stack *next; ///<wskaźnik na strukturę z kolejnym wielomianem

}; typedef struct stack stack; ///<nazwa struktury

///Funkcja inicjująca stos.
static void Init(stack **s) {
    *s = NULL;
}

/**
Funkcja sprawdzająca czy stos jest pusty.
@return czy stos jest pusty?
*/
static bool Empty(stack *s) {
    return (s == NULL);
}

///Funkcja dokładająca wielomian na stos.
static void Push(stack **s, Poly p) {
    stack *temp;
    temp = malloc(sizeof(stack));
    if (temp == NULL) exit(1);

    temp->next = *s;
    temp->v = p;
    *s = temp;
}

/**
Funkcja zdejmująca wielomian ze stosu.
@return wielomian z wierzchołka stosu
*/
static Poly Pop(stack **s) {
    stack *temp = *s; Poly p;
    p = (*s)->v;
    *s = (*s)->next;
    free(temp);
    return(p);
}

/**
Funkcja sprawdzająca wielomian z wierzchołka stosu.
@return wielomian z wierzchołka stosu
*/
static Poly Top(stack *s) {
    return (s->v);
}

/**
Funkcja sprawdzająca czy znak jest literą.
@return czy znak jest literą?
*/
static bool IsLetter(char c) {
    return ((c >= A_CHAR_CAPITAL && c <= Z_CHAR_CAPITAL) ||
           (c >= A_CHAR && c <= Z_CHAR));
}

/**
Funkcja sprawdzająca czy znak jest cyfrą.
@return czy znak jest cyrfą?
*/
static bool IsDigit(char c) {
    return (c >= ZERO_CHAR && c <= NINE_CHAR);
}

/**
Funkcja wykonująca jedno z poleceń kalkulatora: ADD / SUB / MUL
w zależności od podanego parametru (instruction).
Wykonuje operację na dwóch pierwszych wielomianach ze stosu.
Jeżeli na stosie jest za mało wielomianów wypisuje odpowiedni komunikat
na standardowe wyjście diagnostyczne.
*/
static void BinaryOperation(enum binary_operation instruction,
                     stack **polynomials, int line_number, bool null_char) {

    if (null_char) fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
    else if (!Empty(*polynomials)) {
        Poly poly1 = Pop(polynomials);
        if (!Empty(*polynomials)) {
            Poly poly2 = Pop(polynomials);

            Poly result;
            if (instruction == ADD) result = PolyAdd(&poly1, &poly2);
            else if (instruction == MUL) result = PolyMul(&poly1, &poly2);
            else if (instruction == SUB) result = PolySub(&poly1, &poly2);
            Push(polynomials, result);

            PolyDestroy(&poly1);
            PolyDestroy(&poly2);
        }
        else {
            //jeśli nie ma na stosie drugiego argumentu, odkładamy pierwszy
            Push(polynomials, poly1);
            fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
        }
    }
    else fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
}

/**
Funkcja wykonująca polecenie kalkulatora: IS_EQ.
Sprawdza czy dwa pierwsze wielomiany ze stosu są równe.
Wypisuje na wyjście 0 lub 1.
Jeżeli na stosie jest za mało wielomianów wypisuje odpowiedni komunikat
na standardowe wyjście diagnostyczne.
*/
static void IsEq(stack **polynomials, int line_number, bool null_char) {
    if (null_char) fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
    else if (!Empty(*polynomials)) {
        Poly poly1 = Pop(polynomials);
        if (!Empty(*polynomials)) {
            Poly poly2 = Top(*polynomials);

            if (PolyIsEq(&poly1, &poly2)) printf("1\n");
            else printf("0\n");

            Push(polynomials, poly1);
        }
        else {
            Push(polynomials, poly1);
            fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
        }
    }
    else fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
}

/**
Funkcja wykonująca polecenie kalkulatora: AT.
Wylicza wartość wielomianu w zadanym punkcie,
usuwa wielomian z wierzchołka i wstawia na stos wynik operacji.
Jeżeli na stosie jest za mało wielomianów wypisuje odpowiedni komunikat
na standardowe wyjście diagnostyczne.
*/
static void At(stack **polynomials, char *line, int line_number) {
    if (IsDigit(*line) || *line == MINUS) { //nie akceptujemy wiodącego plusa
        char *end;
        errno = 0;
        long n = strtol(line, &end, DECIMAL_BASE);
        if (n == LONG_MIN || n == LONG_MAX) { //sprawdzamy czy nie wyszliśmy poza zasięg long
            if (errno == ERANGE) fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
        }
        if(*end == 0) { //po wczytanym parametrze powinien być koniec linii
            if (!Empty(*polynomials)) {
                Poly p = Pop(polynomials);
                Poly p2 = PolyAt(&p, n);
                Push(polynomials, p2);
                PolyDestroy(&p);
            }
            else fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
        }
        else fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
    }
    else fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
}

/**
Funkcja wykonująca polecenie kalkulatora: DEG_BY.
Wypisuje na wyjście stopień wielomianu ze względu na zadaną zmienną.
Jeżeli na stosie jest za mało wielomianów wypisuje odpowiedni komunikat
na standardowe wyjście diagnostyczne.
*/
static void DegBy(stack **polynomials, char *line, int line_number) {
    if (!IsDigit(*line)) {
        fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
        return;
    }

    char *end;
    errno = 0;
    unsigned long n = strtoul(line, &end, DECIMAL_BASE);
    if (errno == ERANGE) {
        fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
        return;
    }
    if(*end == 0) { //po wczytanym parametrze powinien być koniec linii
        if (!Empty(*polynomials)) {
            Poly p = Top(*polynomials);
            poly_exp_t deg = PolyDegBy(&p, n);
            printf("%d\n", deg);
        }
        else fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
    }
    else fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
}

/**
Funkcja wykonująca polecenie kalkulatora: COMPOSE.
Polecenie to zdejmuje z wierzchołka stosu najpierw wielomian p,
a potem kolejno wielomiany q[k - 1], q[k - 2], …, q[0].
Umieszcza na stosie wynik operacji złożenia.
*/
static void Compose(stack **polynomials, char *line, int line_number) {
    if (!IsDigit(*line)) { //na początku musi być cyfra - liczba jest dodatnia
        fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);
        return;
    }
    char *end; errno = 0;
    unsigned long k = strtoul(line, &end, DECIMAL_BASE);
    if (errno == ERANGE) {
        fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);
        return;
    }
    if (*end == 0) { //po wczytanym parametrze powinien być koniec linii
        Poly p;
        if (!Empty(*polynomials)) p = Pop(polynomials);
        else { //nie ma na stosie żadnego wielomianu
            fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
            return;
        }

        stack *helper; //stos pomocniczy - jeśli nie wystarczy wielomianów wielomiany
        Init(&helper); //zostaną odłożone na stos we właściwej kolejności
        for (size_t i = 0; i < k; i++) {
            if (!Empty(*polynomials)) Push(&helper, Pop(polynomials));

            else { //odkładamy zdjęte wielomiany wielomiany na stos
                while(!Empty(helper)) Push(polynomials, Pop(&helper));
                Push(polynomials, p);
                free(helper);
                fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
                return; //wychodzimy z funkcji
            }
        }
        Poly *q = malloc(k * sizeof(Poly));
        if (q == NULL) exit(1);
        for (size_t i = 0; i < k; i++) q[i] = Pop(&helper);
        free(helper);

        Poly result = PolyCompose(&p, k, q);
        Push(polynomials, result);

        for (size_t i = 0; i < k; i++) PolyDestroy(&q[i]);
        free(q);
        PolyDestroy(&p);
    }
    else fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);
}

/**
* Funkcja wykonująca polecenie kalkulatora: NEG.
* Neguje wielomian z wierzchołka stosu.
*/
static void Neg(stack **polynomials) {
    Poly p = Pop(polynomials);
    Poly neg = PolyNeg(&p);
    Push(polynomials, neg);
    PolyDestroy(&p);
}

/**
* Funkcja wykonująca polecenie kalkulatora: IS_COEFF.
* Sprawdza czy wielomian z wierzchołka stosu jest współczynnikiem.
* Wypisuje na wyjście 0 lub 1.
*/
static void IsCoeff(stack **polynomials) {
    Poly p = Top(*polynomials);
    if (PolyIsCoeff(&p)) printf("1\n");
    else printf("0\n");
}

/**
* Funkcja wykonująca polecenie kalkulatora: IS_ZERO.
* Sprawdza czy wielomian z wierzchołka stosu jest tożsamościowo równy zero.
* Wypisuje na wyjście 0 lub 1.
*/
static void IsZero(stack **polynomials) {
    Poly p = Top(*polynomials);
    if (PolyIsZero(&p)) printf("1\n");
    else printf("0\n");
}

/**
* Funkcja wykonująca polecenie kalkulatora: CLONE.
* Wstawia na stos kopię wielomianu z wierzchołka stosu.
*/
static void Clone(stack **polynomials) {
    Poly p = Top(*polynomials);
    Poly clone = PolyClone(&p);
    Push(polynomials, clone);
}

/**
* Funkcja wykonująca polecenie kalkulatora: DEG.
* Wypisuje na wyjście stopień wielomianu z wierzchołka stosu.
*/
static void Deg(stack **polynomials) {
    Poly p = Top(*polynomials);
    printf("%d\n", PolyDeg(&p));
}

/**
* Funkcja wykonująca polecenie kalkulatora: PRINT.
* Wypisuje na wyjście wielomian z wierzchołka stosu.
*/
static void Print(stack **polynomials) {
    Poly p = Top(*polynomials);
    PolyPrint(&p);
    printf("\n");
}

/**
* Funkcja wykonująca polecenie kalkulatora: POP.
* Usuwa wielomian z wierzchołka stosu.
*/
static void PopIns(stack **polynomials) {
    Poly p = Pop(polynomials);
    PolyDestroy(&p);
}

/**
* Funkcja obsługująca bezparametrowe operacje kalkulatora wymagające jednego wielomianu na stosie.
* W przypadku braku wielomianu lub niepoprawnej nazwy operacji
* wypisuje na standardowe wyjście diagnostyczne odpowiedni komunikat.
*/
static void BasicOperation(void (*fun_ptr)(stack **), stack **polynomials, int line_number, bool null_char) {
    if (null_char) fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
    else if (!Empty(*polynomials)) {
        (*fun_ptr)(polynomials);
    }
    else fprintf(stderr, "ERROR %d STACK UNDERFLOW\n", line_number);
}

/**
* Funkcja obsługująca operacje kalkulatora z parametrem.
* W przypadku wykrycia braku parametru lub błędu w nazwie funkcji
* wypisuje na standardowe wyjście diagnostyczne odpowiedni komunikat.
*/
static void ParameterOperation(void (*fun_ptr)(stack **, char *, int), stack **polynomials,
                               bool null_char, char *name, char *line, int line_number) {

    int command_len = strlen(name);
    if (isspace(line[command_len])) {
        if (!(line[command_len] == SPACE) || null_char) {
            if (strcmp(name, "AT") == 0)
                fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
            else if (strcmp(name, "DEG_BY") == 0)
                fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
            else fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);
            return;
        }
        line += (command_len + 1); //pomijamy spację i nazwę instruckji

        (*fun_ptr)(polynomials, line, line_number);
    }
    else fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
}

/**
* Funkcja wykonująca polecenie kalkulatora: ZERO.
* Wstawia na stos wielomian tożsamościowo równy zero.
*/
static void Zero(stack **polynomials, int line_number, bool null_char) {
    if (null_char) fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
    else Push(polynomials, PolyZero());
}

///Funkcja zczytująca instrukcję z linii.
static char *GetInstruction(char *line) {
    char *ins = malloc((MAX_INSTRUCION_LEN + 1) * sizeof(char));
    if (ins == NULL) exit(1);

    size_t i = 0;
    while ((IsLetter(line[i]) || line[i] == UNDERSCORE) && i < MAX_INSTRUCION_LEN) {
        ins[i] = line[i];
        i++;
    }
    ins[i] = 0; //ręcznie dodajemy znak końca napisu
    return ins;
}

/**
* Funkcja wykonująca instrukcję podaną w argumencie jeśli jest ona poprawna.
* W przeciwnym wypadku wypisuje odpowiedni komunikat na standardowe wyjście diagnostyczne.
*/
static void ExecuteInstruction(int ins_len, stack **polynomials,
                        char *instruction, int line_number, bool null_char) {

    //sprawdzamy czy po instrukcji nie było zbędnych białych znaków
    if (ins_len == (int)strlen(instruction)) {

        //sprawdzamy czy którą z operacji kalkulatora powinniśmy wykonać
        if (strcmp(instruction, "IS_COEFF") == 0) BasicOperation(&IsCoeff, polynomials, line_number, null_char);
        else if (strcmp(instruction, "IS_ZERO") == 0) BasicOperation(&IsZero, polynomials, line_number, null_char);
        else if (strcmp(instruction, "CLONE") == 0) BasicOperation(&Clone, polynomials, line_number, null_char);
        else if (strcmp(instruction, "NEG") == 0) BasicOperation(&Neg, polynomials, line_number, null_char);
        else if (strcmp(instruction, "DEG") == 0) BasicOperation(&Deg, polynomials, line_number, null_char);
        else if (strcmp(instruction, "PRINT") == 0) BasicOperation(&Print, polynomials, line_number, null_char);
        else if (strcmp(instruction, "POP") == 0) BasicOperation(&PopIns, polynomials, line_number, null_char);
        else if (strcmp(instruction, "ADD") == 0) BinaryOperation(ADD, polynomials, line_number, null_char);
        else if (strcmp(instruction, "MUL") == 0) BinaryOperation(MUL, polynomials, line_number, null_char);
        else if (strcmp(instruction, "SUB") == 0) BinaryOperation(SUB, polynomials, line_number, null_char);
        else if (strcmp(instruction, "IS_EQ") == 0) IsEq(polynomials, line_number, null_char);
        else if (strcmp(instruction, "ZERO") == 0) Zero(polynomials, line_number, null_char);

        //w przypadku deg_by, compose i at skoro linia jest długości polecenia, nie podano parametru
        else if(strcmp(instruction, "DEG_BY") == 0) {
            if (null_char) fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
            else fprintf(stderr, "ERROR %d DEG BY WRONG VARIABLE\n", line_number);
        }
        else if(strcmp(instruction, "AT") == 0) {
            if (null_char) fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
            else fprintf(stderr, "ERROR %d AT WRONG VALUE\n", line_number);
        }
        else if(strcmp(instruction, "COMPOSE") == 0) {
            if (null_char) fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
            else fprintf(stderr, "ERROR %d COMPOSE WRONG PARAMETER\n", line_number);
        }
        else fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
    }
    else {
        if (*instruction == AT_COM[0] || *instruction == DEG_COM[0] || *instruction == COMPOSE_COM[0]) {
            char *ins = GetInstruction(instruction);
            if (strcmp(ins, "DEG_BY") == 0)
                ParameterOperation(&DegBy, polynomials, null_char, "DEG_BY", instruction, line_number);
            else if (strcmp(ins, "AT") == 0)
                ParameterOperation(&At, polynomials, null_char, "AT", instruction, line_number);
            else if (strcmp(ins, "COMPOSE") == 0)
                ParameterOperation(&Compose, polynomials, null_char, "COMPOSE", instruction, line_number);

            else fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
            free(ins);
        }
        else fprintf(stderr, "ERROR %d WRONG COMMAND\n", line_number);
    }
}

static Poly PolyFromString(char **s);

/**
Funkcja tworząca jednomian z napisu.
Zaczyna od znaku po nawiasie otwierającym.
Zakłada, że napis jest poprawnym jednomianem.
*/
static Mono MonoFromString(char **s) {
    Mono result;
    result.p = PolyFromString(s);
    *s += 1; //pomijamy przecienk po wczytaniu współczynnika

    //poprawność wykładnika jest zapewniona przez CorrectPoly
    char *end;
    result.exp = strtol(*s, &end, DECIMAL_BASE);
    *s = end;
    *s += 1; //na końcu jest nawias zamykający - pomijamy go

    return result;
}

/**
Funkcja tworząca wielomian z napisu.
Zakłada, że napis jest poprawnym wielomianem.
*/
static Poly PolyFromString(char **s) {
    char *end;
    //wielomian jest współczynnikiem
    if (**s == MINUS || IsDigit(**s)) {
        poly_coeff_t coeff = strtol(*s, &end, DECIMAL_BASE);
        *s = end;
        return PolyFromCoeff(coeff);
    }
    //wielomian jest sumą jendomianów
    else {
        size_t poly_size = 0; //ilość wczytanych jednomianów
        size_t array_size = INITIAL_ARR_SIZE;

        Mono *monos_array = malloc(array_size * sizeof(Mono));
        if (monos_array == NULL) exit(1);

        //wczytujemy do końca linii lub przecinka - jeśli wczytujemy współczynnik jednomianu
        while (**s != 0 && **s != COMMA) {
            *s += 1; //pomijamy nawias otwietający
            if (poly_size + 1 == array_size) {
                array_size = array_size * 2 + 1;
                monos_array = realloc(monos_array, array_size * sizeof(Mono));
                if (monos_array == NULL) exit(1);
            }
            monos_array[poly_size] = MonoFromString(s); //zaczynamy od znaku po nawiasie
            poly_size += 1;
            if (**s == PLUS) *s += 1; //pomijamy plus - przechodzimy do kolejnego jednomianu
        }
        Poly result = PolyAddMonos(poly_size, monos_array);
        free(monos_array);
        return result;
    }
}

///Funkcja sprawdzająca czy zapis wielomianu jest poprawny.
static bool CorrectPoly(char *s) {
    int depth_counter = 0;
    int state = READING_COEFF; //zmienna wyznaczająca stan zczytywania wielomaiu
    poly_coeff_t coeff;
    poly_exp_t exp;
    bool first_mono = true;
    char *end;
    while (*s != 0) {
        switch (state) {
            case READING_COEFF: //zczytujemy współczynnik - powinien być nawias lub liczba
                if (*s == OPEN_PARENTHESIS) depth_counter += 1;
                else if (*s == MINUS || IsDigit(*s)) { //sprawdzamy poprawność liczby
                    errno = 0;
                    coeff = strtol(s, &end, DECIMAL_BASE);
                    if (coeff == LONG_MIN || coeff == LONG_MAX) {
                        if (errno == ERANGE) return false;
                    }
                    s = end;
                    //po współczynniku mamy wykładnik lub koniec linii
                    if (*s == COMMA && depth_counter > 0) state = READING_EXP;
                    else if ((*s == 0) && depth_counter == 0 && first_mono) return true;
                    else return false;
                }
                else return false;
                break;
            case READING_EXP: //był przecinek - zczytujemy wykładnik
                if (!IsDigit(*s)) return false;
                exp = strtol(s, &end, DECIMAL_BASE); //sprawdzamy czy exp nie wychodzi za zakres int
                if (exp < 0 || exp > INT_MAX) return false;
                s = end;
                if (*s == CLOSE_PARENTHESIS) {
                    state = END_OF_MONO;
                    depth_counter -= 1;
                }
                else return false;
                break;
            case END_OF_MONO: //koniec jednomianu - powinien być plus lub przecinek
                if (*s == PLUS) {
                    state = READING_COEFF; //kolejny wielomian - najpierw współczynnik
                    first_mono = false;
                }
                else if (*s == COMMA && depth_counter > 0) state = READING_EXP;
                else return false;
                break;
        }
        s++;
    }
    if (state == END_OF_MONO && depth_counter == 0) return true;
    else return false;
}

///Funkcja sprawdzająca czy linia jest pusta.
static bool EmptyLine(char *line) {
    if (*line == 0) return true;
    else if (line[0] == NEWLINE) return true;
    else return false;
}

/**
Funkcja zliczająca długość pierwszego słowa
składającego się z liter i podkreślników.
@return długość pierwszego słowa w linii
*/
static int GetInstructionLen(char *line) {
    size_t i = 0;
    while (IsLetter(line[i]) || line[i] == UNDERSCORE) {
        i++;
    }
    return i;
}

/**
Funkcja przetwarzająca linię.
Dodaje poprawny wielomian na stos lub wykonuje poprawne polecenie kalkulatora.
Jeśli wykryje niepoprawny wielomian lub polecenie
wypisuje odpowiedni komunikat na standardowe wyjście diagnostyczne.
*/
static void ProcessLine(char *line, stack **polynomials, int number, bool null_char) {
    //najpierw sprawdzamy czy linia na początku zawiera niepoprawny znak \0
    if (null_char && *line == 0) fprintf(stderr, "ERROR %d WRONG POLY\n", number);
    else if (line[0] == OPEN_PARENTHESIS || IsDigit(line[0]) || line[0] == MINUS) {
        if (null_char) { //w środku znajduje się niepoprawny znak \0
            fprintf(stderr, "ERROR %d WRONG POLY\n", number);
            return;
        }
        if (CorrectPoly(line)) {
            Poly new_poly = PolyFromString(&line);
            Push(polynomials, new_poly);
        }
        else {
            fprintf(stderr, "ERROR %d WRONG POLY\n", number);
        }
    }
    else if (IsLetter(line[0])) { //litera na początku - wykonujemy instrukcje
        int instruction_len = GetInstructionLen(line);
        ExecuteInstruction(instruction_len, polynomials, line, number, null_char);
    }
    else if (line[0] == COMMENT_PREF) return;
    else if (EmptyLine(line)) return;
    else fprintf(stderr, "ERROR %d WRONG POLY\n", number);
}

/**
Funkcja main kalkulatora.
Obsługuje wejście i przetworzenie poleceń danych kalkulatorowi.
*/
int main() {
    char *current_line = NULL;
    size_t size;

    stack *polynomials; //stos wielomianów
    Init(&polynomials);

    int line_number = 1;
    bool null_char;

    int line_len = getline(&current_line, &size, stdin);
    if (current_line == NULL) exit(1);

    while (line_len > 0) {
        null_char = false;
        //zmienna pomagająca w wychwyceniu przypadku ze znakiem \0 w środku linii
        if (line_len != (int)strlen(current_line)) {
            null_char = true;
        }
        //dla ułatwienia dodajemy \0 na koniec linii
        if (strlen(current_line) > 0 &&
            current_line[strlen(current_line) - 1] == NEWLINE) {

            current_line[strlen(current_line) - 1] = 0;
        }
        ProcessLine(current_line, &polynomials, line_number, null_char);
        line_number++;
        line_len = getline(&current_line, &size, stdin);
    }
    free(current_line);

    Poly p; //zwalniamy pozostałe na stosie wielomiany
    while (!Empty(polynomials)) {
        p = Pop(&polynomials);
        PolyDestroy(&p);
    }
    free(polynomials);
    return 0;
}
