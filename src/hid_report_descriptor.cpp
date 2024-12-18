/*
 * Copyright (C) 2020  Dygma Lab S.L.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * The purpose of this module is for defining the HID descriptors and functions for
 * getting the USB and BLE HID descriptor versions in hidDefy module
 */

#include "hidDefy.h"

const uint8_t hid_report_descriptor_defy[] = HID_DEFY_REPORT_DESCRIPTOR( RAW_USAGE_DEFY );

void hid_report_descriptor_get( const uint8_t ** pp_desc, uint32_t * p_desc_len )
{
    *pp_desc = &hid_report_descriptor_defy[0];
    *p_desc_len = sizeof( hid_report_descriptor_defy );
}

void hid_report_descriptor_usb_get( const uint8_t ** pp_desc, uint32_t * p_desc_len )
{
    /* Get the valid descriptor */
    hid_report_descriptor_get( pp_desc, p_desc_len );
}
