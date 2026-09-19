#include <vector>
#include "SudokuGridSquare.h"

void UpdateNumberCandidates(int squareToCheck, std::vector<sudokuGridSquare>& SudokuGrid, int l, std::vector<int>& sudokuBoxesToCheck);

void FillSoleCandidates(int& squareToFill, std::vector<sudokuGridSquare>& SudokuGrid);

void NakedSubsets(std::vector<sudokuGridSquare>& SudokuGrid, std::vector<int>& sudokuBoxesToCheck);

void BlockAndCollumnLineInteraction(std::vector<sudokuGridSquare>& SudokuGrid, std::vector<int>& sudokuBoxesToCheck);

void BlockBlockInteraction(std::vector<sudokuGridSquare>& SudokuGrid, std::vector<int>& sudokuBoxesToCheck);

void HiddenSubsets(std::vector<sudokuGridSquare>& SudokuGrid, std::vector<int>& sudokuBoxesToCheck);

void XWingAndAboveTechniques(std::vector<sudokuGridSquare>& SudokuGrid, std::vector<int>& sudokuBoxesToCheck);

void FirstSorting(std::vector<sudokuGridSquare>& SudokuGrid, std::vector<int>& sudokuBoxesToCheck);

void DisqualifyNumber(std::vector<sudokuGridSquare>& SudokuGrid, int& squareToCheck, int sudokuNumber, std::vector<int>& sudokuBoxesToCheck);

void WriteGrid(std::vector<sudokuGridSquare>& SudokuGrid);
