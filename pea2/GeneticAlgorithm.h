#pragma once
#ifndef G
#define G

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

extern int liczba_miast;

class GeneticAlgorithm
{
public:
	static pair<vector<int>, vector<int>> krzyzowanie_ox(const vector<int>& rodzic1, const vector<int>& rodzic2);
	static pair<vector<int>, vector<int>> krzyzowanie_cx(const vector<int>& rodzic1, const vector<int>& rodzic2);
	static vector<int> mutacja_swap(vector<int> permutacja);
	static void inicjalizacja_populacji();
	static vector<int> selekcja_turniejowa(int rozmiar_turnieju);
	static void algorytm_genetyczny(const vector<vector<int>>& macierz_kosztow, int czas_w_sekundach, string nazwa_pliku, bool pierwszy_raz);
};

#endif // !G