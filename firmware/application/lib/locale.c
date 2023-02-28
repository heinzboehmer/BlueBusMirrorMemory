/*
 * File:   locale.c
 * Author: Aliaksei Prybytkin <alexeypribytkin@gmail.com>
 * Description:
 *     Contains functionality for string localization
 */
#include "locale.h"

static char *LOCALE_LANG_ENGLISH[] = {
    "- Not Playing -",
    "About",
    "Audio",
    "Autoplay: Off",
    "Autoplay: On",
    "Back",
    "Blinkers: %d",
    "Bluetooth",
    "Built: %d/%d",
    "Calling",
    "Car: E3x/E53",
    "Car: E46/Z4",
    "Car: Unset",
    "Clear Pairings",
    "Comfort",
    "Dashboard",
    "Devices",
    "DSP: Default",
    "DSP: Analog",
    "DSP: Digital",
    "FW: %s",
    "Handsfree: Off",
    "Handsfree: On",
    "Lock: 10km/h",
    "Lock: 20km/h",
    "Lock: Off",
    "Main Menu",
    "Menu: Dashboard",
    "Menu: Main",
    "Metadata: Chunk",
    "Metadata: Off",
    "Metadata: Party",
    "Mic Bias: Off",
    "Mic Bias: On",
    "Mic Gain: %idB",
    "No Device",
    "Pairing: Off",
    "Pairing: On",
    "S/N: %u",
    "Settings",
    "Settings > About",
    "Settings > Audio",
    "Settings > Calling",
    "Settings > Comfort",
    "Settings > UI",
    "Temps: Coolant",
    "Temps: Off",
    "Temps: Ambient",
    "Temps: Oil",
    "UI",
    "Unknown Artist",
    "Unknown Title",
    "Unlock: Off",
    "Unlock: Pos 0",
    "Unlock: Pos 1",
    "Volume: -%ddB",
    "Volume: +%ddB",
    "Volume: +24dB",
    "Volume: 0dB",
    "Lang: %s",
    "Park Lts.: On",
    "Park Lts.: Off",
    "Vol. Mgmt: On",
    "Vol. Mgmt: Off",
    "Vol. Rev: On",
    "Vol. Rev: Off",
    "Dash. OBC: On",
    "Dash. OBC: Off",
    "BMBT Off: Off",
    "BMBT Off: On",
    "Vol. Offset: %+d",
    "Mode: Default",
    "Mode: TCU",
    "Mode: No Mute",
    "Voice Assistant",
    "Call",
};

static char *LOCALE_LANG_ITALIAN[] = {
    "- In Pausa -",
    "Info su...",
    "Audio",
    "Autoplay: Off",
    "Autoplay: On",
    "Indietro",
    "Lampeggi: %d",
    "Bluetooth",
    "Built: %d/%d",
    "Chiamate",
    "Auto: E3x/E53",
    "Auto: E46/Z4",
    "Auto: -?-",
    "Disassocia",
    "Comfort",
    "Dashboard",
    "Dispositivi",
    "DSP: Predefini.",
    "DSP: Analogico",
    "DSP: Digitale",
    "FW: %s",
    "Vivavoce: Off",
    "Vivavoce: On",
    "Porte: 10km/h",
    "Porte: 20kmh",
    "Porte: Off",
    "Menu Principale",
    "Menu: Dashboard",
    "Menu: Iniziale",
    "Metadati: Chunk",
    "Metadati: Off",
    "Metadati: Party",
    "Mic Bias: Off",
    "Mic Bias: On",
    "Mic Gain: %idB",
    "Non Associato",
    "Pairing: Off",
    "Pairing: On",
    "N. Seriale: %u",
    "Impostazioni",
    "Impostazioni > Info",
    "Impostazioni > Audio",
    "Impostazioni > Chiamate",
    "Impostazioni > Comfort",
    "Impostazioni > Aspetto",
    "Temp: Acqua",
    "Temp: Spe.",
    "Temp: Ambiente",
    "Temp: Olio",
    "Interfaccia",
    "Artista non noto",
    "Brano non noto",
    "Apertura: Off",
    "Apertura: Pos 0",
    "Apertura: Pos 1",
    "Volume: -%ddB",
    "Volume: +%ddB",
    "Volume: +24dB",
    "Volume: 0dB",
    "Lingua: %s",
    "Park Lts.: Off",
    "Park Lts.: On",
    "Vol. Mgmt: Off",
    "Vol. Mgmt: On",
    "Vol. Rev: Off",
    "Vol. Rev: On",
    "Dash. OBC: On",
    "Dash. OBC: Off",
    "BMBT Off: Off",
    "BMBT Off: On",
    "Vol. Offset: %+d",
    "Mode: Default",
    "Mode: TCU",
    "Mode: No Mute",
    "Voice Assistant",
    "Telefonata",
};

static char *LOCALE_LANG_DUTCH[] = {
    "- Geen weergave -",
    "Info",
    "Audio",
    "Autoplay: Uit",
    "Autoplay: Aan",
    "Terug",
    "Knipperl: %d",
    "Bluetooth",
    "Bouw: %d/%d",
    "Oproep",
    "Auto: E3X/E53",
    "Auto: E46/Z4",
    "Auto: NB",
    "Reset koppeling",
    "Luxe opties",
    "Muziekweergave",
    "Apparaten",
    "DSP: Standaard",
    "DSP: Analoog",
    "DSP: Digitaal",
    "FW: %s",
    "Handsfree: Uit",
    "Handsfree: Aan",
    "Slot: 10km/h",
    "Slot: 20km/h",
    "Slot: Uit",
    "Hoofdmenu",
    "Menu: Dashboard",
    "Menu: Menu",
    "Weergave: Deel",
    "Weergave: Uit",
    "Weergave: Lopend",
    "Mic Bias: Uit",
    "Mic Bias: Aan",
    "Mic Gain: %idB",
    "Geen Apparaat",
    "Koppelen: Uit",
    "Koppelen: Aan",
    "Serienr: %u",
    "Instellingen",
    "Instellingen > Over",
    "Instellingen > Geluid",
    "Instellingen > Telefonie",
    "Instellingen > Luxe opties",
    "Instellingen > Weergave",
    "Temp: Koelwater",
    "Temp: Uit",
    "Temp: Lucht",
    "Temp: Olie",
    "Weergave",
    "Onbekend Artiest",
    "Onbekend Titel",
    "Open slot: Uit",
    "Open slot: Pos 0",
    "Open slot: Pos 1",
    "Volume: -%ddB",
    "Volume: +%ddB",
    "Volume: +24dB",
    "Volume: 0dB",
    "Taal: %s",
    "Parklicht: Aan",
    "Parklicht: Uit",
    "Vol. Mgmt: Aan",
    "Vol. Mgmt: Uit",
    "Vol. Rev: Aan",
    "Vol. Rev: Uit",
    "Dashtemp: Aan",
    "Dashtemp: Uit",
    "Schermlock: Uit",
    "Schermlock: Aan",
    "Vol. Offset: %+d",
    "Mode: Default",
    "Mode: TCU",
    "Mode: No Mute",
    "Voice Assistant",
    "Oproep",
};

static char *LOCALE_LANG_ESTONIAN[] = {
    "- Ei mängi -",
    "Teave",
    "Audio",
    "Aut.esitlus: Off",
    "Aut.esitlus: On",
    "Tagasi",
    "Suunatuled: %d",
    "Bluetooth",
    "Andmestik: %d/%d",
    "Kõne",
    "Auto: E3x/E53",
    "Auto: E46/Z4",
    "Auto: Määramata",
    "Katkesta ühendus",
    "Mugavus",
    "Audio inf",
    "Seadmed",
    "DSP: Vaikimisi",
    "DSP: Analoogne",
    "DSP: Digitaalne",
    "Püsivara: %s",
    "Handsfree: Off",
    "Handsfree: On",
    "Aut.lukk: 10km/h",
    "Aut.lukk: 20km/h",
    "Aut.lukk: OFF",
    "Peamenüü",
    "Menüü: Audio inf",
    "Menüü: Põhi",
    "Metandmed: Osal.",
    "Metandmed: OFF",
    "Metandmed: Järj.",
    "Mikri Bias: Off",
    "Mikri Bias: On",
    "Mikri Võim: %idB",
    "Seadet pole",
    "Ühilduvus: Off",
    "Ühilduvus: On",
    "S/N: %u",
    "Seaded",
    "Seaded > Teave",
    "Seaded > Audio",
    "Seaded > Kõne",
    "Seaded > Mugavus",
    "Seaded > KasLiid",
    "Temp: Jahutusved",
    "Temp: Off",
    "Temp: Ambient",
    "Temp: Oil",
    "Kasutajaliides",
    "Tundmatu Artist",
    "Tundmatu Tiitel",
    "Lukuvaba: Off",
    "Lukuvaba: Asend0",
    "Lukuvaba: Asend1",
    "Heli: -%ddB",
    "Heli: +%ddB",
    "Heli: +24dB",
    "Heli: 0dB",
    "Keel: %s",
    "Park Lts.: On",
    "Park Lts: Off",
    "Vol. Mgmt: On",
    "Vol. Mgmt: Off",
    "Rev. Vol.: On",
    "Rev. Vol.: Off",
    "Põhi OBC: On",
    "Põhi OBC: Off",
    "BMBT Off: Off",
    "BMBT Off: On",
    "Vol. Offset: %+d",
    "Mode: Default",
    "Mode: TCU",
    "Mode: No Mute",
    "Voice Assistant",
    "Kõne",
};

static char LOCALE_LANG_RUSSIAN[][24] = {
    {205,229,' ',226,238,241,239,240,238,232,231,226,238,228,232,242,241,255,0},
    {206,' ',239,240,238,227,240,224,236,236,229,0},
    {192,243,228,232,238,0},
    {192,226,242,238,226,238,241,239,':',' ',194,251,234,235,0},
    {192,226,242,238,226,238,241,239,':',' ',194,234,235,0},
    {205,224,231,224,228,0},
    {207,238,226,238,240,238,242,237,232,234,232,':',' ','%','d',0},
    {'B','l','u','e','t','o','o','t','h',0},
    {209,225,238,240,234,224,':',' ','%','d','/','%','d',0},
    {199,226,238,237,234,232,0},
    {202,243,231,238,226,':',' ','E','3','x','/','E','5','3',0},
    {202,243,231,238,226,':',' ','E','4','6','/','Z','4',0},
    {202,243,231,238,226,':',' ',205,229,243,241,242,0},
    {206,247,232,241,242,232,242,252,' ',241,226,255,231,232,0},
    {202,238,236,244,238,240,242,0},
    {206,225,231,238,240,237,238,229,' ',236,229,237,254,0},
    {211,241,242,240,238,233,241,242,226,224,0},
    "DSP: Default",
    {'D','S','P',':',' ',192,237,224,235,238,227,238,226,251,233,0},
    {'D','S','P',':',' ',214,232,244,240,238,226,238,233,0},
    {'F','W',':',' ','%','s',0},
    {195,240,' ',241,226,255,231,252,':',' ',194,251,234,235,0},
    {195,240,' ',241,226,255,231,252,':',' ',194,234,235,0},
    {199,224,234,240,':',' ','1','0',' ',234,236,'/',247,0},
    {199,224,234,240,':',' ','2','0',' ',234,236,'/',247,0},
    {199,224,234,240,':',' ',194,251,234,235,0},
    {195,235,224,226,237,238,229,' ',236,229,237,254,0},
    {204,229,237,254,':',' ',206,225,231,238,240,237,238,229,0},
    {204,229,237,254,':',' ',195,235,224,226,237,238,229,0},
    {204,'.',228,224,237,237,251,229,':',' ',215,224,241,242,252,0},
    {204,'.',228,224,237,237,251,229,':',' ',194,251,234,235,0},
    {204,'.',228,224,237,237,251,229,':',' ',193,229,227,0},
    {205,224,239,240,255,230,' ',236,232,234,':',' ',194,251,234,235,0},
    {205,224,239,240,255,230,' ',236,232,234,':',' ',194,234,235,0},
    {211,241,232,235,' ',236,232,234,240,':',' ','%','i','d','B',0},
    {205,229,242,' ',243,241,242,240,238,233,241,242,226,224,0},
    {209,238,239,240,255,230,':',' ',194,251,234,235,0},
    {209,238,239,240,255,230,':',' ',194,234,235,0},
    {'S','/','N',':',' ','%','u',0},
    {205,224,241,242,240,238,233,234,232,0},
    {205,224,241,242,240,238,233,234,232,' ','>',' ',206,' ',239,240,238,227,240,224,236,236,229,0},
    {205,224,241,242,240,238,233,234,232,' ','>',' ',192,243,228,232,238,0},
    {205,224,241,242,240,238,233,234,232,' ','>',' ',199,226,238,237,234,232,0},
    {205,224,241,242,240,238,233,234,232,' ','>',' ',202,238,236,244,238,240,242,0},
    {205,224,241,242,240,238,233,234,232,' ','>',' ',200,237,242,229,240,244,229,233,241,0},
    {210,229,236,239,'-',240,224,':',' ',206,198,0},
    {210,229,236,239,'-',240,224,':',' ',194,251,234,235,0},
    "Temp: Ambient",
    "Temp: Oil",
    {200,237,242,229,240,244,229,233,241,0},
    {205,229,232,231,226,229,241,242,237,251,233,' ',232,241,239,238,235,237,232,242,229,235,252,0},
    {205,229,232,231,226,229,241,242,237,238,229,' ',237,224,231,226,224,237,232,229,0},
    {206,242,234,240,':',' ',194,251,234,235,0},
    {206,242,234,240,':',' ',207,238,235,238,230,' ','0',0},
    {206,242,234,240,':',' ',207,238,235,238,230,' ','1',0},
    {195,240,238,236,234,':',' ','-','%','d','d','B',0},
    {195,240,238,236,234,':',' ','+','%','d','d','B',0},
    {195,240,238,236,234,':',' ','+','2','4','d','B',0},
    {195,240,238,236,234,':',' ','0','d','B',0},
    {223,231,251,234,':',' ','%','s',0},
    "Park Lts.: Off",
    "Park Lts.: On",
    "Vol. Mgmt: Off",
    "Vol. Mgmt: On",
    "Vol. Rev: Off",
    "Vol. Rev: On",
    "Dash. OBC: On",
    "Dash. OBC: Off",
    "BMBT Off: Off",
    "BMBT Off: On",
    "Vol. Offset: %+d",
    "Mode: Default",
    "Mode: TCU",
    "Mode: No Mute",
    "Voice Assistant",
    "Call",
};

static char *LOCALE_LANG_GERMAN[] = {
    "- Keine Wiedergabe -",
    "\xdc""ber",
    "Audio",
    "Autoplay: Aus",
    "Autoplay: An",
    "Zur\xfc""ck",
    "Blinker: %d",
    "Bluetooth",
    "Herst-D.: %d/%d",
    "Anruf",
    "Fhrz.: E3x/E53",
    "Fhrz.: E46/Z4",
    "Fhrz.: keins",
    "Zur\xfc""cksetzen",
    "Komfort",
    "\xdc""bersicht",
    "Ger\xe4""te",
    "DSP: Standard",
    "DSP: Analog",
    "DSP: Digital",
    "FW: %s",
    "FSE: Aus",
    "FSE: An",
    "Verrieg.: 10km/h",
    "Verrieg.: 20km/h",
    "Verrieg.: Aus",
    "Hauptmen\xfc""",
    "Men\xfc"": \xdc""bersicht",
    "Men\xfc"": Haupt",
    "Meta: Abschnitt",
    "Meta: Aus",
    "Meta: Party",
    "Mikro Bias: Aus",
    "Mikro Bias: Ein",
    "Mikro Zuw: %idB",
    "Kein Ger\xe4""t",
    "Kopplung: Aus",
    "Kopplung: Ein",
    "S/N: %u",
    "Einstellungen",
    "Einstellungen > \xdc""ber",
    "Einstellungen > Audio",
    "Einstellungen > Anruf",
    "Einstellungen > Komfort",
    "Einstellungen > Anzeige",
    "Temp: Wasser",
    "Temp: Aus",
    "Temp: Ambient",
    "Temp: Oil",
    "Anzeige",
    "Unbek. K\xfc""nstler",
    "Unbek. Titel",
    "Entrieg: Aus",
    "Entrieg: Pos 0",
    "Entrieg: Pos 1",
    "Lautst: -%ddB",
    "Lautst: +%ddB",
    "Lautst: +24dB",
    "Lautst: 0dB",
    "Sprache: %s",
    "Park Lts.: Ein",
    "Park Lts.: Aus",
    "Vol. Mgmt: Ein",
    "Vol. Mgmt: Aus",
    "Vol. Rev: Ein",
    "Vol. Rev: Aus",
    "Haupt OBC: Ein",
    "Haupt OBC: Aus",
    "BMBT Off: Aus",
    "BMBT Off: Ein",
    "Vol. Offset: %+d",
    "Mode: Default",
    "Mode: TCU",
    "Mode: No Mute",
    "Voice Assistant",
    "Anruf",
};

static char *LOCALE_LANG_SPANISH[] = {
    "- En Pausa -",
    "Informaci\xd3""n",
    "Audio",
    "Autoplay: Apg.",
    "Autoplay: Enc.",
    "Atr\xe1""s",
    "Intermitente: %d",
    "Bluetooth",
    "Fabric.: %d/%d",
    "Llamando",
    "Modelo: E3x/E53",
    "Modelo: E46/Z4",
    "Modelo: Ninguno",
    "Desenlazar",
    "Confort",
    "Tablero",
    "Dispositivos",
    "DSP: Est\xe1ndar",
    "DSP: Anal\xf3""gico",
    "DSP: Digital",
    "FW: %s",
    "Manos Lib.: Enc.",
    "Manos Lib.: Apg.",
    "Cierre: 10km/h",
    "Cierre: 20km/h",
    "Cierre: Apg.",
    "Men\xfa"" Principal",
    "Men\xfa"": Dashboard",
    "Men\xfa"": Principal",
    "Metadatos: Trozo",
    "Metadatos: Apg.",
    "Metadatos: Fest.",
    "Mic Parc.: Apg.",
    "Mic Parc.: Enc.",
    "Gana Mic: %idB",
    "Sin Dispositivo",
    "Enlazar: Enc.",
    "Enlazar: Apg.",
    "N/S: %u",
    "Ajustes",
    "Ajustes > Informaci\xd3""n",
    "Ajustes > Audio",
    "Ajustes > Llamada",
    "Ajustes > Confort",
    "Ajustes > Interfaz",
    "Temper.: Refrig",
    "Temper.: Apg.",
    "Temper.: Ambient",
    "Temper.: Oil",
    "Interfaz",
    "Artista Desc.",
    "T\xed""tulo Desc.",
    "Abrir: Apg.",
    "Abrir.: Pos 0",
    "Abrir.: Pos 1",
    "Volumen: -%ddB",
    "Volumen: +%ddB",
    "Volumen: +24dB",
    "Volumen: 0dB",
    "Idioma: %s",
    "Park Lts.: Enc.",
    "Park Lts.: Apg.",
    "Vol. Mgmt: Enc.",
    "Vol. Mgmt: Apg.",
    "Vol. Rev: Enc.",
    "Vol. Rev: Apg.",
    "Dash. OBC: Enc.",
    "Dash. OBC: Apg.",
    "BMBT Off: Apg.",
    "BMBT Off: Enc.",
    "Vol. Offset: %+d",
    "Mode: Default",
    "Mode: TCU",
    "Mode: No Mute",
    "Voice Assistant",
    "Llamar",
};

/**
 * LocaleGetText()
 *     Description:
 *         Returns localized string
 *     Params:
 *         uint16_t stringIndex - string identifier
 *     Returns:
 *         char *
 */
char *LocaleGetText(uint16_t stringIndex)
{
    if (stringIndex>LOCALE_STRING_MAX_INDEX) {
        return "Err: Missing txt";
    }

    char *text = NULL;
    unsigned char language = ConfigGetSetting(CONFIG_SETTING_LANGUAGE);

    switch (language) {
        case CONFIG_SETTING_LANGUAGE_DUTCH:
            text = LOCALE_LANG_DUTCH[stringIndex];
            break;
        case CONFIG_SETTING_LANGUAGE_ESTONIAN:
            text = LOCALE_LANG_ESTONIAN[stringIndex];
            break;
        case CONFIG_SETTING_LANGUAGE_GERMAN:
            text = LOCALE_LANG_GERMAN[stringIndex];
            break;
        case CONFIG_SETTING_LANGUAGE_ITALIAN:
            text = LOCALE_LANG_ITALIAN[stringIndex];
            break;
        case CONFIG_SETTING_LANGUAGE_RUSSIAN:
            text = LOCALE_LANG_RUSSIAN[stringIndex];
            break;
        case CONFIG_SETTING_LANGUAGE_SPANISH:
            text = LOCALE_LANG_SPANISH[stringIndex];
            break;
    }

    if ( text == NULL ) {
        text = LOCALE_LANG_ENGLISH[stringIndex];
    }
    
    return text;
}
