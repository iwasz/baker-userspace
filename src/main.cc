/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include <unistd.h>
#include <iostream>
#include "UsbService.h"
#include "Constants.h"

/*--------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
        UsbServiceGuard <UsbService> guard;
        UsbService::Buffer buffer (OUTPUT_DATA_SIZE);

        while (true) {
                guard.service.transmit (buffer);
                sleep (1);
        }
        return 0;
}


