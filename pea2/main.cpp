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
#include "GeneticAlgorithm.h"
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

void zapis_do_pliku(const string& nazwa_pliku, vector<int>& najlepsza_trasa, int spr) {
	ofstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		cerr << "Nie udało się otworzyć pliku: " << nazwa_pliku << endl;
		return;
	}
	// Zapis liczby wierzchołków
	plik << liczba_miast << endl;
	// Zapis trasy, ścieżka ma być zapętlona, więc ostatni wierzchołek łączy się z pierwszym
	for (int miasto : najlepsza_trasa) {
		plik << miasto << endl;
	}
	if (spr == 1) {
		plik << najlepsza_trasa.front() << endl;
	}
	// Zamknięcie pliku
	plik.close();
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
				zapis_do_pliku("ts_rbg358.txt", najlepsza_trasa_ts, 1);
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
				zapis_do_pliku("sw_rbg358.txt", najlepsza_trasa_sw, 1);
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
				GeneticAlgorithm::algorytm_genetyczny(macierz_kosztow, czas_w_sekundach, nazwa_pliku, pierwszy_raz);
				pierwszy_raz = false;
			}
			break;
		}
		case 13: {
				zapis_do_pliku("wynik_tabu.txt", najlepsza_trasa_ts, 1);
				zapis_do_pliku("wynik_sw.txt", najlepsza_trasa_sw, 1);
				zapis_do_pliku("wynik_z.txt", najlepsza_trasa_z, 0);
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