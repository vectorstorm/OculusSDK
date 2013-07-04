/************************************************************************************

 Filename    :   OVR.h
 Content     :   This contains references to all OVR-specific headers in Src folder.
 Should be generated automatically based on PublicHeader tags.

 Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

 Use of this software is subject to the terms of the Oculus license
 agreement provided at the time of installation or download, or which
 otherwise accompanies this software in either electronic or hard copy form.

 *************************************************************************************/

#pragma once
#include "OVRSimpleTypes.h"

extern "C" {

/**
 * Initialize the Oculus VR library
 */
void ovrInit();
/**
 * Shutdown the Oculus VR library and release any resources
 */
void ovrShutdown();

/**
 * Return a null terminated list of sensors
 */
HANDLE_SENSOR* ovrEnumerateSensors();

/**
 * Sets report rate (in Hz) of MessageBodyFrame messages (delivered through
 * MessageHandler::OnMessage call).
 *
 * Currently supported maximum rate is 1000Hz. If the rate is set to 500 or 333 Hz then
 * OnMessage will be called twice or thrice at the same 'tick'.
 *
 * If the rate is  < 333 then the OnMessage / MessageBodyFrame will be called three
 * times for each 'tick': the first call will contain averaged values, the second
 * and third calls will provide with most recent two recorded samples.
 */
void ovrSetSensorRate(HANDLE_SENSOR sensor, unsigned int rate);
unsigned int ovrGetSensorRate(HANDLE_SENSOR sensor);


/**
 * Sets maximum range settings for the sensor described by SensorRange.
 * The function will fail if you try to pass values outside Maximum supported
 * by the HW, as described by SensorInfo.
 */
void ovrSetSensorRange(HANDLE_SENSOR sensor, SensorRange * ranges);
void ovrGetSensorRange(HANDLE_SENSOR sensor, SensorRange * ranges);

void ovrGetDisplayInfo(HANDLE_SENSOR sensor, DisplayInfo * result);

void ovrSensorRegisterCallback(HANDLE_SENSOR sensor, SENSOR_CALLBACK callback);

/**
 * Sets maximum range settings for the sensor described by SensorRange.
 * The function will fail if you try to pass values outside Maximum supported
 * by the HW, as described by SensorInfo.
 */
void ovrSensorSetRange(HANDLE_SENSOR sensor, SensorRange * ranges);

}
