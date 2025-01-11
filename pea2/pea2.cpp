#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <chrono>
#include <limits>
#include <cmath>
#include <sstream>
#include <random>
#include <cmath>
#include <functional>
#include <numeric>
#include <deque>
#include <utility>
using namespace std;
using namespace std::chrono;

int liczba_miast;
vector<int> najlepsza_trasa_z;
int najlepszy_koszt_z;
vector<int> najlepsza_trasa_ts;
int najlepszy_koszt_ts;
vector<int> najlepsza_trasa_sw;
int najlepszy_koszt_sw;
int wielkosc_populacji_poczatkowej;
double wspolczynnik_mutacji;
double wspolczynnik_krzyzowania;
vector<vector<int>> populacja;
int rozmiar_turnieju = 4;
vector<int > oceny; // Wektor z kosztami wszystkich aktualnych permutacji algorytmu genetycznego

vector<vector<int>> wczytywanie_macierzy(const string& nazwa_pliku) {

	ifstream plik(nazwa_pliku);
	string linia;
	vector<vector<int>>macierz_kosztow;

	if (!plik.is_open()) {
		cout << "Blad otwierania pliku!" << endl;
		return {};
	}
	
	while (getline(plik, linia)) {
		if (linia.find("DIMENSION:") != string::npos) {
			istringstream iss(linia);
			string temp;
			iss >> temp >> liczba_miast;
		}
		else if (linia.find("EDGE_WEIGHT_SECTION") != string::npos) {
			macierz_kosztow.resize(liczba_miast, vector<int>(liczba_miast));
			int i =0, j = 0;
			while (i < liczba_miast && getline(plik, linia)) {
				istringstream iss(linia);
				int value;
				while (iss >> value) {
					macierz_kosztow[i][j] = value;
					j++;
					if (j == liczba_miast) { // Jeśli dotarliśmy do końca wiersza
						j = 0;
						i++;
					}
				}
			}
			break;
		}
	}
	plik.close();
	return macierz_kosztow;
}

void wypisanie_macierzy(const vector<vector<int>>& macierz_kosztow) {
	for (int i = 0; i < liczba_miast; i++) {
		for (int j = 0; j < liczba_miast; j++) {
			cout << macierz_kosztow[i][j] << ' ';
		}
		cout << endl;
	}
}

void rozwiazanie_zachlanne(const vector<vector<int>>& macierz_kosztow) {
	najlepszy_koszt_z = numeric_limits<int>::max(); // Najlepszy koszt

	// Iterujemy przez każde miasto jako punkt początkowy
	for (int startowe_miasto = 0; startowe_miasto < liczba_miast; ++startowe_miasto) {
		vector<bool> odwiedzone(liczba_miast, false);
		vector<int> trasa_lokalna;  // Lokalne rozwiązanie
		int koszt_lokalny = 0;

		// Startujemy z danego miasta
		int obecne_miasto = startowe_miasto;
		odwiedzone[obecne_miasto] = true;
		trasa_lokalna.push_back(obecne_miasto);

		// Algorytm zachłanny dla tego startowego miasta
		for (int i = 0; i < liczba_miast - 1; ++i) {
			int najlepszy_koszt_lokalny = numeric_limits<int>::max();
			int nastepne_miasto = -1;

			// Znajdujemy najbliższe nieodwiedzone miasto
			for (int j = 0; j < liczba_miast; ++j) {
				if (!odwiedzone[j] && macierz_kosztow[obecne_miasto][j] < najlepszy_koszt_lokalny) {
					najlepszy_koszt_lokalny = macierz_kosztow[obecne_miasto][j];
					nastepne_miasto = j;
				}
			}

			// Przejście do najbliższego miasta
			if (nastepne_miasto != -1) {
				koszt_lokalny += najlepszy_koszt_lokalny;
				obecne_miasto = nastepne_miasto;
				odwiedzone[obecne_miasto] = true;
				trasa_lokalna.push_back(obecne_miasto);
			}
		}

		// Powrót do miasta początkowego
		koszt_lokalny += macierz_kosztow[obecne_miasto][startowe_miasto];
		trasa_lokalna.push_back(startowe_miasto);

		// Aktualizacja globalnie najlepszego rozwiązania
		if (koszt_lokalny < najlepszy_koszt_z) {
			najlepszy_koszt_z = koszt_lokalny;
			najlepsza_trasa_z = trasa_lokalna;
		}
	}

	// Wypisanie wyników
	cout << "Znalezione rozwiazanie zachlanne: ";
	for (int miasto : najlepsza_trasa_z) {
		cout << miasto << " ";
	}
	cout << endl;
	cout << "Calkowity koszt rozwiazania: " << najlepszy_koszt_z << endl;
}

// Funkcja kosztu (np. całkowita długość trasy)
int oblicz_koszt(const vector<int>& trasa, const vector<vector<int>>& macierz_kosztow) {
	int koszt = 0;
	for (size_t i = 0; i < trasa.size() - 1; ++i) {
		koszt += macierz_kosztow[trasa[i]][trasa[i + 1]];
	}
	koszt += macierz_kosztow[trasa.back()][trasa.front()]; // Powrót do punktu początkowego
	return koszt;
}

// Sąsiedztwo: zamiana dwóch miast miejscami
vector<int> generuj_sasiedztwo(const vector<int>& trasa) {
	vector<int> nowa_trasa = trasa;
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> dist(0, trasa.size() - 1); // Mieszamy wszystkie indeksy, w tym startowy

	int i = dist(gen);
	int j = dist(gen);
	while (i == j) {
		j = dist(gen); // Unikamy identycznych indeksów
	}

	swap(nowa_trasa[i], nowa_trasa[j]);
	return nowa_trasa;
}

// Oblicz temperaturę początkową (np. na podstawie różnic w kosztach sąsiednich rozwiązań)
double oblicz_temperatura_poczatkowa(const vector<int>& trasa, const vector<vector<int>>& macierz_kosztow) {
	double suma_roznic = 0.0;
	int liczba_prob = 100;
	random_device rd;
	mt19937 gen(rd());

	for (int i = 0; i < liczba_prob; ++i) {
		vector<int> sasiedztwo = generuj_sasiedztwo(trasa);
		int koszt1 = oblicz_koszt(trasa, macierz_kosztow);
		int koszt2 = oblicz_koszt(sasiedztwo, macierz_kosztow);
		suma_roznic += abs(koszt2 - koszt1);
	}

	return suma_roznic / liczba_prob;
}

// Algorytm Symulowanego Wyżarzania
void symulowane_wyzarzanie(const vector<vector<int>>& macierz_kosztow, double wspolczynnik_a, int czas_w_sekundach) {
	najlepsza_trasa_sw.clear();
	najlepszy_koszt_sw = 0;
	double czas_znalezienia = 0.0;
	// Inicjalizacja trasy początkowej
	vector<int> obecna_trasa(liczba_miast);
	iota(obecna_trasa.begin(), obecna_trasa.end(), 0); // iota wypełnia wektor obecna_trasa kolejnymi liczbami zaczynając od 0

	random_device rd;
	mt19937 gen(rd());
	shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen); // losowe wymieszanie wartości wektora obecna_trasa

	int obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow); // liczy koszt początkowej trasy na podstawie macierzy kosztów
	najlepsza_trasa_sw = obecna_trasa; // zapisuje początkową trasę jako najlepszą
	najlepszy_koszt_sw = obecny_koszt; // i początkowy koszt jako najlepszy

	double temperatura = oblicz_temperatura_poczatkowa(obecna_trasa, macierz_kosztow); // liczy temperaturę początkową

	// Rozpoczęcie pomiaru czasu
	auto start_sw = chrono::steady_clock::now();

	// Degeneracja
	int iter_count = 0;
	int limit_iteracji_bez_poprawy = 1000000; // Ustalona liczba iteracji bez poprawy

	while (true) {
		// Sprawdzenie, czy przekroczono czas w sekundach
		auto teraz_sw = chrono::steady_clock::now();
		double czas_uplyniety = chrono::duration_cast<chrono::seconds>(teraz_sw - start_sw).count();
		if (czas_uplyniety >= czas_w_sekundach) {
			break; // Przerwij, jeśli czas został przekroczony
		}

		// Generowanie sąsiedztwa
		vector<int> nowa_trasa = generuj_sasiedztwo(obecna_trasa); // generuje nową trasę w sąsiedztwie poprzedniej
		int nowy_koszt = oblicz_koszt(nowa_trasa, macierz_kosztow);

		// Decyzja o zaakceptowaniu nowej trasy
		if (nowy_koszt < obecny_koszt || exp((obecny_koszt - nowy_koszt) / temperatura) > uniform_real_distribution<>(0, 1)(gen)) {
			obecna_trasa = nowa_trasa; // Aktualizacja trasy
			obecny_koszt = nowy_koszt;

			// Aktualizacja najlepszego rozwiązania
			if (nowy_koszt < najlepszy_koszt_sw) {
				najlepsza_trasa_sw = nowa_trasa;
				najlepszy_koszt_sw = nowy_koszt;
				iter_count = 0; // Resetuj licznik iteracji bez poprawy
				czas_znalezienia = chrono::duration_cast<chrono::seconds>(teraz_sw - start_sw).count();
			}
		}
		else {
			iter_count++;
		}

		// Schładzanie temperatury
		temperatura *= wspolczynnik_a; // Zmniejszenie temperatury
		if (temperatura < 1e-5) break; // Zatrzymanie algorytmu, gdy temperatura jest bardzo niska

		// Degeneracja: Losowe restartowanie trasy po określonej liczbie iteracji bez poprawy
		if (iter_count >= limit_iteracji_bez_poprawy) {
			shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen); // Losowe wymieszanie trasy
			obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow); // Oblicz koszt nowej trasy
			iter_count = 0; // Resetuj licznik iteracji bez poprawy
		}
	}

	cout << "Najlepsza znaleziona trasa: ";
	for (int miasto : najlepsza_trasa_sw) {
		cout << miasto << " ";
	}
	cout << endl;
	cout << "Calkowity koszt: " << najlepszy_koszt_sw << endl;
	cout << "Temperatura koncowa: " << temperatura << endl;
	cout << "Czas znalezienia najlepszego rozwiazania: " << czas_znalezienia << "s\n" << endl;
}


// Funkcja do zamiany dwóch miast w trasie (generowanie sąsiedztwa)
vector<int> zamien_miasta(const vector<int>& trasa, int miasto1, int miasto2) {
	vector<int> nowa_trasa = trasa;
	swap(nowa_trasa[miasto1], nowa_trasa[miasto2]);
	return nowa_trasa;
}

// Algorytm Tabu Search
void tabu_search(const vector<vector<int>>& macierz_kosztow, int czas_w_sekundach, int dlugosc_listy_tabu) {
	najlepsza_trasa_ts.clear();
	najlepszy_koszt_ts = 0;
	vector<int> obecna_trasa(liczba_miast);
	iota(obecna_trasa.begin(), obecna_trasa.end(), 0);

	random_device rd;
	mt19937 gen(rd());
	shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen);  // Losowe permutowanie miast

	int obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow);  // Koszt początkowy
	najlepsza_trasa_ts = obecna_trasa;
	najlepszy_koszt_ts = obecny_koszt;  // Przypisanie kosztu do zmiennej globalnej

	// Lista tabu
	deque<pair<int, int>> lista_tabu;
	auto start = chrono::steady_clock::now();
	int iteracja_bez_poprawy = 0;
	double czas_znalezienia = 0.0;

	while (true) {
		// Zakończenie, jeśli przekroczono czas
		auto teraz = chrono::steady_clock::now();
		double czas_uplyniety = chrono::duration_cast<chrono::seconds>(teraz - start).count();
		if (czas_uplyniety >= czas_w_sekundach) {
			break;
		}
		// Sprawdzanie sąsiedztwa
		vector<int> najlepszy_sasiad;
		int najlepszy_koszt_sasiada = INT_MAX;
		pair<int, int> najlepszy_ruch;
		for (int i = 0; i < liczba_miast - 1; ++i) {
			for (int j = i + 1; j < liczba_miast; ++j) {
				vector<int> sasiad = zamien_miasta(obecna_trasa, i, j);
				int koszt_sasiada = oblicz_koszt(sasiad, macierz_kosztow);

				// Sprawdzanie, czy ruch nie jest na liście tabu
				bool ruch_na_tabu = find(lista_tabu.begin(), lista_tabu.end(), make_pair(i, j)) != lista_tabu.end();
				if (!ruch_na_tabu || koszt_sasiada < najlepszy_koszt_ts) {
					if (koszt_sasiada < najlepszy_koszt_sasiada) {
						najlepszy_sasiad = sasiad;
						najlepszy_koszt_sasiada = koszt_sasiada;
						najlepszy_ruch = { i, j };
					}
				}
			}
		}
		// Jeśli nie znaleziono lepszego sąsiada
		if (najlepszy_sasiad.empty()) {
			break;
		}
		// Aktualizacja trasy
		obecna_trasa = najlepszy_sasiad;
		obecny_koszt = najlepszy_koszt_sasiada;

		// Jeśli poprawiliśmy najlepsze rozwiązanie, aktualizujemy zmienną globalną
		if (obecny_koszt < najlepszy_koszt_ts) {
			najlepsza_trasa_ts = obecna_trasa;
			najlepszy_koszt_ts = obecny_koszt;  // Aktualizacja globalnego kosztu
			iteracja_bez_poprawy = 0;  // Resetujemy licznik iteracji bez poprawy
			czas_znalezienia = chrono::duration_cast<chrono::seconds>(teraz - start).count();
		}
		else {
			iteracja_bez_poprawy++;  // Zwiększamy licznik iteracji bez poprawy
		}

		// Dywersyfikacja: jeśli brak poprawy przez kilka iteracji, wprowadzamy restart
		if (iteracja_bez_poprawy > 2000) { 
			shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen);  // Restart trasy
			iteracja_bez_poprawy = 0;  // Resetujemy licznik iteracji bez poprawy
		}

		// Dodanie ruchu do listy tabu
		lista_tabu.push_back(najlepszy_ruch);
		if (lista_tabu.size() > dlugosc_listy_tabu) {
			lista_tabu.pop_front();  // Usuwanie najstarszego ruchu
		}
	}

	cout << "Najlepsza znaleziona trasa: ";
	for (int miasto : najlepsza_trasa_ts) {
		cout << miasto << " ";
	}
	cout << endl;
	cout << "Calkowity koszt: " << najlepszy_koszt_ts << endl;
	cout << "Czas znalezienia najlepszego rozwiazania: " << czas_znalezienia << "s\n" << endl;
}

vector<int> wczytaj_sciezke(const string& nazwa_pliku) {
	vector<int> sciezka;
	ifstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		cerr << "Nie udało się otworzyć pliku: " << nazwa_pliku << endl;
		return sciezka;
	}
	int liczba_wierzcholkow;
	plik >> liczba_wierzcholkow;  // Wczytujemy liczbę wierzchołków
	// Wczytujemy całą ścieżkę
	int miasto;
	while (plik >> miasto) {
		sciezka.push_back(miasto);
	}
	// Usuwamy ostatni element, ponieważ to będzie powrót do miasta początkowego
	if (!sciezka.empty()) {
		sciezka.pop_back();
	}
	plik.close();
	// Sprawdzamy, czy liczba wczytanych miast zgadza się z liczbą wierzchołków
	if (sciezka.size() != liczba_wierzcholkow) {
		cerr << "Błąd: liczba wczytanych miast nie zgadza się z liczbą wierzchołków w pliku." << endl;
		return {};  // Zwracamy pustą ścieżkę w przypadku błędu
	}
	return sciezka;
}

void oblicz_droge_z_wczytanej_sciezki(const vector<int>& sciezka, const vector<vector<int>>& macierz_kosztow) {
	if (sciezka.empty()) {
		cerr << "Ścieżka jest pusta." << endl;
		return;
	}
	int koszt_calkowity = 0;
	for (size_t i = 0; i < sciezka.size() - 1; ++i) {
		int start = sciezka[i];
		int koniec = sciezka[i + 1];
		// Sprawdzenie poprawności indeksów
		if (start >= macierz_kosztow.size() || koniec >= macierz_kosztow.size()) {
			cerr << "Niepoprawny indeks w ścieżce: " << start << " -> " << koniec << endl;
			return;
		}
		koszt_calkowity += macierz_kosztow[start][koniec];
	}
	// Dodanie kosztu powrotu do miasta początkowego (zakładając, że wczytaliśmy zapętloną ścieżkę)
	int start = sciezka.back();
	int koniec = sciezka.front();

	if (start >= macierz_kosztow.size() || koniec >= macierz_kosztow.size()) {
		cerr << "Niepoprawny indeks w ścieżce: " << start << " -> " << koniec << endl;
		return;
	}
	koszt_calkowity += macierz_kosztow[start][koniec];  // Koszt powrotu

	cout << "Całkowity koszt drogi: " << koszt_calkowity << endl;
}

void zapis_do_pliku_zachlanne(const string& nazwa_pliku) {
	ofstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		cerr << "Nie udało się otworzyć pliku: " << nazwa_pliku << endl;
		return;
	}
	// Zapis liczby wierzchołków
	plik << liczba_miast << endl;
	// Zapis trasy, ścieżka ma być zapętlona, więc ostatni wierzchołek łączy się z pierwszym
	for (int miasto : najlepsza_trasa_z) {
		plik << miasto << endl;
	}
	// Zamknięcie pliku
	plik.close();
}

void zapis_do_pliku_symulowane_wyzarzanie(const string& nazwa_pliku) {
	ofstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		cerr << "Nie udało się otworzyć pliku: " << nazwa_pliku << endl;
		return;
	}
	// Zapis liczby wierzchołków
	plik << liczba_miast << endl;
	// Zapis trasy, ścieżka ma być zapętlona, więc ostatni wierzchołek łączy się z pierwszym
	for (int miasto : najlepsza_trasa_sw) {
		plik << miasto << endl;
	}
	plik << najlepsza_trasa_sw.front() << endl;
	// Zamknięcie pliku
	plik.close();
}

void zapis_do_pliku_tabu(const string& nazwa_pliku) {
	// Otwarcie pliku do zapisu
	ofstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		cerr << "Nie udało się otworzyć pliku: " << nazwa_pliku << endl;
		return;
	}
	// Zapis liczby wierzchołków
	plik << liczba_miast << endl;
	// Zapis trasy, ścieżka ma być zapętlona, więc ostatni wierzchołek łączy się z pierwszym
	for (int miasto : najlepsza_trasa_ts) {
		plik << miasto << endl;
	}
	plik << najlepsza_trasa_ts.front() << endl;
	// Zamknięcie pliku
	plik.close();
}


// -------------------------------------- ALGORYTM GENETYCZNY ------------------------------------------

pair<vector<int>, vector<int>> krzyzowanie_ox(const vector<int>& rodzic1, const vector<int>& rodzic2){
	vector<int> dziecko1(liczba_miast, -1);
	vector<int> dziecko2(liczba_miast, -1);

	// Losowanie punktów krzyżowania, które dzielą sekwencję w sposób: <0, p1>, <p1+1, p2>, <p2+1, ostatni_gen>
	int p1 = rand() % (liczba_miast - 2);                   // Pierwszy punkt
	int p2 = rand() % (liczba_miast - (p1 + 2)) + (p1 + 1); // Drugi punkt

	// Kopiowanie segmentów od p1+1 do p2
	for (int i = p1 + 1; i <= p2; i++)
	{
		dziecko1[i] = rodzic1[i];
		dziecko2[i] = rodzic2[i];
	}

	// Wypełnianie pozostałych miejsc w dziecko1 i dziecko2
	int idx1 = (p2 + 1) % liczba_miast; // Start od pozycji po p2
	for (int i = 0; i < liczba_miast; i++)
	{
		int el = rodzic2[(p2 + 1 + i) % liczba_miast];
		if (find(dziecko1.begin(), dziecko1.end(), el) == dziecko1.end()) // Jeśli element nie jest w dziecko1
		{
			dziecko1[idx1] = el;
			idx1 = (idx1 + 1) % liczba_miast;
		}
	}

	int idx2 = (p2 + 1) % liczba_miast; // Start od pozycji po p2
	for (int i = 0; i < liczba_miast; i++)
	{
		int el = rodzic1[(p2 + 1 + i) % liczba_miast];
		if (find(dziecko2.begin(), dziecko2.end(), el) == dziecko2.end()) // Jeśli element nie jest w dziecko2
		{
			dziecko2[idx2] = el;
			idx2 = (idx2 + 1) % liczba_miast;
		}
	}

	return {dziecko1, dziecko2};
}

vector<int> mutacja_swap(vector<int> permutacja) {
	int p1 = rand() % (liczba_miast - 1);
	int p2 = rand() % (liczba_miast - 1);

	while (p1 == p2) {
		p2 = rand() % (liczba_miast - 1);
	}

	swap(permutacja[p1], permutacja[p2]);
	return permutacja;
}

void inicjalizacja_populacji() {
	vector<int> permutacja(liczba_miast);
	for (int i = 0; i < liczba_miast; i++) {
		permutacja[i] = i;
	}
	// Inicjalizacja generatora losowego
	random_device rd;
	default_random_engine rng(rd());
	for (int i = 0; i < wielkosc_populacji_poczatkowej; i++)
	{
		// Losowe tasowanie permutacji
		auto rng = std::default_random_engine{};
		shuffle(begin(permutacja), end(permutacja), rng);
		populacja.push_back(permutacja); // Dodanie permutacji do populacji
	}
}

int ocena(const vector<int>& trasa, const vector<vector<int>>& macierz_kosztow) {
	int koszt = 0;
	for (size_t i = 0; i < liczba_miast - 1; i++) {
		koszt += macierz_kosztow[trasa[i] - 1][trasa[i + 1] - 1];
	}
	// Dodaj koszt powrotu do początkowego miasta
	koszt += macierz_kosztow[trasa.back() - 1][trasa[0] - 1];
	return koszt;
}

vector<int> selekcja_turniejowa() {
	vector<int> najlepszy;
	int najlepszy_koszt = INT_MAX;

	for (int i = 0; i < rozmiar_turnieju; i++) {
		int indeks = rand() % wielkosc_populacji_poczatkowej;
		if (oceny[indeks] < najlepszy_koszt) { // Porównanie na podstawie oceny
			najlepszy_koszt = oceny[indeks];
			najlepszy = populacja[indeks];
		}
	}
	return najlepszy;
}

void algorytm_genetyczny(const vector<vector<int>>& macierz_kosztow) {
	inicjalizacja_populacji();
	// Uzupełnienie tablicy kosztów wszystkich permutacji
	for (int i = 0; i < wielkosc_populacji_poczatkowej; i++) {
		oceny[i] = ocena(populacja[i], macierz_kosztow);
	}

	for (int i = 0; i < wielkosc_populacji_poczatkowej; i++) {
		
	}

}

int main()
{
	srand(time(0));
	vector<vector<int>> macierz_kosztow;;
	int czas_w_sekundach = 10;
	int dlugosc_listy_tabu = 10;
	double wspolczynnik_a = 0.999999;

	while (true) {

		int menu = 0;
		cout << "MENU:\n1. Wczytanie pliku. \n2. Wprowadzenie kryterium stopu. \n3. Obliczenie rozwiazania metoda zachlanna \n4. Algorytm TS.";
		cout << "\n5. Ustawienie wspolczynnika zmiany temperatury dla SW \n6. Algorytm SW. \n7. Ustawienie wielkosci populacji poczatkowej. \n8. Ustawienie wspolczynnika mutacji. \n9. Ustawienie wspolczynnika krzyzowania.";
		cout << "\n10. Algorytm genetyczny. \n11. Zapis sciezki rozwiazania do pliku txt. \n12. Wczytanie sciezki z pliku txt i obliczenie drogi na podstawie wczytanej tabeli kosztow." << endl;
		cin >> menu;

		switch (menu) {
		case 1: {
			string nazwa_pliku;
			cout << "Podaj nazwe pliku: ";
			cin >> nazwa_pliku;
			macierz_kosztow = wczytywanie_macierzy(nazwa_pliku);
			if (!macierz_kosztow.empty()) {
				cout << "Macierz kosztow wczytana pomyslnie.\n";
				//wypisanie_macierzy(macierz_kosztow);
			}
			break;
		}
		case 2: {
			cout << "Podaj czas dzialania algorytmow w sekundach:  ";
			cin >> czas_w_sekundach;
			cout << "Podaj dlugosc listy tabu: ";
			cin >> dlugosc_listy_tabu;
			break;
		}
		case 3: {
			if (macierz_kosztow.empty()) {
				cout << "Brak wczytanych danych. Najpierw wczytaj dane z pliku.\n";
			}
			else {
				rozwiazanie_zachlanne(macierz_kosztow);
			}
			break;
		}
		case 4: {
			if (macierz_kosztow.empty()) {
				cout << "Brak wczytanych danych. Najpierw wczytaj dane z pliku.\n";
			}
			else {
				int ilosc = 0;
				vector<int> trasa_do_pliku;
				int koszt_do_pliku = INT_MAX;
				cout << "Ile razy: ";
				cin >> ilosc;
				for (int i = 0; i < ilosc; i++) {
					tabu_search(macierz_kosztow, czas_w_sekundach, dlugosc_listy_tabu);
					if (najlepszy_koszt_ts < koszt_do_pliku) {
						koszt_do_pliku = najlepszy_koszt_ts;
						trasa_do_pliku = najlepsza_trasa_ts;
					}
				}
				najlepsza_trasa_ts = trasa_do_pliku;
				zapis_do_pliku_tabu("ts_rbg358.txt");
			}
			break;
		}
		case 5: {
			cout << "Podaj wspolczynnik zmiany temperatury (0 < a < 1): ";
			cin >> wspolczynnik_a;
			break;
		}
		case 6: {
			if (macierz_kosztow.empty()) {
				cout << "Brak wczytanych danych. Najpierw wczytaj dane z pliku.\n";
			}
			else {
				int ilosc = 0;
				vector<int> trasa_do_pliku;
				int koszt_do_pliku = INT_MAX;
				cout << "Ile razy: ";
				cin >> ilosc;
				for (int i = 0; i < ilosc; i++) {
				symulowane_wyzarzanie(macierz_kosztow, wspolczynnik_a, czas_w_sekundach);
				if (najlepszy_koszt_sw < koszt_do_pliku) {
					koszt_do_pliku = najlepszy_koszt_sw;
					trasa_do_pliku = najlepsza_trasa_sw;
				}
				}
				najlepsza_trasa_sw = trasa_do_pliku;
				zapis_do_pliku_symulowane_wyzarzanie("sw_rbg358.txt");
			}
			break;
		}
		case 7: {
			cout << "Podaj wielkosc populacji poczatkowej: ";
			cin >> wielkosc_populacji_poczatkowej;
			break;
		}
		case 8: {
			cout << "Podaj wspolczynik mutacji: ";
			cin >> wspolczynnik_mutacji;
			break;
		}
		case 9: {
			cout << "Podaj wspolczynnik krzyzowania: ";
			cin >> wspolczynnik_krzyzowania;
			break;
		}
		case 10: {
			// uruchomienie algorytmu genetycznego
			break;
		}
		case 11: {
				zapis_do_pliku_tabu("wynik_tabu.txt");
				zapis_do_pliku_symulowane_wyzarzanie("wynik_sw.txt");
				zapis_do_pliku_zachlanne("wynik_z.txt");
			break;
		}

		case 12: {
			string nazwa_pliku;
			cout << "Podaj nazwe pliku do wczytania sciezki: ";
			cin >> nazwa_pliku;
			auto sciezka = wczytaj_sciezke(nazwa_pliku);
			if (!sciezka.empty()) {
				oblicz_droge_z_wczytanej_sciezki(sciezka, macierz_kosztow);
			}
			break;
		}

		default:
			cout << "Zly wybor" << endl;
		}
	}
	return 0;
}