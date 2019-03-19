/*
 * clock_manager.c
 * Copyright (C) 2018  jasLogic
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "clock_manager.h"

#include <stdint.h>
#include <stddef.h>

#include "peripherals.h"

uint32_t *
clock_map(void)
{
    clock_manager_base_ptr = (volatile uint32_t *)peripheral_map(PERIPHERAL_BASE
        + CLOCK_MANAGER_OFFSET, CLOCK_MANAGER_SIZE);
    return (uint32_t *)clock_manager_base_ptr;
}

void
clock_unmap(void)
{
    peripheral_unmap((uint32_t *)clock_manager_base_ptr, CLOCK_MANAGER_SIZE);
}


void
clock_enable(volatile uint32_t *reg)
{
    *reg |= CM_PASSWD | 0x10;
}

void
clock_disable(volatile uint32_t *reg)
{
    if (*reg & 0x80) {
		// *reg |= CM_PASSWD | (1 << 5); // Kill clock?
		*reg = CM_PASSWD | (*reg & ~0x10); // Disable clock
		while(*reg & 0x80); // Wait for busy flag to turn off
	}
}

void
clock_configure(volatile uint32_t *reg, clock_source_t src,
                unsigned int divisor, unsigned int mash)
{
    clock_disable(reg);

    *(reg + 1) = CM_PASSWD | divisor << 12;
    *reg |= CM_PASSWD | mash << 9;
    *reg |= CM_PASSWD | src;
}
