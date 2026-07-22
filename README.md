# Additional Features
This fork adds extra functionality to the BlueBus when used in an E46 with the CD53 radio. 

## Seat/Mirror Memory

This overloads the functionality of button `5` on the CD53. Repeatedly pressing will cycle through the following modes:

1. Recall Mirror Memory
2. Save Mirror Memory
3. Device selection mode

When in modes (1) or (2), use buttons `1`, `2` or `3` to recall/save memory 1, 2 or 3. Mode (3) works like stock.

Additionally, a handler is added to fix an E46 bug where the passenger mirror won't exactly return to its previous position after auto dipping in reverse.

The handler is scheduled after the transmission is taken out of reverse. When the timer elapses, the handler checks that the transmission is still out of reverse and issues the "Recall Mem X" IBus command (where "X" is the selected memory index). This feature can be configured to recall any mirror memory or turned off entirely in the settings.

## Extended Low OBC

This feature adds an extra page to the low OBC on the IKE. This page can be configured to show any data that the BlueBus has access to. This includes data being relayed from Gauge.S through the `D Bus->IKE->I Bus` message relay trick.

Usage:
1. Ensure `Extended Low OBC` is enabled in the settings menu.
2. Click `R/T` button on the steering wheel to enter extended low OBC scroll mode.
3. Scroll through available changes with `next` and `prev` steering wheel (or radio) buttons. Note that the selected page will be shown on the CD53 screen.
4. Click `R/T` button on the steering wheel to save the page to display and exit extended low OBC scroll mode.
5. Use the `BC` button on the turn signal stalk to scroll through the OBC pages as normal. The extended page that is being served by the BlueBus will behave like an additional stock page.

Refer to `firmware/application/ui/extended_low_obc.h` for both the D Bus relay message format and the pages configured to be shown by this feature.

Refer to https://github.com/heinzboehmer/GaugeSConfigs for the appropriate Gauge.S config files.

## Auto D Bus GPS Time

If Gauge.S has a GPS module connected to it, it can send out the GPS time in the same D Bus command as used by the extended low OBC. On startup, the BlueBus will wait to receive a valid time from Gauge.S and then set it.

Enablement of this feature, current timezone and daylight savings offsets are configurable in the settings menu. Note that when adjusting the timezone or DST setting, the clock will reset to 00:00 until a valid time is received from Gauge.S.

## **_NOTE:_**

Please keep in mind that these features are hacked in and are very much E46 specific. This code should not be merged back to the BlueBus upstream source unless refactored to be cleaner and universal.