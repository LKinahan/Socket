#pragma once
#pragma once

#include <opencv2/opencv.hpp>
#include <vector>

/**
* @brief Base Class for Update, Draw, and Run functions
* This class communcates between CSort
* @author Laurel Kinahan
*/

class CBase4618
{
private:

public:
	/**
	* @brief This is the class constructor
	* @param None
	* @return None
	*/
	CBase4618(void);

	/**
	* @brief This is the class destructor
	* @param None
	* @return None
	*/
	~CBase4618(void);

	/**
	* @brief Updates internal variables
	* @param nothing to receive
	* @return nothing to return
	*/
	virtual void update() = 0;

	/**
	* @brief Draws on the created image
	* @param nothing to receive
	* @return nothing to return
	*/
	virtual void draw() = 0;

	/**
	* @brief Initializes the COM port
	* @param nothing to receive
	* @return nothing to return
	*/
	void run();
};