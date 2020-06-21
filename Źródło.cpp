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
//TODO : grid should be able to include rects that aren square if we cant get squares.
vector<Rect> findCells(Mat img, int GRID_SIZE, int i, int j)
{
    vector<Rect> cells;
    int width = img.cols;
    //cout << "Cols:" << width << endl;
    int height = img.rows;
    //cout << "Rows:" << height << endl;
    for (int y = j; y < height; y += GRID_SIZE) {
        for (int x = i; x < width; x += GRID_SIZE) {
            //int k = x * y + x;
            int xw = GRID_SIZE;
            int yh = GRID_SIZE;
            if (y + GRID_SIZE > height) {
                yh = height - y;
            }
            if (x + GRID_SIZE > width) {
                xw = width - x;
            }
            Rect grid_rect(x, y, xw, yh);
            //cout << grid_rect << endl;
            cells.push_back(grid_rect);
            //waitkey deleted
        }
    }
    return cells;
}

Mat imagePrep(Mat img) {
    // color ---> bw
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    Mat done;
    Mat thresh;
    Size s = Size(8, 2);
    cvtColor(img, done, COLOR_BGR2GRAY);
    //sobel filter
    Sobel(done, done, CV_8U, 1, 0);
    //threshold     
    threshold(done, thresh, 0, 255, THRESH_OTSU + THRESH_BINARY);
    //structuring element
    getStructuringElement(MORPH_RECT, s);
    //i have. no idea if below will work
    morphologyEx(thresh, thresh, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    //contours
    return thresh;
}

vector<Mat> findHistograms(Mat img, vector<Rect> gr) {
    vector<Mat> hists;
    Mat temp = img;
    int k = 1;
    temp = imagePrep(temp);
    int histSize = 256; // bin size
    float range[] = { 0, 256 };
    const float* histRange = { range };

    bool uniform = true;
    bool accumulate = false;

    Mat hist;

    int channels[] = { 0 };
    for (Rect r : gr) {
        Mat tempHist;
        Mat tempCropped = temp(r);
        calcHist(&tempCropped, 1, channels, Mat(), tempHist, 1, &histSize, &histRange, uniform, accumulate);
        hists.push_back(tempHist);
        // Access histogram value of white by:
        // tempHist.at<float>(255);
    }
    return hists;
}

void processHistograms(vector<Mat> hist, int size, int framecount) {
    int pixelCount = size * size;
    float curr = 0;
    float max = 0;
    int rectCounter = 1;
    int maxRectIndex = 0;
    for (Mat h : hist) {
        curr = h.at<float>(255);
        if (max < curr) {
            max = curr;
            maxRectIndex = rectCounter;
        }
        rectCounter++;
    }
    float perc = max / pixelCount;
    cout << "Frame " << framecount << "has the biggest percentage of white pixels on rect " << maxRectIndex << ": " << perc << endl;
}

int main() {

    // EXAMPLE OF CORRECT APPLICATION OF FILTER TO ONLY ONE RECT
    //GaussianBlur(img(grid[1]), img(grid[1]), Point(101, 101), 5, 5, 0);
    VideoCapture cap("ad2.mp4");
    if (!cap.isOpened())
    {
        cout << "Nie mozna by³o odtworzyæ video.";
        return -1;
    }
    Mat img;
    cap >> img;
    if (img.empty()) {
        return 0;
    }
    int x = 0;
    int y = 0;
    int frameCount = 1;
    vector<Rect> grid = findCells(img, 240, x, y);
    vector<Mat> hists = findHistograms(img, grid);
    for (Rect r : grid)
        rectangle(img, r, (255, 255, 255), 1, 8, 0);
    while (true) {
        cap >> img;
        if (img.empty()) {
            return 0;
        }
        int button = (char)waitKey(10);
        if (button == 27) break;
        // s
        if (button == 119 && y >= 10) {
            y -= 10;
            grid = findCells(img, 240, x, y);
        }
        // w
        if (button == 115 && y < img.rows) {
            y += 10;
            grid = findCells(img, 240, x, y);
        }
        // d
        if (button == 100 && x < img.cols) {
            x += 10;
            grid = findCells(img, 240, x, y);
        }
        // a
        if (button == 97 && x >= 10) {
            x -= 10;
            grid = findCells(img, 240, x, y);
        }
        hists = findHistograms(img, grid);
        for (Rect r : grid)
            rectangle(img, r, (255, 255, 255), 1, 8, 0);
        processHistograms(hists, 240, frameCount);
        imshow("test", img);
        frameCount++;
    }

    destroyAllWindows();
    return 0;
}