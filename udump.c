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
#include <stdlib.h>

static long get_tos_header(void)
{
    return *_sysbase;
}

static const char* const countries[] = {
    "us",
    "de",
    "fr",
    "uk",
    "es",
    "it",
    "se",
    "ch-fr",
    "ch-de"
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
    char tos_filename[8+3+1+3+1];       // filename + possible "-{fr,de}" + "." + ext + nil
    char* error_str;
    FILE* f;

    tos_header = (OSHEADER*)(Supexec(get_tos_header));
    tos_base = (uintptr_t)tos_header;
    tos_version = tos_header->os_version;
    tos_build_date = tos_header->os_date;
    tos_country = tos_header->os_conf >> 1;

    printf("TOS %02x.%02x%s (%02x.%02x.%04x)\n",
        tos_version >> 8, tos_version & 0x00ff,
        tos_country < countries_size ? countries[tos_country] : "",
        (tos_build_date >> 16) & 0xff, tos_build_date >> 24, tos_build_date & 0xffff);

    sprintf(tos_filename, "tos%03x%s.img",
        tos_version,
        tos_country < countries_size ? countries[tos_country] : "");

    if (tos_version < 0x0106) {
        tos_size = 192 * 1024;
    } else if (tos_version < 0x0300) {
        tos_size = 256 * 1024;
    } else if (tos_version < 0x0500) {
        tos_size = 512 * 1024;
    }

    if (tos_size == 0) {
        fprintf(stderr, "Couldn't determine TOS size,\npress enter to exit.");
        getchar();
        return EXIT_FAILURE;
    }

    printf("\n");
    printf("Saving %ld KiB to %s ... ", tos_size / 1024, tos_filename);

    f = fopen(tos_filename, "wb");
    error_str = "fail";
    if (f != NULL) {
        if (fwrite((const void*)tos_base, 1, tos_size, f) == tos_size) {
            error_str = "done";
        }
        fclose(f);
    }

    printf("%s,\npress enter to exit.", error_str);
    getchar();
    return EXIT_SUCCESS;
}
