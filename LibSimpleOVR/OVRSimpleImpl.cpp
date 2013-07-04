#include "OVRSimpleImpl.h"

namespace OVR {

namespace Simple {

#define HID_BUFFER_SIZE 1024
static uint8_t HID_BUFFER[HID_BUFFER_SIZE];

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
    memset(HID_BUFFER, 0, HID_BUFFER_SIZE);
    HID_BUFFER[0] = 8;
    static short commandId = 0;
    memcpy(HID_BUFFER + 3, &duration, sizeof(duration));
    return setFeatureReport(HID_BUFFER, 5);
}

void sensor::getDisplayInfo(DisplayInfo * result) {
    memset(HID_BUFFER, 0, HID_BUFFER_SIZE);
    HID_BUFFER[0] = 9;
    getFeatureReport(HID_BUFFER, 56);
    memcpy(result, HID_BUFFER + 2, sizeof(DisplayInfo));
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

}
}

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

//
//    public DisplayInfo() {
//        super(FEATURE_ID, FEATURE_SIZE);
//    }
//
//    public DisplayInfo(HIDDevice device) throws IOException {
//        super(FEATURE_ID, FEATURE_SIZE, device);
//    }
//
//    @Override
//    protected void parse(ByteBuffer buffer) {
//        buffer.position(1);
//        commandId = buffer.getShort();
//        distortion = buffer.get();
//        xres = buffer.getShort();
//        yres = buffer.getShort();
//        xsize = buffer.getInt();
//        ysize = buffer.getInt();
//        center = buffer.getInt();
//        sep = buffer.getInt();
//        zeye = new int[2];
//        buffer.asIntBuffer().get(zeye);
//        buffer.position(buffer.position() + 8);
//        distortionCoefficients = new float[6];
//        buffer.asFloatBuffer().get(distortionCoefficients);
//        buffer.position(buffer.position() + 8);
//    }
//
//    @Override
//    protected void pack(ByteBuffer buffer) {
//        buffer.putShort(commandId);
//        buffer.put(distortion);
//        buffer.putShort(xres);
//        buffer.putShort(yres);
//        buffer.putInt(xsize);
//        buffer.putInt(ysize);
//        buffer.putInt(center);
//        buffer.putInt(sep);
//        for (int i = 0; i < 2; ++i) {
//            buffer.putInt(zeye[i]);
//        }
//        for (int i = 0; i < 6; ++i) {
//            buffer.putFloat(distortionCoefficients[i]);
//        }
//


}
