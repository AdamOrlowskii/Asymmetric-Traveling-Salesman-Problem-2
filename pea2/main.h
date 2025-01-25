#pragma once
#ifndef MAIN
#define MAIN

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

extern int liczba_miast, najlepszy_koszt_z, najlepszy_koszt_ts, najlepszy_koszt_sw, wielkosc_populacji, jakie_krzyzowanie, dlugosc_listy_tabu;
extern vector<int> najlepsza_trasa_z, najlepsza_trasa_ts, najlepsza_trasa_sw, oceny;
extern double wspolczynnik_mutacji, wspolczynnik_krzyzowania, czas_w_sekundach, wspolczynnik_a;
extern vector<vector<int>> populacja, macierz_kosztow;
//Oceny to wektor z kosztami wszystkich aktualnych permutacji algorytmu genetycznego

vector<vector<int>> wczytywanie_macierzy(const string& nazwa_pliku);
void wypisanie_macierzy(const vector<vector<int>>& macierz_kosztow);
int oblicz_koszt(const vector<int>& trasa, const vector<vector<int>>& macierz_kosztow);
vector<int> generuj_sasiedztwo(const vector<int>& trasa);
vector<int> zamien_miasta(const vector<int>& trasa, int miasto1, int miasto2);
vector<int> wczytaj_sciezke(const string& nazwa_pliku);
void oblicz_droge_z_wczytanej_sciezki(const vector<int>& sciezka, const vector<vector<int>>& macierz_kosztow);
void zapis_do_pliku(const string& nazwa_pliku, vector<int>& najlepsza_trasa, int spr);


#endif // !MAIN