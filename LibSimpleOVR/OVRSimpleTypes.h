#pragma once

#include <stdint.h>

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

union vector3_16_t {
    int16_t         values[3];
    struct {
        int16_t     x;
        int16_t     y;
        int16_t     z;
    };
};

struct TrackerVector {
    vector3_16_t    accel;
    vector3_16_t    gyro;
};

struct TrackerMessage {
    uint8_t         type;
    uint8_t         count;
    uint16_t        timestamp;
    uint16_t        command_id;
    uint16_t        temperature;
    TrackerVector   samples[3];
    vector3_16_t    mag;
};


typedef void* HANDLE_SENSOR;
typedef void (*SENSOR_CALLBACK)(const TrackerMessage &);

