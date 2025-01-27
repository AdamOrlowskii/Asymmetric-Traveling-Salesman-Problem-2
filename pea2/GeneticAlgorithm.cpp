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
#include "GeneticAlgorithm.h"
#include "main.h"
using namespace std;
using namespace std::chrono;

pair<vector<int>, vector<int>> GeneticAlgorithm::krzyzowanie_ox(const vector<int>& rodzic1, const vector<int>& rodzic2) {
	vector<int> dziecko1(liczba_miast, -1);
	vector<int> dziecko2(liczba_miast, -1);

	// Losowanie punktów krzy¿owania, które dziel¹ sekwencjê w sposób: <0, p1>, <p1+1, p2>, <p2+1, ostatni_gen>
	int p1 = rand() % (liczba_miast - 2);                   // Pierwszy punkt
	int p2 = rand() % (liczba_miast - (p1 + 2)) + (p1 + 1); // Drugi punkt

	// Kopiowanie segmentów od p1+1 do p2
	for (int i = p1 + 1; i <= p2; i++)
	{
		dziecko1[i] = rodzic1[i];
		dziecko2[i] = rodzic2[i];
	}

	// Wype³nianie pozosta³ych miejsc w dziecko1 i dziecko2
	int idx1 = (p2 + 1) % liczba_miast; // Start od pozycji po p2
	for (int i = 0; i < liczba_miast; i++)
	{
		int el = rodzic2[(p2 + 1 + i) % liczba_miast];
		if (find(dziecko1.begin(), dziecko1.end(), el) == dziecko1.end()) // Jeœli element nie jest w dziecko1
		{
			dziecko1[idx1] = el;
			idx1 = (idx1 + 1) % liczba_miast;
		}
	}

	int idx2 = (p2 + 1) % liczba_miast; // Start od pozycji po p2
	for (int i = 0; i < liczba_miast; i++)
	{
		int el = rodzic1[(p2 + 1 + i) % liczba_miast];
		if (find(dziecko2.begin(), dziecko2.end(), el) == dziecko2.end()) // Jeœli element nie jest w dziecko2
		{
			dziecko2[idx2] = el;
			idx2 = (idx2 + 1) % liczba_miast;
		}
	}

	return { dziecko1, dziecko2 };
}

pair<vector<int>, vector<int>> GeneticAlgorithm::krzyzowanie_cx(const vector<int>& rodzic1, const vector<int>& rodzic2) {
	int n = liczba_miast;
	vector<int> dziecko1(n, -1), dziecko2(n, -1); // Wektory dzieci wype³nione -1 (puste miejsca)

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

	// Uzupe³niamy brakuj¹ce geny (te, które nie by³y w cyklu)
	for (int i = 0; i < n; i++) {
		if (dziecko1[i] == -1) dziecko1[i] = rodzic2[i];
		if (dziecko2[i] == -1) dziecko2[i] = rodzic1[i];
	}

	return { dziecko1, dziecko2 };
}

vector<int> GeneticAlgorithm::mutacja_swap(vector<int> permutacja) {
	int p1 = rand() % (liczba_miast - 1);
	int p2 = rand() % (liczba_miast - 1);

	while (p1 == p2) {
		p2 = rand() % (liczba_miast - 1);
	}

	swap(permutacja[p1], permutacja[p2]);
	return permutacja;
}

void GeneticAlgorithm::inicjalizacja_populacji() {
	random_device rd;
	default_random_engine rng(rd());

	// Pêtla tworz¹ca populacjê
	for (int i = 0; i < wielkosc_populacji; i++) {
		vector<int> permutacja(liczba_miast);

		// Wype³nienie permutacji wartoœciami od 0 do liczba_miast-1
		for (int j = 0; j < liczba_miast; j++) {
			permutacja[j] = j;
		}

		// Losowe tasowanie permutacji
		shuffle(permutacja.begin(), permutacja.end(), rng);

		// Dodanie permutacji do populacji
		populacja.push_back(permutacja);
	}
}

vector<int> GeneticAlgorithm::selekcja_turniejowa(int rozmiar_turnieju) {
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

void GeneticAlgorithm::algorytm_genetyczny(const vector<vector<int>>& macierz_kosztow, int czas_w_sekundach, string nazwa_pliku, bool pierwszy_raz) {
	double czas_w_milisekundach = czas_w_sekundach * 1000;
	int rozmiar_turnieju = 4;
	populacja.clear();
	oceny.clear();
	inicjalizacja_populacji(); // Uzupe³nienie tablicy kosztów wszystkich permutacji
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
	vector<int> najlepszy_z_poprzedniej_generacji;
	int pierwszy_wynik = 1;

	// Wektor oceny to inty koszty ka¿dej permutacji
	while (true) {
		nowa_populacja.clear();
		auto teraz = chrono::steady_clock::now();
		double czas_uplyniety = chrono::duration_cast<chrono::milliseconds>(teraz - start).count();

		// Sprawdzenie, czy nadszed³ czas na zapisanie œrednich ocen
		if (czas_uplyniety - ostatni_pomiar_czasu >= odcinek_czasu) {
			srednie_oceny_w_przedziale.push_back(srednia_ocen);
			ostatni_pomiar_czasu = czas_uplyniety;
			najlepsza_srednia_ocen = DBL_MAX;
		}
		// Sprawdzenie, czy ju¿ koniec algorytmu
		if (czas_uplyniety >= czas_w_milisekundach) {
			ofstream plik;
			ios_base::openmode tryb_pliku;
			if (pierwszy_raz) {
				plik.open(nazwa_pliku, std::ios::out); // Nadpisz plik, jeœli to pierwszy raz
			}
			else {
				plik.open(nazwa_pliku, std::ios::app); // Dopisz do pliku, jeœli to kolejny raz
			}

			if (!plik.is_open()) {
				cerr << "Nie uda³o siê otworzyæ pliku" << endl;
				return;
			}
			else {

				for (double ocena : srednie_oceny_w_przedziale)
				{
					plik << ocena << ";";
				}
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

		// Ten kod liczy œredni¹ kosztów ca³ej populacji, je¿eli jest ona lepsza ni¿ œrednia uzyskana wczeœniej w poprzednich populacjach w aktualnym przedziale czasowym to ona siê staje najlepsz¹
		// Œrednia to jeden int, a najlepsze oceny w przedziale to ju¿ wektor kosztów ka¿dej permutacji w populacji
		// Najlepsze oceny w przedziale to wektor kosztów najlepszej znalezionej populacji w przedziale czasowym.

		// ---------------- elityzm ---------------
		auto min = min_element(oceny.begin(), oceny.end());
		int index = distance(oceny.begin(), min);
		najlepszy_z_poprzedniej_generacji = populacja[index];
		nowa_populacja.push_back(najlepszy_z_poprzedniej_generacji);
		// ----------------------------------------
		if (pierwszy_wynik == 1) {
			srednie_oceny_w_przedziale.push_back(srednia_ocen);
			pierwszy_wynik = 0;
		}


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
					tie(dziecko1, dziecko2) = krzyzowanie_cx(rodzic1, rodzic2); // Rozpakowanie pary wektorów
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