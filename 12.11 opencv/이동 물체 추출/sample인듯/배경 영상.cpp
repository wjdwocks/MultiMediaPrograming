//#include <opencv2/opencv.hpp>
//
//
//int main() {
//    cv::VideoCapture capture("C:\\Users\\user\\Desktop\\수업들\\2학년 2학기\\멀티미디어 프로그래밍\\12.11 opencv\\ball.avi");
//    if (!capture.isOpened()) {
//        std::cout << "The video file was not found." << std::endl;
//        return -1;
//    }
//
//    int width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
//    int height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);
//
//    cv::Size frameSize = cv::Size(width, height);
//
//    cv::Mat grayImage;
//    cv::Mat sumImage = cv::Mat::zeros(frameSize, CV_32F);
//
//    cv::Mat frame;
//    int nFrameCount = 0;
//
//    for (;;) {
//        capture >> frame;
//        if (frame.empty()) {
//            break;
//        }
//
//        cvtColor(frame, grayImage, cv::COLOR_BGR2GRAY);
//        accumulate(grayImage, sumImage);
//
//        imshow("grayImage", grayImage);
//        char chKey = cv::waitKey(50);
//        if (chKey == 27)
//            break;
//        nFrameCount++;
//    }
//
//    sumImage /= nFrameCount;
//
//    imwrite("C:\\Users\\user\\Desktop\\수업들\\2학년 2학기\\멀티미디어 프로그래밍\\12.11 opencv\\ballBkg.jpg", sumImage);
//    cv::destroyAllWindows();
//
//    return 0;
//}