#include <opencv2/opencv.hpp>

int main() {
    cv::VideoCapture capture("C:\\Users\\user\\Desktop\\수업들\\2학년 2학기\\멀티미디어 프로그래밍\\12.11 opencv\\ball.avi");
    if (!capture.isOpened()) {
        std::cerr << "\n -------------- The video file was not found. -----------------" << std::endl;
        return 0;
    }

    int width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    int height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    cv::Size size(width, height);

    cv::Mat bkgImage = cv::imread("C:\\Users\\user\\Videos\\Captures\\ballbkg.jpg", cv::IMREAD_GRAYSCALE); // 얘는 왜있는지 모르겠지만 그냥 크기 맞는 사진 하나 넣은거임.

    if (bkgImage.empty()) {
        std::cerr << "\n --------------Failed to load background image.----------------" << std::endl;
        return -1;
    }

    cv::Mat grayImage, diffImage, frame;
    int nFrame = 0;
    int nThreshold = 50;

    for (;;) {
        capture >> frame;
        if (frame.empty())
            break;
        nFrame++;
        std::cout << "nFrame = " << nFrame << std::endl;

        cv::cvtColor(frame, grayImage, cv::COLOR_BGR2GRAY);
        cv::absdiff(grayImage, bkgImage, diffImage);
        

        cv::threshold(diffImage, diffImage, nThreshold, 255, cv::THRESH_BINARY);

        cv::imshow("grayImage", grayImage);
        cv::imshow("diffImage", diffImage);


        char chKey = cv::waitKey(10);
        if (chKey == 27)
            break;
    }

    cv::destroyAllWindows();
    return 0;
}
