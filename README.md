# invisibility-cloak
A simple colour detection and segmentation program to help alleviate the disappointment in never receiving your letter from Hogwarts. 

It was quite impressive to see how simple it would be to make such a program. The code however is very much tailored to my input video as such simple techniques require. However, this should work well enough if you have a good lighting and a similar coloured cloth. You can make a slight modification to the code if you want to use a cloth of a different colour. You can view `video.mp4` and `result.mp4` to see the input I used and the resulting video.  

## Files
- `.vscode/`
    - contains the files required to build and debug the program if you're using Visual Studio Code
- `invisibility_cloak.cpp`
    - the main program which takes a video as input and outputs a new video with the invisibility cloak effect implemented
- `invisibility_cloak_steps.cpp`
    - essentially a copy of the main program however it illustrates the step-by-step process to achieve the invisbility cloak effect on a single frame
- `video.mp4`
    - the video I used as my input
- `result.mp4`
    - my result

## Usage
*Note the following instructions apply to a Ubuntu 18.04 system using VS Code. They may be slightly different if your environment is different. Ensure you have properly set up a C++ environment on your system*
1. Download and set up OpenCV. I referenced [these steps](https://cv-tricks.com/installation/opencv-4-1-ubuntu18-04/) following for my Ubuntu 18.04 system.
2. Edit the program to reference your input video, or name your input video `result.mp4` and keep it in the working directory
3. Build the program via 
    - ```g++ -std=c++11 {program_name}.cpp `pkg-config --libs --cflags opencv4` -o {program_name}```
    - If you're using VS code, build by selecting `Terminal -> Run Build Task` or by pressing `Ctrl+Shift+B`. Ensure that the `./vscode/tasks.json` file is set up properly for your environment. 
4. Run the program `./{program_name}` to generate `result.mp4`
5. Enjoy!

## Acknowledgments
This project was largely inspired by the following from [Learn OpenCV](https://www.learnopencv.com/invisibility-cloak-using-color-detection-and-segmentation-with-opencv/)
