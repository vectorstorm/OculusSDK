/************************************************************************************

 Filename    :   OVRSimpleImpl.h
 Created     :   July 3, 2013
 Authors     :   Brad Davis <bdavis@saintandreas.org>
 Copyright   :   Copyright 2013 Oculus VR, Inc. All Rights reserved.

 Use of this software is subject to the terms of the Oculus license
 agreement provided at the time of installation or download, or which
 otherwise accompanies this software in either electronic or hard copy form.

 *************************************************************************************/

#pragma once

#include "../OVRSimpleTypes.h"
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/smart_ptr.hpp>

#define DLL_PUBLIC __attribute__ ((visibility ("default")))
#define DLL_LOCAL  __attribute__ ((visibility ("hidden")))

namespace OVR {
namespace Simple {


class sensor;

typedef boost::shared_ptr<sensor> sensor_ptr ;


class DLL_PUBLIC impl {

protected:
    boost::asio::io_service         service;
    boost::asio::io_service::work   work;
    boost::asio::deadline_timer     timer;
    boost::thread                   thread_;
    std::vector<sensor_ptr>         devices;
    HANDLE_SENSOR *                 sensors;

    impl();

protected:
    virtual void populateDevices() = 0;
    sensor_ptr getDeviceForHandle(HANDLE_SENSOR sensor);

public:
    static impl * create();

    virtual ~impl();
    void run();
    void shutdown();
    HANDLE_SENSOR * enumerateSensors();


    virtual void registerCallback(HANDLE_SENSOR, SENSOR_CALLBACK) = 0;
    virtual void getDisplayInfo(HANDLE_SENSOR sensor, DisplayInfo * result);

};

class sensor {
protected:
    boost::asio::io_service &       service;
    boost::asio::deadline_timer     timer;
    std::string                     path;
    SENSOR_CALLBACK                 callback;

protected:
    sensor(boost::asio::io_service & service, const std::string & path);
    virtual ~sensor();
    bool sendKeepAlive(short duration = 3 * 1000);
    void onTimer(const boost::system::error_code& error);
    void onError(const boost::system::error_code& error);
    static void decodeTrackerBuffer(const uint8_t * data, TrackerMessage & result);

protected:
    virtual bool setFeatureReport(uint8_t* data, size_t length) = 0;
    virtual bool getFeatureReport(uint8_t* data, size_t length) = 0;
    virtual void close() = 0;

public:
    virtual void registerCallback(SENSOR_CALLBACK callback) = 0;
    virtual void getDisplayInfo(DisplayInfo * result);

};


} }

//public final class DisplayInfo extends HidFeatureReport {
//    public static final byte  FEATURE_ID   = 9;
//    public static final int   FEATURE_SIZE = 56;
//public final class SensorConfig extends HidFeatureReport {
//    public static final byte FEATURE_ID = 2;
//    public static final int FEATURE_SIZE = 7;
//public final class SensorRange extends HidFeatureReport {
//    public static final byte FEATURE_ID = 4;
//    public static final int FEATURE_SIZE = 8;
