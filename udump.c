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
 */

#include <mint/osbind.h>
#include <mint/sysvars.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint16_t tos_version;
static uint32_t tos_build_date;
static uint8_t tos_country;
static void get_tos_info(void)
{
    OSHEADER* header = *((OSHEADER**)_sysbase);
    tos_version = header->os_version;
    tos_build_date = header->os_date;
    tos_country = header->os_conf >> 1;
}

static size_t tos_size;
static char tos_filename[8+3+1+3+1];       // filename + possible "-{fr,de}" + "." + ext + nil
static uintptr_t tos_base;
static void save_tos(void)
{
    FILE* f = fopen(tos_filename, "wb");
    if (f != NULL) {
        fwrite((const void*)tos_base, 1, tos_size, f);
        fclose(f);
    }
}

static const char* countries[] = {
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
    Supexec(get_tos_info);

    printf("TOS %02x.%02x%s (%02x.%02x.%04x)\n",
        tos_version >> 8, tos_version & 0x00ff,
        tos_country < countries_size ? countries[tos_country] : "",
        (tos_build_date >> 16) & 0xff, tos_build_date >> 24, tos_build_date & 0xffff);

    sprintf(tos_filename, "tos%03x%s.img",
        tos_version,
        tos_country < countries_size ? countries[tos_country] : "");

    if (tos_version < 0x0106) {
        tos_base = 0x00FC0000;
        tos_size = 192 * 1024;
    } else if (tos_version < 0x0300) {
        tos_base = 0x00E00000;
        tos_size = 256 * 1024;
    } else if (tos_version <= 0x0404) {
        tos_base = 0x00E00000;
        tos_size = 512 * 1024;
    }

    if (tos_size == 0) {
        fprintf(stderr, "Couldn't determine TOS size,\npress enter to exit.");
        getchar();
        return EXIT_FAILURE;
    }

    printf("\n");
    printf("Saving %ld KiB to %s ... ", tos_size / 1024, tos_filename);

    Supexec(save_tos);

    printf("done,\npress enter to exit.");
    getchar();
    return EXIT_SUCCESS;
}
