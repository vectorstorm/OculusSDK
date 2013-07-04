#include "OVRSimpleImpl.h"

namespace OVR {

namespace Simple {

impl::impl() :
        service(), work(service), timer(service), thread_(boost::bind(&impl::run, this)) {
}

void impl::shutdown() {
    service.stop();
    thread_.join();
}

impl::~impl() {
}

sensor_ptr impl::getDeviceForHandle(HANDLE_SENSOR sensor) {
    for (size_t i = 0; i < devices.size();  ++i) {
        if (sensors[i] == sensor) {
            return devices[i];
        }
    }
    return sensor_ptr();
}

void impl::run() {
    while (!service.stopped()) {
        service.run();
    }
}

HANDLE_SENSOR * impl::enumerateSensors() {
    if (!sensors) {
        populateDevices();
    }
    return sensors;
}

void impl::getDisplayInfo(HANDLE_SENSOR sensor, DisplayInfo * result) {
    sensor_ptr psensor = getDeviceForHandle(sensor);
    if (psensor) {
        psensor->getDisplayInfo(result);
    }
}



sensor::sensor(boost::asio::io_service & service, const std::string & path) :
        service(service), timer(service), path(path), callback(NULL) {
}

sensor::~sensor() {
}

bool sensor::sendKeepAlive(short duration) {
    static uint8_t KEEP_ALIVE_BUFFER[5];
    KEEP_ALIVE_BUFFER[0] = 8;
    memcpy(KEEP_ALIVE_BUFFER + 3, &duration, sizeof(duration));
    return setFeatureReport(KEEP_ALIVE_BUFFER, 5);
}

void sensor::getDisplayInfo(DisplayInfo * result) {
    static uint8_t DISPLAY_INFO_BUFFER[56];
    memset(DISPLAY_INFO_BUFFER, 0, 56);
    DISPLAY_INFO_BUFFER[0] = 9;
    getFeatureReport(DISPLAY_INFO_BUFFER, 56);
    memcpy(result, DISPLAY_INFO_BUFFER + 2, sizeof(DisplayInfo));
}

void sensor::onError(const boost::system::error_code& error) {
    close();
    std::cerr << error << std::endl;
}

void sensor::onTimer(const boost::system::error_code& error) {
    if (error) {
        onError(error);
        return;
    }
    // trigger the reads
    sendKeepAlive();

    std::cout << "timer" << std::endl;
    // trigger the next timer event
    timer.expires_from_now(boost::posix_time::seconds(3));
    timer.async_wait(boost::bind(&sensor::onTimer, this, _1));
}

static void unpackSensor(const uint8_t * buffer, vector3_16_t & result) {
    // Sign extending trick
    // from http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
    struct {int32_t x:21;} s;

    result.x = s.x = (buffer[0] << 13) | (buffer[1] << 5) | ((buffer[2] & 0xF8) >> 3);
    result.y = s.x = ((buffer[2] & 0x07) << 18) | (buffer[3] << 10) | (buffer[4] << 2) |
               ((buffer[5] & 0xC0) >> 6);
    result.z = s.x = ((buffer[5] & 0x3F) << 15) | (buffer[6] << 7) | (buffer[7] >> 1);
}

void sensor::decodeTrackerBuffer(const uint8_t * data, TrackerMessage & result) {
    memset(&result, 0, sizeof(TrackerMessage));
    // Need to fix endian-ness here.
    memcpy(&result, data, 8);  // Copy type, count, lastCommandId, and temperature
    data += 8;
    for (int i = 0; i < 3; ++i) {
        unpackSensor(data, result.samples[i].accel);
        data += sizeof(vector3_16_t);
        unpackSensor(data, result.samples[i].gyro);
        data += sizeof(vector3_16_t);
    }
    memcpy(&result.mag, data, sizeof(vector3_16_t));

}

} // namespace Simple

} // namespace OVR

using namespace OVR::Simple;

typedef boost::shared_ptr<impl> ovr_impl_ptr;

boost::mutex GLOBAL_MUTEX;
ovr_impl_ptr GLOBAL_IMPL;

extern "C" {

DLL_PUBLIC void ovrInit() {
    boost::mutex::scoped_lock lock(GLOBAL_MUTEX);
    if (GLOBAL_IMPL) {
        return;
    }
    GLOBAL_IMPL = ovr_impl_ptr(impl::create());
}

DLL_PUBLIC void ovrShutdown() {
    boost::mutex::scoped_lock lock(GLOBAL_MUTEX);
    if (!GLOBAL_IMPL) {
        return;
    }
    impl * impl = GLOBAL_IMPL.get();
    impl->shutdown();
    GLOBAL_IMPL.reset();
}

DLL_PUBLIC HANDLE_SENSOR * ovrEnumerateSensors() {
    boost::mutex::scoped_lock lock(GLOBAL_MUTEX);
    impl * impl = GLOBAL_IMPL.get();
    return impl->enumerateSensors();
}

DLL_PUBLIC void ovrSensorRegisterCallback(HANDLE_SENSOR sensor, SENSOR_CALLBACK callback) {
    boost::mutex::scoped_lock lock(GLOBAL_MUTEX);
    impl * impl = GLOBAL_IMPL.get();
    impl->registerCallback(sensor, callback);
}

DLL_PUBLIC void ovrGetDisplayInfo(HANDLE_SENSOR sensor, DisplayInfo * result) {
    boost::mutex::scoped_lock lock(GLOBAL_MUTEX);
    impl * impl = GLOBAL_IMPL.get();
    impl->getDisplayInfo(sensor, result);
}

}
