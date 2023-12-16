//#include "C:\Program Files\opencv\build\include\opencv2\highgui.hpp"
//#include <iostream>
//
//int main() {
//    try {
//        // 이미지 파일 로드
//        cv::Mat srcImage = cv::imread("C:\\Users\\user\\Pictures\\Screenshots\\스크린샷 2023 - 11 - 26 152903.png");
//
//        // 예외 처리: 이미지를 읽을 수 없는 경우
//        if (srcImage.empty()) {
//            std::cerr << "이미지를 불러올 수 없습니다." << std::endl;
//            return -1;
//        }
//
//        // 윈도우 생성 및 이미지 표시
//        cv::namedWindow("srcImage", cv::WINDOW_AUTOSIZE);
//        cv::imshow("srcImage", srcImage);
//        cv::waitKey(0);
//        cv::destroyAllWindows();
//    }
//    catch (const cv::Exception& e) {
//        std::cerr << "OpenCV 예외: " << e.what() << std::endl;
//        return -1;
//    }
//    catch (const std::exception& e) {
//        std::cerr << "일반 예외: " << e.what() << std::endl;
//        return -1;
//    }
//
//    return 0;
//}
