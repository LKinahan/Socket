#pragma once

#include <opencv2/opencv.hpp>
#include "CBase4618.h"
#include "server.h"
#include <pigpio.h>
#include <vector>
#include <string>

using namespace std;

#define SERVO1 13
#define SERVO2 21
#define BLULED 17
#define GRNLED 27
#define REDLED 22
#define BUTTON1 23
#define BUTTON2 24
#define JOYBUTTON 3
#define ON 1
#define OFF 0

const int GATE_DOWN = 1400;
const int GATE_UP = 1250;
const int SORT_RIGHT = 1700;
const int SORT_LEFT = 1100;
const int SORT_ = 1400;

//string mode_text;
/**
* @brief
* This class inherits from CBase4618
* @author Laurel Kinahan
*/

class CSort :public CBase4618 {
private:
    cv::VideoCapture vid;
    cv::Mat rgb, hsv, mask, temp, hsvlarge, yellow, balls, check;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours_yellow;
    std::vector<std::vector<cv::Point>> contours_balls;
    cv::Point roi_PointOffset;
    cv::Point mode_textpoint;
    cv::Point count_textpoint;
    std::string Count_text;
    std::string other_count_text;
    std::string display_mode_text;
    std::string server_count_str;

public:
    CSort();
    ~CSort();
    int detectImage();
    void draw();
    void update();
    int kbhit(void);
    bool StartStop(int OnOff);
    void NothingDetected();
    void GateKeeper();
    void SortRight();
    void SortLeft();
    bool ManualY(int YLW);
    bool ManualO(int OTHER);
    char input;
    char keyPress;
    int OnOffFlag;

    void delay(double milliseconds);

    // static server serv;

    static Server imserv;
    static Server cmdserv;
    static void imserverthread();
    static void cmdserverthread();
    void server();

    bool serverthreadexit;
};