/*
 * This file is part of uDump.
 *
 * uDump is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * uDump is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with uDump.  If not, see <https://www.gnu.org/licenses/>.
 *
 * (c) 2018 Miro Kropacek; miro.kropacek@gmail.com
 */

#include <getcookie.h>

#include <mint/osbind.h>
#include <mint/sysvars.h>

#include <stdint.h>
#include <stdio.h>

static long get_tos_header(void)
{
    return *_sysbase;
}

static long get_memory_size(void)
{
    return *phystop;
}

static const char country_codes[] =
    "usdefrukesitsefsgstrfinodksanlczhuplltrueebyuaskrobgslhrcscsmkgrlvilzaptbejpcnkpvninirmnnplakhidbd";
static const char* countries[] = {
    "United States",
    "Germany",
    "France",
    "United Kingdom",
    "Spain",
    "Italy",
    "Sweden",
    "Switzerland (French)",
    "Switzerland (German)",
    "Turkey",
    "Finland",
    "Norway",
    "Denmark",
    "Saudi Arabia",
    "Netherlands",
    "Czech Republic",
    "Hungary",
    "Poland",
    "Lithuania",
    "Russia",
    "Estonia",
    "Belarus",
    "Ukraine",
    "Slovak Republic",
    "Romania",
    "Bulgaria",
    "Slovenia",
    "Croatia",
    "Serbia",
    "Montenegro",
    "Macedonia",
    "Greece",
    "Latvia",
    "Israel",
    "South Africa",
    "Portugal",
    "Belgium",
    "Japan",
    "China",
    "Korea",
    "Vietnam",
    "India",
    "Iran",
    "Mongolia",
    "Nepal",
    "Lao People's Democratic Republic",
    "Cambodia",
    "Indonesia",
    "Bangladesh"
};
static const size_t countries_size = sizeof(countries) / sizeof(countries[0]);

int main(int argc, const char* argv[])
{
    OSHEADER* tos_header;
    uintptr_t tos_base;
    uint16_t tos_version;
    uint32_t tos_build_date;
    uint8_t tos_country;
    int is_emutos;
    unsigned long mch_value = 0;
    unsigned long ct60_value = 0;
    char* machine;
    uint32_t machine_mem;
    size_t tos_size = 0;
    char tos_filename[8+1+3+1] = "filename.ext";
    char country_code[2+1] = "";
    char* error_str;
    FILE* f;

    tos_header = (OSHEADER*)(Supexec(get_tos_header));
    tos_base = (uintptr_t)tos_header;
    tos_version = tos_header->os_version;
    tos_build_date = tos_header->os_date;
    tos_country = tos_header->os_conf >> 1;
    is_emutos = (uint32_t)tos_header->p_rsv2 == 0x45544f53UL;	/* 'ETOS' */

    if (is_emutos) {
        /*
         * EmuTOS before 2011-04-27 had the date in format YYYY-MM-DD
         * but TOS ROMs expect it to be in MM-DD-YYYY
         */
        if ((tos_build_date & 0xff000000UL) >= 0x19000000UL) {
            tos_build_date = ((tos_build_date & 0xffff) << 16) | ((tos_build_date >> 16) & 0xffff);
        }
    }

    getcookie(0x5f4d4348UL, &mch_value);	/* '_MCH' */
    getcookie(0x43543630UL, &ct60_value);	/* 'CT60' */

    if (tos_country < countries_size) {
        country_code[0] = country_codes[tos_country*2];
        country_code[1] = country_codes[tos_country*2+1];
        country_code[2] = '\0';
    }
    
    switch (mch_value) {
        case 0x00000000UL:
            machine = "Atari ST";
            break;
        case 0x00010000UL:
            machine = "Atari STE";
            break;
        case 0x00010001UL:
            machine = "ST Book";
            break;
        case 0x00010010UL:
            machine = "Atari Mega STE";
            break;
        case 0x00010100UL:
            machine = "Atari Sparrow";
            break;
        case 0x00020000UL:
            machine = "Atari TT/Hades";
            break;
        case 0x00030000UL:
            machine = (ct60_value == 0) ? "Atari Falcon" : "Atari Falcon (CT60)";
            break;
        case 0x00040000UL:
            machine = "Milan";
            break;
        case 0x00050000UL:
            machine = "ARAnyM";
            break;
        default:
            machine = "unknown";
    }
    
    machine_mem = Supexec(get_memory_size);

    printf("%s %02x.%02x%s\r\n\r\n"
        "Build date: %02x.%02x.%04x\r\n"
        "Country:    %s\r\n"
        "Machine:    %s\r\n"
        "Memory:     %d KiB\r\n",
        is_emutos ? "EmuTOS" : "TOS",
        tos_version >> 8, tos_version & 0x00ff,
        country_code,
        (tos_build_date >> 16) & 0xff, tos_build_date >> 24, tos_build_date & 0xffff,
        tos_country < countries_size ? countries[tos_country] : "n/a",
        machine,
        machine_mem / 1024);

    sprintf(tos_filename, "tos%03x%s.img", tos_version, country_code);

    if (tos_version < 0x0106) {
        tos_size = 192 * 1024;
    } else if (tos_version < 0x0300) {
        tos_size = 256 * 1024;
    } else if (tos_version < 0x0500) {
        tos_size = 512 * 1024;
        if (ct60_value != 0) {
            tos_size *= 2;
        }
    }

    if (is_emutos && tos_version == 0x0206) {
        /* a humble attempt to correct EmuTOS ROM size ... */
        if ((mch_value & 0xffff0000UL) > 0x00010000UL) {
            tos_size = 512 * 1024;
        }
    }

    if (tos_size == 0) {
        fprintf(stderr, "Couldn't determine TOS size,\r\npress a key to exit.");
        Cconin();
        return 1;
    }

    printf("\r\n");
    printf("Saving %ld KiB to %s ... ", tos_size / 1024, tos_filename);

    f = fopen(tos_filename, "wb");
    error_str = "fail";
    if (f != NULL) {
        if (fwrite((const void*)tos_base, 1, tos_size, f) == tos_size) {
            error_str = "done";
        }
        fclose(f);
    }

    printf("%s,\r\npress a key to exit.", error_str);
    Cconin();
    return 0;
}
