/*
 * date_converter.c
 *
 *  Created on: Jan 17, 2026
 *      Author: Valentin
 *
 *  This file provides:
 *   - UTC to Paris time conversion (CET / CEST)
 *   - Day of week calculation
 *   - Moon phase calculation
 */

#include "date_converter.h"

/**
 * @brief Check if a year is a leap year
 *
 * @param year Full year (e.g. 2026)
 * @return 1 if leap year, 0 otherwise
 */
static uint8_t is_leap_year(uint16_t year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0));
}

/**
 * @brief Get number of days in a month
 *
 * @param month Month index (0 = January ... 11 = December)
 * @param year  Full year
 * @return Number of days in the given month
 */
static uint8_t days_in_month(uint8_t month, uint16_t year)
{
    static const uint8_t days[12] =
        {31,28,31,30,31,30,31,31,30,31,30,31};

    /* February adjustment for leap year */
    if (month == 1 && is_leap_year(year))
        return 29;

    return days[month];
}

/**
 * @brief Compute day of the week using Zeller’s congruence
 *
 * @param y Year
 * @param m Month (0 = January)
 * @param d Day of month
 * @return Day of week (0 = Monday ... 6 = Sunday)
 */
static uint8_t day_of_week(uint16_t y, uint8_t m, uint8_t d)
{
	/* Convert month to Zeller format */
	m++;
    if (m < 3) {
        m += 12;
        y--;
    }
    uint16_t k = y % 100; /* Year of the century */
    uint16_t j = y / 100; /* Zero-based century */

    /* Zeller algorithm */
    uint8_t h = (d + (13*(m + 1))/5 + k + k/4 + j/4 +5*j) % 7;

    /* Convert result so that Sunday = 6 */
    return (h + 5) % 7;
}

/**
 * @brief Get the date of the last Sunday of a month
 *
 * @param year  Full year
 * @param month Month index (0 = January)
 * @return Day of month corresponding to last Sunday
 */
static uint8_t last_sunday(uint16_t year, uint8_t month)
{
    uint8_t d = days_in_month(month, year);
    while (day_of_week(year, month, d) != 6)
        d--;
    return d;
}

/**
 * @brief Determine if Paris daylight saving time is active
 *
 * @param year    Full year
 * @param month   Month index (0 = January)
 * @param day     Day of month
 * @param minute  Minutes since midnight (UTC)
 * @return 1 if DST (CEST), 0 if standard time (CET)
 */
static uint8_t is_dst_paris(uint16_t year, uint8_t month, uint8_t day, uint16_t minute)
{
    uint8_t march_sunday = last_sunday(year, 2); /* March */
    uint8_t october_sunday = last_sunday(year, 9); /* October */

    uint16_t minutes_today = minute;

    /* Outside DST months */
    if (month < 2 || month > 9)
        return 0;

    /* Fully inside DST */
    if (month > 2 && month < 9)
        return 1;

    /* March transition */
    if (month == 2) {
        if (day > march_sunday) return 1;
        if (day < march_sunday) return 0;
        return (minutes_today >= 60);  /* After 01:00 UTC */
    }

    /* October transition */
    if (month == 9) {
        if (day < october_sunday) return 1;
        if (day > october_sunday) return 0;
        return (minutes_today < 60);   /* Before 01:00 UTC */
    }

    return 0;
}

/**
 * @brief Count total days since 1 January 2014
 *
 * @param day  Day of month
 * @param month  Month index (0 = January)
 * @param year Full year
 * @return Total number of days since reference date
 */
static int days_from_2014(int day, int month, int year) {
    int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int nb_days = (year-2014) * 365;

    /* Add leap days */
    for(int i = 2014; i < year; i++) {
        if(is_leap_year(i)) {
        	nb_days++;
        }
    }

    /* Add days from previous months */
    for(int i = 0; i < month; i++) {
    	nb_days += days_in_month[i];
    }

    /* Leap year adjustment for February */
    if(month > 1 && is_leap_year(year)) {
    	nb_days += 1;
    }

    /* Add current day */
    nb_days += day;

    return nb_days;
}

/**
 * @brief Convertit une date/heure UTC en heure locale Paris (CET / CEST)
 *
 * @param day    Jour de la semaine (0 = Dimanche ... 6 = Samedi)
 * @param dd     Jour du mois (1–31)
 * @param mm     Mois (0 = Janvier ... 11 = Décembre)
 * @param yy     Année (ex: 2026)
 * @param minute Minutes depuis minuit (0–1439)
 *
 * La fonction :
 *  - applique le décalage UTC → Paris
 *  - gère l'heure d'été / hiver
 *  - gère les fins de mois
 *  - gère les années bissextiles
 */
void UTC_to_Paris(uint8_t *day, uint8_t *dd, uint8_t *mm, uint16_t *yy, uint16_t *minute)
{
    uint8_t dst = is_dst_paris(*yy, *mm, *dd, *minute);
    uint16_t offset = dst ? 120 : 60;  /* Minutes */

    *minute += offset;

    /* Handle day rollover */
    if (*minute >= 1440) {
        *minute -= 1440;
        (*dd)++;

        if (*dd > days_in_month(*mm, *yy)) {
            *dd = 1;
            (*mm)++;

            if (*mm > 12) {
                *mm = 1;
                (*yy)++;
            }
        }

        *day = (*day + 1) % 7;
    }
}

/**
 * @brief Calculate moon phase
 *
 * @param day   Day of month
 * @param month Month index (0 = January)
 * @param year  Full year
 * @return Moon phase index (0–7)
 */
uint8_t Moon_Phase(uint8_t day, uint8_t month, uint16_t year)
{

	/* Reference: 1 March 2014 was a new moon */
	int D = days_from_2014(day,month,year);
	D -=60; /* Remove days between 1 Jan and 1 Mar 2014 */

	/* Average lunar cycle length (days) */
    double lunar_cycle = 29.53058867;

    /* Moon age within the cycle */
    double age = fmod(D, lunar_cycle);

    /* Phase determination */
	if (age < 1.8) return 0;
	else if (age < 5.5) return 1;
	else if (age < 9.2) return 2;
	else if (age < 12.9) return 3;
	else if (age < 16.6) return 4;
	else if (age < 20.3) return 5;
	else if (age < 24) return 6;
	else if (age < 27.7) return 7;
	else return 0;
}


