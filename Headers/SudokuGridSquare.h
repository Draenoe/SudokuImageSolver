#pragma once
#include <vector>
using namespace std;
struct sudokuGridSquare {
	int number;
	vector<bool> potentialNum;
	int filledNums;
	bool solved;

};