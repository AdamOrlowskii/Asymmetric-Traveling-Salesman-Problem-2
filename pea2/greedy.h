#pragma once
#ifndef GREEDY_H
#define GREEDY_H

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

extern int liczba_miast, najlepszy_koszt_z;
extern vector<int> najlepsza_trasa_z;

class Greedy
{
	public:
		static void rozwiazanie_zachlanne(const vector<vector<int>>& macierz_kosztow);
};

#endif // !GREEDY_H