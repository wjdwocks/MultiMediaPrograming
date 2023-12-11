//#include <opencv2/opencv.hpp>
//
//int main()
//{
//    cv::VideoCapture capture("C:\\Users\\user\\Desktop\\������\\2�г� 2�б�\\��Ƽ�̵�� ���α׷���\\12.11 opencv\\ball.avi");
//
//    if (!capture.isOpened())
//    {
//        std::cerr << "Error: The video file was not found." << std::endl;
//        return 0;
//    }
//
//    cv::Mat background, frame, foreground, alpha;
//
//    // ù ��° �������� ������� ����
//    capture >> background;
//
//    // ��� �̹��� ����
//    cv::Mat result(background.size(), CV_8UC4);
//
//    int nFrameCount = 0;
//
//    for (;;)
//    {
//        capture >> frame;
//
//        if (frame.empty())
//            break;
//
//        // ���� ���
//        cv::absdiff(frame, background, foreground);
//
//        // ���̸� �̿��Ͽ� ������ �̹��� ����
//        cv::cvtColor(foreground, alpha, cv::COLOR_BGR2GRAY);
//        cv::threshold(alpha, alpha, 50, 255, cv::THRESH_BINARY);
//        alpha.convertTo(alpha, CV_32F);
//        alpha /= 255.0;
//
//        // ��� �̹��� ����
//        result.setTo(cv::Scalar(0, 0, 0, 0));
//        frame.convertTo(frame, CV_32F);
//        background.convertTo(background, CV_32F);
//        cv::multiply(frame, alpha, frame);
//        cv::multiply(background, cv::Scalar(1, 1, 1, 1) - alpha, background);
//        result = frame + background;
//        result.convertTo(result, CV_8U);
//
//        // ��� ����
//        cv::accumulateWeighted(frame, background, 0.01);
//
//        cv::imshow("Result", result);
//
//        char chKey = cv::waitKey(50);
//        if (chKey == 27)
//            break;
//
//        nFrameCount++;
//    }
//
//    cv::destroyAllWindows();
//
//    return 0;
//}
