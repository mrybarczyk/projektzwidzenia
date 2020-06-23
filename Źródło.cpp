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
vector<Rect> findCells(Mat img, int GRID_SIZE, int i, int j, int width, int height)
{
    vector<Rect> cells;
    for (int y = j; y < height; y += GRID_SIZE) {
        for (int x = i; x < width; x += GRID_SIZE) {

            int xw = GRID_SIZE;
            int yh = GRID_SIZE;
            if (y + GRID_SIZE > height) {
                yh = height - y;
            }
            if (x + GRID_SIZE > width) {
                xw = width - x;
            }
            Rect grid_rect(x, y, xw, yh);
            cells.push_back(grid_rect);
        }
    }
    return cells;
}

Mat imagePrep(Mat img) {
    // color ---> bw
    vector<vector<Point>> contours;
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

    threshold(done, thresh, 0, 255, THRESH_OTSU + THRESH_BINARY);
    Mat kernel2 = getStructuringElement(MORPH_RECT, Size(5, 3));
    //connect letters
    dilate(thresh, thresh, kernel2, Point(-1, -1), 5, 0);
    //contours
    findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_NONE);
    for (int i = 0; i < contours.size(); i++) {
        Rect r = boundingRect(contours[i]);
        int x = r.x;
        int y = r.y;
        int h = r.height;
        int w = r.width;
        float ar = w / h;
        if (ar < 3) drawContours(thresh, contours, i, Scalar(0, 0, 0), CV_FILLED);
        rectangle(img, r, Scalar(0, 0, 255));
    }

    return thresh;
}

vector<Mat> findHistograms(Mat img, vector<Rect> gr) {
    vector<Mat> hists;
    Mat temp = img;
    int k = 1;
    int histSize = 256;
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
    }
    return hists;
}

void processHistograms(vector<Mat> hist, int size, int framecount, ofstream& filestream) {
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
    if (perc > 0)
    {
        filestream << "Frame " << framecount << "has the biggest percentage of white pixels on rect " << maxRectIndex << ": " << perc << endl;
        cout << "Frame " << framecount << "has the biggest percentage of white pixels on rect " << maxRectIndex << ": " << perc << endl;
    }
}

int main() {
    VideoCapture cap("ad4.mp4");
    Mat img;
    Mat resized(Size(990, 540), CV_64FC1);
    ofstream f1, f2;
    f1.open("subtitles_detection.txt");
    f2.open("white_pixels_percentages.txt");
    if (!cap.isOpened())
    {
        cout << "Nie mozna by³o odtworzyæ video.";
        return -1;
    }
    int x = 0;
    int y = 0;
    int height = 540;
    int width = 990;
    int frameCount = 1;
    while (true) {
        cap >> img;
        if (img.empty()) {
            return 0;
        }
        boolean detected = false;
        int sizecounter = 0;
        resize(img, resized, resized.size(), 0, 0, 1);
        Mat temp = imagePrep(resized);
        vector<Rect> grid = findCells(resized, 120, x, y, width, height);
        vector<Mat> hists = findHistograms(temp, grid);
        for (Rect r : grid)
            rectangle(resized, r, (255, 255, 255), 1, 8, 0);

        int button = (char)waitKey(10);
        if (button == 27) break;
        // s
        if (button == 119 && y >= 50) y -= 50;
        // w
        if (button == 115 && y < resized.rows - 50) y += 50;
        // d
        if (button == 100 && x < resized.cols - 50) x += 50;
        // a
        if (button == 97 && x >= 50) x -= 50;
        // z - smaller width
        if (button == 122 && width >= 50) width -= 50;
        // x - bigger width
        if (button == 120 && width <= resized.cols - 50) width += 50;
        // c - smaller height
        if (button == 99 && height >= 50) height -= 50;
        // v - bigger height
        if (button == 118 && height <= resized.rows - 50) height += 50;
        processHistograms(hists, 120, frameCount, f2);
        int i = 1;
        for (Rect r : grid) {
            rectangle(resized, r, (255, 255, 255), 1, 8, 0);
            putText(resized(r), to_string(i), Point(0, 45),
                cv::FONT_HERSHEY_DUPLEX,
                1,
                CV_RGB(118, 185, 0), //font color
                2);
            i++;
        }
        putText(resized, to_string(frameCount), Point(20, 80),
            cv::FONT_HERSHEY_DUPLEX,
            2,
            CV_RGB(255, 255, 255), //font color
            2);
        for (int i = x; i < height; i++) {
            for (int j = y; j < width; j++) {
                Scalar color = temp.at<uchar>(i, j);
                if (color[0] == 255) sizecounter += 1;
            }
        }
        if ((float)((float)sizecounter / (float)(temp.rows * temp.cols)) >= 0.03) detected = true;
        if (detected) {
            f1 << "Subtitles detected: " << to_string(frameCount) << "\n";
            cout << "Subtitles detected: " << to_string(frameCount) << endl;
            cout << "=======================" << endl;
        }
        imshow("test", temp);
        imshow("resized", resized);
        frameCount++;
    }
    f1.close();
    f2.close();
    destroyAllWindows();
    return 0;
}