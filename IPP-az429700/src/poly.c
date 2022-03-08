/** @file
  Implementacja podstawowych funkcji na wielomianach rzadkich wielu zmiennych

  @author Agata Załęska
  @date 2021
*/

#include "poly.h"
#include <stdlib.h>
#include <stdio.h>

#define INITIAL_ARR_SIZE 5 ///< początkowa dlugość tablicy zmiennej długości

/*
* Funkcja usuwa wielomian z pamięci.
* Zwalnia elementy jego tablicy jednomianów (oraz tablicę).
*/
void PolyDestroy(Poly *p) {
    assert(p != NULL);

    if (p->arr != NULL) { // zwalniamy tablice i jej zawartosc
        for (size_t i = 0; i < p->size; i++) {
            MonoDestroy(&(p->arr[i]));
        }
        free(p->arr);
    }
}

//drukuje wielomian - po kolei drukuje kolejne jednomiany
void PolyPrint(const Poly *p) {
    if (PolyIsCoeff(p)) {
        printf("%ld", p->coeff);
    }
    else {
        for (size_t i = 0; i < p->size; i++) {
            printf("(");
            PolyPrint(&p->arr[i].p);
            printf(",");
            printf("%d", p->arr[i].exp);
            printf(")");
            if (i != p->size - 1) printf("+");
        }
    }
}

/*
* Funkcja kopiuje wielomian.
* Jeśli wielomian nie jest współczynnikiem, powiela jego tablicę jednomianów.
*/
Poly PolyClone(const Poly *p) {
    assert(p != NULL);

    Poly clone;/*Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {

}*/
    if (p->arr == NULL) {
        clone.arr = NULL;
        clone.coeff = p->coeff;
    }
    else { //kopiujemy tablicę i jej zawartość
        clone.size = p->size;
        clone.arr = malloc(clone.size * sizeof(Mono));
        if (clone.arr == NULL) exit(1);

        for (size_t i = 0; i < clone.size; i++) {
            clone.arr[i] = MonoClone(&(p->arr[i]));
        }
    }
    return clone;
}

/**
* Funkcja pomocnicza do AddArrays.
* Dopelnia takblice result elementami z p_arr o indexach j: i >= j > p_size
*/
static void FillRemainingMonos(Mono *p_arr, size_t p_size, size_t i, Mono **result,
                        size_t arr_size, int arr_index, size_t *result_size) {
    while (i < p_size) {
        if (*result_size == arr_size) {
            arr_size = arr_size * 2 + 1;
            *result = realloc(*result, arr_size * sizeof(Mono));
            if (*result == NULL) exit(1);
        }

        Poly clone = PolyClone(&(p_arr[i].p));
        (*result)[arr_index].exp = p_arr[i].exp;
        (*result)[arr_index].p = clone;
        i++;
        arr_index++;
        *result_size += 1;
    }
}

/**
* Funkcja dodaje tablice jednomianow (Mono) p_arr oraz q_arr.
* Zaklada, ze tablice sa posortowane wg wykladnikow jednomianow.
* @return tablica jednomianów (p_arr + q_arr)
*/
static Mono *AddArrays(Mono *p_arr, size_t p_size, Mono *q_arr,
               size_t q_size, size_t *result_size) {

    size_t arr_size = INITIAL_ARR_SIZE;
    Mono *result = malloc(arr_size * sizeof(Mono));
    if (result == NULL) exit(1);

    size_t i = 0, j = 0, arr_index = 0;
    while (i < p_size && j < q_size) {
        if (*result_size == arr_size) { //powiekszamy tablice
            arr_size = arr_size * 2 + 1;
            result = realloc(result, arr_size * sizeof(Mono));
            if (result == NULL) exit(1);
        }
        //jednomiany o tym samym wykladniku chcemy zsumowac
        //i wartosc tej sumy dodac do wynikowej tablicy
        if (p_arr[i].exp == q_arr[j].exp) {
            Poly p_neg = PolyNeg(&(p_arr[i].p));
            if (!PolyIsEq(&p_neg, &(q_arr[j].p))) { //nie chcemy otrzymac wspoczynnika = 0
                result[arr_index].exp = p_arr[i].exp;
                result[arr_index].p = PolyAdd(&(p_arr[i].p), &(q_arr[j].p)); //dodajemy wspolczynniki
                *result_size += 1;
                arr_index++;
            }
            PolyDestroy(&p_neg); //usuwamy wielomian pomocniczy
            j++;
            i++;
        }
        else {
            //dodajemy jednomian o mniejszym wykładniku do wynikowej tablicy
            if (p_arr[i].exp < q_arr[j].exp) {
                Poly clone = PolyClone(&(p_arr[i].p));
                result[arr_index].exp = p_arr[i].exp;
                result[arr_index].p = clone;
                i++;
            }
            else {
                Poly clone = PolyClone(&(q_arr[j].p));
                result[arr_index].exp = q_arr[j].exp;
                result[arr_index].p = clone;
                j++;
            }
            *result_size += 1;
            arr_index++;
        }
    }
    //uzupełniamy wynik o pozostałę elementy tablicy q_array lub p_array
    FillRemainingMonos(p_arr, p_size, i, &result, arr_size, arr_index, result_size);
    FillRemainingMonos(q_arr, q_size, j, &result, arr_size, arr_index, result_size);

    if (*result_size == 0) {
        free(result);
        return NULL;
    }
    return result;
}

/**
* Funkcja dodaje wspolczynnik (p->coeff) do tablicy jednomianow (array).
* @return tablica jednomianów (array + p->coeff)
*/
static Mono *AddCoeffToArray(const Poly *p, Mono *array,
                             size_t arr_size, size_t *result_size) {

    poly_coeff_t coeff = p->coeff;
    Mono *result;
    //osobno rozważamy przypadki: - w tablicy mamy jednomian o exp = 0
    if (array[0].exp == 0) {
        //zmienna pomocnicza w przypadku, gdy pierwszy wyraz tablicy się zeruje
        size_t begin;
        result = malloc(arr_size * sizeof(Mono));
        if (result == NULL) exit(1);

        //bedziemy uzupełniać tablicę od indexu 0
        if (array[0].p.arr == NULL && array[0].p.coeff == -coeff) begin = 0;
        else { //ręcznie uzupełniamy pierwszy element; w pętli zaczynamy od indexu 1
            result[0].exp = 0;
            result[0].p = PolyAdd(p, &(array[0].p));
            begin = 1;
        }

        //jeśli begin = 0, jesteśmy przesunięci o 1 w przechodzeniu po tablicy
        //tzn pomijamy 1 element tablicy array (zredukował się ze współczynnikiem)
        //jeśli begin = 1, pierwszy element tablicy wynikowej jest już uzupełniony
        for (size_t i = begin; i < arr_size - (1 - begin); i++) {
            result[i].exp = array[i + (1 - begin)].exp;
            result[i].p = PolyClone(&(array[i + (1 - begin)].p));
        }
        //jesli begin = 0, rozmiar jest o 1 krótszy (1 element się wyzerował)
        *result_size = arr_size - (1 - begin);
    }
    else { //w tablicy nie ma jednomianu o exp = 0
        //dodajemy na niego miejsce w pamięci
        result = malloc((arr_size + 1) * sizeof(Mono));
        if (result == NULL) exit(1);

        result[0].exp = 0;
        result[0].p.arr = NULL;
        result[0].p.coeff = coeff;

        //resztę tablicy uzupełniamy bez zmian
        for (size_t i = 0; i < arr_size; i++) {
            result[i+1].exp = array[i].exp;
            result[i+1].p = PolyClone(&(array[i].p));
        }
        *result_size = arr_size + 1;
    }
    return result;
}

/*
 * Funkcja rozważa przypadki ze względu na to, które tablice jednomianów są = null.
 * Dodaje współczynniki, gdy obie tablice są nullami,
 * wywołuje AddCoeffToArray w przypadku gdy jedna z tablic jest nullem,
 * AddArrays gdy obie tablice nie są nullami.
*/
Poly PolyAdd(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);

    Poly result;
    size_t result_size = 0;

    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        result.arr = NULL;
        result.coeff = p->coeff + q->coeff;
        return result;
    }
    else if (PolyIsCoeff(p) && !PolyIsCoeff(q)) {
        if (p->coeff == 0) return PolyClone(q);
        else {
            result.arr = AddCoeffToArray(p, q->arr, q->size, &result_size);
        }
    }
    else if (!PolyIsCoeff(p) && PolyIsCoeff(q)) {
        if (q->coeff == 0) return PolyClone(p);
        else {
            result.arr = AddCoeffToArray(q, p->arr, p->size, &result_size);
        }
    }
    else { //wielomiany nie są współczynnikami
        result.arr = AddArrays(p->arr, p->size, q->arr, q->size, &result_size);
    }

    if (result.arr == NULL) return PolyFromCoeff(0); //otrzymaliśmy pustą tablicę
    //sprawdzamy warunki wskazujące, że wynik jest współczynnikiem

    if (result_size == 1 && result.arr[0].p.arr == NULL && result.arr[0].exp == 0) {
        poly_coeff_t coeff = result.arr[0].p.coeff;
        free(result.arr);
        return PolyFromCoeff(coeff);
    }
    else result.size = result_size;

    return result;
}

/*
* Podnosi p do potęgi exp.
* Korzysta z algorytmu szybkiego potęgowania
* oraz mnożenia wielomianów
*/
Poly PolyPower(const Poly *p, poly_exp_t exp) {
    Poly poly = PolyClone(p);
    assert(exp >= 0);

    Poly result = PolyFromCoeff(1);
    Poly new_result, new_p;
    while (exp > 0) {
        if (exp % 2 == 1) {
            new_result = PolyMul(&result, &poly);
            PolyDestroy(&result);
            result = new_result;
        }

        new_p = PolyMul(&poly, &poly);
        PolyDestroy(&poly);
        poly = new_p;
        exp /= 2;
    }
    PolyDestroy(&poly);
    return result;
}

/**
* Funkcja pomocnicza do PolyCompose.
* Wstawia 0 za wszytkie zmienne wielomianu.
* Po zastąpieniu każdej zmiennej zerem, otrzymamy liczbę.
*/
static poly_coeff_t PolyZeroForAllVariables(const Poly *p) {
    if (PolyIsCoeff(p)) return p->coeff;

    poly_coeff_t result = 0;
    for (size_t i = 0; i < p->size; i++) {
        if (p->arr[i].exp == 0) { //wpp mnożymy współczynnik przez 0
            result += PolyZeroForAllVariables(&p->arr[i].p);
        }
    }
    return result;
}

/*
* Funkcja wykonująca operacja złożenia wielomianu.
* Mnoży dla każdego jednomianu z tablicy wielomianu p
* Podstawia odpowiedni wielomian pod wykładnik i mnoży go
* przez złożony współczynnik.
*/
Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {
    if (PolyIsCoeff(p)) return PolyFromCoeff(p->coeff);

    else {
        //tworzymy wielomian zerowy, do którego będziemy dodawać kolejne wielomiany
        Poly result = PolyFromCoeff(0);
        Poly new_poly, composed_exp, composed_coeff, new_result;
        if (k > 0) {
            for (size_t i = 0; i < p->size; i++) {
                composed_exp = PolyPower(q, p->arr[i].exp);
                composed_coeff = PolyCompose(&(p->arr[i].p), k - 1, q + 1);

                new_poly = PolyMul(&composed_exp, &composed_coeff);
                new_result = PolyAdd(&result, &new_poly);

                PolyDestroy(&new_poly);
                PolyDestroy(&result);
                PolyDestroy(&composed_coeff);
                PolyDestroy(&composed_exp);

                result = new_result;
            }
            return result;
        }
        else { //wstawiamy zero za każdą zmienną
            poly_coeff_t coeff = PolyZeroForAllVariables(p);
            return PolyFromCoeff(coeff);
        }
    }
}

/**
* Funkcja porównująca jednomiany w formacie odpowiednim dla qsorta.
* Porównuje jednomiany według wykładników.
*/
static int MonoCompareByExp(const void *m1, const void *m2) {
    Mono mono1 = *((Mono *)m1);
    Mono mono2 = *((Mono *)m2);

    if (mono1.exp == mono2.exp ) return 0;
    else if (mono1.exp < mono2.exp) return -1;
    else return 1;
}

///Funckja usuwa jednomiany zerowe (o wspolczynniku = 0) z tablicy wielomianu
static void RemoveZeros(Poly *p) {
    /*licznik znalezionych zer - każdy element w tablicy będziemy przesuwać
      o ilość zer go poprzedzających*/
    size_t zeros = 0;

    for (size_t i = 0; i < p->size; i++) {
        if (PolyIsZero(&p->arr[i].p)) {
            zeros++;
        }
        else if (zeros > 0) { //przesuwamy element i-ty o ilość znalezionych zer
            p->arr[i - zeros] = p->arr[i];
        }
    }
    p->size -= zeros;
    if (p->size == 0) { //wszystkie elementy były zerowe
        free(p->arr);
        p->arr = NULL;
        p->coeff = 0;
    }
}

/**
* Funkcja tworząca wielomian z posortowanej tablicy jednomianów.
* Dodaje do wynikowej tablicy jednomiany
* będące sumą jednomianów o tym sammym wykładniku w wejściowej tablicy.
* Usuwa ewentualne zera powstałe przy dodawaniu współczynników jednomianów.
*/
Poly PolyFromSortedMonoArray(size_t count, Mono monos[]) {
    Poly result;
    size_t i = 0, poly_size = 1, arr_index = 0; //ustawiamy rozmiar poczatkowy wyniku 1
    size_t arr_size = INITIAL_ARR_SIZE;
    result.arr = malloc(arr_size * sizeof(Mono));
    if (result.arr == NULL) exit(1);

    result.arr[i++] = monos[0]; //ręcznie ustawiamy pierwszy element tablicy
    poly_exp_t curr_exp = monos[0].exp; //zmienna pomocnicza - obecny wykładnik

    while (i < count) {
        if (arr_index + 1 == arr_size) {
            arr_size = arr_size * 2 + 1;
            result.arr = realloc(result.arr, arr_size * sizeof(Mono));
            if (result.arr == NULL) exit(1);
        }
        if (monos[i].exp == curr_exp) { //wykładnik się nie zmienił - dodajemy współczynniki
            Poly new_poly = PolyAdd(&(result.arr[arr_index].p),
                                              &(monos[i].p));

            PolyDestroy(&result.arr[arr_index].p);
            PolyDestroy(&monos[i].p); //usuwamy niepotrzebne wielomiany
            result.arr[arr_index].p = new_poly;
        }
        else {
            arr_index++; //nowy wykładnik - przechodzimy do kolejnego elementu tablicy wynikowej
            result.arr[arr_index] = monos[i];
            poly_size++;
            curr_exp = monos[i].exp; //aktualizujemy wykładnik
        }
        i++;
    }
    free(monos);
    result.size = poly_size;

    RemoveZeros(&result); //usuwamy jendomiany o zerowych współczynnikach
    if (result.size == 1) { //ani razu nie zwiększył się rozmiar wyniku
        if (PolyIsZero(&result.arr[0].p)) {
            PolyDestroy(&result);
            return PolyZero();
        } //jedyny współczynnik w tablicy był zerowy
        if (PolyIsCoeff(&result.arr[0].p) && result.arr[0].exp == 0) {
            poly_coeff_t coeff = result.arr[0].p.coeff;
            PolyDestroy(&result);
            return PolyFromCoeff(coeff);
        } //wyłapujemy przypadek x_0^0 * coeff = coeff
    }
    return result;
}

/*
* Tworzy nową tablicę jednomianów z głębokimi kopiami.
* Następnie wywołuje PolyFromSortedMonoArray
*/
Poly PolyCloneMonos(size_t count, const Mono monos[]) {
    if (count == 0) return PolyFromCoeff(0);

    Mono *new_monos = malloc(count * sizeof(Mono));
    if (new_monos == NULL) exit(1);
    for (size_t i = 0; i < count; i++) {
        new_monos[i] = MonoClone(&monos[i]);
    }
    qsort(new_monos, count, sizeof(Mono), MonoCompareByExp);

    return PolyFromSortedMonoArray(count, new_monos);
}

/*
* Sprawdza warunek brzegowy.
* Następnie wywołuje PolyFromSortedMonoArray
*/
Poly PolyOwnMonos(size_t count, Mono monos[]) {
    if (count == 0) {
        free(monos);
        return PolyFromCoeff(0);
    }
    qsort(monos, count, sizeof(Mono), MonoCompareByExp);

    return PolyFromSortedMonoArray(count, monos);
}


/*
* Tworzy nową tablicę jednomianów z zawartością tablicy monos.
* Następnie wywołuje PolyFromSortedMonoArray
*/
Poly PolyAddMonos(size_t count, const Mono monos[]) {
    if (count == 0) return PolyFromCoeff(0);

    Mono *new_monos = malloc(count * sizeof(Mono));
    if (new_monos == NULL) exit(1);

    for (size_t i = 0; i < count; i++) {
        new_monos[i] = monos[i];
    }
    qsort(new_monos, count, sizeof(Mono), MonoCompareByExp);

    return PolyFromSortedMonoArray(count, new_monos);
}


/**
* Funkcja mnoży tablice jednomianow (array) przez dany wspolczynnik (coeff).
* @return tablica jednomianów (array * coeff)
*/
static Mono *MulArrayByCoeff(Mono *array, size_t arr_size,
                             poly_coeff_t coeff, size_t *result_size) {

    Mono *new_array = malloc(arr_size * sizeof(Mono));
    if (new_array == NULL) exit(1);

    int arr_index = 0;
    for (size_t i = 0; i < arr_size; i++) {
        if (array[i].p.arr == NULL) { //przypadek coeff * coeff
            if (array[i].p.coeff * coeff != 0) { //zerowych elementów nie dodajemy
                new_array[arr_index].p.arr = NULL;
                new_array[arr_index].p.coeff = array[i].p.coeff * coeff;
                new_array[arr_index].exp = array[i].exp;
                arr_index++;
                *result_size += 1;
            }
        }
        else { //przypadek arr * coeff - rekurencyjne wywołanie
            size_t new_size = 0; //aktualizujemy rozmiar w razie występujących zer
            new_array[arr_index].p.arr = MulArrayByCoeff(array[i].p.arr,
                                                         array[i].p.size, coeff, &new_size);
            new_array[arr_index].p.size = new_size;
            new_array[arr_index].exp = array[i].exp;
            arr_index++;
            *result_size += 1;
        }
    }
    if (arr_index == 0) { //żaden element nie został dodany
        free(new_array);
        return NULL;
    }
    return new_array;
}

/*
 * Funkcja rozważa przypadki ze względu na to, które tablice jednomianów są = null.
 * Mnoży współczynniki, gdy obie tablice są nullami,
 * wywołuje MulArrayByCoeff w przypadku gdy jedna z tablic jest nullem,
 * Mnoży tablice z wykorzystaniem PolyAddMonos gdy obie tablice nie są nullami.
*/
Poly PolyMul(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);

    if (PolyIsZero(p) || PolyIsZero(q)) {
        return PolyFromCoeff(0);
    }
    Poly result;

    if (PolyIsCoeff(p) && PolyIsCoeff(q)) { //wystarczy wymnożyć współczynniki
        result.arr = NULL;
        result.coeff = p->coeff * q->coeff;
        return result;
    }
    else if (PolyIsCoeff(p) && !PolyIsCoeff(q)) {
        size_t result_size = 0;
        Mono *arr = MulArrayByCoeff(q->arr, q->size, p->coeff, &result_size);
        if (arr == NULL) return PolyFromCoeff(0); //otrzymaliśmy pustą tablicę
        result.arr = arr;
        result.size = result_size;
    }
    else if (!PolyIsCoeff(p) && PolyIsCoeff(q)) { //przypadek analogiczny do poprzedniego
        size_t result_size = 0;
        Mono *arr = MulArrayByCoeff(p->arr, p->size, q->coeff, &result_size);
        if (arr == NULL) return PolyFromCoeff(0);
        result.arr = arr;
        result.size = result_size;
    }
    else { //żaden wielomian nie jest współczynnikiem
        Mono *temp = malloc(p->size * q->size * sizeof(Mono));
        if (temp == NULL) exit(1);

        Poly mul; //zmienna przechowująca wynik mnożenia współczynników jednomianów
        int temp_i = 0; //tablica przechowująca jednomiany - wyniki wymnażania
        //w pętli wymnażamy po kolei wszystkie współczynniki między sobą
        for (size_t i = 0; i < p->size; i++) {
            for (size_t j = 0; j < q->size; j++) {
                mul = PolyMul(&(p->arr[i].p), &(q->arr[j].p));
                if (!PolyIsZero(&mul)) { //nie dodajemy jednomianów o zerowym współczynniku
                    temp[temp_i].exp = p->arr[i].exp + q->arr[j].exp;
                    temp[temp_i].p = mul;
                    temp_i++;
                }
            }
        }
        result = PolyAddMonos(temp_i, temp); //sumujemy wyniki wymnażania
        free(temp);
    }

    return result;
}


//Funkcja uzyskuje wielomian odwrotny wykorzystując własność: -p = p * (-1).
Poly PolyNeg(const Poly *p) {
    assert(p != NULL);

    Poly dummy = PolyFromCoeff(-1);
    return PolyMul(p, &(dummy));
}


//Funkcja odejmuje wielomiany wykorzystując własność: p - q = p + (-q).
Poly PolySub(const Poly *p, const Poly *q) {
    assert(p != NULL);

    Poly q_neg = PolyNeg(q);
    Poly result = PolyAdd(p, &q_neg);
    PolyDestroy(&q_neg);
    return result;
}

/*
* Funkcja rekurencyjnie wylicza stopień wielomianu wg danej zmiennej.
* Korzysta z założenia, że tablica jednomianów jest posortowana wg wykładników.
*/
poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {
    assert(p != NULL);

    if (PolyIsZero(p)) return -1;
    else if (p->arr == NULL) return 0;

    //tablicca jest posortowana - najwyższy stopień ma ostatni element tablicy
    if (var_idx == 0) return (p->arr[p->size - 1].exp);
    else {
        poly_exp_t max_exp = 0; //pętla szuka (max stopnia współczynników jednomianów)
        poly_exp_t current_exp; //bieżący współczynnik
        for (size_t i = 0; i < p->size; i++) {
            //gdy var_idx w rekurencyjnym wywołaniu osiągnie 0, wiemy, że
            //badany jest stopień właściwej zmiennej
            current_exp = PolyDegBy(&(p->arr[i].p), var_idx - 1);
            if (current_exp > max_exp) max_exp = current_exp;
        }
        return max_exp;
    }
}

/*
* Funkcja rekurencyjnie bada stopień każdego jednomianu.
* Mkasymalny stopień jednomianu to szukany wynik - stopień wielomianu.
*/
poly_exp_t PolyDeg(const Poly *p) {
    assert(p != NULL);

    if (PolyIsZero(p)) return -1;
    else if (p->arr == NULL) return 0;

    else {
        poly_exp_t max_deg = 0;
        poly_exp_t current_deg = 0;
        for (size_t i = 0; i < p->size; i++) {
            //stopień jednomianu = stopień wykładnika + stopień współczynnika
            current_deg = p->arr[i].exp + PolyDeg(&(p->arr[i].p));
            if (current_deg > max_deg) max_deg = current_deg;
        }
        return max_deg;
    }
}

/*
* Funkcja sprawdza równość wielomianów p i q.
* W tym celu porównuje tablice i rozmiary/współczynniki p i q.
*/
bool PolyIsEq(const Poly *p, const Poly *q) {
    assert(p != NULL && q != NULL);

    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return (p->coeff == q->coeff); //sprawdzamy tylko współczynniki
    }
    else if (!PolyIsCoeff(p) && !(PolyIsCoeff(q))) {
        if (p->size != q->size) return false; //tablice różnią się rozmiarem, p != q

        //gdy chociaż jeden jednomian się nie zgadza zwracamy false
        for (size_t i = 0; i < p->size; i++) { //p->size == q->size
            if (q->arr[i].exp != p->arr[i].exp) {
                return false;
            }
            if (!PolyIsEq(&(p->arr[i].p), &(q->arr[i].p))) {
                return false;
            }
        }
        return true;
    }
    else return false; //jeden z wielomianów jest współczynnikiem; drugi nie.
}

/**
* Funkcja podnosi x do potegi exp.
* Korzysta z algorytmu szybkiego potęgowania.
* @return x^(exp)
*/
static poly_coeff_t Power(poly_coeff_t x, poly_exp_t exp) {
    assert(exp >= 0);

    poly_coeff_t result = 1;
    while (exp > 0) {
        if (exp % 2 == 1) result *= x;

        x *= x;
        exp /= 2;
    }
    return result;
}

/*
* Funkcja szuka wartości wielomianu p w punkcie x.
* Oblicza tą wartość dla każdego jednomianu i dodaje do wynikowej sumy.
*/
Poly PolyAt(const Poly *p, poly_coeff_t x) {
    assert(p != NULL);

    if (PolyIsCoeff(p)) { //wielomian jest współczynnikiem - x nie wpływa na wartość
        return PolyFromCoeff(p->coeff);
    }
    else {
        if (x == 0) {
            if (p->arr[0].exp == 0) return PolyClone(&p->arr[0].p); //x nie wpływa na wartość
            else return PolyZero(); //wykładniki są > 0 - wszystkie jednomiany się zerują
        }

        Poly power_poly, new_poly, new_result; //wielomiany pomocnicze

        poly_exp_t exp = p->arr[0].exp;
        poly_coeff_t power = Power(x, exp);
        power_poly = PolyFromCoeff(power);

        Poly result = PolyMul(&p->arr[0].p, &power_poly);

        for (size_t i = 1; i < p->size; i++) {
            exp = p->arr[i].exp;
            power = Power(x, exp);

            power_poly = PolyFromCoeff(power);
            //mnożymy współczynnik przez obliczoną potęgę x_0
            new_poly = PolyMul(&p->arr[i].p, &power_poly);
            //wynik mnożenia dodajemy do wielomianu wynikowego
            new_result = PolyAdd(&result, &new_poly);

            PolyDestroy(&result); //zwalniamy wielomiany pomocnicze
            PolyDestroy(&new_poly);
            result = new_result; //poprzedni wynik podmieniamy na uzyskaną sumą
        }
        return result;
    }
}
