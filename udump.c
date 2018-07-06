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

#include <mint/osbind.h>
#include <mint/sysvars.h>

#include <stdint.h>
#include <stdio.h>

static long get_tos_header(void)
{
    return *_sysbase;
}

static const char country_codes[] =
    "endefrukesitsefsgstrfinodksanlczhuplltrueebyuaskrobgslhrcscsmkgrlvilzaptbejpcnkpvninirmnnplakhidbd";
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

    if (tos_country < countries_size) {
        country_code[0] = country_codes[tos_country*2];
        country_code[1] = country_codes[tos_country*2+1];
        country_code[2] = '\0';
    }

    printf("TOS %02x.%02x%s\r\n\r\nBuild date: %02x.%02x.%04x\r\nCountry:    %s\r\n",
        tos_version >> 8, tos_version & 0x00ff,
        country_code,
        (tos_build_date >> 16) & 0xff, tos_build_date >> 24, tos_build_date & 0xffff,
        tos_country < countries_size ? countries[tos_country] : "n/a");

    sprintf(tos_filename, "tos%03x%s.img", tos_version, country_code);

    if (tos_version < 0x0106) {
        tos_size = 192 * 1024;
    } else if (tos_version < 0x0300) {
        tos_size = 256 * 1024;
    } else if (tos_version < 0x0500) {
        tos_size = 512 * 1024;
    }

    if (tos_size == 0) {
        fprintf(stderr, "Couldn't determine TOS size,\r\npress enter to exit.");
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

    printf("%s,\r\npress enter to exit.", error_str);
    Cconin();
    return 0;
}
