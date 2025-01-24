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
#include "TabuSearch.h"
#include "main.h"
using namespace std;
using namespace std::chrono;

// Algorytm Tabu Search
void TabuSearch::tabu_search(const vector<vector<int>>& macierz_kosztow, int czas_w_sekundach, int dlugosc_listy_tabu) {
	najlepsza_trasa_ts.clear();
	najlepszy_koszt_ts = 0;
	vector<int> obecna_trasa(liczba_miast);
	iota(obecna_trasa.begin(), obecna_trasa.end(), 0);

	random_device rd;
	mt19937 gen(rd());
	shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen);  // Losowe permutowanie miast

	int obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow);  // Koszt pocz¹tkowy
	najlepsza_trasa_ts = obecna_trasa;
	najlepszy_koszt_ts = obecny_koszt;  // Przypisanie kosztu do zmiennej globalnej

	// Lista tabu
	deque<pair<int, int>> lista_tabu;
	auto start = chrono::steady_clock::now();
	int iteracja_bez_poprawy = 0;
	double czas_znalezienia = 0.0;

	while (true) {
		// Zakoñczenie, jeœli przekroczono czas
		auto teraz = chrono::steady_clock::now();
		double czas_uplyniety = chrono::duration_cast<chrono::seconds>(teraz - start).count();
		if (czas_uplyniety >= czas_w_sekundach) {
			break;
		}
		// Sprawdzanie s¹siedztwa
		vector<int> najlepszy_sasiad;
		int najlepszy_koszt_sasiada = INT_MAX;
		pair<int, int> najlepszy_ruch;
		for (int i = 0; i < liczba_miast - 1; ++i) {
			for (int j = i + 1; j < liczba_miast; ++j) {
				vector<int> sasiad = zamien_miasta(obecna_trasa, i, j);
				int koszt_sasiada = oblicz_koszt(sasiad, macierz_kosztow);

				// Sprawdzanie, czy ruch nie jest na liœcie tabu
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
		// Jeœli nie znaleziono lepszego s¹siada
		if (najlepszy_sasiad.empty()) {
			break;
		}
		// Aktualizacja trasy
		obecna_trasa = najlepszy_sasiad;
		obecny_koszt = najlepszy_koszt_sasiada;

		// Jeœli poprawiliœmy najlepsze rozwi¹zanie, aktualizujemy zmienn¹ globaln¹
		if (obecny_koszt < najlepszy_koszt_ts) {
			najlepsza_trasa_ts = obecna_trasa;
			najlepszy_koszt_ts = obecny_koszt;  // Aktualizacja globalnego kosztu
			iteracja_bez_poprawy = 0;  // Resetujemy licznik iteracji bez poprawy
			czas_znalezienia = chrono::duration_cast<chrono::seconds>(teraz - start).count();
		}
		else {
			iteracja_bez_poprawy++;  // Zwiêkszamy licznik iteracji bez poprawy
		}

		// Dywersyfikacja: jeœli brak poprawy przez kilka iteracji, wprowadzamy restart
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
};