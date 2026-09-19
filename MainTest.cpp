
// My Own Custom Headers
#include "IslandStructure.h"
#include "SudokuGridSquare.h"
#include "Vector2Structure.h"
#include "MainTest.h"
#include <iostream>
#include <list>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include<cmath>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <filesystem>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <fstream>

//OpenCV stuff
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>




using namespace std;
using namespace cv;


int PixPosMargin = 10;
int PixValueMargin = 10;
int solvedCount = 0;

void CheckBelongingToIsland(vector<int>&PixStatus, vector<int>& PixStatusCurrentLoop, int Pixel, int lastPix, bool islandType, unsigned char* image, vector<int>& pixToCheckTemp)
{
    if (PixStatus[Pixel] == 0 && PixStatusCurrentLoop[Pixel] == 0 && Pixel < lastPix) { //Check we arent out of bounds and the the pixel hasn't been checked
    
        if (islandType ? image[Pixel] > 255 - PixValueMargin*10:image[Pixel] < 255 - PixValueMargin * 10) { // Check if pixel would fit with Island
            pixToCheckTemp.push_back(Pixel);
            PixStatus[Pixel] = 2;
        }
        else {   //doesntFit the island
            PixStatusCurrentLoop[Pixel] = 1;
        }
        
    }
}

void CheckIfCorners(int width, int length, int pixel, Vector2& leftUp, Vector2& rightUp, Vector2& leftBot, Vector2& rightBot)
{

    //ConvertToGrid
    int y;
    int x;

    y = pixel / width;
    x = pixel - width * y;

    //Compartison for Corners
    if (x + y > rightBot.x + rightBot.y) {
        rightBot.x = x;
        rightBot.y = y;
    }
    if (x + y < leftUp.x + leftUp.y) {
        leftUp.x = x;
        leftUp.y = y;

    }
    if (x - y > rightUp.x - rightUp.y) {
        rightUp.x = x;
        rightUp.y = y;
    }
    if (x - y < leftBot.x - leftBot.y) {
        leftBot.x = x;
        leftBot.y = y;

    }
    // so basically  left up needs lowest

}
void CheckIfExtremes(Vector2 pixPos,int& minX, int& maxX,int& minY, int& maxY)
{
    if (pixPos.x > maxX)
        maxX = pixPos.x;
    if (pixPos.x < minX)
        minX = pixPos.x;
    if (pixPos.y > maxY)
        maxY = pixPos.y;
    if (pixPos.y < minY)
        minY = pixPos.y; 

}

Vector2 ConvertToGrid(int pixel,int width) {
   int y = pixel / width;
   int x= pixel - width * y;

    return Vector2{ x,y };

}
int GridToRank(Vector2 Pixel,int width) {
    int rank = Pixel.y * width + Pixel.x;
    if (rank < 0)
        rank = 0;
    return rank;

}


bool CheckIfWithinBounds(Vector2 checkPosition, Vector2 boundUpleft ,   Vector2 boundUpRight,   Vector2 boundBotLeft,Vector2 boundBotRight) {
    if (boundUpleft.x < checkPosition.x && checkPosition.x < boundUpRight.x && boundUpleft.y < checkPosition.y && checkPosition.y < boundBotLeft.y)
        return true;
    else
        return false;
}







void start() {
 

   

}



//::::::: SUDOKU GRID UPDATE FUNCTIONS :::::://

//This function serves to remove a candidate from a given square and fill it in if only one candidate is Left
void DisqualifyNumber(std::vector<sudokuGridSquare>& SudokuGrid, int& squareToCheck, int sudokuNumber)
{
  

   // cout << "PotentialNUm" << SudokuGrid[squareToCheck].potentialNum[sudokuNumber] << endl;

    if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1 && SudokuGrid[squareToCheck].solved==0) { //We check if the number is a candidate for the square

    
        SudokuGrid[squareToCheck].potentialNum[sudokuNumber] = 0;
       
      
        SudokuGrid[squareToCheck].filledNums++;                     //filledNums allows us to tally disqualified candidates 

        //cout << "FILLEDNUMS" << endl << SudokuGrid[squareToCheck].filledNums << endl;
        if (SudokuGrid[squareToCheck].filledNums == 8) {            //When it reaches 8 we know only one candidate is left
           
            FillSoleCandidates(squareToCheck, SudokuGrid);
        }
    }
}

//:::::: This is for when we fill in a number for a square, we can remove the number as a candidate for the box collumn and line it belongs to
void UpdateNumberCandidates(int squareToCheck, std::vector<sudokuGridSquare>& SudokuGrid, int sudokuNumber)
{
    //::::::UPDATE ALL ROW,COLLUMNS AND BOXES::::://
    int line = squareToCheck / 9;
    int collumn = squareToCheck - (line * 9);
    int boxOuterLine = squareToCheck / 27;
    int boxOuterCollumn = collumn / 3;
  
    //We can start with removing candidates in the same line
    for (int lineIndex = 0; lineIndex < 9; lineIndex++) {
        int squareToCheck2 = lineIndex + line * 9;
        
        DisqualifyNumber(SudokuGrid, squareToCheck2, sudokuNumber);
    }
    //Then we do it with collumn
    for (int collumnIndex = 0; collumnIndex < 9; collumnIndex++) {
        int squareToCheck2 = collumn + collumnIndex * 9;

        DisqualifyNumber(SudokuGrid, squareToCheck2, sudokuNumber);

    }

    // Then we can do it with boxes

    for (int boxInnerLine = 0; boxInnerLine < 3; boxInnerLine++) {
        for (int boxInnerCollumn = 0; boxInnerCollumn < 3; boxInnerCollumn++) {
            int squareToCheck2 = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerLine * 9 + boxInnerCollumn;
        
            DisqualifyNumber(SudokuGrid, squareToCheck2, sudokuNumber);
        }
    }
}

void FillSoleCandidates(int& squareToFill, std::vector<sudokuGridSquare>& SudokuGrid)
{
    

     
        
            for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) { // We know there is one number left as a candidate now we need to figure out which number is the right one
                if (SudokuGrid[squareToFill].potentialNum[sudokuNumber] == 1) {
                    SudokuGrid[squareToFill].number = sudokuNumber + 1;
                    SudokuGrid[squareToFill].solved = 1;
                    solvedCount++;
                   
                 
                    UpdateNumberCandidates(squareToFill, SudokuGrid, sudokuNumber);
                    
                }
            }
        
    
}




//::::::::::::::::::::::SUDOKU TECHNIQUES:::::::::::::::::::::://
void NakedSubsets(std::vector<sudokuGridSquare>& SudokuGrid)
{
    //::::::::::::::::::::::::::::::NAKED SUBSETS

    //::: x SQUARES IN A SECTION CONTAIN SOME COMBINATION OF THE SAME x NUMBERS


   
    //NAKED SUBSTET ALT CODE COLLUMN
  
    for (int collumn = 0; collumn < 9; collumn++) {
        
        for (int setSize = 2; setSize < 6; setSize++) { // This allows us to start with pairs then triples, then quads for each collumn
            vector<int> alreadyCountedNumber;
            for (int line = 0; line < 9; line++) {  // We choose one square with the same amount of numbers as the set size and then look for square that hold exclusively candidates that are already in the first square

                int squareToCheck = collumn + line * 9;

                int numberAlreadyCounted = count(alreadyCountedNumber.begin(), alreadyCountedNumber.end(), line);
                if (numberAlreadyCounted != 0)
                    continue;

                if (SudokuGrid[squareToCheck].filledNums >= 9 - setSize) // We want our square to contain as many number or less than our set size
                    continue;


                vector<int> sharedNumbers;
                vector<int> squaresWithSharedNumbersIndex;
                // Now we register the numbers of this square that would be the shared numbers for our subset to compare against
                for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
                    if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1)
                        sharedNumbers.push_back(sudokuNumber);
                }

                squaresWithSharedNumbersIndex.push_back(line);

                for (int line2 = 0; line2 < 9; line2++) {

                    if (line == line2)
                        continue;
                    int numberAlreadyCounted2 = count(alreadyCountedNumber.begin(), alreadyCountedNumber.end(), line2);
                    if (numberAlreadyCounted2 != 0)
                        continue;

                    int squareToCheck2 = collumn + line2 * 9;

                    if (SudokuGrid[squareToCheck2].filledNums >= 9 - setSize)
                        continue;

                    bool numbersMatch = true;

                    //Here we figure out if the numbers inside this square are all a part of the number set
                    for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {

                        bool sharedNumber = false;
                        int cnt = count(sharedNumbers.begin(), sharedNumbers.end(), sudokuNumber);
                        if (cnt != 0)
                            sharedNumber = true;

                        if (sharedNumber == false) {
                            if (SudokuGrid[squareToCheck2].potentialNum[sudokuNumber] == 1) {
                                numbersMatch = false;
                            }
                        }
                    }
                    if (numbersMatch == true) {
                        squaresWithSharedNumbersIndex.push_back(line2);
                    }
                }
                //Now weve got all the squares that only have candidates that match the candidates for our first square
                // Now we can check if there are the same amount of squares as there are candidates

                if (squaresWithSharedNumbersIndex.size() == sharedNumbers.size()) {
                    alreadyCountedNumber.push_back(line);
                    //That means we can start eliminating

                    for (int line2 = 0; line2 < 9; line2++) {

                        // We skip the squares that are a part of the set
                        int cnt = count(squaresWithSharedNumbersIndex.begin(), squaresWithSharedNumbersIndex.end(), line2);
                        if (cnt != 0)
                            continue;

                        int squareToCheck2 = collumn + line2 * 9;
                        alreadyCountedNumber.push_back(line2);

                        //We remove all numbers that are a part of the set from the squares that arent from the set
                        for (int i = 0; i < sharedNumbers.size(); i++) {
                            DisqualifyNumber(SudokuGrid, squareToCheck2, sharedNumbers[i]);
                        }
                    }
                }
            }
        }
    }
    


    //NAKED SUBSTET ALT CODE lINES

    for (int line = 0; line < 9; line++) {
        for (int setSize = 2; setSize < 6; setSize++) { // This allows us to start with pairs then triples, then quads for each collumn
            vector<int> alreadyCountedNumber;
            for (int collumn = 0; collumn < 9; collumn++) {  // We choose one square with the same amount of numbers as the set size and then look for square that hold exclusively candidates that are already in the first square

                int squareToCheck = line * 9 + collumn;

                int numberAlreadyCounted = count(alreadyCountedNumber.begin(), alreadyCountedNumber.end(), collumn);
                if (numberAlreadyCounted != 0)
                    continue;

                if (SudokuGrid[squareToCheck].filledNums >= 9 - setSize) // We want our square to contain as many number or less than our set size
                    continue;


                vector<int> sharedNumbers;
                vector<int> squaresWithSharedNumbersIndex;
                // Now we register the numbers of this square that would be the shared numbers for our subset to compare against
                for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
                    if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1)
                        sharedNumbers.push_back(sudokuNumber);
                }

                squaresWithSharedNumbersIndex.push_back(collumn);

                for (int collumn2 = 0; collumn2 < 9; collumn2++) {

                    if (collumn == collumn2)
                        continue;
                    int numberAlreadyCounted2 = count(alreadyCountedNumber.begin(), alreadyCountedNumber.end(), collumn2);
                    if (numberAlreadyCounted2 != 0)
                        continue;

                    int squareToCheck2 = line * 9 + collumn2;

                    if (SudokuGrid[squareToCheck2].filledNums >= 9 - setSize)
                        continue;

                    bool numbersMatch = true;

                    //Here we figure out if the numbers inside this square are all a part of the number set
                    for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {

                        bool sharedNumber = false;
                        int cnt = count(sharedNumbers.begin(), sharedNumbers.end(), sudokuNumber);
                        if (cnt != 0)
                            sharedNumber = true;

                        if (sharedNumber == false) {
                            if (SudokuGrid[squareToCheck2].potentialNum[sudokuNumber] == 1) {
                                numbersMatch = false;
                            }
                        }
                    }
                    if (numbersMatch == true) {
                        squaresWithSharedNumbersIndex.push_back(collumn2);
                    }
                }
                //Now weve got all the squares that only have candidates that match the candidates for our first square
                // Now we can check if there are the same amount of squares as there are candidates

                if (squaresWithSharedNumbersIndex.size() == sharedNumbers.size()) {
                    alreadyCountedNumber.push_back(collumn);
                    //That means we can start eliminating

                    for (int collumn2 = 0; collumn2 < 9; collumn2++) {

                        // We skip the squares that are a part of the set
                        int cnt = count(squaresWithSharedNumbersIndex.begin(), squaresWithSharedNumbersIndex.end(), collumn2);
                        if (cnt != 0)
                            continue;

                        int squareToCheck2 = line * 9 * collumn2;
                        alreadyCountedNumber.push_back(collumn2);

                        //We remove all numbers that are a part of the set from the squares that arent from the set
                        for (int i = 0; i < sharedNumbers.size(); i++) {
                            DisqualifyNumber(SudokuGrid, squareToCheck2, sharedNumbers[i]);
                        }
                    }
                }
            }
        }
    }


    //NAKED SUBSTET ALT CODE Boxes
    for (int boxOuterLine = 0; boxOuterLine < 3; boxOuterLine++) {


        for (int boxOuterCollumn = 0; boxOuterCollumn < 3; boxOuterCollumn++)
        {

            for (int setSize = 2; setSize < 6; setSize++) { // This allows us to start with pairs then triples, then quads for each collumn
                vector<int> alreadyCountedNumber;
                for (int boxIndex = 0; boxIndex < 9; boxIndex++) {  // We choose one square with the same amount of numbers as the set size and then look for square that hold exclusively candidates that are already in the first square
                    int boxInnerLine = boxIndex / 3;
                    int boxInnerCollumn = boxIndex - boxInnerLine * 3;

                    int squareToCheck = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerLine * 9 + boxInnerCollumn;

                    int numberAlreadyCounted = count(alreadyCountedNumber.begin(), alreadyCountedNumber.end(), boxIndex);
                    if (numberAlreadyCounted != 0)
                        continue;

                    if (SudokuGrid[squareToCheck].filledNums >= 9 - setSize) // We want our square to contain as many number or less than our set size
                        continue;


                    vector<int> sharedNumbers;
                    vector<int> squaresWithSharedNumbersIndex;
                    // Now we register the numbers of this square that would be the shared numbers for our subset to compare against
                    for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
                        if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1)
                            sharedNumbers.push_back(sudokuNumber);
                    }

                    squaresWithSharedNumbersIndex.push_back(boxIndex);

                    for (int boxIndex2 = 0; boxIndex2 < 9; boxIndex2++) {
                        int boxInnerLine2 = boxIndex2 / 3;
                        int boxInnerCollumn2 = boxIndex2 - boxInnerLine2 * 3;


                        if (boxIndex == boxIndex2)
                            continue;
                        int numberAlreadyCounted2 = count(alreadyCountedNumber.begin(), alreadyCountedNumber.end(), boxIndex2);
                        if (numberAlreadyCounted2 != 0)
                            continue;

                        int squareToCheck2 = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerLine2 * 9 + boxInnerCollumn2;

                        if (SudokuGrid[squareToCheck2].filledNums >= 9 - setSize)
                            continue;

                        bool numbersMatch = true;

                        //Here we figure out if the numbers inside this square are all a part of the number set
                        for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {

                            bool sharedNumber = false;
                            int cnt = count(sharedNumbers.begin(), sharedNumbers.end(), sudokuNumber);
                            if (cnt != 0)
                                sharedNumber = true;

                            if (sharedNumber == false) {
                                if (SudokuGrid[squareToCheck2].potentialNum[sudokuNumber] == 1) {
                                    numbersMatch = false;
                                }
                            }
                        }
                        if (numbersMatch == true) {
                            squaresWithSharedNumbersIndex.push_back(boxIndex2);
                        }
                    }
                    //Now weve got all the squares that only have candidates that match the candidates for our first square
                    // Now we can check if there are the same amount of squares as there are candidates

                    if (squaresWithSharedNumbersIndex.size() == sharedNumbers.size()) {
                        alreadyCountedNumber.push_back(boxIndex);
                        //That means we can start eliminating

                        for (int boxIndex2 = 0; boxIndex2 < 9; boxIndex2++) {
                            int boxInnerLine2 = boxIndex2 / 3;
                            int boxInnerCollumn2 = boxIndex2 - boxInnerLine2 * 3;
                            // We skip the squares that are a part of the set
                            int cnt = count(squaresWithSharedNumbersIndex.begin(), squaresWithSharedNumbersIndex.end(), boxIndex2);
                            if (cnt != 0)
                                continue;

                            int squareToCheck2 = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerLine2 * 9 + boxInnerCollumn2;
                            alreadyCountedNumber.push_back(boxIndex2);

                            //We remove all numbers that are a part of the set from the squares that arent from the set
                            for (int i = 0; i < sharedNumbers.size(); i++) {
                                DisqualifyNumber(SudokuGrid, squareToCheck2, sharedNumbers[i]);
                            }
                        }
                    }
                }
            }
        }
    }
}

void BlockAndCollumnLineInteraction(std::vector<sudokuGridSquare>& SudokuGrid)
{
    //:::::BLOCK AND COLLUMN /LINE INTERACTION:::://
    // We check each box, if all potential squares for a number within that box are with the same line or collumn it means place in guaranteed to be in that row or line//
    // That means we can remove that number as a candidate for squares in the same line or collumn that are outside the box//

    // I'm doing both collumns and lines within this script as it's relatively straightforward
    for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {

        for (int boxOuterLine = 0; boxOuterLine < 3; boxOuterLine++) {

            //:::This is where we start scanning within Box:::://
            //Everytime we encounter the number within the box we log that numbers presence in it's collumn and line within the box//
            vector<bool> presenceOfNumberCollumn(3, 0);
            vector<bool> presenceOfNumberLine(3, 0);
            for (int boxOuterCollumn = 0; boxOuterCollumn < 3; boxOuterCollumn++) {

                for (int boxInnerLine = 0; boxInnerLine < 3; boxInnerLine++) {

                    for (int boxInnerCollumn=0; boxInnerCollumn < 3; boxInnerCollumn++) {
                        int squareToCheck = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerLine * 9 + boxInnerCollumn;

                        if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1) {

                            presenceOfNumberCollumn[boxInnerCollumn] = 1;
                            presenceOfNumberLine[boxInnerLine] = 1;

                        }
                    }


                }
                //Now we've got all the data we need from within th box

                // In this loop we figure out in the number only appear in one collumn within the box
                int cntCollumn = count(presenceOfNumberCollumn.begin(), presenceOfNumberCollumn.end(), 1);

                if (cntCollumn == 1) { // That means we can remove that number as a candidate for squares in the same collumn that are outside the box
                    int collumnWithNumber;
                    for (int i = 0; i < 3; i++) {
                        if (presenceOfNumberCollumn[i] == 1)
                            collumnWithNumber = i;
                    }

                    //Now we can go through collumn to eliminate numbers

                    for (int boxOuterLine2 = 0; boxOuterLine2 < 3; boxOuterLine2++) { // We check in boxes above and below
                        if (boxOuterLine2 == boxOuterLine)
                            continue;

                        for (int boxInnerLine2 = 0; boxInnerLine2 < 3; boxInnerLine2++) { // We only check within the same Collumn
                            int squareToCheck = boxOuterLine2 * 27 + boxOuterCollumn * 3 + boxInnerLine2 * 9 + collumnWithNumber;
                            if (SudokuGrid[squareToCheck].solved == 1)
                                continue;

                            DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                        }
                    }
                }
                // We can now do the same thing but with lines only appearing onece within the box
                int cntLine = count(presenceOfNumberLine.begin(), presenceOfNumberLine.end(), 1);

                if (cntLine == 1) { // That means we can remove that number as a candidate for squares in the same collumn that are outside the box
                    int lineWithNumber;
                    for (int i = 0; i < 3; i++) {
                        if (presenceOfNumberLine[i] == 1)
                            lineWithNumber = i;
                    }

                    //Now we can go through collumn to eliminate numbers

                    for (int boxOuterCollumn2 = 0; boxOuterCollumn2 < 3; boxOuterCollumn2++) { // We check in boxes above and below
                        if (boxOuterCollumn2 == boxOuterCollumn)
                            continue;

                        for (int boxInnerCollumn2 = 0; boxInnerCollumn2 < 3; boxInnerCollumn2++) { // We only check within the same Collumn
                            int squareToCheck = boxOuterCollumn2 * 3 + boxOuterLine * 27 + boxInnerCollumn2 + lineWithNumber * 9;
                            if (SudokuGrid[squareToCheck].solved == 1)
                                continue;

                            DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                        }
                    }
                }




            }
        }
    }
}

void BlockBlockInteraction(std::vector<sudokuGridSquare>& SudokuGrid)
{
    //::::::BlockBlockInteractions::::://
    // Baically 2 boxes share the same 2 rows or collumns where a number candidate can exclusively go, that means one box will take one of these rows/Collumns and the otherone will take the leftoverone
    // This means there is no possibility that the third box can contain that number for these collumns as they are already accounted for in the other two boxes meaning we we can remove these squares as candidates.


    for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
        //::::::::HORIZONTAL BOX INTERACTION:::::://
        for (int boxOuterLine = 0; boxOuterLine < 3; boxOuterLine++) {

            //For the Horizontal Box Interaction we work with a line of 3 boxes
            vector<vector<bool>> linesWithNumberBoxes(3, vector<bool>(3, 0));      // This variable stores information on which lines contain the number within each box
            vector<int> lineCountPerBox(3, 0);                                     // This variable serves as a tally, we could use a count on the previous vector as well


            for (int boxOuterCollumn = 0; boxOuterCollumn < 3; boxOuterCollumn++) {
                // We are now Within the box
                // In this loop we check if the number is a candidate within each line of the box
                for (int boxInnerLine = 0; boxInnerLine < 3; boxInnerLine++) {
                    for (int boxInnerLineIndex = 0; boxInnerLineIndex < 3; boxInnerLineIndex++) { // Since were working on lines I used line Index instead of collumn for legibility purposes

                        int squareToCheck = boxOuterCollumn * 3 + boxOuterLine * 27 + boxInnerLine * 9 + boxInnerLineIndex;
                        if (SudokuGrid[squareToCheck].solved == 1)
                            continue;

                        if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1) {
                            linesWithNumberBoxes[boxOuterCollumn][boxInnerLine] = 1;
                            lineCountPerBox[boxOuterCollumn]++;
                            break; // We dont need to check how many we just need to know if the number is present;
                        }
                    }

                }

            }
            //this is the part where we compare the lines
            for (int boxIndex = 0; boxIndex < 2; boxIndex++) { // This is the box that we compare to every other box
                // We check if the box had exactly 2 line with the number as a candidate
                if (lineCountPerBox[boxIndex] != 2)
                    continue;

                for (int boxIndex2 = boxIndex + 1; boxIndex2 < 3; boxIndex2++) { // This is the box we compare the previous box to

                    if (linesWithNumberBoxes[boxIndex] == linesWithNumberBoxes[boxIndex2]) { //If the lines matchup betwwen boxes that means our block/block condition is validated and we can eliminate candidates
                        //::::::here we disqualifyLines from of other box
                        int lineWithoutNumber;
                        int nonCommonBox;

                        for (int i = 0; i < 3; i++) {  // We determine which box in the line we need to eliminate from
                            if (i != boxIndex && i != boxIndex2)
                                nonCommonBox = i;

                        }


                        for (int i = 0; i < 3; i++) {  // We determine which line isn't a part of the 2 lines for which we need to remove candidates
                            if (linesWithNumberBoxes[boxIndex][i] == 0)
                                lineWithoutNumber = i;
                        }

                        for (int line = 0; line < 3; line++) { // Now we eliminate candidate number for relevantsquare
                            if (line == lineWithoutNumber)
                                continue;

                            for (int lineIndex = 0; lineIndex < 3; lineIndex++) {
                                int squareToCheck = boxOuterLine * 27 + nonCommonBox * 3 + line * 9 + lineIndex;
                                if (SudokuGrid[squareToCheck].solved == 0)
                                    continue;

                                DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                            }
                        }
                    }
                }
            }
        }


        //::::::::VERTICAL BLOCK BLOCK INTERACTION:::::::://
        //Since we are doing vertical BlockBlock Interactions we now check collumns of boxex

        for (int boxOuterCollumn = 0; boxOuterCollumn < 3; boxOuterCollumn++) {
            vector<vector<bool>> collumnWithNumberBoxes(3, vector<bool>(3, 0));
            vector<int> collumnCountPerBox(3, 0);

            for (int boxOuterLine = 0; boxOuterLine < 3; boxOuterLine++) {

                //We are now within the box
                // In this loop we check if the number is a candidate within each collumn of the box
                for (int boxInnerCollumn = 0; boxInnerCollumn < 3; boxInnerCollumn++) {

                    for (int boxInnerCollumnIndex = 0; boxInnerCollumnIndex < 3; boxInnerCollumnIndex++) { //we go down the collumn with the index

                        int squareToCheck = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerCollumn + boxInnerCollumnIndex * 9;
                        if (SudokuGrid[squareToCheck].solved == 1)
                            continue;

                        if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1) {  // 
                            collumnWithNumberBoxes[boxOuterLine][boxInnerCollumn] = 1;
                            collumnCountPerBox[boxOuterLine]++;

                            break;
                        }
                    }
                }
            }

            //this is the part where we compare the lines
            for (int boxIndex = 0; boxIndex < 2; boxIndex++) {
                if (collumnCountPerBox[boxIndex] == 2) {

                    for (int boxIndex2 = boxIndex + 1; boxIndex2 < 3; boxIndex2++) {

                        if (collumnWithNumberBoxes[boxIndex] == collumnWithNumberBoxes[boxIndex2]) {


                            int collumnWithoutNumber;
                            int nonCommonBox;

                            for (int i = 0; i < 3; i++) {
                                if (i != boxIndex && i != boxIndex2)
                                    nonCommonBox = i;

                            }


                            for (int i = 0; i < 3; i++) {

                                if (collumnWithNumberBoxes[boxIndex][i] == 0)
                                    collumnWithoutNumber = i;

                            }
                            //::We got through the relevant squares of the third box to eliminate candidate:::://
                            for (int collumn = 0; collumn < 3; collumn++) {
                                if (collumn == collumnWithoutNumber)
                                    continue;

                                for (int collumnIndex = 0; collumnIndex < 3; collumnIndex++) {

                                    int squareToCheck = boxOuterCollumn * 3 + nonCommonBox * 27 + collumn + collumnIndex * 9;
                                    if (SudokuGrid[squareToCheck].solved == 0)
                                        continue;

                                    DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void HiddenSubsets(std::vector<sudokuGridSquare>& SudokuGrid)
{

  
    //:::::::: NAKED SUBSET and HIDDENT SUBSET::::::://
    // This part of the script deals with NakedSubset for pairs as well as those with more numbers
    //Basically we are looking for set of numbers who share the same potential locations ( the amount of locations needs to be equal to the amount of numbers in the set)

    //::::::::::::::HIDDENSUBSET:::::::::::::::::
    //Hiddent subset Collumn Version

    for (int collumn = 0; collumn < 9; collumn++) {
        
      

        vector <vector<int>> numberPositions(9, vector<int>(0, 0));   // the bigger vector container is the number and within you have a vector with it's different positions
      
        for (int line = 0; line < 9; line++) {
            int squareToCheck = collumn + line * 9;
          
            if (SudokuGrid[squareToCheck].solved == 1)
                continue;

            //In this case we operate with a number centric approach
            //We add to the list every possible position for the Number
            for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {

                if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1) {
                    numberPositions[sudokuNumber].push_back(squareToCheck);
                }
            }
        }

   
        //::::::::::::SINGLE SUBSET:::::::::://
        // First we check for single subset we just check if there is only one possibe location within zone for the number
        for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
          
            if (numberPositions[sudokuNumber].size() == 1) {
              
                SudokuGrid[numberPositions[sudokuNumber][0]].solved = 1;
                SudokuGrid[numberPositions[sudokuNumber][0]].number = sudokuNumber+1;
                solvedCount++;

              
              
                UpdateNumberCandidates(numberPositions[sudokuNumber][0], SudokuGrid, sudokuNumber);
             
            }
           
        }

   
       
      
        //:::::::Hidden MULTIPLE Subset::::://
        vector<int> alreadyCountedNumber;
        for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
            bool skipAlreadyCounted = false;
            for (int i = 0; i < alreadyCountedNumber.size(); i++) {
                if (sudokuNumber == alreadyCountedNumber[i])
                    skipAlreadyCounted = true;
            }

            if (skipAlreadyCounted == true)
                continue;
            vector<int> numbersWithSamePositions;
            numbersWithSamePositions.push_back(sudokuNumber);
            for (int sudokuNumber2 = 0; sudokuNumber2 < 9; sudokuNumber2++) {
                //Skip AllreadyCounted elements
                bool skipAlreadyCounted2 = false;
                if (sudokuNumber == sudokuNumber2)
                    skipAlreadyCounted2 = true;
                for (int i = 0; i < alreadyCountedNumber.size(); i++) {
                    if (sudokuNumber2 == alreadyCountedNumber[i])
                        skipAlreadyCounted2 = true;
                }

                if (skipAlreadyCounted2 == true)
                    continue;
                // END OF SKIP


                if (numberPositions[sudokuNumber] == numberPositions[sudokuNumber2]) { // We check if numbers share the same positions
                    numbersWithSamePositions.push_back(sudokuNumber2);
                }

            }
          
            // Here we check if the number set contains as many numbers as possible positions for these sets.
            // If this condition is met only the numbers that are a part of the set can go in these squares so we can eliminate every other number from these locations
            if (numbersWithSamePositions.size() > 1 && numberPositions[sudokuNumber].size() == numbersWithSamePositions.size()) {
              
                for (int i = 0; i < numbersWithSamePositions.size(); i++) {   // We register the numbers that have already accounted for so were not redoing the same set
                    alreadyCountedNumber.push_back(numbersWithSamePositions[i]);
                }

                for (int setSquareIndex = 0; setSquareIndex < numberPositions[sudokuNumber].size(); setSquareIndex++) {

                   
                    int squareToCheck = numberPositions[sudokuNumber][setSquareIndex];

                    //RemoveCandidates Part
                    for (int number = 0; number < 9; number++) {
                        bool skipNumber = false;;

                        for (int i = 0; i < numbersWithSamePositions.size(); i++) { //We skip the numbers that are a part of the set
                            if (numbersWithSamePositions[i] == number) {
                                skipNumber = true;
                            }
                        }

                        if (skipNumber == true)
                            continue;

                       
                        DisqualifyNumber(SudokuGrid, squareToCheck, number);
                    }
                }
            }
        }
   
       
    }
    
    

    //:::::::::::::::HIDDENT SUBSET LINE::::::::::::::::::::://


    for (int line = 0; line < 9; line++) {
      
        vector <vector<int>> numberPositions(9, vector<int>(0, 0));   // the bigger vector container is the number and within you have a vector with it's different positions
        for (int collumn = 0; collumn < 9; collumn++) {
            int squareToCheck = line * 9 + collumn;
            if (SudokuGrid[squareToCheck].solved == 1)
                continue;

            //In this case we operate with a number centric approach
            //We add to the list every possible position for the Number
            for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {

                if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1) {
                    numberPositions[sudokuNumber].push_back(squareToCheck);
                }
            }
        }

        //::::::::::::SINGLE SUBSET:::::::::://
        // First we check for single subset we just check if there is only one possibe location within zone for the number
        for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {

            if (numberPositions[sudokuNumber].size() == 1) {
                SudokuGrid[numberPositions[sudokuNumber][0]].solved = 1;
                SudokuGrid[numberPositions[sudokuNumber][0]].number = sudokuNumber+1;
                solvedCount++;
                UpdateNumberCandidates(numberPositions[sudokuNumber][0], SudokuGrid, sudokuNumber);
            }
        }



        //:::::::Hidden MULTIPLE Subset::::://
        vector<int> alreadyCountedNumber;
        for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
            bool skipAlreadyCounted = false;
            for (int i = 0; i < alreadyCountedNumber.size(); i++) {
                if (sudokuNumber == alreadyCountedNumber[i])
                    skipAlreadyCounted = true;
            }

            if (skipAlreadyCounted == true)
                continue;
            vector<int> numbersWithSamePositions;
            numbersWithSamePositions.push_back(sudokuNumber);
            for (int sudokuNumber2 = 0; sudokuNumber2 < 9; sudokuNumber2++) {
                //Skip AllreadyCounted elements
                bool skipAlreadyCounted2 = false;
                if (sudokuNumber == sudokuNumber2)
                    skipAlreadyCounted2 = true;
                for (int i = 0; i < alreadyCountedNumber.size(); i++) {
                    if (sudokuNumber2 == alreadyCountedNumber[i])
                        skipAlreadyCounted2 = true;
                }

                if (skipAlreadyCounted2 == true)
                    continue;
                // END OF SKIP


                if (numberPositions[sudokuNumber] == numberPositions[sudokuNumber2]) { // We check if numbers share the same positions
                    numbersWithSamePositions.push_back(sudokuNumber2);
                }

            }
            // Here we check if the number set contains as many numbers as possible positions for these sets.
            // If this condition is met only the numbers that are a part of the set can go in these squares so we can eliminate every other number from these locations
            if (numbersWithSamePositions.size() > 1 && numberPositions[sudokuNumber].size() == numbersWithSamePositions.size()) {

                for (int i = 0; i < numbersWithSamePositions.size(); i++) {   // We register the numbers that have already accounted for so were not redoing the same set
                    alreadyCountedNumber.push_back(numbersWithSamePositions[i]);
                }

                for (int setSquareIndex = 0; setSquareIndex < numberPositions[sudokuNumber].size(); setSquareIndex++) {

                    
                    int squareToCheck = numberPositions[sudokuNumber][setSquareIndex];

                    //RemoveCandidates Part
                    for (int number = 0; number < 9; number++) {
                        bool skipNumber = false;;

                        for (int i = 0; i < numbersWithSamePositions.size(); i++) { //We skip the numbers that are a part of the set
                            if (numbersWithSamePositions[i] == number) {
                                skipNumber = true;
                            }
                        }

                        if (skipNumber == true)
                            continue;
                       
                        DisqualifyNumber(SudokuGrid, squareToCheck, number);
                    }
                }
            }
        }
    }

    //::::::HIDDENT SUBSET BOX::::::///

    for (int boxOuterLine = 0; boxOuterLine < 3; boxOuterLine++) {
        
        for (int boxOuterCollumn = 0; boxOuterCollumn < 3; boxOuterCollumn++) {
          
            vector <vector<int>> numberPositions(9, vector<int>(0, 0));   // the bigger vector container is the number and within you have a vector with it's different positions
            for (int boxIndex = 0; boxIndex < 9; boxIndex++) {
                int boxInnerLine = boxIndex / 3;
                int boxInnerCollumn = boxIndex - boxInnerLine * 3;

                int squareToCheck = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerLine * 9 + boxInnerCollumn;
                
                if (SudokuGrid[squareToCheck].solved == 1)
                    continue;

                //In this case we operate with a number centric approach
                //We add to the list every possible position for the Number
                for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
                   
                    if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1) {
                        numberPositions[sudokuNumber].push_back(squareToCheck);
                    }
                   
                }
            }



       
          

            //::::::::::::SINGLE SUBSET:::::::::://
            // First we check for single subset we just check if there is only one possibe location within zone for the number
            for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {

                if (numberPositions[sudokuNumber].size() == 1) {
                    SudokuGrid[numberPositions[sudokuNumber][0]].solved = 1;
                    SudokuGrid[numberPositions[sudokuNumber][0]].number = sudokuNumber+1;
                    solvedCount++;
                    UpdateNumberCandidates(numberPositions[sudokuNumber][0], SudokuGrid, sudokuNumber);
                }
            }

        

            //:::::::Hidden MULTIPLE Subset::::://
            vector<int> alreadyCountedNumber;
            for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
                bool skipAlreadyCounted = false;
                for (int i = 0; i < alreadyCountedNumber.size(); i++) {
                    if (sudokuNumber == alreadyCountedNumber[i])
                        skipAlreadyCounted = true;
                }

                if (skipAlreadyCounted == true)
                    continue;
                vector<int> numbersWithSamePositions;
                numbersWithSamePositions.push_back(sudokuNumber);
                for (int sudokuNumber2 = 0; sudokuNumber2 < 9; sudokuNumber2++) {
                    //Skip AllreadyCounted elements
                    bool skipAlreadyCounted2 = false;
                    if (sudokuNumber == sudokuNumber2)
                        skipAlreadyCounted2 = true;
                    for (int i = 0; i < alreadyCountedNumber.size(); i++) {
                        if (sudokuNumber2 == alreadyCountedNumber[i])
                            skipAlreadyCounted2 = true;
                    }

                    if (skipAlreadyCounted2 == true)
                        continue;
                    // END OF SKIP


                    if (numberPositions[sudokuNumber] == numberPositions[sudokuNumber2]) { // We check if numbers share the same positions
                        numbersWithSamePositions.push_back(sudokuNumber2);
                    }

                }
                // Here we check if the number set contains as many numbers as possible positions for these sets.
                // If this condition is met only the numbers that are a part of the set can go in these squares so we can eliminate every other number from these locations
                if (numbersWithSamePositions.size() > 1 && numberPositions[sudokuNumber].size() == numbersWithSamePositions.size()) {

                    for (int i = 0; i < numbersWithSamePositions.size(); i++) {   // We register the numbers that have already accounted for so were not redoing the same set
                        alreadyCountedNumber.push_back(numbersWithSamePositions[i]);
                    }

                    for (int setSquareIndex = 0; setSquareIndex < numberPositions[sudokuNumber].size(); setSquareIndex++) {

                    
                        int squareToCheck = numberPositions[sudokuNumber][setSquareIndex];


                        //RemoveCandidates Part
                        for (int number = 0; number < 9; number++) {
                            bool skipNumber = false;;

                            for (int i = 0; i < numbersWithSamePositions.size(); i++) { //We skip the numbers that are a part of the set
                                if (numbersWithSamePositions[i] == number) {
                                    skipNumber = true;
                                }
                            }

                            if (skipNumber == true)
                                continue;
                        
                            DisqualifyNumber(SudokuGrid, squareToCheck, number);
                        }
                    }
                }
            }
        }

    }
  
}

void XWingAndAboveTechniques(std::vector<sudokuGridSquare>& SudokuGrid)
{
    //:::::::::::SWORDFISH AND XWING:::::::::::://

    //AltCode SwordFish and Larger for Lines
    cout << "XWING BEGINNING" << endl;
    for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
        vector<vector <int>> linesWithIndexOfAppearances(9, vector<int>(0, 0));
        for (int line = 0; line < 9; line++) { // first we scan each line to figure out each instance of appearances for the number as a candidate

            for (int lineIndex = 0; lineIndex < 9; lineIndex++) {
                int squareToCheck = lineIndex + line * 9;
                if (SudokuGrid[squareToCheck].solved == 1)
                    continue;
                if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1) {
                    linesWithIndexOfAppearances[line].push_back(lineIndex);
                }
            }
        }
        
        //Now we've got every instance of the number for each line now we can create vectors of potential swordfish lines
        
        for (int line = 0; line < 9; line++) {
            vector<int> matchingLines;
            matchingLines.push_back(line);

            for (int line2 = 0; line2 < 9; line2++) {

                if (line2 == line)
                    continue;

                if (linesWithIndexOfAppearances[line].size() < linesWithIndexOfAppearances[line2].size())
                    continue;

                //Now we need to check if the number only belongs to the same collumn
                bool lineIsAMatch = true;

                for (int line2Indexes = 0; line2Indexes < linesWithIndexOfAppearances[line2].size(); line2Indexes++) {

                    int cnt = count(linesWithIndexOfAppearances[line].begin(), linesWithIndexOfAppearances[line].end(), linesWithIndexOfAppearances[line2][line2Indexes]);
                    if (cnt == 0)
                        lineIsAMatch = false;

                }

                if (lineIsAMatch == true)
                    matchingLines.push_back(line2);


            }
            //If this condition is met we can start eliminating candidates
            if (matchingLines.size() == linesWithIndexOfAppearances[line].size()) { // That means we have as many appearances exclusively on the same collumns as there are lines that match
                //Now we need to go through each index  for each collumn (apart for the numbers that are a part of the set) to remove the number as a candidate
                for (int collumnToRemoveFrom = 0; collumnToRemoveFrom < linesWithIndexOfAppearances[line].size(); collumnToRemoveFrom++) {
                    int collumn = linesWithIndexOfAppearances[line][collumnToRemoveFrom];
                    //Now we go down collumn
                    for (int collumnIndex = 0; collumnIndex < 9; collumnIndex++) {
                        //find out if the number is a part of our set
                        int cnt = count(matchingLines.begin(), matchingLines.end(), collumnIndex);
                        if (cnt != 0)
                            continue;
                        int squareToCheck = collumnIndex * 9 + collumn;
                        DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                    }
                }
            }
        }
    }

    
    //AltCode SwordFish and Larger for Collumns

   
    for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
        vector<vector <int>> collumnsWithIndexOfAppearances(9, vector<int>(0, 0));
        for (int collumn = 0; collumn < 9; collumn++) { // first we scan each line to figure out each instance of appearances for the number as a candidate

            for (int collumnIndex = 0; collumnIndex < 9; collumnIndex++) {
                int squareToCheck = collumnIndex * 9 + collumn;
                if (SudokuGrid[squareToCheck].solved == 1)
                    continue;
                if (SudokuGrid[squareToCheck].potentialNum[sudokuNumber] == 1) {
                    collumnsWithIndexOfAppearances[collumn].push_back(collumnIndex);
                }
            }
        }
        //Now we've got every instance of the number for each line now we can create vectors of potential swordfish lines
      
        cout << "XWING end" << endl;
        for (int collumn = 0; collumn < 9; collumn++) {
            vector<int> matchingLines;
            matchingLines.push_back(collumn);

            for (int collumn2 = 0; collumn2 < 9; collumn2++) {

                if (collumn2 == collumn)
                    continue;

                if (collumnsWithIndexOfAppearances[collumn].size() < collumnsWithIndexOfAppearances[collumn2].size())
                    continue;

                //Now we need to check if the number only belongs to the same collumn
                bool lineIsAMatch = true;

                for (int line2Indexes = 0; line2Indexes < collumnsWithIndexOfAppearances[collumn2].size(); line2Indexes++) {

                    int cnt = count(collumnsWithIndexOfAppearances[collumn].begin(), collumnsWithIndexOfAppearances[collumn].end(), collumnsWithIndexOfAppearances[collumn2][line2Indexes]);
                    if (cnt == 0)
                        lineIsAMatch = false;

                }

                if (lineIsAMatch == true)
                    matchingLines.push_back(collumn2);


            }
            //If this condition is met we can start eliminating candidates
            if (matchingLines.size() == collumnsWithIndexOfAppearances[collumn].size()) { // That means we have as many appearances exclusively on the same collumns as there are lines that match
                //Now we need to go through each index  for each collumn (apart for the numbers that are a part of the set) to remove the number as a candidate
                for (int lineToRemoveFrom = 0; lineToRemoveFrom < collumnsWithIndexOfAppearances[collumn].size(); lineToRemoveFrom++) {
                    int line = collumnsWithIndexOfAppearances[collumn][lineToRemoveFrom];
                    //Now we go down collumn
                    for (int lineIndex = 0; lineIndex < 9; lineIndex++) {
                        //find out if the number is a part of our set
                        int cnt = count(matchingLines.begin(), matchingLines.end(), lineIndex);
                        if (cnt != 0)
                            continue;
                        int squareToCheck = lineIndex + line * 9;
                        DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                    }
                }
            }
        }
        cout << "XWING END2" << endl;
    }



}

void FirstSorting(std::vector<sudokuGridSquare>& SudokuGrid)
{
  

    vector<bool> sudokuCandidateReset(9, 1);
    vector<bool> sudokuCandidateTemp(9, 1);

    // Here we sort out candidates by Collumn
    for (int collumn = 0; collumn < 9; collumn++) {


        //Gothrought collumn And sort out which numbers are already filled in it;
        for (int collumnIndex = 0; collumnIndex < 9; collumnIndex++) {

            int squareToCheck = collumn + collumnIndex * 9;
            int sudokuNumber = SudokuGrid[squareToCheck].number;
            
            if (sudokuNumber != 0) { // We could also sort with the solved bool but this works as well since default value for non filled in square is 0
                sudokuCandidateTemp[sudokuNumber-1] = 0;
            }
        }

        //Now with sudokuCandidateTemp we have determined which numbers are no longer available for the collumn so we can remove them for each candidate within the collumn
        for (int collumnIndex = 0; collumnIndex < 9; collumnIndex++) {

            int squareToCheck = collumn + collumnIndex * 9;

           
            if (SudokuGrid[squareToCheck].solved == 1)
                continue;
          

            for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
                
                if (sudokuCandidateTemp[sudokuNumber] == 0) {
                  
                   // cout << "Collumns" << endl;
                    DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                }
        
            }
        }
        

     
       
        sudokuCandidateTemp = sudokuCandidateReset;
    }
  


    

    //Here we sort out candidate by Line no further comments as it works basically the same as the collumn one
   
    for (int line = 0; line < 9; line++) {


        for (int lineIndex = 0; lineIndex < 9; lineIndex++) {

            int squareToCheck = line * 9 + lineIndex;
            int sudokuNumber = SudokuGrid[squareToCheck].number;
            if (sudokuNumber != 0)
                sudokuCandidateTemp[sudokuNumber-1 ] = 0;
        }


        for (int lineIndex = 0; lineIndex < 9; lineIndex++) {

            int squareToCheck = line * 9 + lineIndex;

            if (SudokuGrid[squareToCheck].solved == 1)
                continue;

            for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
                if (sudokuCandidateTemp[sudokuNumber] == 0) {
                  //  cout << "LINES" << endl;
                    DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                }
                   
            }

        }
        sudokuCandidateTemp = sudokuCandidateReset;
    }

  

    //Now we sort out candidates within a box//
    for (int boxOuterLine = 0; boxOuterLine < 3; boxOuterLine++) {

        for (int boxOuterCollumn = 0; boxOuterCollumn < 3; boxOuterCollumn++) {
            //This is where we cycle within the box in a 3X3 grid
            for (int boxInnerLine = 0; boxInnerLine < 3; boxInnerLine++) {

                for (int boxInnerCollumn=0; boxInnerCollumn < 3; boxInnerCollumn++) {

                    int squareToCheck = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerLine * 9 + boxInnerCollumn;
                    int sudokuNumber = SudokuGrid[squareToCheck].number;

                    if (sudokuNumber != 0) {
                        sudokuCandidateTemp[sudokuNumber-1] = 0;
                    }
                }
            }
            //::::End of first Box Cycle now we go through box Again to remove number Candidates::://

            for (int boxInnerLine = 0; boxInnerLine < 3; boxInnerLine++) {

                for (int boxInnerCollumn=0; boxInnerCollumn < 3; boxInnerCollumn++) {

                    int squareToCheck = boxOuterLine * 27 + boxOuterCollumn * 3 + boxInnerLine * 9 + boxInnerCollumn;
                    if (SudokuGrid[squareToCheck].solved == 1)
                        continue;

                    for (int sudokuNumber = 0; sudokuNumber < 9; sudokuNumber++) {
                        if (sudokuCandidateTemp[sudokuNumber] == 0)
                        {
                            //cout << "Boxes" << endl;
                            DisqualifyNumber(SudokuGrid, squareToCheck, sudokuNumber);
                        }
                         
                       
                    }

                }
            }
            sudokuCandidateTemp = sudokuCandidateReset;
        }
    }



}

void WriteGrid(std::vector<sudokuGridSquare>& SudokuGrid,string inputFileName)
{
    //CREATE GRID FOR SOLUTION
    //BOX Info
    int imageSize = 2100;
    int NormalLine = 10;
    int ThickLineRatio = 2;
    int ThickLine = NormalLine * ThickLineRatio;

    int borderMargins = 100;
    unsigned char black = 0;
    unsigned char white = 255;
    int fontSize = 4;
    vector<unsigned char> finalResult(imageSize * imageSize, white);  //Creates a white box

    

    //:::::::Paint the 4 boundaries of the box::::::://

    //Paint left side of Box
    int boxLeftCollumn = borderMargins;
    int boxRightCollumn = borderMargins + ThickLine;
    int boxHigherLine = borderMargins;
    int boxLowerLine = imageSize - borderMargins;

    for (int collumn = boxLeftCollumn; collumn <boxRightCollumn; collumn++) {
        for (int line = boxHigherLine; line < boxLowerLine; line++)
        {
            int pixelToColor = line * imageSize + collumn;
            finalResult[pixelToColor] = black;
        }
    }

   

    boxLeftCollumn = imageSize - borderMargins - ThickLine;
    boxRightCollumn = imageSize - borderMargins;
    boxHigherLine = borderMargins;
    boxLowerLine = imageSize - borderMargins;
    //Paint right side of Box
    for (int collumn = boxLeftCollumn; collumn < boxRightCollumn; collumn++) {
        for (int line = boxHigherLine; line < boxLowerLine; line++)
        {
            int pixelToColor = line * imageSize + collumn;
            finalResult[pixelToColor] = black;
        }
    }
    boxLeftCollumn = borderMargins + ThickLine;
    boxRightCollumn = imageSize - borderMargins - ThickLine;
    boxHigherLine = borderMargins;
    boxLowerLine = borderMargins + ThickLine;
    //Paint Top of Box
    for (int collumn = boxLeftCollumn; collumn < boxRightCollumn; collumn++) {
        for (int line = boxHigherLine; line < boxLowerLine; line++)
        {
            int pixelToColor = line * imageSize + collumn;
            finalResult[pixelToColor] = black;
        }
    }
    boxLeftCollumn = borderMargins + ThickLine;
    boxRightCollumn = imageSize - borderMargins - ThickLine;
    boxHigherLine = imageSize - borderMargins - ThickLine;
    boxLowerLine = imageSize - borderMargins;
    //Paint Bottom of Box
    for (int collumn = boxLeftCollumn; collumn < boxRightCollumn; collumn++) {
        for (int line = boxHigherLine; line < boxLowerLine; line++)
        {
            int pixelToColor = line * imageSize + collumn;
            finalResult[pixelToColor] = black;
        }
    }

   

    //:::::::::::::MAKE THE GRID LINES:::::::::::://
    //Make 8 horizontal grid lines
    int squareSize = (imageSize - ( borderMargins * 2 ) - (ThickLine * 4) - (NormalLine * 6))/9;
    int horizontalLeftCollumn = borderMargins + ThickLine;
    int horizontalRightCollumn = imageSize - borderMargins - ThickLine;
    int numberOfThickLines=0;
    int lineThickCounter=0;

  
    for (int gridLine = 1; gridLine < 9; gridLine++) {
       
        bool drawThickLine = false;
        lineThickCounter++;
        if (lineThickCounter == 3) {
            lineThickCounter = 0;
            drawThickLine = true;
        }
        int highestLine;
        int lowestLine;
     
      
        if (drawThickLine == true) {
             highestLine = borderMargins + ThickLine + gridLine * squareSize + NormalLine * (gridLine - 1 - numberOfThickLines) + numberOfThickLines * ThickLine;
             lowestLine = borderMargins + ThickLine + gridLine * squareSize + NormalLine * (gridLine - 1 - numberOfThickLines) + numberOfThickLines * ThickLine + ThickLine;
        }
        else{
           highestLine = borderMargins + ThickLine + gridLine * squareSize + NormalLine * (gridLine - 1 - numberOfThickLines) + numberOfThickLines * ThickLine;
             lowestLine = borderMargins + ThickLine + gridLine * squareSize + NormalLine * (gridLine - numberOfThickLines) + numberOfThickLines * ThickLine;
        }

      

        if (drawThickLine == true)
            numberOfThickLines++;
    
   

        for (int collumn = horizontalLeftCollumn; collumn < horizontalRightCollumn; collumn++) {
            for (int line = highestLine; line < lowestLine; line++) {
                
                int pixelToColor = line * imageSize + collumn;
                finalResult[pixelToColor] = black;
            }
        }
    }


    //Make 8 vertical grid lines
    int verticalTopLine = borderMargins + ThickLine;
    int verticalBottomLine = imageSize - borderMargins - ThickLine;
    numberOfThickLines = 0;
    lineThickCounter = 0;
    for (int gridCollumn = 1; gridCollumn < 9; gridCollumn++) {

        bool drawThickLine = false;
        lineThickCounter++;
        if (lineThickCounter == 3) {
            lineThickCounter = 0;
            drawThickLine = true;
        }
        int verticalLeftCollumn;
        int verticalRightCollumn;





        int numberOfThickCollumns = (gridCollumn - 1) / 3;
       
        if (drawThickLine == true) {
             verticalLeftCollumn = borderMargins + ThickLine + gridCollumn * squareSize + NormalLine * (gridCollumn - 1 - numberOfThickLines) + numberOfThickLines * ThickLine;
             verticalRightCollumn = borderMargins + ThickLine + gridCollumn * squareSize + NormalLine * (gridCollumn-1 - numberOfThickLines) + numberOfThickLines * ThickLine+ThickLine;

        }
        else {
             verticalLeftCollumn = borderMargins + ThickLine + gridCollumn * squareSize + NormalLine * (gridCollumn - 1 - numberOfThickLines) + numberOfThickLines * ThickLine;
             verticalRightCollumn = borderMargins + ThickLine + gridCollumn * squareSize + NormalLine * (gridCollumn - numberOfThickLines) + numberOfThickLines * ThickLine;
        }



        if (drawThickLine == true)
            numberOfThickLines++;


        for (int collumn = verticalLeftCollumn; collumn < verticalRightCollumn; collumn++) {

            for (int line = verticalTopLine; line < verticalBottomLine; line++) {
                int pixelToColor = line*imageSize + collumn;
                finalResult[pixelToColor] = black;

            }


        }
    }

  



    //ADDING NUMBERS TO THE PICTURE
    
    int offsetForCentering = (squareSize ) / 2;
    Mat image2(imageSize, imageSize, CV_8UC1, finalResult.data());

    for (int line = 0; line < 9; line++) {
        int lineThickLines = line / 3;
        int squareToImageLine = borderMargins + ThickLine + NormalLine * (line - lineThickLines) + lineThickLines * ThickLine + line * squareSize + offsetForCentering;
        for (int collumn = 0; collumn < 9; collumn++) {
            int collumnThickLines = collumn / 3;
            int squareToFillIndex = line * 9 + collumn;
            int numberForSquare = SudokuGrid[squareToFillIndex].number;
            int squareToImageCollumn = borderMargins + ThickLine + NormalLine * (collumn - collumnThickLines) + collumnThickLines * ThickLine + collumn * squareSize + offsetForCentering;

            string numString = to_string(numberForSquare);
            

    
            // Writing over the Image
            Point origin(squareToImageCollumn, squareToImageLine);
            putText(image2, numString, origin,
                FONT_HERSHEY_DUPLEX, fontSize,
                Scalar(0, 0, 0), 2, LINE_AA);

            // Show our image 


        }
    }
    cout << "WRITING IMAGE" << endl;
    string filenameWrite ="output\\"+inputFileName + "_Solved.png";
    stbi_write_png(filenameWrite.c_str(), imageSize, imageSize, 1, finalResult.data(), imageSize);
}

void main() {
      
//Variables that serve as location to store Image data.
    int width=100;      
    int height;    
    int channels;


// Fetch images
    string fileName;
    cout << endl << "Please Input The name of the sudoku file within the images folder (without the file extension) you want to solve" << endl;
    cin >> fileName;    
    string filePath = "images\\"+fileName+".png";
    unsigned char* image = stbi_load(
        filePath.c_str(),
        &width,
        &height,
        &channels,
        1 // Makes it Grayscale
    );
        

 
 
//Island Identifier
//Create a list of pixel Status to help when scanning.
 vector<int> PixStatus; //0 = Not Scanned, 1 = check neighbours , 2= completed
 vector<int> PixStatusCurrentLoop;
 vector<PixelIsland> valueIslands;
 int lastPix=width*height;
 int floodInd;
 bool islandType;
 bool islandFormed;
 vector<Vector2> StoredPixels;
 
//fill List
    for (int i = 0; i < width * height; i++) {
        PixStatus.push_back(0);
       
    }
    PixStatusCurrentLoop = PixStatus;



    for (int ind = 0; ind < width * height; ind++) { // Begin Island formation
     

   

        if (PixStatus[ind] == 0) { //Start scanning island
            Vector2 leftUp = { width,height };
            Vector2 rightUp = { -width,height };
            Vector2 leftBot = { width,-height };
            Vector2 rightBot = { -width,-height };
            int bigXYSum = 0;
            int smallXYSum = lastPix;
            islandFormed = 0;
            StoredPixels.clear();
            int minX = width * height;
            int maxY = 0;
            int minY = width * height;
            int maxX = 0;
        
            //Find out What kind of island we are scanning
            if (image[ind] > 255 - PixValueMargin*10) {
                islandType = 1; //White
                
            }
            else {
                islandType = 0;  //Black
                StoredPixels.push_back(ConvertToGrid(ind, width));
            }
            PixStatus[ind] = 2;
            //IslandChecker
            vector<int> pixToCheck;
            vector<int> pixToCheckTemp;
            pixToCheck.push_back(ind);

            CheckIfCorners( width, height, ind,  leftUp,  rightUp,  leftBot,  rightBot);

         
            int islandSize=0;
            while (islandFormed == 0) {
                int lowerPixel;
                int nextPixel;
                int upperPixel;
                int prevPixel;
                for (int i = 0; i < pixToCheck.size(); i++ ) {

                    
                   
                     lowerPixel= pixToCheck[i] + width;
                     nextPixel= pixToCheck[i] + 1;
                     upperPixel = pixToCheck[i] - width;
                     prevPixel = pixToCheck[i] -1;
                     if (prevPixel>0)
                     CheckBelongingToIsland(PixStatus, PixStatusCurrentLoop, prevPixel, lastPix, islandType, image, pixToCheckTemp);
                     if (lowerPixel < lastPix)
                     CheckBelongingToIsland(PixStatus, PixStatusCurrentLoop, lowerPixel, lastPix, islandType, image, pixToCheckTemp);
                     if(nextPixel<lastPix)
                     CheckBelongingToIsland(PixStatus, PixStatusCurrentLoop, nextPixel, lastPix, islandType, image, pixToCheckTemp);
                     if(upperPixel>0)
                     CheckBelongingToIsland(PixStatus, PixStatusCurrentLoop, upperPixel, lastPix, islandType, image, pixToCheckTemp);
                  
                }
              
                for (int i = 0; i < pixToCheckTemp.size(); i++) {
                    islandSize++;
                    CheckIfCorners(width, height, pixToCheckTemp[i], leftUp, rightUp, leftBot, rightBot);
                    
                    if (islandType == 0) { // We store only black pixels

                        Vector2 pixelConvertedToGrid = ConvertToGrid(pixToCheckTemp[i], width);
                        CheckIfExtremes(pixelConvertedToGrid, minX, maxX, minY, maxY);
                        StoredPixels.push_back(pixelConvertedToGrid);
                    }
                }
                pixToCheck = pixToCheckTemp;
                pixToCheckTemp.clear();

                if ( pixToCheck.size() == 0) { //Means done with Island
                    islandFormed = 1;
                    float size = pow((leftUp.x - rightUp.x), 2)+pow((leftUp.y - rightUp.y), 2);
                    PixelIsland islandTemp = {0,islandType,0,0,leftUp,rightUp,leftBot,rightBot,size,StoredPixels,minX, maxX, minY, maxY };
                 
                   // cout << endl<<"NewIsland" << endl << "LeftupX" << leftUp.x << endl << "LestUpY" << leftUp.y;
                   // cout << endl << "islandsize" << islandSize;
                    //cout << endl << "islandType" <<islandType<< endl;
                    valueIslands.push_back(islandTemp);
                    fill(PixStatusCurrentLoop.begin(), PixStatusCurrentLoop.end(), 0);
                }
            }
        }
     
    } //End of pixel Scanning
  
    int numberOfWhiteIslands=0;

    int numberOfBlackIslands = 0;
    for (int i = 0; i < valueIslands.size(); i++) {
        if (valueIslands[i].type == 1) {
            numberOfWhiteIslands++;
        }
   
    }
    for (int i = 0; i < valueIslands.size(); i++) {
        if (valueIslands[i].type == 0) {
            numberOfBlackIslands++;
        }

    }
    //cout << endl << "NUMBER OF WHITE ISLANDS"<<numberOfWhiteIslands;
   // cout << endl << "NUMBER OF Black ISLANDS" << numberOfBlackIslands;

    //Start of Island Sorting
    int biggestSizeBlack=0;
    int biggestSizeBlackRank=0;
    int biggestSizeWhite=0;
    int biggestSizeWhiteRank=0;
    for (int i = 0; i < valueIslands.size(); i++) {
        if(valueIslands[i].type==0){
            if (valueIslands[i].size > biggestSizeBlack) {
                biggestSizeBlackRank = i;
                biggestSizeBlack = valueIslands[i].size;
            }
        }
        if (valueIslands[i].type == 1) {
            if (valueIslands[i].size > biggestSizeWhite) {
                biggestSizeWhiteRank = i;
                biggestSizeWhite = valueIslands[i].size;
            }
        }
    }


    valueIslands[biggestSizeWhiteRank].type = 3;
    valueIslands[biggestSizeBlackRank].type = 4;

    //gotta sort out with size which islands are square through and average or something
    // Add a tenth margin as a contingency
    int averagSizeWhite=0;
    int whiteZoneCount=0;
    // Sort out number of white zones remaining and average size
    for (int i = 0; i < valueIslands.size(); i++) {
        if (valueIslands[i].type == 1) {
            averagSizeWhite = averagSizeWhite + valueIslands[i].size;
            whiteZoneCount++;
        }   
    }

    int averagSizeBlack = 0;
    int blackZoneCount = 0;
    for (int i = 0; i < valueIslands.size(); i++) {
        if (valueIslands[i].type == 0) {
            averagSizeBlack = averagSizeBlack + valueIslands[i].size;
            blackZoneCount++;
        }
    }
    int newBlackCound = 0;
 
    for (int i = 0; i < valueIslands.size(); i++) {
        if (valueIslands[i].type == 0 && valueIslands[i].size > 1) {
            valueIslands[i].type = 6;
           
        }
    }
    averagSizeWhite = averagSizeWhite / whiteZoneCount;
    //cout << endl << "AVERAGEWhiteSize" << averagSizeWhite << endl << "WHiteZONECound" << whiteZoneCount;
    //Seperate white sqares from gaps in between letters
    int gridSquaresCount=0;


    for (int i = 0; i < valueIslands.size(); i++) {
        if (valueIslands[i].type == 1 && valueIslands[i].size>averagSizeWhite*0.9) {
            valueIslands[i].type = 5;
            gridSquaresCount++;
        }
    }
    
    int rowCount=sqrt(gridSquaresCount);
    int smallestXYSum=width*height;
    int smallestXYSumRank=1;
    for (int i = 0; i < valueIslands.size(); i++) {
        if (valueIslands[i].type == 5) {
            int XYSum = valueIslands[i].leftUp.x + valueIslands[i].leftUp.y;
            if (XYSum < smallestXYSum) {
                smallestXYSum = XYSum;
                smallestXYSumRank = i ;
            }

        }
       
    }

    valueIslands[smallestXYSumRank].collumn = 1;
    valueIslands[smallestXYSumRank].row = 1;

    // Now that we've got base 0 and rows and collumns we can infer their placement
    int maxDistanceX=0;
    int maxDistanceY=0;
    int firstSqrX=valueIslands[smallestXYSumRank].leftUp.x;
    int firstSqrY=valueIslands[smallestXYSumRank].leftUp.y;
    
    for (int i = 0; i < valueIslands.size(); i++) {

        if (valueIslands[i].type == 5) {
            int distanceX = valueIslands[i].leftUp.x - firstSqrX;
            if (distanceX >maxDistanceX ) 
                maxDistanceX = distanceX;

            //We also do Y to account for mishaps that can happen with a photo that's not perfecttly forward facing
            int distanceY = valueIslands[i].leftUp.y - firstSqrY;
            if (distanceY > maxDistanceY)
                maxDistanceY = distanceY;

        }
    }

    float avgDistanceX=maxDistanceX/(rowCount-1);
    float avgDistanceY=maxDistanceY/(rowCount-1);

    //Yay! we have all the information we need to sort out the order now
    for (int i = 0; i < valueIslands.size(); i++) {

        if (valueIslands[i].type == 5) {
            valueIslands[i].row = round((valueIslands[i].leftUp.y - firstSqrY) / avgDistanceY)+1;
            valueIslands[i].collumn = round((valueIslands[i].leftUp.x - firstSqrX) / avgDistanceX)+1;

          
        }
    }



  
    //Now we can determin row and collumns for contents of these boxes
    //First step we determine groups of rows

  
    for (int i = 0; i < valueIslands.size(); i++) {

        if (valueIslands[i].type == 1) {
        
            for (int l = 0; l < valueIslands.size(); l++) {

                if (valueIslands[l].type == 5) {
                    //Check if black pixels are within bounds
                    if (valueIslands[l].leftUp.x > valueIslands[i].leftUp.x > valueIslands[l].rightUp.x && valueIslands[l].leftUp.y > valueIslands[i].leftUp.y > valueIslands[l].leftBot.y) {
                        valueIslands[i].row = valueIslands[l].row;
                        valueIslands[i].collumn = valueIslands[l].collumn;
                    }
                }
            }
        }
    }


    //We need to filter out smaller black islands
    // Then figure out the bounds

   
    vector<bool> numCandidateTemplate(9,1);
    sudokuGridSquare squareTemplate{ 0,numCandidateTemplate,0,0 };
    vector<sudokuGridSquare> SudokuGrid(81,squareTemplate);

    int numberPadding=60;

   
    for (int i = 0; i < valueIslands.size(); i++) {

        if (valueIslands[i].type == 6) {


            for (int l = 0; l < valueIslands.size(); l++) {

                if (valueIslands[l].type == 5) {

                    if (CheckIfWithinBounds(valueIslands[i].leftUp, valueIslands[l].leftUp, valueIslands[l].rightUp, valueIslands[l].leftBot, valueIslands[l].rightBot))
                    {
                        bool personalCharacterReader =true;

                        if(personalCharacterReader==true)
                        {
                            //cout << "BEGINNING";


                            //PERSONAL IMAGE RECOGNITION
                           
                            Vector2 point0NoMargins = { valueIslands[i].minX , valueIslands[i].minY };
                            int minMaxDistXNoMargins = valueIslands[i].maxX - valueIslands[i].minX;
                            int minMaxDistYNoMargins = valueIslands[i].maxY - valueIslands[i].minY;
                            int boxSizeNoMargins;
                            int boxSizeY;
                            int boxSizeX;
                            if (minMaxDistXNoMargins > minMaxDistYNoMargins)
                                boxSizeNoMargins = minMaxDistXNoMargins+1;
                            else
                                boxSizeNoMargins = minMaxDistYNoMargins+1;

                            boxSizeY = minMaxDistYNoMargins + 1;
                            boxSizeX = minMaxDistXNoMargins + 1;

                            for (int u = 0; u < valueIslands[i].StoredPixels.size(); u++) {
                                valueIslands[i].StoredPixels[u].x = valueIslands[i].StoredPixels[u].x - point0NoMargins.x;
                                valueIslands[i].StoredPixels[u].y = valueIslands[i].StoredPixels[u].y - point0NoMargins.y;
                            }
                        
                            vector<unsigned char> numberImageNoMargins(boxSizeX* boxSizeY, 255);
                           
                           // cout << "STORED PIXELS SIZE" << endl << valueIslands[i].StoredPixels.size() << endl;
                           // cout << GridToRank(valueIslands[i].StoredPixels[0], boxSizeX);
                            for (int u = 0; u < valueIslands[i].StoredPixels.size(); u++) {
                             
                                numberImageNoMargins[GridToRank(valueIslands[i].StoredPixels[u], boxSizeX)] = 0;
                            }

                           // cout << "END";
                            string filename = "debugImages\\extractedNumber_" + to_string(i) + ".png";
                            stbi_write_png(filename.c_str(), boxSizeX, boxSizeY, 1, numberImageNoMargins.data(), boxSizeX);
                            //END OF IMAGE CREATION
                           
                           // START OF CUSTOM IMAGE RECOGNITION 
                           // WE SAMPLE HOW FAR THE NUMBERS PIXELS ARE FROM THE BORDER ON THE LEFT, TOP AND RIGHT
                           // WE CAN USE THIS INFORMATION TO IDENTIFY WHICH NUMBER WE ARE DEALING WITH BASED ON UNIQUE CHARACTERISTICS
                            int noiseMargin = 1;
                            int tenthOfSize = (boxSizeY - noiseMargin * 2) / 9;
                            
                            //Sample Top leftmost and rightmost Pixel
                            int topLine = noiseMargin;
                            int bottomLine = boxSizeY - noiseMargin;
                            int sudokuNumber = 0;
                            vector<int> distancesToBorderLeft;
                            vector<int> distancesToBorderRight;


                            //HigherSampling
                            vector<int> distancesToBorderLeft20;
                            vector<int> distancesToBorderRight20;

                            int size20th;

                            int biggestDistanceLeft = 0;
                            int biggestDistanceRight = 0;
                            int lowestDistanceRight = 0;
                            int lowestDistanceLeft= 0;

                            int biggestDistanceLeftIndex=0;
                            int biggestDistanceRightIndex=0;
                            int lowestDistanceRightIndex = 0;
                            int lowestDistanceLeftIndex = 0;


                            //leftSampling
                            for (int line20th = 0; line20th < 20; line20th++) {
                                size20th= (boxSizeY - noiseMargin * 2) *0.05*line20th;
                                int line = size20th + noiseMargin;
                                int startingPixelLine = line * boxSizeX;
                                for (int lineIndex = 0; lineIndex < boxSizeX; lineIndex++) {

                                    int pixelToCheck = startingPixelLine + lineIndex;
                                    if (numberImageNoMargins[pixelToCheck] == 0) { //Found the blackPixel;
                                        int currentDistance = lineIndex;
                                        distancesToBorderLeft20.push_back(lineIndex);
                                    ///   cout << "LeftDISTANCES" << endl << lineIndex<< endl;
                                        if (currentDistance > biggestDistanceLeft) {
                                            biggestDistanceLeftIndex = line20th;
                                            biggestDistanceLeft = currentDistance;

                                        }
                                        if (currentDistance < lowestDistanceLeft) {
                                            lowestDistanceLeftIndex = line20th;
                                            lowestDistanceLeft = currentDistance;
                                        }
                                        break;
                                    }
                                }
                            }
                            for (int line20th = 0; line20th < 20; line20th++) {
                                size20th = (boxSizeY - noiseMargin * 2) * 0.05 * line20th;
                                int line = size20th + noiseMargin;
                                int startingPixelLine = line * boxSizeX;
                                for (int lineIndex = boxSizeX; lineIndex > 0; lineIndex--) {

                                    int pixelToCheck = startingPixelLine + lineIndex;
                                    if (numberImageNoMargins[pixelToCheck] == 0) { //Found the blackPixel;
                                        int currentDistance = boxSizeX - lineIndex;
                                        distancesToBorderRight20.push_back(boxSizeX - lineIndex);
                                      //  cout << "RIGHTDISTANCES" << endl << boxSizeX - lineIndex << endl;
                                        if (currentDistance > biggestDistanceRight) {
                                            biggestDistanceRightIndex = line20th;
                                            biggestDistanceRight = currentDistance;
                                        }
                                        if (currentDistance < lowestDistanceRight) {
                                            lowestDistanceRightIndex = line20th;
                                            lowestDistanceRight = currentDistance;
                                        }

                                        break;
                                    }
                                }
                            }


                         //   cout << "biggestDistanceLeftIndex" << endl << biggestDistanceLeftIndex << endl << biggestDistanceLeft << endl;

                          //  cout << "biggestDistanceRightIndex" << endl << biggestDistanceRightIndex << endl << biggestDistanceRight << endl;

                            //7,8,6,1,9,5,3




                            //:::for 7:::://
                            if (biggestDistanceLeftIndex < 4 && biggestDistanceRightIndex>17)
                                sudokuNumber = 7;


                            //:::for 5::://
                            if (sudokuNumber == 0) {
                                if (biggestDistanceLeftIndex > 10 && biggestDistanceRightIndex < 10) {
                                    int biggestDistanceRight = distancesToBorderRight20[biggestDistanceRightIndex];
                                    int biggestDistanceLeft = distancesToBorderLeft20[biggestDistanceLeftIndex];
                                    if (biggestDistanceLeft > biggestDistanceRight * 0.8 && biggestDistanceRight > biggestDistanceLeft * 0.8) {
                                        sudokuNumber = 5;

                                    }
                                }

                            }


                            //Checkfor 1 and 4
                            int onSameCollumnCountLeft = 0;
                            int onSameCollumnCountRight = 0;
                            for (int line = 19; line >= 0; line--) {
                                
                                if (abs(distancesToBorderRight20[2] - distancesToBorderRight20[line]) < 2)
                                    onSameCollumnCountRight++;

                                if (abs(distancesToBorderLeft20[17] - distancesToBorderLeft20[line]) < 2)
                                    onSameCollumnCountLeft++;


                            }

                           // cout << onSameCollumnCountRight << "SAMECOLLUMNRight" << endl;
                            if (onSameCollumnCountRight > 14 && onSameCollumnCountLeft > 12) {
                                sudokuNumber = 1;
                            }
                            else if (onSameCollumnCountRight > 13)
                                sudokuNumber = 4;

                           
                            int marginOfErrorFor8 = 3;
                            int marginOfErrorFor1 = 2;










                            if (sudokuNumber == 0) {
                                //CHECKFOR 8
                           //:::Check for 8 and 1:::://
                         
                                int equalDistanceLRCount = 0;
                                bool numberIs6or9 = false;
                                for (int line = 0; line < distancesToBorderLeft20.size(); line++) {
                                    if (abs(distancesToBorderRight20[line] - distancesToBorderLeft20[line]) < marginOfErrorFor8)
                                        equalDistanceLRCount++;
                                }
                                

                              
                                if (equalDistanceLRCount > 15) { // Now we distinguish if it's 8 or 1
                                    if (distancesToBorderRight20[biggestDistanceRightIndex] > boxSizeX / 2)
                                        sudokuNumber = 6;
                                    else if (distancesToBorderLeft20[biggestDistanceLeftIndex] > boxSizeX / 2) {
                                        sudokuNumber = 9;
                                    }
                                    else {
                                     
                                      
                                            sudokuNumber = 8;
                                        
                                    }
                                }

                            }

                            if (7 > lowestDistanceRightIndex > 11) {
                           

                            }
                       


                            if (sudokuNumber == 0) {
                                //::for 3 ::://
                                bool largeDistTop = false;
                                bool largeDistBottom = false;
                                if (biggestDistanceLeft > boxSizeX / 2) {


                                    for (int line = 0; line < distancesToBorderLeft20.size(); line++) {

                                        if (line > 10) {
                                            if (distancesToBorderLeft20[line] > biggestDistanceLeft * 0.9) {
                                          
                                                largeDistBottom = true;

                                            }

                                        }
                                        else {
                                            if (distancesToBorderLeft20[line] > biggestDistanceLeft * 0.9) {
                                               
                                                largeDistTop = true;
                                            }

                                        }

                                    }

                                }
                                if (largeDistTop == true && largeDistBottom == true) {
                                    sudokuNumber = 3;
                                }

                                if (largeDistTop == true && biggestDistanceRightIndex > 17)
                                    sudokuNumber = 2;

                            }
                        
                            if (sudokuNumber == 0) {
                                if (biggestDistanceLeftIndex > 9 && biggestDistanceLeft > boxSizeX / 2)
                                    sudokuNumber = 9;
                            }


                            if (sudokuNumber == 0) {
                                if (biggestDistanceRightIndex < 10 && biggestDistanceRight > boxSizeX / 2)
                                    sudokuNumber = 6;
                            }

                            
                            if (sudokuNumber == 0) {
                                if (biggestDistanceRightIndex > 15 && biggestDistanceRight > boxSizeX / 2 && biggestDistanceLeftIndex<9)
                                    sudokuNumber = 2;
                            }
                       




                           
                            int sudokuGridRank = valueIslands[l].collumn - 1 + (valueIslands[l].row - 1) * 9;
                            SudokuGrid[sudokuGridRank].number = sudokuNumber;
                            SudokuGrid[sudokuGridRank].solved = true;
                            solvedCount++;
                            //cout << "SUDOKU GRID RANK" << endl << sudokuGridRank << "Sudoku Number" << endl << sudokuNumber << endl;
                           // cout << endl << endl << endl << endl << endl << "WE ARE DEALING WITH IMAGE" << sudokuGridRank << endl;
                            //END OF PERSONAL IMAGE REGOGNITION
                        
                        }
                       else { //To keep Tesseract code for tests I kepts a bool switch to manually swap between methods (Tesseract kept getting the numbers wrong(!Really bad for a sudoku solver) and took too long so I just wrote my own code)
                           //cout << endl << "BlackIslandLeftUp" << endl << valueIslands[i].leftUp.x << endl;
                       //cout << endl << "NormalIslandLeftuo" << endl << valueIslands[l].leftUp.x << endl<< endl << valueIslands[l].rightUp.x << endl;
                       Vector2 point0 = { valueIslands[i].minX - numberPadding / 2, valueIslands[i].minY - numberPadding / 2 };

                       int minMaxDistX = valueIslands[i].maxX - valueIslands[i].minX + numberPadding;

                       int minMaxDistY = valueIslands[i].maxY - valueIslands[i].minY + numberPadding;

                       int boxSize;

                       if (minMaxDistX > minMaxDistY)
                           boxSize = minMaxDistX;
                       else
                           boxSize = minMaxDistY;

                       for (int u = 0; u < valueIslands[i].StoredPixels.size(); u++) {
                           valueIslands[i].StoredPixels[u].x = valueIslands[i].StoredPixels[u].x - point0.x;
                           valueIslands[i].StoredPixels[u].y = valueIslands[i].StoredPixels[u].y - point0.y;
                           //cout << endl << "StoredPixel" << endl << valueIslands[i].StoredPixels[u].y << endl;
                           //cout <<endl<< "GRIDTORANK" << endl<<GridToRank(valueIslands[i].StoredPixels[u], boxSize)<<endl;
                       }




                       vector<unsigned char> numberImage(boxSize * boxSize, 255);

                       for (int u = 0; u < valueIslands[i].StoredPixels.size(); u++) {

                           numberImage[GridToRank(valueIslands[i].StoredPixels[u], boxSize)] = 0;
                       }
                       cv::Mat resizedImage;
                       cv::Mat resizedImage2;
                       cv::Mat numberMat(
                           boxSize,
                           boxSize,
                           CV_8UC1,
                           numberImage.data()
                       );
                       cv::resize(
                           numberMat,
                           resizedImage,
                           cv::Size(),
                           8.0,
                           8.0,
                           cv::INTER_CUBIC
                       );








                       string filename = "images\\NewImaging_" + to_string(i) + ".png";
                       stbi_write_png(filename.c_str(), boxSize, boxSize, 1, numberImage.data(), boxSize);
                       cv::imwrite(
                           "images\\resized_" + to_string(i) + ".png",
                           resizedImage);


                       tesseract::TessBaseAPI tessImage;


                       if (tessImage.Init("./TessData", "eng") != 0)
                       {
                           cout << "Tesseract failed to initialize" << std::endl;
                           return;
                       }
                       tessImage.SetPageSegMode(tesseract::PSM_SINGLE_CHAR);
                       tessImage.SetVariable("tessedit_char_whitelist", "123456789");
                       tessImage.SetImage(
                           resizedImage.data,
                           resizedImage.cols,
                           resizedImage.rows,
                           1,
                           resizedImage.step
                       );

                       //  tessImage.SetImage(resizedImage.data(), boxSize, boxSize, 1, boxSize);

                       char* sudokuNumber = tessImage.GetUTF8Text();
                       int digit = -1;





                       if (sudokuNumber != nullptr && sudokuNumber[0] >= '1' && sudokuNumber[0] <= '9')
                       {
                           digit = sudokuNumber[0] - '0';
                       }


                       int sudokuGridRank = valueIslands[l].collumn - 1 + (valueIslands[l].row - 1) * 9;
                       SudokuGrid[sudokuGridRank].number = digit;
                       SudokuGrid[sudokuGridRank].solved = true;
                       solvedCount++;


                       }






                     





                       
                      

                   
                                


                    }




                }
            }
          
        }
    }
    //:::::::::::::::::::::::::::: SUDOKU SOLVING ALGORITHM::::::::::::::::::::::::::::://
    //::::::::::::::::::::::WORKING BY ELIMINATION:::::::::::://

    //::::::::::::::::::: I could have added more databases such as line/box and collumn based information for number candidates but I found easier to manages and more legible to just recheck local zones based on techniques:::::://


    //:::::::::::::::: IN THIS FIRST PART OF THE ALGORITHM WE START BY SORTING OUT THE NUMBER CANDIDATES FOR EVERY SQUARE BY GOING THROUGH COLLUMNS,LINES AND BOXES::::::::::::::::://

    vector<int> sudokuBoxesToCheck;

   
    //cout << "SolvedCount" << solvedCount << endl;

   
    FirstSorting(SudokuGrid);

  
    //:::::::::END OF FIRST SORING OF NUMBER CANDIDATES::::::://
    
    //:::::::::::NOW WE CAN START USING DIFFERENT TECHNIQUES TO NARROW DOWN CANDIDATES::::::::://

    //We Loop techniques until we solve the sudoku

    cout << "SolvedCount" << solvedCount << endl;
    while (solvedCount < 81) {
       
       
        BlockAndCollumnLineInteraction(SudokuGrid);

        cout << "BLOACK AND COLLUMN" << endl << SudokuGrid[12].solved << endl << SudokuGrid[14].solved << endl;
      
      //  cout << "LOOP 1" << endl;
        BlockBlockInteraction(SudokuGrid);

        cout << "BLOACK AND Block" << endl << SudokuGrid[12].solved << endl << SudokuGrid[14].solved << endl;
  
        //cout << "SolvedCount" << solvedCount << endl;
        //cout << "LOOP 2" << endl;

        HiddenSubsets(SudokuGrid);
        cout << "HiddenSubset" << endl << SudokuGrid[12].solved << endl << SudokuGrid[14].solved << endl;
      //  cout << "LOOP 3" << endl;
        NakedSubsets(SudokuGrid);
        cout << "MakedSubset" << endl << SudokuGrid[12].solved << endl << SudokuGrid[14].solved << endl;
       // cout << "LOOP 4" << endl;
        XWingAndAboveTechniques(SudokuGrid);
        cout << "XWing and AboveTechniques" << endl << SudokuGrid[12].solved << endl << SudokuGrid[14].solved << endl;
       // cout << "LOOP Ends" << endl;
    }
    WriteGrid(SudokuGrid,fileName);
    






}




