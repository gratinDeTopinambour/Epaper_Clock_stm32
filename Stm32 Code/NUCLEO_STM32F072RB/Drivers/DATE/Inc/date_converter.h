/*
 * date_converter.h
 *
 *  Created on: Jan 17, 2026
 *      Author: Valentin
 *
 *  Public interface for date and time conversion utilities.
 *  This module provides:
 *   - Conversion from UTC to Paris local time (CET / CEST)
 *   - Daylight Saving Time handling according to European rules
 *   - Moon phase calculation
 */

#ifndef DATE_INC_DATE_CONVERTER_H_
#define DATE_INC_DATE_CONVERTER_H_

#include "main.h"
#include <stdint.h>
#include <math.h>

/**
 * @brief Convert a UTC date/time to Paris local time (CET / CEST)
 *
 * This function applies the appropriate UTC offset for Paris:
 *  - UTC+1 during standard time (CET)
 *  - UTC+2 during daylight saving time (CEST)
 *
 * The function updates the provided date and time in place and
 * handles:
 *  - daylight saving time transitions,
 *  - day, month, and year rollovers,
 *  - day-of-week increment.
 *
 * @param day    Pointer to day of week (0 = Sunday ... 6 = Saturday)
 * @param dd     Pointer to day of month (1–31)
 * @param mm     Pointer to month (0 = January ... 11 = December)
 * @param yy     Pointer to year (e.g. 2026)
 * @param minute Pointer to minutes since midnight (0–1439)
 */
void UTC_to_Paris(uint8_t *day, uint8_t *dd, uint8_t *mm, uint16_t *yy, uint16_t *minute);

/**
 * @brief Compute the moon phase for a given date
 *
 * The calculation is based on a reference new moon
 * (1 March 2014) and the average synodic month duration.
 *
 * @param day   Day of month (1–31)
 * @param month Month index (0 = January ... 11 = December)
 * @param year  Full year (e.g. 2026)
 *
 * @return Moon phase index:
 *         0 = New Moon
 *         1 = Waxing Crescent
 *         2 = First Quarter
 *         3 = Waxing Gibbous
 *         4 = Full Moon
 *         5 = Waning Gibbous
 *         6 = Last Quarter
 *         7 = Waning Crescent
 */
uint8_t Moon_Phase(uint8_t day, uint8_t month, uint16_t year);


#endif /* DATE_INC_DATE_CONVERTER_H_ */
