////////////////////////////////////////////////////////////////
// Member function definition for
// This program runs a simple Pong Game with input from user
// Created Feb 20, 2021 by Laurel Kinahan
////////////////////////////////////////////////////////////////
#include <iostream>
#include <cstdlib>
#include <string>
#include <cmath>
#include <vector>
#include "pigpio.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <thread>
#include "CBase4618.h"
#include "CSort.h"
#include "server.h"

using namespace std;
CSort::CSort()
{
    vid.open(0);
    if (gpioInitialise() < 0)
    {
        cout << "GPIO Init BAD" << endl;
    }
    else
    {
        cout << "GPIO Init GOOD" << endl;
    }

    serverthreadexit = false;
    server();

    roi_PointOffset.x = 150;
    roi_PointOffset.y = 100;
    mode_textpoint.x = 200;
    mode_textpoint.y = 20;
    count_textpoint.x = 5;
    count_textpoint.y = 20;
    keyPress = 'z';
    OnOffFlag = 1;
    //char keyPress = getchar();

    //Ball traffic SERVO
    gpioSetMode(SERVO1, PI_OUTPUT);
    gpioServo(SERVO1, 1400);

    //Sorting SERVO
    gpioSetMode(SERVO2, PI_OUTPUT);
    gpioServo(SERVO2, 1400);

    //RED LED
    gpioSetMode(REDLED, PI_OUTPUT);

    //GRN LED
    gpioSetMode(GRNLED, PI_OUTPUT);

    //BLU LED
    gpioSetMode(BLULED, PI_OUTPUT);

    //PUSHBUTTON1
    gpioSetMode(BUTTON1, PI_INPUT);

    //PUSHBUTTON2
    gpioSetMode(BUTTON2, PI_INPUT);

    //Joystick PushButton
    gpioSetMode(JOYBUTTON, PI_INPUT);
}

CSort::~CSort()
{
    gpioWrite(BLULED, OFF);
    vid.release();
    cv::destroyWindow("RecycleSort");
    gpioTerminate();
    imserv.stop();
    cmdserv.stop();
    cv::waitKey(100);
}

Server CSort::cmdserv;
Server CSort::imserv;

void CSort::update()
{
    static int YLWCount = 0;
    static int BallzCount = 0;

    string mode_text;
    gpioWrite(BLULED, OFF);
    gpioWrite(GRNLED, OFF);
    gpioWrite(REDLED, OFF);
    //    vid.open(0);
    vid >> rgb;
    cv::resize(rgb, rgb, rgb.size() / 2);

    if (kbhit())
        keyPress = getchar();

    std::vector<std::string> cmds;

    cmdserv.get_cmd(cmds);

    if (cmds.size() > 0)
    {
        for (int i = 0; i < cmds.size(); i++)
        {
            if (cmds.at(i) == "S01 \n")
            {
                std::cout << "\nInitiate Auto Mode";
                keyPress = 's';
                //std::cout << "cmds";
                std::string reply = "\nSorter Automatic Mode Initiated";
                cmdserv.send_string(reply);
            }
            else if (cmds.at(i) == "S00 \n")
            {
                std::cout << "\nInitiate Manual Mode";
                keyPress = 's';
                std::string reply = "\nSorter Manual Mode Initiated";
                cmdserv.send_string(reply);
            }
            else if (cmds.at(i) == "S10 \n")
            {
                std::cout << "\nSort to Yellow Bin";
                keyPress = 'y';
                std::string reply = "\nSort to Yellow Bin";
                cmdserv.send_string(reply);
                //keyPress = 'z';
            }
            else if (cmds.at(i) == "S11 \n")
            {
                std::cout << "\nSort to Other Bin";
                keyPress = 'o';
                std::string reply = "\nSort to Other Bin";
                cmdserv.send_string(reply);
                //keyPress = 'z';
            }
            else if (cmds.at(i) == "G10 \n")
            {
                std::cout << "\nSort Count sent to Client";

                std::string reply = server_count_str;
                cmdserv.send_string(reply);
            }
            else if (cmds.at(i) == "G11 \n")
            {
                std::cout << "\nCount Cleared";
                keyPress = 'c';
                std::string reply = "\nCount Cleared";
                cmdserv.send_string(reply);
                //keyPress = 'z';
            }
            else
            {
                std::string reply = "\nGot some other message";
                cmdserv.send_string(reply);
            }
        }
    }


    // turn ON and OFF (start/stop) the sorting system with a PUSHBUTTON.
    // The keyboard ‘S’ key can also turn on and off the system
    // If the System in ON
    if ((keyPress == 'S') || (keyPress == 's'))
    {
        OnOffFlag *= -1;
        keyPress = 'z';
    }
    if (StartStop(JOYBUTTON))
    {
        OnOffFlag *= -1;
    }

    if (OnOffFlag == -1)
    {
        //        bool StartSort = true;
        mode_text = "Auto";
        gpioWrite(BLULED, ON);
        {
            cv::waitKey(1);

            if (kbhit())
                keyPress = getchar();

            vid >> rgb;

            //The ‘C’ clear key is used to reset the bin counters
            if ((keyPress == 'c') || (keyPress == 'C'))
            {
                YLWCount = 0;
                BallzCount = 0;
                keyPress = 'z';
            }
            //1 LED should indicate if the system is ON or OFF

            //cv::waitKey(1);
            // If Nothing detected
            int state = detectImage();
            if (state == 2)
            {
                NothingDetected();
            }
            // IF GRN Ball is detected in detectImage()
            else
            {
                vid.release();
                if (state == 1)//Sort GRN Ballz Left
                {
                    YLWCount++;
                    SortLeft();
                }
                //State == false OTHER ballz detected
                else //Sort OTHER Ballz Right
                {
                    BallzCount++;
                    SortRight();
                }
                vid.open(0);
            }
            Count_text = "Yellow: " + std::to_string(YLWCount);
            other_count_text = "Other: " + std::to_string(BallzCount);
            display_mode_text = "Mode: " + mode_text;
            server_count_str = "\n" + Count_text + "  " + other_count_text;
            draw();
        }
    }
    //If the system is OFF, add the ability to sort to BIN1 (with a button) or BIN2 (with another button) the current object in the sorting queue.
    //You can also BIN1 ‘ 1’ or BIN2 ‘2’ the object with the keyboard
    // If the system is OFF
    //else
    if (OnOffFlag == 1)
    {
        mode_text = "Manual";
        //Sort YLW Ballz Left with key stroke
        if ((keyPress == 'y') || (keyPress == 'Y'))
        {
            YLWCount++;
            SortLeft();
            keyPress = 'z';
        }
        //Sort YLW Ballz Left with PushButton1
        if (ManualY(BUTTON1))
        {
            YLWCount++;
            SortLeft();
        }
        //Sort OTHER Ballz Right
        if ((keyPress == 'o') || (keyPress == 'O'))
        {
            BallzCount++;
            SortRight();
            keyPress = 'z';
        }
        //Sort OTHER ballz right with PuchButton2
        if (ManualO(BUTTON2))
        {
            BallzCount++;
            SortRight();
        }
        //The ‘C’ clear key is used to reset the bin counters
        if ((keyPress == 'c') || (keyPress == 'C'))
        {
            BallzCount = 0;
            YLWCount = 0;
            keyPress = 'z';
        }
        Count_text = "Yellow: " + std::to_string(YLWCount);
        other_count_text = "Other: " + std::to_string(BallzCount);
        display_mode_text = "Mode: " + mode_text;
        server_count_str = "\n" + Count_text + "  " + other_count_text;
        draw();
    }
    //draw();
    //cv::waitKey(500);
}

void CSort::SortRight()
{
    //1 LED should indicate if the system is ON or OFF
    gpioWrite(BLULED, OFF);
    //2 LED’s should indicate if the material was BIN1 or BIN2
    gpioWrite(GRNLED, OFF);
    gpioWrite(REDLED, ON);
    //GATEKEEPER
    GateKeeper();
    //cv::waitKey(150);
    //SORTER
    gpioServo(SERVO2, SORT_RIGHT);
    cv::waitKey(300); //change based on timing
    gpioServo(SERVO2, SORT_);
    // 2 LED’s should indicate if the material was BIN1 or BIN2
    gpioWrite(GRNLED, OFF);
    gpioWrite(REDLED, OFF);
}

void CSort::SortLeft()
{
    //1 LED should indicate if the system is ON or OFF
    gpioWrite(BLULED, OFF);
    // 2 LED’s should indicate if the material was BIN1 or BIN2
    gpioWrite(GRNLED, ON);
    gpioWrite(REDLED, OFF);
    //GATEKEEPER
    GateKeeper();
    //cv::waitKey(150);
    //SORTER
    gpioServo(SERVO2, SORT_LEFT);
    cv::waitKey(300); //change based on timing
    gpioServo(SERVO2, SORT_);
    // 2 LED’s should indicate if the material was BIN1 or BIN2
    gpioWrite(GRNLED, OFF);
    gpioWrite(REDLED, OFF);
}

void CSort::NothingDetected()
{
    gpioServo(SERVO1, GATE_DOWN);
    gpioServo(SERVO2, SORT_);
    gpioWrite(GRNLED, OFF);
    gpioWrite(REDLED, OFF);
    gpioWrite(BLULED, ON);
}

void CSort::GateKeeper()
{
    gpioServo(SERVO1, GATE_UP);
    cv::waitKey(250);
    gpioServo(SERVO1, GATE_DOWN);
}

void CSort::draw()
{

    cv::putText(rgb, Count_text, count_textpoint, 1, 1, cv::Scalar(30, 170, 240), 1, 1, false);
    cv::putText(rgb, display_mode_text, mode_textpoint, 1, 1, cv::Scalar(209, 206, 0), 1, 1, false);
    //cv::imshow("RecycleSort", rgb);
    imserv.set_txim(rgb);
    //delay(500);
    //cv::waitKey(500);
}

void CSort::delay(double milliseconds)
{
    double start_time = cv::getTickCount();
    double freq = cv::getTickFrequency();
    double elapsed_time;
    double current_time;

    do
    {
        current_time = cv::getTickCount();
        elapsed_time = (current_time - start_time) / freq * 1000;
    } while ((int)elapsed_time < milliseconds);
}

int CSort::detectImage()
{
    //vid.open(0);
    //if(vid.isOpened() == true)
    {
        //vid.set(cv::CAP_PROP_BRIGHTNESS, 200);
        //vid.set(cv::CAP_PROP_EXPOSURE, 0.25);
        {
            vid >> rgb;
            cv::resize(rgb, rgb, rgb.size() / 2);

            cv::cvtColor(rgb, hsvlarge, CV_BGR2HSV);
            cv::Rect roi = cv::Rect(200, 85, 40, 40);//Rect(X, Y, Width, Height)
            cv::Mat hsv = hsvlarge(roi);
            //cv::imshow("RGB2HSV", hsv);

            cv::inRange(hsv, cv::Scalar(18, 50, 50), cv::Scalar(30, 150, 255), yellow);
            cv::inRange(hsv, cv::Scalar(61, 0, 80), cv::Scalar(180, 150, 255), balls);
            cv::findContours(yellow, contours_yellow, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            cv::findContours(balls, contours_balls, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            // If GRN Ball Detected
            if ((contours_yellow.size() || contours_balls.size()) == 0)
            {
                return 2;
            }
            if ((contours_yellow.size() != 0))
            {
                for (unsigned int i = 0; i < contours_yellow.size(); i++)
                {
                    cv::Rect rect_yellow = cv::boundingRect(contours_yellow.at(i));
                    if (rect_yellow.area() > 1500)
                    {
                        cv::drawContours(rgb, contours_yellow, i, cv::Scalar(127, 127, 127), 1, 8, false, false, roi_PointOffset);
                        return 1;
                    }
                }
            }
            // If OTHER BAllz Detected
            if ((contours_balls.size() != 0))
            {
                for (unsigned int i = 0; i < contours_balls.size(); i++)
                {
                    cv::Rect rect_balls = cv::boundingRect(contours_balls.at(i));
                    if (rect_balls.area() > 1500)
                    {
                        cv::drawContours(rgb, contours_balls, i, cv::Scalar(127, 127, 127), 1, 8, false, false, roi_PointOffset);
                        return 0;
                    }
                }
            }
            // cv::erode(hsvsel,hsvsel,cv::Mat());
            // cv::imshow("erode",hsvsel);

            // cv::dilate(hsvsel,hsvsel, cv::Mat());
            // cv::imshow("Dilate",hsvsel)
        }
    }
    //vid.release();
}

// JoyStick puchbutton (select) control for turning system on/off
bool CSort::StartStop(int OnOff)
{
    int data;
    static int timerCount = 1;
    static bool statusChanged = false;

    data = gpioRead(OnOff);

    if (data == 0)
    {
        if ((timerCount == 0) && (!statusChanged))
        {
            statusChanged = true;
            return true;
        }
        else
        {
            timerCount--;
        }
    }
    else
    {
        timerCount = 1;
        statusChanged = false;
    }
    return false;
}

// Button control for YELLOW sort
bool CSort::ManualY(int YLW)
{
    int data;
    static int timerCount = 1;
    static bool statusChanged = false;

    data = gpioRead(YLW);

    if (data == 0)
    {
        if ((timerCount == 0) && (!statusChanged))
        {
            statusChanged = true;
            return true;
        }
        else
        {
            timerCount--;
        }
    }
    else
    {
        timerCount = 1;
        statusChanged = false;
    }
    return false;
}

// button control for OTHER sort
bool CSort::ManualO(int OTHER)
{
    int data;
    static int timerCount = 1;
    static bool statusChanged = false;

    data = gpioRead(OTHER);

    if (data == 0)
    {
        if ((timerCount == 0) && (!statusChanged))
        {
            statusChanged = true;
            return true;
        }
        else
        {
            timerCount--;
        }
    }
    else
    {
        timerCount = 1;
        statusChanged = false;
    }
    return false;
}

void CSort::imserverthread()
{
    // Start server
    imserv.start(4618);
}

void CSort::cmdserverthread()
{
    // Start server
    cmdserv.start(4619);
}

void CSort::server()
{
    //  char inputchar;
    //  std::vector<std::string> cmds;

    //  // Start image send to server thread
    //  std::thread t1(&serverimagethread);
    //  t1.detach();

      // Start server thread
    std::thread t2(&imserverthread);
    t2.detach();

    std::thread t3(&cmdserverthread);
    t3.detach();
    /*
    cv::namedWindow("WindowForWaitkey");
    do
    {
      inputchar = cv::waitKey(100);
      if (inputchar == 'q')
      {
        serverthreadexit = true;
      }

      serv.get_cmd(cmds);

      if (cmds.size() > 0)
      {
        for (int i = 0; i < cmds.size(); i++)
        {
          if (cmds.at(i) == "a")
          {
            std::cout << "\nReceived 'a' command";

            // Send an 'a' message
            std::string reply = "Hi there from Server";
            serv.send_string(reply);
          }
          else
          {
            std::string reply = "Got some other message";
            serv.send_string(reply);
          }
        }
      }
    } while (serverthreadexit == false);

    serv.stop();

    Sleep(100);*/
}

/*////////////////////////////////////////////////////////////////////////////////////////////////
/
/This function was taken from https://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html
/It is used to implement kbhit on linux
/
////////////////////////////////////////////////////////////////////////////////////////////////*/
int CSort::kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}