#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main() {
    VideoCapture capture("C:\\Users\\user\\Desktop\\수업들\\2학년 2학기\\멀티미디어 프로그래밍\\12.11 opencv\\ball.avi");
    if (!capture.isOpened()) {
        cout << "The video file was not found." << endl;
        return -1;
    }

    int width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    int height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

    Size frameSize = Size(width, height);

    Mat grayImage;
    Mat sumImage = Mat::zeros(frameSize, CV_32F);

    Mat frame;
    int nFrameCount = 0;

    for (;;) {
        capture >> frame;
        if (frame.empty()) {
            break;
        }

        cvtColor(frame, grayImage, COLOR_BGR2GRAY);
        accumulate(grayImage, sumImage);

        imshow("grayImage", grayImage);
        char chKey = waitKey(50);
        if (chKey == 27)
            break;
        nFrameCount++;
    }

    sumImage /= nFrameCount;

    imwrite("C:\\Users\\user\\Desktop\\수업들\\2학년 2학기\\멀티미디어 프로그래밍\\12.11 opencv\\ballBkg.jpg", sumImage);
    destroyAllWindows();

    return 0;
}