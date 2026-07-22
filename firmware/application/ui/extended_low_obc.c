/*
 * File: extended_low_obc.c
 * Author: heinzboehmer (but you can steal all of this, Ted)
 * Description:
 *     Implement the extended low OBC display.
 */
#include "extended_low_obc.h"
#include <string.h>
#include "../lib/config.h"
#include "../lib/event.h"
#include "../lib/ibus.h"
#include "../lib/log.h"
#include "../lib/timer.h"

static uint8_t EXTENDED_LOW_OBC_PAGES[EXTENDED_LOW_OBC_LAST_PAGE + 1] = {
    EXTENDED_LOW_OBC_GEAR_PAGE,
    EXTENDED_LOW_OBC_GEAR_OVERREV_PAGE,
    EXTENDED_LOW_OBC_RPM_PAGE,
    EXTENDED_LOW_OBC_RPM_OVERREV_PAGE,
    EXTENDED_LOW_OBC_OIL_TEMP_PAGE,
    EXTENDED_LOW_OBC_COOLANT_TEMP_PAGE,
    EXTENDED_LOW_OBC_VEHICLE_SPEED_PAGE,
    EXTENDED_LOW_OBC_46_M_PAGE,
    EXTENDED_LOW_OBC_LAST_PAGE
};

// Try to limit to 11 characters so the CD53 display doesn't have to scroll.
static char *EXTENDED_LOW_OBC_PAGES_NAMES_MAP[EXTENDED_LOW_OBC_LAST_PAGE + 1] = {
    "Gear",
    "Gear Overrv",
    "RPM",
    "RPM Overrev",
    "Oil Temp",
    "Coolant Tmp",
    "Vehicle Spd",
    "46M",
    "Invalid"
};

static uint8_t EXTENDED_LOW_OBC_PAGES_BCD_VALUES_MAP[EXTENDED_LOW_OBC_LAST_PAGE];

static uint8_t EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[EXTENDED_LOW_OBC_LAST_PAGE];

// Maps the gear to the max RPM you can be at to not overrev the engine on a downshift.
static uint16_t MONEY_SHIFT_RPM_MAP[NUM_GEARS_GETRAG_420G] = {
    S54_REDLINE,
    (S54_REDLINE * (GETRAG_420G_RATIO_SECOND / GETRAG_420G_RATIO_FIRST)),
    (S54_REDLINE * (GETRAG_420G_RATIO_THIRD / GETRAG_420G_RATIO_SECOND)),
    (S54_REDLINE * (GETRAG_420G_RATIO_FOURTH / GETRAG_420G_RATIO_THIRD)),
    (S54_REDLINE * (GETRAG_420G_RATIO_FIFTH / GETRAG_420G_RATIO_FOURTH)),
    (S54_REDLINE * (GETRAG_420G_RATIO_SIXTH / GETRAG_420G_RATIO_FIFTH))
};

// Be lazy and copy Casio-style timezones (e.g. https://www.casio.com/content/dam/casio/global/support/manuals/watches/pdf/32/3299/qw3299_EN.pdf).
// If we really want to complicate this in the future, we can have Gauge.S send
// out latitude, longitude and date, then do a lookup to find the appropriate
// timezone and DST setting.
static const DBusTimeZone_t DBusTimeZones[] = {
    {"PPG", "Pago Pago", -11, 0, 0},
    {"HNL", "Honolulu", -10, 0, 0},
    {"ANC", "Anchorage", -9, 0, 60},
    {"LAX", "Los Angeles", -8, 0, 60},
    {"YVR", "Vancouver", -7, 0, 0},
    {"YEA", "Edmonton", -7, 0, 60},
    {"DEN", "Denver", -7, 0, 60},
    {"MEX", "Mexico City", -6, 0, 0},
    {"CHI", "Chicago", -6, 0, 60},
    {"NYC", "New York City", -5, 0, 60},
    {"SCL", "Santiago", -4, 0, 60},
    {"YHZ", "Halifax", -4, 0, 60},
    {"YYT", "St. Johns", -3, -30, 60},
    {"RIO", "Rio de Janeiro", -3, 0, 0},
    {"FEN", "Fernando de Noronha", -2, 0, 0},
    {"RAI", "Praia", -1, 0, 0},
    {"UTC", "UTC", 0, 0, 0},
    {"LIS", "Lisbon", 0, 0, 60},
    {"LON", "London", 0, 0, 60},
    {"MAD", "Madrid", 1, 0, 60},
    {"PAR", "Paris", 1, 0, 60},
    {"ROM", "Rome", 1, 0, 60},
    {"BER", "Berlin", 1, 0, 60},
    {"STO", "Stockholm", 1, 0, 60},
    {"ATH", "Athens", 2, 0, 60},
    {"CAI", "Cairo", 2, 0, 60},
    {"JRS", "Jerusalem", 2, 0, 60},
    {"MOW", "Moscow", 3, 0, 0},
    {"JED", "Jeddah", 3, 0, 0},
    {"THR", "Tehran", 3, 30, 0},
    {"DXB", "Dubai", 4, 0, 0},
    {"KBL", "Kabul", 4, 30, 0},
    {"KHI", "Karachi", 5, 0, 0},
    {"DEL", "Delhi", 5, 30, 0},
    {"KTM", "Kathmandu", 5, 45, 0},
    {"DAC", "Dhaka", 6, 0, 0},
    {"RGN", "Yangon", 6, 30, 0},
    {"BKK", "Bangkok", 7, 0, 0},
    {"SIN", "Singapore", 8, 0, 0},
    {"HKG", "Hong Kong", 8, 0, 0},
    {"BJS", "Beijing", 8, 0, 0},
    {"TPE", "Taipei", 8, 0, 0},
    {"SEL", "Seoul", 9, 0, 0},
    {"TYO", "Tokyo", 9, 0, 0},
    {"ADL", "Adelaide", 9, 30, 60},
    {"GUM", "Guam", 10, 0, 0},
    {"SYD", "Sydney", 10, 0, 60},
    {"NOU", "Noumea", 11, 0, 0},
    {"WLG", "Wellington", 12, 0, 60}
};

#define NUM_TIME_ZONES (sizeof(DBusTimeZones) / sizeof(DBusTimeZones[0]))

/**
 * ExtendedLowObcInit()
 *     Description:
 *         Initialize the struct for the extended low OBC
 *     Params:
 *         ExtendedLowObcContext_t *context - Pointer to the ExtendedLowObcContext_t struct
 *         IBus_t *ibus - Pointer to the IBus_t struct
 *     Returns:
 *         void
 */
void ExtendedLowObcInit(
    ExtendedLowObcContext_t *context,
    IBus_t *ibus
) {
    context->ibus = ibus;
    context->displayMode = EXTENDED_LOW_OBC_DISPLAY_STATIC;
    context->currentPage = ConfigGetSetting(CONFIG_SETTING_EXTENDED_LOW_OBC_LAST_PAGE_ADDRESS);
    context->currentDisplayedValue = 0;
    context->currentDisplayedFormat = IBUS_IKE_LOW_OBC_FORMAT_CLEAR;
    context->gearUpdateStatus = GEAR_0_UPDATE_UNSCHEDULED;
    context->dBusTimeStatus = D_BUS_TIME_NOT_SET;

    memset(
        &EXTENDED_LOW_OBC_PAGES_BCD_VALUES_MAP,
        0,
        EXTENDED_LOW_OBC_LAST_PAGE
    );
    memset(
        &EXTENDED_LOW_OBC_PAGES_FORMATS_MAP,
        IBUS_IKE_LOW_OBC_FORMAT_CLEAR,
        EXTENDED_LOW_OBC_LAST_PAGE
    );
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_46_M_PAGE, 46, BCD_FORMAT_APPEND_M);

    EventRegisterCallback(
        IBUS_EVENT_LOW_OBC_D_BUS_VALUES_UPDATE,
        &ExtendedLowObcDBusValuesUpdate,
        context
    );
    EventRegisterCallback(
        IBUS_EVENT_CLEAR_LOW_OBC,
        &ClearExtendedLowObc,
        context
    );
    EventRegisterCallback(
        IBUS_EVENT_LOW_OBC_SET_LAST_PAGE,
        &ExtendedLowObcSetLastPage,
        context
    );
    EventRegisterCallback(
        IBUS_EVENT_D_BUS_TIME_UNSET,
        &DBusTimeUnset,
        context
    );
}

/**
 * ExtendedLowObcDestroy()
 *     Description:
 *         Unregister all scheduled tasks
 *     Params:
 *         void
 *     Returns:
 *         void
 */
void ExtendedLowObcDestroy()
{
    // Cleanup is done in this roundabout way because we don't have access to
    // the context in this scope.
    TimerUnregisterScheduledTask(&ExtendedLowObcRefreshHandler);
    EventTriggerCallback(IBUS_EVENT_CLEAR_LOW_OBC, 0);
    EventTriggerCallback(IBUS_EVENT_LOW_OBC_SET_LAST_PAGE, 0);

    EventUnregisterCallback(
        IBUS_EVENT_LOW_OBC_D_BUS_VALUES_UPDATE,
        &ExtendedLowObcDBusValuesUpdate
    );
    EventUnregisterCallback(
        IBUS_EVENT_CLEAR_LOW_OBC,
        &ClearExtendedLowObc
    );
    EventUnregisterCallback(
        IBUS_EVENT_LOW_OBC_SET_LAST_PAGE,
        &ExtendedLowObcSetLastPage
    );
    EventUnregisterCallback(
        IBUS_EVENT_D_BUS_TIME_UNSET,
        &DBusTimeUnset
    );
}

void Gear0UpdateHandler(ExtendedLowObcContext_t *context) {
    if (ConfigGetSetting(CONFIG_SETTING_EXTENDED_LOW_OBC) == CONFIG_SETTING_OFF) {
        return;
    }
    
    // Timeout expired, store 0.
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_GEAR_PAGE, 0, BCD_FORMAT_OMIT_M);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_GEAR_OVERREV_PAGE, 0, BCD_FORMAT_OMIT_M);

    // Refresh low OBC if either of these values are in focus.
    if (context->currentPage == EXTENDED_LOW_OBC_GEAR_PAGE ||
        context->currentPage == EXTENDED_LOW_OBC_GEAR_OVERREV_PAGE
    ) {
        ExtendedLowObcRefreshHandler(context);
    }

    // Don't update `context->gearUpdateStatus`, as we still want to know if
    // the handler has already been scheduled (even if the timeout has expired)
    // for this most recent `0` gear. The only thing that should update
    // `context->gearUpdateStatus` is an event where `gear != 0`, otherwise
    // this handler will continue to get scheduled as long as `gear == 0`.
}

/**
 * ExtendedLowObcDBusValuesUpdate()
 *     Description:
 *         Handle update events when D Bus values are received. These are expected
 *         to be sent by Gauge.S and relayed by the IKE onto I Bus.
 *     Params:
 *         void *ctx - Pointer to the context
 *         uint8_t *pkt - Pointer to the IBus packet
 *     Returns:
 *         void
 */
void ExtendedLowObcDBusValuesUpdate(void *ctx, uint8_t *pkt)
{
    ExtendedLowObcContext_t *context = (ExtendedLowObcContext_t *) ctx;

    // Gauge.S will report 00:00 (HH:MM) if it doesn't have GPS lock. Although
    // this _is_ a valid time, we assume it's not. It's much more likely that
    // we're not getting valid data than it is that the car was started at
    // exactly midnight. Worst case, a minute elapses and we set the time.
    if (ConfigGetSetting(CONFIG_SETTING_DBUS_AUTO_TIME) == CONFIG_SETTING_ON) {
        uint8_t hour = pkt[IBUS_RELAY_PKT_DB7];
        uint8_t minute = pkt[IBUS_RELAY_PKT_DB8];
        if (context->dBusTimeStatus == D_BUS_TIME_NOT_SET &&
            !(hour == 0 && minute == 0) &&
            hour < 24 &&
            minute < 60
        ) {
            DBusSetTime(context, hour, minute);
        }
    }

    if (ConfigGetSetting(CONFIG_SETTING_EXTENDED_LOW_OBC) == CONFIG_SETTING_OFF) {
        return;
    }

    uint8_t gear = pkt[IBUS_RELAY_PKT_DB1];
    uint16_t rpm = (pkt[IBUS_RELAY_PKT_DB2] << 8 | pkt[IBUS_RELAY_PKT_DB3]);
    uint8_t moneyShiftAppendM = BCD_FORMAT_OMIT_M;

    // Use 0 as a failsafe if we receive unexpected values from Gauge.S.
    if (gear > NUM_GEARS_GETRAG_420G) {
        LogError("Invalid gear received: %d", gear);
        gear = 0;
    } else {
        // Special case buffering for gear updates. If the newly reported gear
        // is 0 (i.e. neutral or clutch in), we push out the update. This
        // avoids having the gear display jump around on gear changes.
        if (gear == 0 && context->gearUpdateStatus == GEAR_0_UPDATE_SCHEDULED) {
            // We've already scheduled the handler, move along.
        } else if (gear == 0 && context->gearUpdateStatus == GEAR_0_UPDATE_UNSCHEDULED) {
            TimerRegisterScheduledTask(
                &Gear0UpdateHandler,
                context,
                GEAR_0_UPDATE_TIMEOUT_MS
            );
            context->gearUpdateStatus = GEAR_0_UPDATE_SCHEDULED;
        } else if (gear != 0 && context->gearUpdateStatus == GEAR_0_UPDATE_SCHEDULED) {
            // If we're here, this means we just went into a gear, but are
            // displaying 0. We should update the status immediately so the
            // display gets updated (and unschedule the `0` gear handler).
            TimerUnregisterScheduledTask(&Gear0UpdateHandler);
            context->gearUpdateStatus = GEAR_0_UPDATE_UNSCHEDULED;
        }

        if (gear != 0 && rpm > MONEY_SHIFT_RPM_MAP[gear - 1]) {
            moneyShiftAppendM = BCD_FORMAT_APPEND_M;
        }

        // Make sure only `Gear0UpdateHandler()` does the `0` update.
        if (gear != 0 && context->gearUpdateStatus == GEAR_0_UPDATE_UNSCHEDULED) {
            UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_GEAR_PAGE, gear, BCD_FORMAT_OMIT_M);
            UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_GEAR_OVERREV_PAGE, gear, moneyShiftAppendM);
        }
    }

    // Update all other values unconditionally.
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_RPM_PAGE, rpm, BCD_FORMAT_OMIT_M);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_RPM_OVERREV_PAGE, rpm, moneyShiftAppendM);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_OIL_TEMP_PAGE, pkt[IBUS_RELAY_PKT_DB4], BCD_FORMAT_OMIT_M);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_COOLANT_TEMP_PAGE, pkt[IBUS_RELAY_PKT_DB5], BCD_FORMAT_OMIT_M);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_VEHICLE_SPEED_PAGE, pkt[IBUS_RELAY_PKT_DB6], BCD_FORMAT_OMIT_M);

    // Refresh low OBC only if the value has changed, to avoid unnecessary
    // noise on the I bus.
    if (EXTENDED_LOW_OBC_PAGES_BCD_VALUES_MAP[context->currentPage] != context->currentDisplayedValue ||
        EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[context->currentPage] != context->currentDisplayedFormat
    ) {
        ExtendedLowObcRefreshHandler(context);
    }
}

/**
 * ClearExtendedLowObc()
 *     Description:
 *         Clears the extended low OBC. 
 *     Params:
 *         void *ctx - Pointer to the context
 *         uint8_t *pkt - unused
 *     Returns:
 *         void
 */
void ClearExtendedLowObc(void *ctx, uint8_t *unused)
{
    ExtendedLowObcContext_t *context = (ExtendedLowObcContext_t *) ctx;

    TimerUnregisterScheduledTask(&ExtendedLowObcRefreshHandler);

    IBusCommandIKENumbericDisplayWrite(
        context->ibus,
        0,
        IBUS_IKE_LOW_OBC_FORMAT_CLEAR
    );
    context->currentDisplayedFormat = IBUS_IKE_LOW_OBC_FORMAT_CLEAR;
}

/**
 * ExtendedLowObcSetLastPage()
 *     Description:
 *         Saves the last page selected on the low OBC into the EEPROM.
 *     Params:
 *         void *ctx - Pointer to the context
 *         uint8_t *pkt - unused
 *     Returns:
 *         void
 */
void ExtendedLowObcSetLastPage(void *ctx, uint8_t *unused) {
    ExtendedLowObcContext_t *context = (ExtendedLowObcContext_t *) ctx;
    ConfigSetSetting(CONFIG_SETTING_EXTENDED_LOW_OBC_LAST_PAGE_ADDRESS, context->currentPage);
}

/**
 * Uint16ToLowObcFormattedBcd()
 *     Description:
 *         Translate a uint16_t value into a rounded uint8_t BCD representation of the two most
 *         significant digits and a multiplier that will make the low OBC display the value
 *         with the dame number of places as input.
 *           e.g.
 *             Uint16ToLowObcFormattedBcd(1234, false, &result) -> true
 *             result.bcdValue = 0x12, result.format = IBUS_IKE_LOW_OBC_FORMAT_X100
 *             
 *     Params:
 *         uint16_t value - The value to be displayed in the low OBC.
 *         uint8_t append_m - Flag that controls whether or not to append an "M" to data
 *                         displayed in the low OBC.
 *         LowObcDisplayResult_t *result - Pointer to the destination result struct where the
 *                                         calculated BCD value and format multiplier are stored.
 *     Returns:
 *         uint8_t - true if translation succeeded, false otherwise.
 */
uint8_t Uint16ToLowObcFormattedBcd(uint16_t value, uint8_t appendM, LowObcDisplayResult_t *result)
{
    if (value > 9999) {
        // We can't display anything with five places, so immediately return invalid
        // format flag. Caller is expected to handle this gracefully and not update
        // the display when this flag is present.
        return BCD_FORMAT_TRANSLATE_FAILURE;
    }
    
    // Greedy pick smallest multiplier to keep as much precision as possible from the
    // input value.
    uint8_t outputValue;
    if (value < 100) {
        outputValue = value;
        result->format = IBUS_IKE_LOW_OBC_FORMAT_X1;
    } else if (value + 5 < 1000) {
        // We add 5 to the input to mimic rounding after the integer division. Values
        // >= 995 will round to >= 1000 and be handled by the next block.
        outputValue = (value + 5) / 10;
        result->format = IBUS_IKE_LOW_OBC_FORMAT_X10;
    } else if (value + 50 < 10000) {
        // We add 50 to the input to mimic rounding after the integer division.
        outputValue = (value + 50) / 100;
        result->format = IBUS_IKE_LOW_OBC_FORMAT_X100;
    } else if (value >= 9950 && value < 10000) {
        // This covers an edge case at the very top of the range we can display. The
        // previous block discards any numbers past 9950, since they round to >= 10000.
        // Instead, we choose to not round these and clamp to 9900.
        outputValue = value / 100;
        result->format = IBUS_IKE_LOW_OBC_FORMAT_X100;
    } else {
        // We should never get here thanks to the check at the beginning of the function,
        // but no harm in keeping.
        return BCD_FORMAT_TRANSLATE_FAILURE;
    }

    // Deal with the "m".
    if (appendM == BCD_FORMAT_APPEND_M) {
        result->format &= ~IBUS_IKE_LOW_OBC_FORMAT_M_BIT_MASK;
    }

    // Convert to BCD. output_value is guaranteed to be <= 99 by the conditionals above.
    uint8_t tensPlace = outputValue / 10;
    uint8_t onesPlace = outputValue % 10;
    result->bcdValue = (tensPlace << 4) | (onesPlace);

    return BCD_FORMAT_TRANSLATE_SUCCESS;
}

void UpdateExtendedLowObcPage(uint8_t page, uint16_t value, uint8_t appendM)
{
    if (page >= EXTENDED_LOW_OBC_LAST_PAGE) {
        LogError("Invalid page to update for extended low OBC: 0x%X", page);
        return;
    }

    LowObcDisplayResult_t displayResult;
    Uint16ToLowObcFormattedBcd(value, appendM, &displayResult);

    EXTENDED_LOW_OBC_PAGES_BCD_VALUES_MAP[page] = displayResult.bcdValue;
    EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[page] = displayResult.format;
}

void ExtendedLowObcRefreshHandler(ExtendedLowObcContext_t *context)
{
    // Unschedule handler in case there was an async refresh.
    TimerUnregisterScheduledTask(&ExtendedLowObcRefreshHandler);

    // Display current extended low OBC page
    if (context->currentPage >= EXTENDED_LOW_OBC_LAST_PAGE) {
        LogError("Invalid current page for extended low OBC: 0x%X", context->currentPage);
    } else if (EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[context->currentPage] == IBUS_IKE_LOW_OBC_FORMAT_CLEAR ||
               EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[context->currentPage] == IBUS_IKE_LOW_OBC_INVALID_FORMAT
      ){
        LogError(
            "Invalid format (0x%X) for extended low OBC page 0x%X",
            EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[context->currentPage],
            context->currentPage
        );
    } else {
        // If the only change from the previous thing being displayed is
        // whether or not to append an "m", the low OBC will NOT update.
        // Check to see if this is the case and clear before writing to
        // work around this limitation.
        if ((context->currentDisplayedValue == EXTENDED_LOW_OBC_PAGES_BCD_VALUES_MAP[context->currentPage]) &&
            ((context->currentDisplayedFormat & IBUS_IKE_LOW_OBC_FORMAT_M_BIT_MASK) !=
             (EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[context->currentPage] & IBUS_IKE_LOW_OBC_FORMAT_M_BIT_MASK))
        ) {
            EventTriggerCallback(IBUS_EVENT_CLEAR_LOW_OBC, 0);
        }
        IBusCommandIKENumbericDisplayWrite(
            context->ibus,
            EXTENDED_LOW_OBC_PAGES_BCD_VALUES_MAP[context->currentPage],
            EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[context->currentPage]
        );
        context->currentDisplayedValue = EXTENDED_LOW_OBC_PAGES_BCD_VALUES_MAP[context->currentPage];
        context->currentDisplayedFormat = EXTENDED_LOW_OBC_PAGES_FORMATS_MAP[context->currentPage];
    }

    // Reschedule same handler to run before the stock IKE timeout expires.
    TimerRegisterScheduledTask(
        &ExtendedLowObcRefreshHandler,
        context,
        EXTENDED_LOW_OBC_REFRESH_INT_MS
    );
}

/**
 * ExtendedLowObcMenuScroll()
 *     Description:
 *         Scrolls extended low OBC and returns new page name
 *     Params:
 *         ExtendedLowObcContext_t *context - Pointer to the ExtendedLowObcContext_t struct
 *         unsigned char direction - Backwards or forwards (0x01 and 0x00 respectively)
 *     Returns:
 *         char *
 */
void ExtendedLowObcMenuScroll(ExtendedLowObcContext_t *context, unsigned char direction)
{
    // Don't try scrolling if the extended low OBC is supposed to be static.
    if (context->displayMode == EXTENDED_LOW_OBC_DISPLAY_STATIC) {
        return;
    }

    uint8_t currentPage = context->currentPage;
    if (direction == 0x00) {
        if (EXTENDED_LOW_OBC_PAGES[currentPage + 1] == EXTENDED_LOW_OBC_LAST_PAGE) {
            // Wrap back around to the start.
            context->currentPage = EXTENDED_LOW_OBC_PAGES[0];
        } else {
            context->currentPage = EXTENDED_LOW_OBC_PAGES[currentPage + 1];
        }
    } else {
        if (EXTENDED_LOW_OBC_PAGES[currentPage] == EXTENDED_LOW_OBC_PAGES[0]) {
            // Wrap around to the end, keeping in mind that the last page should never be used.
            context->currentPage = EXTENDED_LOW_OBC_PAGES[EXTENDED_LOW_OBC_LAST_PAGE - 1];
        } else {
            context->currentPage = EXTENDED_LOW_OBC_PAGES[currentPage - 1];
        }
    }

    // Refresh display immediately after a scroll event.
    ExtendedLowObcRefreshHandler(context);
}

char *GetExtendedLowObcPageName(uint8_t page)
{
    if (page >= EXTENDED_LOW_OBC_LAST_PAGE) {
        return EXTENDED_LOW_OBC_PAGES_NAMES_MAP[EXTENDED_LOW_OBC_LAST_PAGE];
    }

    return EXTENDED_LOW_OBC_PAGES_NAMES_MAP[page];
}

/**
 * GetNextDBusTimezoneIndex()
 *     Description:
 *         Scrolls `DBusTimeZones` array and handles wrapping. Returns the index that
 *         was scrolled to.
 *     Params:
 *         uint8_t index - Index in the timezone array
 *         uint8_t direction - Backwards or forwards (0x01 and 0x00 respectively)
 *     Returns:
 *         uint8_t
 */
uint8_t GetNextDBusTimezoneIndex(uint8_t index, uint8_t direction) {
    if (direction == 0x01) {
        if (index == 0) {
            return NUM_TIME_ZONES - 1;
        } else {
            return index - 1;
        }
    } else {
        if (index + 1 == NUM_TIME_ZONES) {
            return 0;
        } else {
            return index + 1;
        }
    }
}

/**
 * GetDBusTimezone()
 *     Description:
 *         Returns a reference to the const DBusTimeZone_t object that corresponds to the
 *         given index. 
 *     Params:
 *         uint8_t index - Index in the timezone array
 *     Returns:
 *         const DBusTimeZone_t*
 */
const DBusTimeZone_t* GetDBusTimezone(uint8_t index) {
    if (index > NUM_TIME_ZONES - 1) {
        LogError("Invalid timezone index: %d", index);
        return NULL;
    }

    return &DBusTimeZones[index];
}

/**
 * DBusSetTime()
 *     Description:
 *         Unsets `dBusTimeStatus`. This is intended to be used as a force refresh
 *         for the clock. The clock will be set next time `ExtendedLowObcDBusValuesUpdate()`
 *         is triggered and a valid time is received.
 *     Params:
 *         void *ctx - Pointer to the context
 *         uint8_t *pkt - unused
 *     Returns:
 *         void
 */
void DBusTimeUnset(void *ctx, uint8_t *unused) {
    ExtendedLowObcContext_t *context = (ExtendedLowObcContext_t *) ctx;

    // We don't have a way of knowing the current set time, so reset the clock
    // for now. The next trigger of `ExtendedLowObcDBusValuesUpdate()`, where
    // the D Bus packet contains a valid time, will set the real time.
    IBusCommandIKESetTime(context->ibus, 0, 0);
    context->dBusTimeStatus = D_BUS_TIME_NOT_SET;
}

/**
 * DBusSetTime()
 *     Description:
 *         Sets the time on the I Bus using the configured timezone and the values passed
 *         to it. These values are expected to come from D Bus. 
 *     Params:
 *         ExtendedLowObcContext_t *context - Pointer to the ExtendedLowObcContext_t struct
 *         uint8_t utcHour - Current hour in UTC
 *         uint8_t utcMinute - Current minute in UTC
 *     Returns:
 *         void
 */
void DBusSetTime(ExtendedLowObcContext_t *context, uint8_t utcHour, uint8_t utcMinute) {
    if (ConfigGetSetting(CONFIG_SETTING_DBUS_AUTO_TIME) == CONFIG_SETTING_OFF) {
        return;
    }

    const DBusTimeZone_t* dBusTimezone = GetDBusTimezone(ConfigGetSetting(CONFIG_SETTING_DBUS_TIMEZONE_IDX));
    if (dBusTimezone == NULL) {
        LogError("Invalid timezone.");
        return;
    }

    int16_t totalMinuteOffset = (dBusTimezone->utcOffsetHours * 60) + dBusTimezone->utcOffsetMinutes;
    if (totalMinuteOffset < -(24 * 60) || totalMinuteOffset > (24 * 60)) {
        LogError("Timezone offset out of bounds: %d", totalMinuteOffset);
        return;
    }

    // Handle daylight savings.
    if (ConfigGetSetting(CONFIG_SETTING_DBUS_DST) == CONFIG_SETTING_ON) {
        totalMinuteOffset += dBusTimezone->dstOffsetMinutes; 
    }

    int16_t localMinutes = (utcHour * 60) + utcMinute + totalMinuteOffset;

    // Handle 24-hour wrapping.
    if (localMinutes < 0) {
        localMinutes += 24 * 60;
    }
    localMinutes = localMinutes % (24 * 60);


    IBusCommandIKESetTime(context->ibus, (uint8_t)(localMinutes / 60), (uint8_t)(localMinutes % 60));
    context->dBusTimeStatus = D_BUS_TIME_SET;
}