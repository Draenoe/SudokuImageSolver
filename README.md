I have an executable version of the project in the “executable version” folder.

Sudoku images that need to be solved go in the images folder.

Solved Sudokus go into the output folder.

In the console, you will be prompted to put the name of the file without the extension.

The main code is all in the sudokusolvingproject.sln file.

As of the current version, input images work with the following conditions:

The solver works with black-and-white images.

The image needs to exclusively feature the Sudoku square containing the numbers, with nothing but white around it.

The solver currently only works with properly oriented pictures.

I also didn't put any fallbacks in case of an inability to solve or if it gets stuck.

All of these are fixable, but when starting the project, I had set strict bounds to avoid feature creep and spending an excess of time on the project.
