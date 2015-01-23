/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "UsbService.h"
#include "Constants.h"
#include "Exception.h"

/**
 * Internal structures hidden from the user.
 */
struct UsbService::Impl {
        libusb_device_handle *device = nullptr;
//        static void onControlTransferCompletion (libusb_transfer *xfr);
};

/*--------------------------------------------------------------------------*/

UsbService::UsbService ()
{
        impl = new Impl;
}

/*--------------------------------------------------------------------------*/

UsbService::~UsbService ()
{
        delete impl;
}

/*--------------------------------------------------------------------------*/

void UsbService::init ()
{
        int r = libusb_init(NULL);

        if (r < 0) {
                std::cerr << "Error libusb_init!" << std::endl;
                return;
        }

        impl->device = libusb_open_device_with_vid_pid (NULL, VENDOR_ID, PRODUCT_ID);

        if (!impl->device) {
                throw Exception ("Error finding USB device");
        } else {
                std::cerr << "USB device has been found." << std::endl;
        }

        int rc = libusb_set_auto_detach_kernel_driver (impl->device, true);

        if (rc == LIBUSB_SUCCESS) {
                std::cerr << "libusb_set_auto_detach_kernel_driver OK" << std::endl;
        }
        else {
                throw Exception ("libusb_set_auto_detach_kernel_driver Failed!");
        }

        rc = libusb_claim_interface(impl->device, 2);

        if (rc < 0) {
                throw Exception ("Error claiming interface : " + std::string (libusb_error_name(rc)));
        } else {
                std::cerr << "Interface claimed OK" << std::endl;
        }

        if ((rc = libusb_set_interface_alt_setting (impl->device, 2, 1)) != 0) {
                throw Exception ("Error libusb_set_interface_alt_setting : " + std::string (libusb_error_name(rc)));
        } else {
                std::cerr << "libusb_set_interface_alt_setting OK" << std::endl;
        }
}

/*--------------------------------------------------------------------------*/

void UsbService::destroy ()
{
        if (!libusb_release_interface(impl->device, 0)) {
                std::cerr << "Interface released OK" << std::endl;
        } else {
                throw Exception ("Error releasing interface.");
        }

        if (impl->device) {
                libusb_close(impl->device);
                std::cerr << "Device closed OK" << std::endl;
        }

        libusb_exit (NULL);
}

/*--------------------------------------------------------------------------*/

void UsbService::transmit (Buffer const &)
{
//        if (buf.size () != OUTPUT_DATA_SIZE) {
//                throw Exception ("UsbService::transmitConfiguration : wrong buffer size.");
//        }

#if 0
        std::cerr  << "Transmiting : ";
        for (unsigned int i = 0; i < buf.size(); ++i) {
                std::cerr << std::hex << (int)buf[i] << " ";
        }
        std::cerr  << std::endl;
#endif

//        int transferred = 0;
        uint8_t buff[12];
        int ret = libusb_control_transfer (impl->device,
                        LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_INTERFACE | LIBUSB_ENDPOINT_IN, // LIBUSB_RECIPIENT_DEVICE
                        GET_TEMP_REQUEST,
                        0x0000,
                        0x0000,
                        buff,
                        12,
                        0);

        float temp = (((buff[0] << 8) | buff[1]) >> 2) / 4.0;
        if (buff[0] & 0x80) {
                temp  = -temp;
        }

//        int16_t p = static_cast <uint16_t> (buff[4]) << 8 | buff[5];
//        int16_t i = static_cast <uint16_t> (buff[6]) << 8 | buff[7];

        int32_t p = *reinterpret_cast <int32_t *> (buff + 4);
        int32_t i = *reinterpret_cast <int32_t *> (buff + 8);

        std::cerr << std::hex << (int)buff[0] << " " <<(int)buff[1] << " " << (int)buff[2] << " " <<(int)buff[4] << std::dec <<
                        ", dec = "<< (int)buff[0] << ", " << (int)buff[1] << ", " << (int)buff[2] << ", " << (int)buff[3] <<
                        ", temp = " << temp << ", p = " << p << ", i = " << i << std::endl;

        if (ret >= 0) {
                return;
        }

        switch (ret) {
        case LIBUSB_ERROR_TIMEOUT:
                throw Exception ("UsbService::transmitConfiguration : timeout reached.");

        case LIBUSB_ERROR_PIPE:
                throw Exception ("UsbService::transmitConfiguration : the control request was not supported by the device.");

        case LIBUSB_ERROR_NO_DEVICE:
                throw Exception ("UsbService::transmitConfiguration : the device has been disconnected.");

        default:
                throw Exception ("UsbService::transmitConfiguration : undefined error : " + std::string (libusb_error_name(ret)) + ".");
        }
}
