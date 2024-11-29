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


vector<vector<int>> wczytywanie_macierzy(const string& nazwa_pliku) {

	ifstream plik(nazwa_pliku);
	int liczba_miast = 0;
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
			for (int i = 0; i < liczba_miast; i++) {
				getline(plik, linia);  // Wczytujemy linię z kosztami
				istringstream iss(linia);
				for (int j = 0; j < liczba_miast; j++) {
					iss >> macierz_kosztow[i][j];
				}
			}
			break;
		}
	}
	plik.close();
	return macierz_kosztow;
}


int main()
{
	srand(time(0));

	int petla = 1;
	while (petla == 1) {

		int menu = 0;
		vector<vector<int>>macierz_kosztow;
		cout << "MENU:\n1. Wczytanie pliku \n2. Wprowadzenie kryterium stopu. \n3. Obliczenie rozwiazania metoda zachłanna \n4. wybor sasiedztwa. \n5. Algorytm TS.";
		cout << "\n6.Ustawienie współczynnika zmiany temperatury dla SW \n7.Algorytm SW. \n8.Zapis sciezki rozwiazania do pliku txt \n9..Wczytanie sciezki z pliku txt i obliczenie drogi na podstawie wczytanej tabeli kosztow" << endl;
		cin >> menu;

		switch (menu) {

		case 1: {
			string nazwa_pliku;
			cout << "Nazwa pliku: ";
			cin >> nazwa_pliku;
			macierz_kosztow = wczytywanie_macierzy(nazwa_pliku);
			if (macierz_kosztow.empty()) {
				cout << "Nie udalo sie wczytac macierzy! " << endl;
			}
			else {
				cout << "Macierz wczytana poprawnie. " << endl;
			}
			break;
		}
		default:
			cout << "Zly wybor" << endl;
		}
	}
	return 0;
}