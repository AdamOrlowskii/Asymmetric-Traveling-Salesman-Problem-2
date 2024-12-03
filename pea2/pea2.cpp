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
		default:
			cout << "Zly wybor" << endl;
		}
	}
	return 0;
}