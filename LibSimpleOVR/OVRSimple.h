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

#include <stdint.h>

typedef void* HANDLE_SENSOR;
typedef void (*SENSOR_CALLBACK)(const unsigned char *, unsigned int);

struct SensorRange {
    // Maximum detected acceleration in m/s^2. Up to 8*G equivalent support guaranteed,
    // where G is ~9.81 m/s^2.
    // Oculus DK1 HW has thresholds near: 2, 4 (default), 8, 16 G.
    float MaxAcceleration;
    // Maximum detected angular velocity in rad/s. Up to 8*Pi support guaranteed.
    // Oculus DK1 HW thresholds near: 1, 2, 4, 8 Pi (default).
    float MaxRotationRate;
    // Maximum detectable Magnetic field strength in Gauss. Up to 2.5 Gauss support guaranteed.
    // Oculus DK1 HW thresholds near: 0.88, 1.3, 1.9, 2.5 gauss.
    float MaxMagneticField;
};

struct DisplayInfo {
    uint8_t distortion;
    uint16_t xres, yres;
    uint32_t xsize, ysize;
    uint32_t center;
    uint32_t sep;
    uint32_t zeye[2];
    float distortionCoefficients[6];
};

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
