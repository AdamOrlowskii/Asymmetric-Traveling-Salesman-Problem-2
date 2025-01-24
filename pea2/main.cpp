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
#include <functional>
#include <numeric>
#include <deque>
#include <utility>
#include <iterator>
#include "Greedy.h"
#include "main.h"
#include "TabuSearch.h"
#include "SimulatedAnnealing.h"
using namespace std;
using namespace std::chrono;

int liczba_miast, najlepszy_koszt_z, najlepszy_koszt_ts, najlepszy_koszt_sw, wielkosc_populacji, jakie_krzyzowanie, dlugosc_listy_tabu;
vector<int> najlepsza_trasa_z, najlepsza_trasa_ts, najlepsza_trasa_sw, oceny;
double wspolczynnik_mutacji, wspolczynnik_krzyzowania, czas_w_sekundach, wspolczynnik_a;
vector<vector<int>> populacja, macierz_kosztow;

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

// Funkcja do zamiany dwóch miast w trasie (generowanie sąsiedztwa)
vector<int> zamien_miasta(const vector<int>& trasa, int miasto1, int miasto2) {
	vector<int> nowa_trasa = trasa;
	swap(nowa_trasa[miasto1], nowa_trasa[miasto2]);
	return nowa_trasa;
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

pair<vector<int>, vector<int>> krzyzowanie_csx(const vector<int>& rodzic1, const vector<int>& rodzic2) {
	int n = liczba_miast;
	vector<int> dziecko1(n, -1), dziecko2(n, -1); // Wektory dzieci wypełnione -1 (puste miejsca)

	// Funkcja pomocnicza do znalezienia cykli
	auto znajdz_cykl = [&](int start, const vector<int>& r1, const vector<int>& r2, vector<bool>& odwiedzone) {
		vector<int> cykl;
		int indeks = start;
		do {
			cykl.push_back(indeks);
			odwiedzone[indeks] = true;
			indeks = find(r1.begin(), r1.end(), r2[indeks]) - r1.begin();
		} while (indeks != start);
		return cykl;
		};

	vector<bool> odwiedzone(n, false);

	// Tworzymy dzieci na podstawie cykli
	for (int i = 0; i < n; i++) {
		if (!odwiedzone[i]) {
			vector<int> cykl = znajdz_cykl(i, rodzic1, rodzic2, odwiedzone);

			// Przenosimy geny z cyklu do dzieci
			for (int indeks : cykl) {
				dziecko1[indeks] = rodzic1[indeks];
				dziecko2[indeks] = rodzic2[indeks];
			}
		}
	}

	// Uzupełniamy brakujące geny (te, które nie były w cyklu)
	for (int i = 0; i < n; i++) {
		if (dziecko1[i] == -1) dziecko1[i] = rodzic2[i];
		if (dziecko2[i] == -1) dziecko2[i] = rodzic1[i];
	}

	return { dziecko1, dziecko2 };
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
	for (int i = 0; i < wielkosc_populacji; i++)
	{
		// Losowe tasowanie permutacji
		auto rng = std::default_random_engine{};
		shuffle(begin(permutacja), end(permutacja), rng);
		populacja.push_back(permutacja); // Dodanie permutacji do populacji
	}
}

vector<int> selekcja_turniejowa(int rozmiar_turnieju) {
	vector<int> najlepszy;
	int najlepszy_koszt = INT_MAX;

	for (int i = 0; i < rozmiar_turnieju; i++) {
		int indeks = rand() % wielkosc_populacji;
		if (oceny[indeks] < najlepszy_koszt) { // Porównanie na podstawie oceny
			najlepszy_koszt = oceny[indeks];
			najlepszy = populacja[indeks];
		}
	}
	return najlepszy;
}


void algorytm_genetyczny(const vector<vector<int>>& macierz_kosztow, int czas_w_sekundach, string nazwa_pliku, bool pierwszy_raz) {
	double czas_w_milisekundach = czas_w_sekundach * 1000;
	int rozmiar_turnieju = 4;
	populacja.clear();
	oceny.clear();
	inicjalizacja_populacji(); // Uzupełnienie tablicy kosztów wszystkich permutacji
	auto start = chrono::steady_clock::now();
	oceny.resize(wielkosc_populacji);
	int liczba_odcinkow_czasu = 40;
	double odcinek_czasu = czas_w_milisekundach / liczba_odcinkow_czasu;
	vector<double> srednie_oceny_w_przedziale;
	vector<vector<int>> oceny_do_wykresu;
	double srednia_ocen = 0;
	double najlepsza_srednia_ocen = DBL_MAX;
	double ostatni_pomiar_czasu = 0;
	oceny.resize(wielkosc_populacji);
	vector<vector<int>> nowa_populacja;
	vector<int> najlepszy_z_poprzeniej_generacji;

	// Mam wektor wektorów populacja, populacja przechowuje wektory permutacja i na tym działam, permutacje mutuję, krzyżuję i dodaję do nowej populacji a potem ją kopiuję do starej
	// Wektor oceny to inty koszty każdej permutacji
	while (true) {
		nowa_populacja.clear();
		auto teraz = chrono::steady_clock::now();
		double czas_uplyniety = chrono::duration_cast<chrono::milliseconds>(teraz - start).count();

		// Sprawdzenie, czy nadszedł czas na zapisanie średnich ocen
		if (czas_uplyniety - ostatni_pomiar_czasu >= odcinek_czasu) {
			srednie_oceny_w_przedziale.push_back(srednia_ocen);
			ostatni_pomiar_czasu = czas_uplyniety;
			najlepsza_srednia_ocen = DBL_MAX;
		}
		// Sprawdzenie, czy już koniec algorytmu
		if (czas_uplyniety >= czas_w_milisekundach) {
			ofstream plik;
			ios_base::openmode tryb_pliku;
			if (pierwszy_raz) {
				plik.open(nazwa_pliku, std::ios::out); // Nadpisz plik, jeśli to pierwszy raz
			}
			else {
				plik.open(nazwa_pliku, std::ios::app); // Dopisz do pliku, jeśli to kolejny raz
			}

			if (!plik.is_open()) {
				cerr << "Nie udało się otworzyć pliku" << endl;
				return;
			}
			else {

				for (double ocena : srednie_oceny_w_przedziale)
				{
					plik << ocena << ";";
				}
				plik << ";";
				auto najlepszy_koszt = *min_element(oceny.begin(), oceny.end());
				plik << najlepszy_koszt;
				plik << endl;
				plik.close();
				break;
			}
		}
		for (int i = 0; i < wielkosc_populacji; i++) {
			oceny[i] = oblicz_koszt(populacja[i], macierz_kosztow);
		}
		
		srednia_ocen = 0;

		for (int i = 0; i < wielkosc_populacji; i++) {
			srednia_ocen += oceny[i];
		}

		srednia_ocen /= wielkosc_populacji; // srednia ocen wszystkich permutacji w aktualnej populacji

		if (srednia_ocen < najlepsza_srednia_ocen) {
			najlepsza_srednia_ocen = srednia_ocen;
		}

		// Czyli ten kod liczy średnią kosztów całej populacji, jeżeli jest ona lepsza niż średnia uzyskana wcześniej w poprzednich populacjach w aktualnym przedziale czasowym to ona się staje najlepszą
		// Ta średnia to jeden int, a najlepsze oceny w przedziale to już wektor kosztów każdej permutacji w populacji: permutacja1 koszt 69, permutacja2 koszt 369 itd
		// Więc najlepsze oceny w przedziale to wektor kosztów najlepszej znalezionej populacji w przedziale czasowym. JEDEN WEKTOR JEDNEJ POPULACJI

		// ---------------- elityzm ---------------
		auto min = min_element(oceny.begin(), oceny.end());
		int index = distance(oceny.begin(), min);
		najlepszy_z_poprzeniej_generacji = populacja[index];
		nowa_populacja.push_back(najlepszy_z_poprzeniej_generacji);
		// ----------------------------------------

		for (int i = 0; i < wielkosc_populacji / 2; i++) {
			// Wybranie rodziców poprzez turniej
			vector<int> rodzic1 = selekcja_turniejowa(rozmiar_turnieju);
			vector<int> rodzic2 = selekcja_turniejowa(rozmiar_turnieju);
			vector<int> dziecko1, dziecko2;
			double krzyzowanie = (double)rand() / RAND_MAX;
			double mutacja = (double)rand() / RAND_MAX;

			if (krzyzowanie <= wspolczynnik_krzyzowania) {
				switch (jakie_krzyzowanie)
				{
				case 1: {
					tie(dziecko1, dziecko2) = krzyzowanie_ox(rodzic1, rodzic2); // Rozpakowanie pary wektorów
					break;
				}
				case 2: {
					tie(dziecko1, dziecko2) = krzyzowanie_csx(rodzic1, rodzic2); // Rozpakowanie pary wektorów
					break;
				}
				}
				
				if (mutacja <= wspolczynnik_mutacji) {
					dziecko1 = mutacja_swap(dziecko1);
					dziecko2 = mutacja_swap(dziecko2);
				}
				nowa_populacja.push_back(dziecko1);
				if (nowa_populacja.size() == populacja.size()) {
					break;
				}
				nowa_populacja.push_back(dziecko2);
			}
			else {
				nowa_populacja.push_back(rodzic1);
				if (nowa_populacja.size() == populacja.size()) {
					break;
				}
				nowa_populacja.push_back(rodzic2);
			}
		}
		populacja = nowa_populacja;
	}
}

int main()
{
	srand(time(0));

	while (true) {

		int menu = 0;
		cout << "MENU:\n1.  Wczytanie pliku. \n2.  Wprowadzenie kryterium stopu. \n3.  Obliczenie rozwiazania metoda zachlanna. \n4.  Dlugosc listy tabu. \n5.  Algorytm TS.";
		cout << "\n6.  Ustawienie wspolczynnika zmiany temperatury dla SW \n7.  Algorytm SW. \n8.  Ustawienie wielkosci populacji poczatkowej. \n9.  Ustawienie wspolczynnika mutacji. \n10. Ustawienie wspolczynnika krzyzowania.";
		cout << "\n11. Wybor sposobu krzyzowania. \n12. Algorytm genetyczny. \n13. Zapis sciezki rozwiazania do pliku txt. \n14. Wczytanie sciezki z pliku txt i obliczenie drogi na podstawie wczytanej tabeli kosztow." << endl;
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
			break;
		}
		case 3: {
			if (macierz_kosztow.empty()) {
				cout << "Brak wczytanych danych. Najpierw wczytaj dane z pliku.\n";
			}
			else {
				Greedy::rozwiazanie_zachlanne(macierz_kosztow);
			}
			break;
		}
		case 4: {
			cout << "Podaj dlugosc listy tabu: ";
			cin >> dlugosc_listy_tabu;
			break;
		}
		case 5: {
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
					TabuSearch::tabu_search(macierz_kosztow, czas_w_sekundach, dlugosc_listy_tabu);
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
		case 6: {
			cout << "Podaj wspolczynnik zmiany temperatury (0 < a < 1): ";
			cin >> wspolczynnik_a;
			break;
		}
		case 7: {
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
				SimulatedAnnealing::symulowane_wyzarzanie(macierz_kosztow, wspolczynnik_a, czas_w_sekundach);
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
		case 8: {
			cout << "Podaj wielkosc populacji poczatkowej: ";
			cin >> wielkosc_populacji;
			break;
		}
		case 9: {
			cout << "Podaj wspolczynik mutacji: ";
			cin >> wspolczynnik_mutacji;
			break;
		}
		case 10: {
			cout << "Podaj wspolczynnik krzyzowania: ";
			cin >> wspolczynnik_krzyzowania;
			break;
		}
		case 11: {
			cout << "Krzyzowanie OX czy CSX? '1' lub '2'. ";
			cin >> jakie_krzyzowanie;
			break;
		}
		case 12: {
			int ilosc;
			cout << "Ile razy: ";
			cin >> ilosc;
			int powtorzenie = 0;
			string nazwa_pliku = "gen_wyniki_wykres.csv";
			bool pierwszy_raz = true;
			for (powtorzenie = 0; powtorzenie < ilosc; powtorzenie++)
			{
				algorytm_genetyczny(macierz_kosztow, czas_w_sekundach, nazwa_pliku, pierwszy_raz);
				pierwszy_raz = false;
			}
			break;
		}
		case 13: {
				zapis_do_pliku_tabu("wynik_tabu.txt");
				zapis_do_pliku_symulowane_wyzarzanie("wynik_sw.txt");
				zapis_do_pliku_zachlanne("wynik_z.txt");
			break;
		}

		case 14: {
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