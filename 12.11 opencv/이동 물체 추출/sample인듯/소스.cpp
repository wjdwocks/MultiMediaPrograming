//#include "C:\Program Files\opencv\build\include\opencv2\highgui.hpp"
//#include <iostream>
//
//int main() {
//    try {
//        // �̹��� ���� �ε�
//        cv::Mat srcImage = cv::imread("C:\\Users\\user\\Pictures\\Screenshots\\��ũ���� 2023 - 11 - 26 152903.png");
//
//        // ���� ó��: �̹����� ���� �� ���� ���
//        if (srcImage.empty()) {
//            std::cerr << "�̹����� �ҷ��� �� �����ϴ�." << std::endl;
//            return -1;
//        }
//
//        // ������ ���� �� �̹��� ǥ��
//        cv::namedWindow("srcImage", cv::WINDOW_AUTOSIZE);
//        cv::imshow("srcImage", srcImage);
//        cv::waitKey(0);
//        cv::destroyAllWindows();
//    }
//    catch (const cv::Exception& e) {
//        std::cerr << "OpenCV ����: " << e.what() << std::endl;
//        return -1;
//    }
//    catch (const std::exception& e) {
//        std::cerr << "�Ϲ� ����: " << e.what() << std::endl;
//        return -1;
//    }
//
//    return 0;
//}
