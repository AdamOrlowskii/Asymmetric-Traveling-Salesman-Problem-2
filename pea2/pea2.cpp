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

vector<int> najlepsza_trasa;
int najlepszy_koszt;

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
	koszt += macierz_kosztow[trasa.back()][trasa.front()]; // Powrót do punktu początkowego
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

// Algorytm Symulowanego Wyżarzania z ograniczeniem czasu
vector<int> symulowane_wyzarzanie(const vector<vector<int>>& macierz_kosztow, double wspolczynnik_a, int czas_w_sekundach, int liczba_miast, int& najlepszy_koszt_sw) {

    // Inicjalizacja trasy początkowej
    vector<int> obecna_trasa(liczba_miast);
    iota(obecna_trasa.begin(), obecna_trasa.end(), 0); // iota wypełnia wektor obecna_trasa kolejnymi liczbami zaczynając od 0

    random_device rd;
    mt19937 gen(rd());
    shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen); // losowe wymieszanie wartości wektora obecna_trasa

    int obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow); // liczy koszt początkowej trasy na podstawie macierzy kosztów
    vector<int> najlepsza_trasa_sw = obecna_trasa; // zapisuje początkową trasę jako najlepszą
    najlepszy_koszt_sw = obecny_koszt; // i początkowy koszt jako najlepszy

    double temperatura = oblicz_temperatura_poczatkowa(obecna_trasa, macierz_kosztow); // liczy temperaturę początkową

    // Rozpoczęcie pomiaru czasu
    auto start_sw = chrono::steady_clock::now();

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
            if (nowy_koszt < najlepszy_koszt) {
                najlepsza_trasa_sw = nowa_trasa;
                najlepszy_koszt_sw = nowy_koszt;
            }
        }

        // Schładzanie temperatury
        temperatura *= wspolczynnik_a; // Zmniejszenie temperatury
        if (temperatura < 1e-5) break; // Zatrzymanie algorytmu, gdy temperatura jest bardzo niska
    }

    cout << "Najlepsza znaleziona trasa: ";
    for (int miasto : najlepsza_trasa_sw) {
        cout << miasto << " ";
    }
    cout << endl;
    cout << "Calkowity koszt: " << najlepszy_koszt_sw << endl;
    cout << "Temperatura koncowa: " << temperatura << endl;

	return najlepsza_trasa_sw;
}


// Funkcja do zamiany dwóch miast w trasie (generowanie sąsiedztwa)
vector<int> zamien_miasta(const vector<int>& trasa, int miasto1, int miasto2) {
	vector<int> nowa_trasa = trasa;
	swap(nowa_trasa[miasto1], nowa_trasa[miasto2]);
	return nowa_trasa;
}

// Algorytm Tabu Search
pair<vector<int>, int> tabu_search(const vector<vector<int>>& macierz_kosztow, int czas_w_sekundach, int dlugosc_listy_tabu) {
	int liczba_miast = macierz_kosztow.size();

	// Losowanie początkowej trasy
	vector<int> obecna_trasa(liczba_miast);
	iota(obecna_trasa.begin(), obecna_trasa.end(), 0);

	random_device rd;
	mt19937 gen(rd());
	shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen); // Losowe permutowanie miast (z wyjątkiem pierwszego)

	int obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow);
	vector<int> najlepsza_trasa = obecna_trasa;
	int najlepszy_koszt = obecny_koszt;

	// Lista tabu jako FIFO (kolejka)
	deque<pair<int, int>> lista_tabu;

	// Licznik odwiedzin miast
	vector<int> licznik_odwiedzin(liczba_miast, 0);

	// Parametry dywersyfikacji
	int liczba_iteracji_bez_poprawy = 0;
	const int prog_stagnacji = 100; // Przykładowy próg stagnacji

	// Rozpoczęcie pomiaru czasu
	auto start = chrono::steady_clock::now();

	while (true) {
		// Sprawdzenie, czy przekroczono czas w sekundach
		auto teraz = chrono::steady_clock::now();
		double czas_uplyniety = chrono::duration_cast<chrono::seconds>(teraz - start).count();
		if (czas_uplyniety >= czas_w_sekundach) {
			break; // Przerwij, jeśli czas został przekroczony
		}

		vector<int> najlepszy_sasiad;
		int najlepszy_koszt_sasiada = INT_MAX;
		pair<int, int> najlepszy_ruch;

		// Przegląd wszystkich sąsiedztw (zamiana dwóch miast w trasie)
		for (int i = 1; i < liczba_miast - 1; ++i) { // Pomijamy pierwsze miasto
			for (int j = i + 1; j < liczba_miast; ++j) {
				vector<int> sasiad = zamien_miasta(obecna_trasa, i, j);
				int koszt_sasiada = oblicz_koszt(sasiad, macierz_kosztow);

				// Penalizacja za często odwiedzane miasta
				int kara_dywersyfikacji = licznik_odwiedzin[obecna_trasa[i]] + licznik_odwiedzin[obecna_trasa[j]];
				koszt_sasiada += kara_dywersyfikacji; // Dodajemy karę do kosztu sąsiada

				// Sprawdzenie, czy ruch (i, j) jest na liście tabu
				bool ruch_na_tabu = find(lista_tabu.begin(), lista_tabu.end(), make_pair(i, j)) != lista_tabu.end();

				// Akceptujemy ruch, jeśli:
				// - Nie jest na liście tabu
				// - Lub poprawia globalne najlepsze rozwiązanie
				if (!ruch_na_tabu || koszt_sasiada < najlepszy_koszt) {
					if (koszt_sasiada < najlepszy_koszt_sasiada) {
						najlepszy_sasiad = sasiad;
						najlepszy_koszt_sasiada = koszt_sasiada;
						najlepszy_ruch = { i, j };
					}
				}
			}
		}

		// Jeśli nie znaleziono żadnego lepszego sąsiada, to wychodzimy z pętli
		if (najlepszy_sasiad.empty()) {
			cout << "Brak lepszego sąsiada, kończymy..." << endl;
			break;
		}

		// Aktualizacja trasy
		obecna_trasa = najlepszy_sasiad;
		obecny_koszt = najlepszy_koszt_sasiada;

		// Aktualizacja licznika odwiedzin dla miast w obecnej trasie
		for (int miasto : obecna_trasa) {
			licznik_odwiedzin[miasto]++;
		}

		// Jeśli poprawiliśmy najlepsze rozwiązanie, aktualizujemy
		if (obecny_koszt < najlepszy_koszt) {
			najlepsza_trasa = obecna_trasa;
			najlepszy_koszt = obecny_koszt;
			liczba_iteracji_bez_poprawy = 0; // Reset stagnacji
		}
		else {
			liczba_iteracji_bez_poprawy++;
		}

		// Dodanie ruchu do listy tabu
		lista_tabu.push_back(najlepszy_ruch);
		if (lista_tabu.size() > dlugosc_listy_tabu) {
			lista_tabu.pop_front(); // Usunięcie najstarszego ruchu
		}

		// Dywersyfikacja: losowy restart po stagnacji
		if (liczba_iteracji_bez_poprawy > prog_stagnacji) {
			shuffle(obecna_trasa.begin() + 1, obecna_trasa.end(), gen);
			obecny_koszt = oblicz_koszt(obecna_trasa, macierz_kosztow);
			liczba_iteracji_bez_poprawy = 0; // Reset stagnacji
			lista_tabu.clear();
		}
	}

	// Zwracamy najlepszą trasę i jej koszt
	cout << "Najlepsza trasa: ";
	for (int miasto : najlepsza_trasa) {
		cout << miasto << " ";
	}
	cout << "\nKoszt: " << najlepszy_koszt << endl;

	return {najlepsza_trasa, najlepszy_koszt};
}

vector<int> wczytaj_sciezke(const string& nazwa_pliku) {
	vector<int> sciezka;
	ifstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		cerr << "Nie udało się otworzyc pliku: " << nazwa_pliku << endl;
		return sciezka;
	}
	int miasto;
	while (plik >> miasto) {
		sciezka.push_back(miasto);
	}
	return sciezka;
}

void oblicz_droge_z_wczytanej_sciezki(const vector<int>& sciezka, const vector<vector<int>>& macierz_kosztow) {
	if (sciezka.empty()) {
		cerr << "Sciezka jest pusta." << endl;
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

	// Dodanie kosztu powrotu do miasta początkowego
	int start = sciezka.back();
	int koniec = sciezka.front();
	if (start >= macierz_kosztow.size() || koniec >= macierz_kosztow.size()) {
		cerr << "Niepoprawny indeks w sciezce: " << start << " -> " << koniec << endl;
		return;
	}

	koszt_calkowity += macierz_kosztow[start][koniec];  // Koszt powrotu

	cout << "Calkowity koszt drogi: " << koszt_calkowity << endl;
}

void zapis_do_pliku_symulowane_wyzarzanie(const string& nazwa_pliku, const vector<int>& najlepsza_trasa_sw, int najlepszy_koszt_sw) {
	ofstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		cerr << "Nie udało się otworzyc pliku: " << nazwa_pliku << endl;
		return;
	}
	// Zapis trasy
	for (int miasto : najlepsza_trasa_sw) {
		plik << miasto << " ";
	}
	// Zapis kosztu
	plik << "\nNajlepszy koszt: " << najlepszy_koszt_sw << endl;
	// Zamknięcie pliku
	plik.close();
}

void zapis_do_pliku_tabu(const vector<int>& najlepsza_trasa, int najlepszy_koszt, const string& nazwa_pliku) {
	// Otwarcie pliku do zapisu
	ofstream plik(nazwa_pliku);
	if (!plik.is_open()) {
		cerr << "Nie udało się otworzyc pliku: " << nazwa_pliku << endl;
		return;
	}
	// Zapis trasy
	for (int miasto : najlepsza_trasa) {
		plik << miasto << " ";
	}
	plik << "\n";
	// Zapis kosztu
	plik << "Koszt: " << najlepszy_koszt << endl;
	// Zamknięcie pliku
	plik.close();
}

int main()
{
	srand(time(0));
	vector<vector<int>> macierz_kosztow;
	int liczba_miast = 0;
	int petla = 1;
	int czas_w_sekundach = 10;
	int dlugosc_listy_tabu = 10;
	double wspolczynnik_a = 0.95;
	vector<int> najlepsza_trasa_sw;
	vector<int> wynik_sw;
	int najlepszy_koszt_sw = 0;
	pair<vector<int>, int> wynik_tabu;
	while (petla == 1) {

		int menu = 0;
		cout << "MENU:\n1. Wczytanie pliku \n2. Wprowadzenie kryterium stopu. \n3. Obliczenie rozwiazania metoda zachłanna \n4. Wybor sasiedztwa. \n5. Algorytm TS.";
		cout << "\n6. Ustawienie wspolczynnika zmiany temperatury dla SW \n7. Algorytm SW. \n8. Zapis sciezki rozwiazania do pliku txt \n9. Wczytanie sciezki z pliku txt i obliczenie drogi na podstawie wczytanej tabeli kosztow" << endl;
		cin >> menu;

		switch (menu) {
		case 1: {
			string nazwa_pliku;
			cout << "Podaj nazwe pliku: ";
			cin >> nazwa_pliku;
			macierz_kosztow = wczytywanie_macierzy(nazwa_pliku, liczba_miast);
			if (!macierz_kosztow.empty()) {
				cout << "Macierz kosztow wczytana pomyslnie.\n";
				wypisanie_macierzy(macierz_kosztow, liczba_miast);
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
				rozwiazanie_zachlanne(macierz_kosztow, liczba_miast);
			}
			break;
		}
		case 4: {
			cout << "Dostepne sasiedztwa dla TS: zamiana dwoch miast.\n";
			break;
		}
		case 5: {
			if (macierz_kosztow.empty()) {
				cout << "Brak wczytanych danych. Najpierw wczytaj dane z pliku.\n";
			}
			else {
				wynik_tabu = tabu_search(macierz_kosztow, czas_w_sekundach, dlugosc_listy_tabu);
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
				wynik_sw = symulowane_wyzarzanie(macierz_kosztow, wspolczynnik_a, czas_w_sekundach, liczba_miast, najlepszy_koszt_sw);
			}
			break;
		}
		case 8: {
				zapis_do_pliku_tabu(wynik_tabu.first, wynik_tabu.second, "wynik_tabu.txt");
				zapis_do_pliku_symulowane_wyzarzanie("wynik_sw.txt", wynik_sw, najlepszy_koszt_sw);
			break;
		}

		case 9: {
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