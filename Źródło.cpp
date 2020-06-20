#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/video.hpp>
#include<opencv2/imgcodecs.hpp>
#include<iostream>
#include<stdio.h>
#include<conio.h>
#include<Windows.h>
#include<wchar.h>
#include<opencv2/videoio.hpp>
#include<opencv2/imgproc/imgproc_c.h>
#include <vector>
#include<opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;

//grid of image
vector<Rect> findCells(Mat img, int GRID_SIZE)
{
    vector<Rect> cells;
    int width = img.cols;
    int height = img.rows;
    for (int y = 0; y < height - GRID_SIZE; y += GRID_SIZE) {
        for (int x = 0; x < width - GRID_SIZE; x += GRID_SIZE) {
            int k = x * y + x;
            Rect grid_rect(x, y, GRID_SIZE, GRID_SIZE);
            cout << grid_rect << endl;
            cells.push_back(grid_rect);
            waitKey();
        }
    }
    return cells;
}

Mat imageCorrection(Mat m) {
    // color ---> bw
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    Mat done;
    Mat thresh;
    Size s = Size(8, 2);
    cvtColor(m, done, COLOR_BGR2GRAY);
    //sobel filter
    Sobel(done, done, CV_8U, 1, 0);
    //threshold     
    threshold(done, thresh, 0, 255, THRESH_OTSU + THRESH_BINARY);
    //structuring element
    getStructuringElement(MORPH_RECT, s);
    //i have. no idea if below will work
    morphologyEx(thresh, thresh, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    //contours


}

int main() {
    Mat img;
    img = imread("image.jpg");
    vector<Rect> grid = findCells(img, 4);

    while (true) {
        imshow("test", img);
        int button = (char)waitKey(10);
        if (button == 27) break;
    }

    destroyAllWindows();
    return 0;
}