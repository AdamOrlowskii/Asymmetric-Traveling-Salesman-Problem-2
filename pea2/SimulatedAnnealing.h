#pragma once
#ifndef SA
#define SA

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

extern int liczba_miast, najlepszy_koszt_sw;
extern vector<int> najlepsza_trasa_sw;

class SimulatedAnnealing
{
public:
	static double oblicz_temperatura_poczatkowa(const vector<int>& trasa, const vector<vector<int>>& macierz_kosztow);
	static void symulowane_wyzarzanie(const vector<vector<int>>& macierz_kosztow, double wspolczynnik_a, int czas_w_sekundach);
};

#endif // !SA