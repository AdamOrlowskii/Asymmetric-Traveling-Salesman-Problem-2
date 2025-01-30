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
#include "SimulatedAnnealing.h"
#include "main.h"
using namespace std;
using namespace std::chrono;

// Oblicz temperaturê pocz¹tkow¹ (np. na podstawie ró¿nic w kosztach s¹siednich rozwi¹zañ)
double SimulatedAnnealing::oblicz_temperatura_poczatkowa(const vector<int>& trasa, const vector<vector<int>>& macierz_kosztow) {
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

// Algorytm Symulowanego Wy¿arzania
void SimulatedAnnealing::symulowane_wyzarzanie(const vector<vector<int>>& macierz_kosztow, double wspolczynnik_a, int czas_w_sekundach, bool pierwszy_raz, string nazwa_pliku) {
	najlepsza_trasa_sw.clear();
	najlepszy_koszt_sw = 0;
	double czas_znalezienia = 0.0;
	double czas_w_milisekundach = czas_w_sekundach * 1000;
	int liczba_odcinkow_czasu = 40;
	double odcinek_czasu = czas_w_milisekundach / liczba_odcinkow_czasu;
	double ostatni_pomiar_czasu = 0;
	// Inicjalizacja trasy pocz¹tkowej
	vector<int> obecna_trasa(liczba_miast);
	vector<int> oceny_do_wykresu;
	iota(obecna_trasa.begin(), obecna_trasa.end(), 0); // iota wype³nia wektor obecna_trasa kolejnymi liczbami zaczynaj¹c od 0

	random_device rd;
	mt19937 gen(rd());
	shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen); // losowe wymieszanie wartoœci wektora obecna_trasa

	int obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow); // liczy koszt pocz¹tkowej trasy na podstawie macierzy kosztów
	najlepsza_trasa_sw = obecna_trasa; // zapisuje pocz¹tkow¹ trasê jako najlepsz¹
	najlepszy_koszt_sw = obecny_koszt; // i pocz¹tkowy koszt jako najlepszy

	double temperatura = oblicz_temperatura_poczatkowa(obecna_trasa, macierz_kosztow); // liczy temperaturê pocz¹tkow¹

	// Rozpoczêcie pomiaru czasu
	auto start_sw = chrono::steady_clock::now();

	// Dywersyfikacja
	int iter_count = 0;
	int limit_iteracji_bez_poprawy = 1000000; // Ustalona liczba iteracji bez poprawy

	while (true) {

		auto teraz_sw = chrono::steady_clock::now();
		double czas_uplyniety = chrono::duration_cast<chrono::milliseconds>(teraz_sw - start_sw).count();

		// Sprawdzenie, czy nadszed³ czas na zapisanie do csv
		if (czas_uplyniety - ostatni_pomiar_czasu >= odcinek_czasu) {
			oceny_do_wykresu.push_back(obecny_koszt);
			ostatni_pomiar_czasu = czas_uplyniety;
		}

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

				for (double koszt : oceny_do_wykresu)
				{
					plik << koszt << ";";
				}
				plik << najlepszy_koszt_sw;
				plik << endl;
				plik.close();
			}
			break; // Przerwij, jeœli czas zosta³ przekroczony
		}

		// Generowanie s¹siedztwa
		vector<int> nowa_trasa = generuj_sasiedztwo(obecna_trasa); // generuje now¹ trasê w s¹siedztwie poprzedniej
		int nowy_koszt = oblicz_koszt(nowa_trasa, macierz_kosztow);

		// Decyzja o zaakceptowaniu nowej trasy
		if (nowy_koszt < obecny_koszt || exp((obecny_koszt - nowy_koszt) / temperatura) > uniform_real_distribution<>(0, 1)(gen)) {
			obecna_trasa = nowa_trasa; // Aktualizacja trasy
			obecny_koszt = nowy_koszt;

			// Aktualizacja najlepszego rozwi¹zania
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

		// Sch³adzanie temperatury
		temperatura *= wspolczynnik_a; // Zmniejszenie temperatury
		if (temperatura < 1e-5) break; // Zatrzymanie algorytmu, gdy temperatura jest bardzo niska

		// Degeneracja: Losowe restartowanie trasy po okreœlonej liczbie iteracji bez poprawy
		if (iter_count >= limit_iteracji_bez_poprawy) {
			shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen); // Losowe wymieszanie trasy
			obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow); // Oblicz koszt nowej trasy
			iter_count = 0; // Resetuj licznik iteracji bez poprawy
		}
	}

	std::cout << "Najlepsza znaleziona trasa: ";
	for (int miasto : najlepsza_trasa_sw) {
		std::cout << miasto << " ";
	}
	std::cout << endl;
	std::cout << "Calkowity koszt: " << najlepszy_koszt_sw << endl;
	std::cout << "Temperatura koncowa: " << temperatura << endl;
	std::cout << "Czas znalezienia najlepszego rozwiazania: " << czas_znalezienia << "s\n" << endl;
}