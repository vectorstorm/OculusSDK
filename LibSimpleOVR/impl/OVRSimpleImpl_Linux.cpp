#include "OVRSimpleImpl.h"

#include <string>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/hidraw.h>
#include <libudev.h>
#include <boost/lexical_cast.hpp>
#include <boost/system/linux_error.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::posix;
using namespace std;


namespace OVR {
namespace Simple {

struct sensor_linux : public sensor {
    typedef boost::asio::streambuf          buf;
    typedef shared_ptr<stream_descriptor>   strd_ptr;

    strd_ptr        sd;
    bool            read_only;
    buf             read_buf;

private:
    bool setFeatureReport(uint8_t* data, size_t length) {
        acquireDescriptor(false);
        int res = ioctl(sd->native(), HIDIOCSFEATURE(length), data);
        return res >= 0;
    }

    bool getFeatureReport(uint8_t* data, size_t length) {
        acquireDescriptor();
        int res = ioctl(sd->native(), HIDIOCGFEATURE(length), data);
        return res >= 0;
    }

    bool acquireDescriptor(bool read_only = true) {
        if (sd) {
            // if only read access is requested,
            // or if we already have read-write,
            // then we're done
            if (read_only || !this->read_only) {
                return true;
            }
        }
        // We're either opening up a new descriptor or switching from ro to rw
        int new_fd = open(path.c_str(), read_only ? O_RDONLY : O_RDWR);
        if (-1 == new_fd) {
            return false;
        }
        this->read_only = read_only;
        stream_descriptor * psd = new stream_descriptor(service, new_fd);
        sd = strd_ptr(psd, boost::bind(::close, new_fd));
        return true;
    }

    void close() {
        sd.reset();
    }

    void initializeRead() {
        boost::asio::async_read(*sd, read_buf,
                boost::bind(&sensor_linux::processReadResult, this, boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }

    void processReadResult(const boost::system::error_code& error, std::size_t length) {
        static TrackerMessage message;
        if (error) {
            onError(error);
            return;
        }

        if (length) {
            const uint8_t* p1 = boost::asio::buffer_cast<const unsigned char*>(read_buf.data());
            // We've got data.
            decodeTrackerBuffer(p1, message);
            if (callback) {
                (*callback)(message);
            }
            read_buf.consume(length);
        }
        initializeRead();
    }

public:
    sensor_linux(io_service & service, const string & path) :
        sensor(service, path), read_only(true), read_buf(62) {}

    void registerCallback(SENSOR_CALLBACK callback) {
        this->callback = callback;
        acquireDescriptor(false);
        onTimer(boost::system::error_code());
        initializeRead();
    }
};


class DLL_PUBLIC impl_linux: public impl {
    typedef shared_ptr<udev> udev_ptr;

    udev_ptr udev_;

public:
    impl_linux() : udev_(udev_new(), ptr_fun(udev_unref)) {
    }

protected:
    void populateDevices() {
        // List all the HID devices
        shared_ptr<udev_enumerate> enumerate(udev_enumerate_new(udev_.get()), ptr_fun(udev_enumerate_unref));
        udev_enumerate_add_match_subsystem(enumerate.get(), "hidraw");
        udev_enumerate_scan_devices(enumerate.get());

        udev_list_entry * entry = udev_enumerate_get_list_entry(enumerate.get());
        while (entry != NULL) {
            string hid_path = udev_list_entry_get_name(entry);
            udev_device * hid_dev = udev_device_new_from_syspath(udev_.get(), hid_path.c_str());
            string node = udev_device_get_devnode(hid_dev);

            shared_ptr<udev_device> usb_dev(
                    udev_device_get_parent_with_subsystem_devtype(hid_dev, "usb", "usb_device"),
                    ptr_fun(udev_device_unref));

            if (!usb_dev.get()) {
                throw "bad usb device";
            }

            string vendorId = udev_device_get_sysattr_value(usb_dev.get(), "idVendor");
            string productId = udev_device_get_sysattr_value(usb_dev.get(), "idProduct");
            if ("2833" == vendorId && "0001" == productId) {
                devices.push_back(sensor_ptr(new sensor_linux(service, node)));
            }
            entry = udev_list_entry_get_next(entry);
        }
        sensors = new HANDLE_SENSOR[devices.size() + 1];
        for (size_t i = 0; i < devices.size(); ++i) {
            sensors[i] = devices[i].get();
        }
        sensors[devices.size()] = 0;
    }

    void registerCallback(HANDLE_SENSOR sensor, SENSOR_CALLBACK callback) {
        for (size_t i = 0; i < devices.size();  ++i) {
            if (sensors[i] == sensor) {
                devices[i]->registerCallback(callback);
            }
        }
    }

}; // class

impl * impl::create() {
    return new impl_linux();
}

} }

