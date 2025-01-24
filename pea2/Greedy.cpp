#include "greedy.h"
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
using namespace std;
using namespace std::chrono;

void Greedy::rozwiazanie_zachlanne(const vector<vector<int>>& macierz_kosztow) {
	najlepszy_koszt_z = numeric_limits<int>::max(); // Najlepszy koszt

	// Iterujemy przez ka¿de miasto jako punkt pocz¹tkowy
	for (int startowe_miasto = 0; startowe_miasto < liczba_miast; ++startowe_miasto) {
		vector<bool> odwiedzone(liczba_miast, false);
		vector<int> trasa_lokalna;  // Lokalne rozwi¹zanie
		int koszt_lokalny = 0;

		// Startujemy z danego miasta
		int obecne_miasto = startowe_miasto;
		odwiedzone[obecne_miasto] = true;
		trasa_lokalna.push_back(obecne_miasto);

		// Algorytm zach³anny dla tego startowego miasta
		for (int i = 0; i < liczba_miast - 1; ++i) {
			int najlepszy_koszt_lokalny = numeric_limits<int>::max();
			int nastepne_miasto = -1;

			// Znajdujemy najbli¿sze nieodwiedzone miasto
			for (int j = 0; j < liczba_miast; ++j) {
				if (!odwiedzone[j] && macierz_kosztow[obecne_miasto][j] < najlepszy_koszt_lokalny) {
					najlepszy_koszt_lokalny = macierz_kosztow[obecne_miasto][j];
					nastepne_miasto = j;
				}
			}

			// Przejœcie do najbli¿szego miasta
			if (nastepne_miasto != -1) {
				koszt_lokalny += najlepszy_koszt_lokalny;
				obecne_miasto = nastepne_miasto;
				odwiedzone[obecne_miasto] = true;
				trasa_lokalna.push_back(obecne_miasto);
			}
		}

		// Powrót do miasta pocz¹tkowego
		koszt_lokalny += macierz_kosztow[obecne_miasto][startowe_miasto];
		trasa_lokalna.push_back(startowe_miasto);

		// Aktualizacja globalnie najlepszego rozwi¹zania
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