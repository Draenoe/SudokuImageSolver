#pragma once
#include "Vector2Structure.h"
#include <vector>
using namespace std;
//Structure for holding island information
struct PixelIsland{
	int number;
	int type;
	int row; // 0= Not calculated
	int collumn; // 0= Not calculated
	Vector2 leftUp;
	Vector2 rightUp;
	Vector2 leftBot;
	Vector2 rightBot;
	float size;
	vector<Vector2> StoredPixels;
	int minX;
	int maxX;
	int minY;
	int maxY;

};