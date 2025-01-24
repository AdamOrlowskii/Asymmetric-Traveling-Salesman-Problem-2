#pragma once
#ifndef TABUSEARCH
#define TABUSEARCH

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

extern int liczba_miast, najlepszy_koszt_ts;
extern vector<int> najlepsza_trasa_ts;

class TabuSearch
{
public:
	static void tabu_search(const vector<vector<int>>& macierz_kosztow, int czas_w_sekundach, int dlugosc_listy_tabu);
};

#endif // !TABUSEARCH