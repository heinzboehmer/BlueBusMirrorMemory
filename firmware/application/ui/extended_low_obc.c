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

// Array of structs involves more overhead than the multiple array strategy
// seen in places like `menu_singleline.c`, but it makes the code easier to
// follow. We have the resources needed to handle the overhead, so choose to
// go down this path anyway.

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
    context->currentDisplayedFormat = IBUS_IKE_LOW_OBC_FORMAT_CLEAR;
    context->gearUpdateStatus = GEAR_0_UPDATE_UNSCHEDULED;

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
}

void Gear0UpdateHandler(ExtendedLowObcContext_t *context) {
    if (ConfigGetSetting(CONFIG_SETTING_EXTENDED_LOW_OBC) == CONFIG_SETTING_OFF) {
        return;
    }
    
    // Timeout expired, display 0.
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
    // `context->gearUpdateStatus` is an event where `gear != 0`.
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
    if (ConfigGetSetting(CONFIG_SETTING_EXTENDED_LOW_OBC) == CONFIG_SETTING_OFF) {
        return;
    }

    ExtendedLowObcContext_t *context = (ExtendedLowObcContext_t *) ctx;
    uint8_t gear = pkt[IBUS_RELAY_PKT_DB1];
    uint16_t rpm = (pkt[IBUS_RELAY_PKT_DB2] << 8 | pkt[IBUS_RELAY_PKT_DB3]);
    uint8_t money_shift_append_m = (rpm > MONEY_SHIFT_RPM_MAP[gear - 1]) ? BCD_FORMAT_APPEND_M : BCD_FORMAT_OMIT_M;

    // Special case buffering for gear updates. If the newly reported gear is 0
    // (i.e. neutral or clutch in), we push out the update. This avoids having
    // the gear display jump around on gear changes.
    if (gear == 0 && context->gearUpdateStatus == GEAR_0_UPDATE_UNSCHEDULED) {
        TimerRegisterScheduledTask(
            &Gear0UpdateHandler,
            context,
            GEAR_0_UPDATE_TIMEOUT_MS
        );
        context->gearUpdateStatus = GEAR_0_UPDATE_SCHEDULED;
    } else if (gear == 0 && context->gearUpdateStatus == GEAR_0_UPDATE_SCHEDULED) {
        // We've already scheduled the handler, move along.
    } else if (gear != 0 && context->gearUpdateStatus == GEAR_0_UPDATE_SCHEDULED) {
        // If we're here, this means we just went into a gear, but are
        // displaying 0. We should update the value immediately (and unschedule
        // the `0` gear handler).
        TimerUnregisterScheduledTask(&Gear0UpdateHandler);
        context->gearUpdateStatus = GEAR_0_UPDATE_UNSCHEDULED;
        UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_GEAR_PAGE, gear, BCD_FORMAT_OMIT_M);
        UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_GEAR_OVERREV_PAGE, gear, money_shift_append_m);
    } else /*(gear != 0 && context->gearUpdateStatus == GEAR_0_UPDATE_UNSCHEDULED)*/ {
        // Otherwise this is just a normal update.
        UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_GEAR_PAGE, gear, BCD_FORMAT_OMIT_M);
        UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_GEAR_OVERREV_PAGE, gear, money_shift_append_m);
    }

    // Update all other values unconditionally.
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_RPM_PAGE, rpm, BCD_FORMAT_OMIT_M);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_RPM_OVERREV_PAGE, rpm, money_shift_append_m);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_OIL_TEMP_PAGE, pkt[IBUS_RELAY_PKT_DB4], BCD_FORMAT_OMIT_M);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_COOLANT_TEMP_PAGE, pkt[IBUS_RELAY_PKT_DB5], BCD_FORMAT_OMIT_M);
    UpdateExtendedLowObcPage(EXTENDED_LOW_OBC_VEHICLE_SPEED_PAGE, pkt[IBUS_RELAY_PKT_DB6], BCD_FORMAT_OMIT_M);

    // Refresh low OBC if any of these values are in focus.
    if (context->currentPage == EXTENDED_LOW_OBC_GEAR_PAGE ||
        context->currentPage == EXTENDED_LOW_OBC_GEAR_OVERREV_PAGE ||
        context->currentPage == EXTENDED_LOW_OBC_RPM_PAGE ||
        context->currentPage == EXTENDED_LOW_OBC_RPM_OVERREV_PAGE ||
        context->currentPage == EXTENDED_LOW_OBC_OIL_TEMP_PAGE ||
        context->currentPage == EXTENDED_LOW_OBC_COOLANT_TEMP_PAGE ||
        context->currentPage == EXTENDED_LOW_OBC_VEHICLE_SPEED_PAGE
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
uint8_t Uint16ToLowObcFormattedBcd(uint16_t value, uint8_t append_m, LowObcDisplayResult_t *result)
{
    if (value > 9999) {
        // We can't display anything with five places, so immediately return invalid
        // format flag. Caller is expected to handle this gracefully and not update
        // the display when this flag is present.
        return BCD_FORMAT_TRANSLATE_FAILURE;
    }
    
    // Greedy pick smallest multiplier to keep as much precision as possible from the
    // input value.
    uint8_t output_value;
    if (value < 100) {
        output_value = value;
        result->format = IBUS_IKE_LOW_OBC_FORMAT_X1;
    } else if (value + 5 < 1000) {
        // We add 5 to the input to mimic rounding after the integer division. Values
        // >= 995 will round to >= 1000 and be handled by the next block.
        output_value = (value + 5) / 10;
        result->format = IBUS_IKE_LOW_OBC_FORMAT_X10;
    } else if (value + 50 < 10000) {
        // We add 50 to the input to mimic rounding after the integer division.
        output_value = (value + 50) / 100;
        result->format = IBUS_IKE_LOW_OBC_FORMAT_X100;
    } else if (value >= 9950 && value < 10000) {
        // This covers an edge case at the very top of the range we can display. The
        // previous block discards any numbers past 9950, since they round to >= 10000.
        // Instead, we choose to not round these and clamp to 9900.
        output_value = value / 100;
        result->format = IBUS_IKE_LOW_OBC_FORMAT_X100;
    } else {
        // We should never get here thanks to the check at the beginning of the function,
        // but no harm in keeping.
        return BCD_FORMAT_TRANSLATE_FAILURE;
    }

    // Deal with the "m".
    if (append_m == BCD_FORMAT_APPEND_M) {
        result->format &= ~IBUS_IKE_LOW_OBC_FORMAT_M_BIT_MASK;
    }

    // Convert to BCD. output_value is guaranteed to be <= 99 by the conditionals above.
    uint8_t tens_place = output_value / 10;
    uint8_t ones_place = output_value % 10;
    result->bcdValue = (tens_place << 4) | (ones_place);

    return BCD_FORMAT_TRANSLATE_SUCCESS;
}

void UpdateExtendedLowObcPage(uint8_t page, uint16_t value, uint8_t append_m)
{
    if (page >= EXTENDED_LOW_OBC_LAST_PAGE) {
        LogError("Invalid page to update for extended low OBC: 0x%X", page);
        return;
    }

    LowObcDisplayResult_t displayResult;
    Uint16ToLowObcFormattedBcd(value, append_m, &displayResult);

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

    uint8_t current_page = context->currentPage;
    if (direction == 0x00) {
        if (EXTENDED_LOW_OBC_PAGES[current_page + 1] == EXTENDED_LOW_OBC_LAST_PAGE) {
            // Wrap back around to the start.
            context->currentPage = EXTENDED_LOW_OBC_PAGES[0];
        } else {
            context->currentPage = EXTENDED_LOW_OBC_PAGES[current_page + 1];
        }
    } else {
        if (EXTENDED_LOW_OBC_PAGES[current_page] == EXTENDED_LOW_OBC_PAGES[0]) {
            // Wrap around to the end, keeping in mind that the last page should never be used.
            context->currentPage = EXTENDED_LOW_OBC_PAGES[EXTENDED_LOW_OBC_LAST_PAGE - 1];
        } else {
            context->currentPage = EXTENDED_LOW_OBC_PAGES[current_page - 1];
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