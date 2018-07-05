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

int main(int argc, const char* argv[])
{
    Supexec(get_tos_info);
    
    printf("%04x / %08x / %02x\n", tos_version, tos_build_date, tos_country);
    getchar();
    
    return EXIT_SUCCESS;
}
