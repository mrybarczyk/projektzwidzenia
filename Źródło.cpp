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
int width = src.cols;
int height = src.rows;
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
    // Ró¿nica miêdzy klatkami
    //VideoCapture cap(0, CAP_DSHOW);
    VideoCapture cap;
    cap.open("path");

    if (!cap.isOpened()) {
        printf("File not found.");
        return -1;
    }

    vector<Point> coords;
    Mat frame1g, frame1, frame2, frame2g, dif, bdif, t, h;
    vector<vector<Point>> contours;
    vector<Point> largest_contour;
    vector<Vec4i> hierarchy;
    double largest_m10;
    double largest_m01;
    double largest_m00;
    int nm = 2;
    int q = 30;
    int b = 5;
    int frameCount = 0;
    int mint = 15;
    int maxt = 100;
    int frame_width = cap.get(CAP_PROP_FRAME_WIDTH);
    int frame_height = cap.get(CAP_PROP_FRAME_HEIGHT);

    SYSTEMTIME lt = { 0 };
    GetLocalTime(&lt);
    string part1 = to_string(lt.wDay) + "." + to_string(lt.wMonth) + "." + to_string(lt.wYear) + " " + to_string(lt.wHour) + "." + to_string(lt.wMinute) + "." + to_string(lt.wSecond);
    string part2 = ".avi";
    string file = part1 + part2;
    VideoWriter video(file, FOURCC('XVID'), 5, Size(frame_width, frame_height), true);

    while (true) {
        cap >> frame1;
        if (frame1.empty()) break;
        cap >> frame2;
        if (frame2.empty()) break;
        cvtColor(frame1, frame1g, COLOR_BGR2GRAY);
        cvtColor(frame2, frame2g, COLOR_BGR2GRAY);
        absdiff(frame1g, frame2g, dif);
        if (b != 0) blur(dif, bdif, Size(b, b));
        threshold(bdif, t, mint, maxt, THRESH_BINARY);
        findContours(t, contours, hierarchy, RETR_LIST, CHAIN_APPROX_NONE);
        vector<Moments> mu(contours.size());
        if (contours.size() == 0) frameCount += 1;
        if (contours.size() != 0) frameCount = 0;
        if (frameCount == 90) break;
        GetLocalTime(&lt);
        string time = to_string(lt.wDay) + "." + to_string(lt.wMonth) + "." + to_string(lt.wYear) + " " + to_string(lt.wHour) + "." + to_string(lt.wMinute) + "." + to_string(lt.wSecond);
        putText(frame1, time, Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 0));
        video.write(frame1);
        imshow("Frame", frame1);
        int button = (char)waitKey(10);
        if (button == 27) break;
    }

    video.release();
    cap.release();
    destroyAllWindows();
    return 0;
}