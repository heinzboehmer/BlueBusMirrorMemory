/*
 * File: extended_low_obc.h
 * Author: heinzboehmer (but you can steal all of this, Ted)
 * Description:
 *     Implement the extended low OBC display.
 */
#include "../lib/ibus.h"

#ifndef EXTENDED_LOW_OBC_H
#define EXTENDED_LOW_OBC_H

#define EXTENDED_LOW_OBC_DISPLAY_STATIC 0
#define EXTENDED_LOW_OBC_DISPLAY_SCROLL 1
#define EXTENDED_LOW_OBC_STOCK_TIMEOUT_MS 15000
#define EXTENDED_LOW_OBC_REFRESH_INT_MS ((uint16_t)(0.75 * EXTENDED_LOW_OBC_STOCK_TIMEOUT_MS))
#define EXTENDED_LOW_OBC_CD53_NAME_DISPLAY_TIMEOUT 10

#define EXTENDED_LOW_OBC_GEAR_PAGE 0
#define EXTENDED_LOW_OBC_GEAR_OVERREV_PAGE 1
#define EXTENDED_LOW_OBC_RPM_PAGE 2
#define EXTENDED_LOW_OBC_RPM_OVERREV_PAGE 3
#define EXTENDED_LOW_OBC_OIL_TEMP_PAGE 4
#define EXTENDED_LOW_OBC_COOLANT_TEMP_PAGE 5
#define EXTENDED_LOW_OBC_VEHICLE_SPEED_PAGE 6
#define EXTENDED_LOW_OBC_46_M_PAGE 7
#define EXTENDED_LOW_OBC_LAST_PAGE 8

// Packet relayed from Gauge.S is expected to look like so:
// | Source | Length | Destination | Gear | RpmHi | RpmLo | Oil Temp | Coolant Temp | Vehicle Speed | GPS Hour | GPS Minute | Checksum |
#define IBUS_RELAY_PKT_SRC 0
#define IBUS_RELAY_PKT_LEN 1
#define IBUS_RELAY_PKT_DST 2
#define IBUS_RELAY_PKT_DB1 3
#define IBUS_RELAY_PKT_DB2 4
#define IBUS_RELAY_PKT_DB3 5
#define IBUS_RELAY_PKT_DB4 6
#define IBUS_RELAY_PKT_DB5 7
#define IBUS_RELAY_PKT_DB6 8
#define IBUS_RELAY_PKT_DB7 9
#define IBUS_RELAY_PKT_DB8 10

#define S54_REDLINE 8000
#define NUM_GEARS_GETRAG_420G 6
#define GETRAG_420G_RATIO_FIRST 4.227
#define GETRAG_420G_RATIO_SECOND 2.528
#define GETRAG_420G_RATIO_THIRD 1.669
#define GETRAG_420G_RATIO_FOURTH 1.226
#define GETRAG_420G_RATIO_FIFTH 1.000
#define GETRAG_420G_RATIO_SIXTH 0.828
#define GEAR_0_UPDATE_UNSCHEDULED 0
#define GEAR_0_UPDATE_SCHEDULED 1
#define GEAR_0_UPDATE_TIMEOUT_MS 1500

#define BCD_FORMAT_TRANSLATE_FAILURE 0
#define BCD_FORMAT_TRANSLATE_SUCCESS 1
#define BCD_FORMAT_OMIT_M 0
#define BCD_FORMAT_APPEND_M 1

#define D_BUS_TIME_NOT_SET 0
#define D_BUS_TIME_SET 1

typedef struct {
    const char *shortName;
    const char *fullName;
    int8_t utcOffsetHours;
    int8_t utcOffsetMinutes;
    uint8_t dstOffsetMinutes;
} DBusTimeZone_t;

typedef struct ExtendedLowObcContext_t {
    IBus_t *ibus;
    uint8_t displayMode: 1;
    uint8_t currentPage;
    uint8_t currentDisplayedValue;
    uint8_t currentDisplayedFormat;
    uint8_t gearUpdateStatus: 1;
    uint8_t dBusTimeStatus: 1;
} ExtendedLowObcContext_t;

typedef struct {
    uint8_t bcdValue;
    uint8_t format;
} LowObcDisplayResult_t;

void ExtendedLowObcInit(ExtendedLowObcContext_t *, IBus_t *);
void ExtendedLowObcDestroy();
void ExtendedLowObcDBusValuesUpdate(void *, uint8_t *);
void ClearExtendedLowObc(void *, uint8_t *);
void ExtendedLowObcSetLastPage(void*, uint8_t*);
uint8_t Uint16ToLowObcFormattedBcd(uint16_t, uint8_t, LowObcDisplayResult_t *);
void UpdateExtendedLowObcPage(uint8_t, uint16_t, uint8_t);
void ExtendedLowObcRefreshHandler(ExtendedLowObcContext_t *);
void ExtendedLowObcMenuScroll(ExtendedLowObcContext_t *, unsigned char);
char *GetExtendedLowObcPageName(uint8_t);
uint8_t GetNextDBusTimezoneIndex(uint8_t, uint8_t);
const DBusTimeZone_t* GetDBusTimezone(uint8_t);
void DBusTimeUnset(void *, uint8_t *);
void DBusSetTime(ExtendedLowObcContext_t *, uint8_t, uint8_t);
#endif /* EXTENDED_LOW_OBC_H */
