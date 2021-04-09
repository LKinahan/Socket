////////////////////////////////////////////////////////////////
// ELEX 4618 Client Template project for BCIT
// Created Oct 5, 2016 by Craig Hennessey
// Last updated March 29, 2019
////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <string>
#include <iostream>
#include <thread>

#include "Client.h"

Client imclient(4618, "192.168.137.99"); //image client port
Client cmdclient(4619, "192.168.137.99"); //cmd client port

int cmd = -1;

void print_menu()
{
	std::cout << "\n***********************************";
	std::cout << "\n* ELEX4618 Client Project";
	std::cout << "\n***********************************";
	std::cout << "\n(1) Auto Mode";
	std::cout << "\n(2) Manual Mode";
	std::cout << "\n(3) Send to Yellow Bin";
	std::cout << "\n(4) Send to Other Bin";
	std::cout << "\n(5) Get Count";
	std::cout << "\n(6) Clear Count";
	std::cout << "\n(0) Exit";
	std::cout << "\nCMD> ";
}

void send_command(std::string cmd)
{
	std::string str;
	cmdclient.tx_str(cmd);

	do
	{
		cmdclient.rx_str(str);
		if (str.length() > 0)
		{
			std::cout << "\nClient Rx: " << str;
		}
	} while (str.length() == 0);
}

void get_image()
{
	cv::Mat im;
	int count = 0;
	do
	{
		imclient.tx_str("im");

		if (imclient.rx_im(im) == true)
		{
			if (im.empty() == false)
			{
				//count++;
				//std::cout << "\nImage received: " << count;
				cv::imshow("rx", im);
				cv::waitKey(100);
			}
		}
	} while (cmd != 0);
}


int main(int argc, char* argv[])
{
	std::thread t(&get_image);
	t.detach();

	cmd = -1;

	do
	{
		print_menu();
		std::cin >> cmd;

		switch (cmd)
		{
		case 1: send_command("S01 \n"); break; //auto //needs to be changed S010....
		case 2: send_command("S00 \n"); break; // manual
		case 3: send_command("S10 \n"); break; // bin 1
		case 4: send_command("S11 \n"); break; // bin 2
		case 5: send_command("G10 \n"); break; // get count
		case 6: send_command("G11 \n"); break; // clear count
		}
	} while (cmd != 0);
}
