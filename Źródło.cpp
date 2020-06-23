#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/video.hpp>
#include<opencv2/imgcodecs.hpp>
#include<iostream>
#include<fstream>
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
    //vector<Vec4i> hierarchy;
    Mat done;
    Mat thresh;
    Size s = Size(5, 5);
    cvtColor(img, done, COLOR_RGB2GRAY);
    //sobel filter
    Sobel(done, done, CV_8U, 1, 1);
    //structuring element
    Mat kernel = getStructuringElement(MORPH_RECT, s);
    dilate(done, done, kernel, Point(-1, -1), 2, 0);
    erode(done, done, Mat(), Point(-1, -1), 1, 0);

    //GaussianBlur(done, done, Point(3, 3), 1, 1, 0);
    //erode(done, done, Mat(), Point(-1, -1), 1, 0);
    //dilate(done, done, kernel, Point(-1, -1), 1, 0);
    //threshold     
    threshold(done, thresh, 0, 255, THRESH_OTSU + THRESH_BINARY);
    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(5, 3));
    //i have. no idea if below will work
    dilate(thresh, thresh, kernel2, Point(-1, -1), 5, 0);
    //morphologyEx(thresh, thresh, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    //contours
    findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (int i = 0; i < contours.size(); i++) {
        Rect r = boundingRect(contours[i]);
        int x = r.x;
        int y = r.y;
        int h = r.height;
        int w = r.width;
        float ar = w / h;
        if (ar >= 3) cout << "Contour found" << endl;
        else drawContours(thresh, contours, i, Scalar(0, 0, 0), CV_FILLED);
        rectangle(img, r, Scalar(0, 0, 255));
    }

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

void processHistogramsRedone(vector<Mat> hist1, vector<Mat> hist2, int size, int framecount) {
    int pixelCount = size * size;
    float curr = 0;
    float max = 0;
    float maxB = 0;
    int rectCounter = 1;
    int maxRectIndex = 0;
    int maxRectIndexB = 0;
    int i = 0;
    for (i = 0; i < hist1.size(); i++)
    {
        float prevFrame = hist1[i].at<float>(255);
        float currFrame = hist2[i].at<float>(255);
        float jump = -((prevFrame / pixelCount) - (currFrame / pixelCount));
        if (max < jump) {
            max = jump;
            maxRectIndex = i + 1;
        }
        prevFrame = hist1[i].at<float>(0);
        currFrame = hist2[i].at<float>(0);
        jump = -((prevFrame / pixelCount) - (currFrame / pixelCount));
        if (maxB < jump) {
            maxB = jump;
            maxRectIndexB = i + 1;
        }
    }
    cout << "Frame " << framecount << "has the biggest jump in quantity of white pixels at rect " << maxRectIndex << ": " << max << "%" << endl;
    cout << "Frame " << framecount << "has the biggest jump in quantity of black pixels at rect " << maxRectIndexB << ": " << maxB << "%" << endl;
    cout << endl << "==================================================" << endl;
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
    VideoCapture cap("video_nocc.mp4");
    Mat img1, img2;
    Mat resized1(Size(990, 540), CV_64FC1);
    Mat resized2(Size(990, 540), CV_64FC1);
    ofstream f;
    f.open("file.txt");
    if (!cap.isOpened())
    {
        cout << "Nie mozna by³o odtworzyæ video.";
        return -1;
    }
    int x = 0;
    int y = 0;
    int frameCount = 1;
    while (true) {
        cap >> img1;
        if (img1.empty()) {
            return 0;
        }
        cap >> img2;
        if (img2.empty()) {
            return 0;
        }
        boolean detected = false;
        int sizecounter = 0;
        resize(img1, resized1, resized1.size(), 0, 0, 1);
        resize(img2, resized2, resized2.size(), 0, 0, 1);
        Mat temp1 = imagePrep(resized1);
        Mat temp2 = imagePrep(resized2);
        vector<Rect> grid1 = findCells(resized1, 80, x, y);
        vector<Rect> grid2 = findCells(resized2, 80, x, y);
        vector<Mat> hists1 = findHistograms(resized1, grid1);
        vector<Mat> hists2 = findHistograms(resized2, grid2);
        for (Rect r : grid1)
            rectangle(resized1, r, (255, 255, 255), 1, 8, 0);
        for (Rect r : grid2)
            rectangle(resized2, r, (255, 255, 255), 1, 8, 0);
        //Mat temp = imagePrep(img);
        int button = (char)waitKey(10);
        if (button == 27) break;
        // s
        if (button == 119 && y >= 10) y -= 10;
        // w
        if (button == 115 && y < resized1.rows) y += 10;
        // d
        if (button == 100 && x < resized1.cols) x += 10;
        // a
        if (button == 97 && x >= 10) x -= 10;
        processHistogramsRedone(hists1, hists2, 80, frameCount);
        int i = 1;
        for (Rect r : grid1) {
            rectangle(resized1, r, (255, 255, 255), 1, 8, 0);
            putText(resized1(r), to_string(i), Point(0, 45),
                cv::FONT_HERSHEY_DUPLEX,
                1,
                CV_RGB(118, 185, 0), //font color
                2);
            i++;
        }
        putText(resized1, to_string(frameCount), Point(20, 80),
            cv::FONT_HERSHEY_DUPLEX,
            2,
            CV_RGB(255, 255, 255), //font color
            2);
        //processHistograms(hists, 80, frameCount);
        for (int i = x; i < temp1.rows; i++) {
            for (int j = y; j < temp1.cols; j++) {
                Scalar color = temp1.at<uchar>(i, j);
                if (color[0] == 255) sizecounter += 1;
            }
        }
        if (sizecounter >= 15000) detected = true;
        if (detected) {
            f << "Subtitles detected: " << to_string(frameCount) << "\n";
        }
        imshow("test", temp1);
        imshow("resized", resized1);
        frameCount++;
    }
    f.close();
    destroyAllWindows();
    return 0;
}