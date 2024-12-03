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
using namespace std;
using namespace std::chrono;


vector<vector<int>> wczytywanie_macierzy(const string& nazwa_pliku, int& liczba_miast) {

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

void wypisanie_macierzy(const vector<vector<int>>& macierz_kosztow, int liczba_miast) {
	for (int i = 0; i < liczba_miast; i++) {
		for (int j = 0; j < liczba_miast; j++) {
			cout << macierz_kosztow[i][j] << ' ';
		}
		cout << endl;
	}
}

void rozwiazanie_zachlanne(const vector<vector<int>>& macierz_kosztow, int liczba_miast) {
	vector<bool> odwiedzone(liczba_miast, false);
	vector<int> sciezka; // Przechowywana ścieżka odwiedzonych miast
	int koszt_calkowity = 0;

	int obecne_miasto = 0; // Startujemy z miasta 0
	odwiedzone[obecne_miasto] = true;
	sciezka.push_back(obecne_miasto);

	for (int i = 0; i < liczba_miast - 1; ++i) {
		int najmniejszy_koszt = numeric_limits<int>::max();
		int nastepne_miasto = -1;

		// Szukamy najbliższego nieodwiedzonego miasta
		for (int j = 0; j < liczba_miast; ++j) {
			if (!odwiedzone[j] && macierz_kosztow[obecne_miasto][j] < najmniejszy_koszt) {
				najmniejszy_koszt = macierz_kosztow[obecne_miasto][j];
				nastepne_miasto = j;
			}
		}

		// Przejście do następnego miasta
		if (nastepne_miasto != -1) {
			koszt_calkowity += najmniejszy_koszt;
			obecne_miasto = nastepne_miasto;
			odwiedzone[obecne_miasto] = true;
			sciezka.push_back(obecne_miasto);
		}
	}

	// Powrót do miasta początkowego
	koszt_calkowity += macierz_kosztow[obecne_miasto][0];
	sciezka.push_back(0);

	// Wypisanie wyników
	cout << "Znalezione rozwiazanie zachlanne: ";
	for (int miasto : sciezka) {
		cout << miasto << " ";
	}
	cout << endl;
	cout << "Calkowity koszt rozwiazania: " << koszt_calkowity << endl;
}

// Funkcja kosztu (np. całkowita długość trasy)
int oblicz_koszt(const vector<int>& trasa, const vector<vector<int>>& macierz_kosztow) {
	int koszt = 0;
	for (size_t i = 0; i < trasa.size() - 1; ++i) {
		koszt += macierz_kosztow[trasa[i]][trasa[i + 1]];
	}
	koszt += macierz_kosztow[trasa.back()][trasa[0]]; // Powrót do punktu początkowego
	return koszt;
}

// Sąsiedztwo: zamiana dwóch miast miejscami
vector<int> generuj_sasiedztwo(const vector<int>& trasa) {
	vector<int> nowa_trasa = trasa;
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<int> dist(1, trasa.size() - 2); // Unikamy pierwszego miasta (startowego)

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
vector<int> symulowane_wyzarzanie(const vector<vector<int>>& macierz_kosztow, double wspolczynnik_a, int liczba_iteracji, int liczba_miast) {

	// Inicjalizacja trasy początkowej
	vector<int> obecna_trasa(liczba_miast);
	iota(obecna_trasa.begin(), obecna_trasa.end(), 0); // iota wypełnia wektor obecna_trasa kolejnymi liczbami zaczynając od 0

	random_device rd;
	mt19937 gen(rd());
	shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen); // losowe wymieszanie wartości wektora obecna_trasa

	int obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow); // liczy koszt początkowej trasy na podstawie macierzy kosztów
	vector<int> najlepsza_trasa = obecna_trasa; // zapisuje początkową trasę jako najlepszą
	int najlepszy_koszt = obecny_koszt; // i początkowy koszt jako najlepszy

	double temperatura = oblicz_temperatura_poczatkowa(obecna_trasa, macierz_kosztow); // liczy temperaturę początkową

	for (int iter = 0; iter < liczba_iteracji; ++iter) {
		vector<int> nowa_trasa = generuj_sasiedztwo(obecna_trasa); // generuje nową trasę w sąsiedztwie poprzedniej(zmienia 2 losowe miasta)
		int nowy_koszt = oblicz_koszt(nowa_trasa, macierz_kosztow);

		if (nowy_koszt < obecny_koszt || exp((obecny_koszt - nowy_koszt) / temperatura) > uniform_real_distribution<>(0, 1)(gen)) { // porównuje koszty i akceptuje jeśli < lub jeśli większy ale o mało
			obecna_trasa = nowa_trasa; // akceptuje = aktualizuje obecne trasy i koszta
			obecny_koszt = nowy_koszt;

			if (nowy_koszt < najlepszy_koszt) { // jeśli były najlepsze to zaoisujemy jako najlepsze
				najlepsza_trasa = nowa_trasa;
				najlepszy_koszt = nowy_koszt;
			}
		}

		// Schładzanie temperatury
		temperatura *= wspolczynnik_a; // zmniejsza temperaturę
		if (temperatura < 1e-5) break; // zatrzymuje algorytm, gdy temperatura jest bardzo niska
	}

	cout << "Najlepsza znaleziona trasa: ";
	for (int miasto : najlepsza_trasa) {
		cout << miasto << " ";
	}
	cout << endl;
	cout << "Calkowity koszt: " << najlepszy_koszt << endl;
	cout << "Temperatura koncowa: " << temperatura << endl;

	return najlepsza_trasa;
}

int main()
{
	srand(time(0));
	vector<vector<int>> macierz_kosztow;
	int liczba_miast = 0;
	int petla = 1;
	while (petla == 1) {

		int menu = 0;
		cout << "MENU:\n1. Wczytanie pliku \n2. Wprowadzenie kryterium stopu. \n3. Obliczenie rozwiazania metoda zachłanna \n4. Wybor sasiedztwa. \n5. Algorytm TS.";
		cout << "\n6.Ustawienie wspolczynnika zmiany temperatury dla SW \n7.Algorytm SW. \n8.Zapis sciezki rozwiazania do pliku txt \n9. Wczytanie sciezki z pliku txt i obliczenie drogi na podstawie wczytanej tabeli kosztow" << endl;
		cin >> menu;

		switch (menu) {

		case 1: {
			string nazwa_pliku;
			cout << "Nazwa pliku: ";
			cin >> nazwa_pliku;
			macierz_kosztow = wczytywanie_macierzy(nazwa_pliku, liczba_miast);
			if (macierz_kosztow.empty()) {
				cout << "Nie udalo sie wczytac macierzy! " << endl;
			}
			else {
				cout << "Macierz wczytana poprawnie. " << endl;
				wypisanie_macierzy(macierz_kosztow, liczba_miast);
			}
			break;
		}
		case 3: {
			rozwiazanie_zachlanne(macierz_kosztow, liczba_miast);
		}
			  break;
		case 7: {
			if (macierz_kosztow.empty() || liczba_miast <= 0) {
				cout << "Macierz kosztow jest pusta lub liczba miast jest nieprawidlowa!" << endl;
				break;
			}

			double wspolczynnik_a;
			cout << "Podaj wspolczynnik schladzania (np. 0.95): ";
			cin >> wspolczynnik_a;

			int liczba_iteracji;
			cout << "Podaj liczbe iteracji: ";
			cin >> liczba_iteracji;

			symulowane_wyzarzanie(macierz_kosztow, wspolczynnik_a, liczba_iteracji, liczba_miast);
			break;
		}

		default:
			cout << "Zly wybor" << endl;
		}
	}
	return 0;
}