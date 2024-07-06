/* Created by rnascunha
*
* Edited by thmalmeida on 20240415
*/

#include "date_time.hpp"

static const uint8_t month_length_[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#define SEC_1970_TO_2000 	946684800
#define SECONDS_DAY			86400		// total of seconds in one day

template <class T>
static inline const T& constrain(const T& n, const T& a, const T& b) {
  return ( n < a ) ? a : ( n > b ? b : n );
}

Date_Time::Date_Time(time_format mode, int fuse) {
	time_format_ = mode;
	time_fuse_ = fuse;
	period_ = day_period::ND;
	date_time(0,0,0,0,0,0);
}

// Set functions
void Date_Time::year(uint16_t yr) {
	year_ = yr;
}
void Date_Time::month(uint8_t mo) {
	month_ = constrain((uint8_t)mo, (uint8_t)1, (uint8_t)12);
}
void Date_Time::day(uint8_t da) {
	uint8_t max_day = month_length_[month_-1];

	max_day = (month_ == 2 && leap_year()) ? max_day + 1 : max_day;

	day_ = constrain((uint8_t)da, (uint8_t)1, (uint8_t)max_day);
}

void Date_Time::hour(uint8_t hor, day_period per) {
	if(time_format_ == time_format::H12){
		period_ = (per == day_period::ND) ? day_period::AM : per;
		if(period_ == day_period::PM)
			hour_ += 12;
	}
	hour_ = constrain((uint8_t)hor, (uint8_t)0, (uint8_t)23);
}
void Date_Time::minute(uint8_t min) {
	minute_ = constrain((uint8_t)min, (uint8_t)0, (uint8_t)59);
}
void Date_Time::second(uint8_t sec) {
	second_ = constrain((uint8_t)sec, (uint8_t)0, (uint8_t)59);
}

void Date_Time::date(uint16_t yr, uint8_t mo, uint8_t da) {
	year(yr);
	month(mo);
	day(da);
}
void Date_Time::time(uint8_t hor, uint8_t min, uint8_t sec, day_period per) {
	hour(hor, per);
	minute(min);
	second(sec);
}
void Date_Time::date_time(uint16_t yr, uint8_t mo, uint8_t da, uint8_t hor, uint8_t min, uint8_t sec, day_period per) {
	date(yr, mo, da);
	time(hor, min, sec, per);
}

void Date_Time::unix_time(uint32_t ut) {
	// since 1970-01-01 00:00:00
	unsigned long days, mins, secs, years, leap;

	days = ut/(24L*60*60);
	secs = ut % (24L*60*60);
	second_ = secs % 60;
	mins = secs / 60;
	hour_ = mins / 60;
	minute_ = mins % 60;
	// ??
	years = (((days * 4) + 2)/1461);
	year_ = years + 1970;
	leap = !(year_ & 3);
	days -= ((years * 1461) + 1) / 4;
//	day_ = days;
	days += (days > 58 + leap) ? ((leap) ? 1 : 2) : 0;
	month_ = ((days * 12) + 6)/367 + 1;
	day_ = days + 1 - (((month_ - 1) * 367) + 5)/12;
}

// Get functions
uint16_t Date_Time::year(void) {
	return year_;
}
uint8_t Date_Time::month(void) {
	return month_;
}
uint8_t Date_Time::day(void) {
	return day_;
}

uint8_t Date_Time::hour(void) {
	if(time_format_ == time_format::H12 && period_ == day_period::PM) {
		return hour_ - 12;
	}
	return hour_;
}
uint8_t Date_Time::minute(void) {
	return minute_;
}
uint8_t Date_Time::second(void) {
	return second_;
}

void Date_Time::date(Date_Time *date) {
	date->year_ = year();
	date->month_ = month();
	date->day_ = day();
}
void Date_Time::time(Date_Time *time) {
	time->hour_ = hour();
	time->minute_ = minute();
	time->second_ = second();
	time->period_ = day_period();
	time->time_format_ = time_mode();
	time->time_fuse_ = time_fuse();
}
void Date_Time::date_time(Date_Time *dt) {
	date(dt);
	time(dt);
}

/* número de segundos desde 1 de janeiro de 1970 UTC
 * Algoritimo só funciona para depois do ano 2000
 */
uint32_t Date_Time::unix_time(void) {
	uint16_t dc;
	dc = day_;
	for (uint8_t i = 0; i<(month_-1); i++)
		dc += month_length_[i];
	if ((month_ > 2) && leap_year())
		++dc;
	dc = dc + (365 * (year_ - 2000)) + (((year_ - 2000) + 3) / 4) - 1;

	return ((((((dc * 24UL) + hour_) * 60) + minute_) * 60) + second_) + SEC_1970_TO_2000;
}

day_period Date_Time::period(void) {
	return hour_ < 12 ? day_period::AM : day_period::PM;
}
time_format Date_Time::time_mode(void) {
	return time_format_;
}
int8_t Date_Time::time_fuse(void) {
	return time_fuse_;
}
/* Ref.: https://en.wikipedia.org/wiki/Leap_year
 * Each leap year has 366 days instead of 365. This extra leap day occurs
 * in each year that is a multiple of 4, except for years evenly divisible
 * by 100 but not by 400.
 */
bool Date_Time::leap_year(void) {
	if(year_ % 4)			
		return false;

	if(year_ % 100)
		return true;	// [YES] divided by 4 only

	if(year_ % 400)
		return false;	// [NO] divided by 4 and divided by 100

	return true;		// [YES] divided by 4, divided by 100 and 400
}
/*
 * https://en.wikipedia.org/wiki/Zeller%27s_congruence
 * http://programacionnerd.blogspot.com/2012/05/c-ejemplos-congruencia-de-zeller-nivel.html
 */
week_day Date_Time::day_of_week(void) {
	return Date_Time::day_of_week(day_, month_, year_);
}
week_day Date_Time::day_of_week(uint16_t year, uint8_t month, uint8_t day) noexcept {
    unsigned h, K, J;
    if(month <= 2)
    {
        month =+ 12;
        year = year - 1;
    }
    else
        month -= 2;

    K = year % 100;
    J = year / 100;

    h = (700 + ((26 * month - 2) / 10) +
        day + K + (K / 4) + ((J / 4) + 5 * J)) % 7;

    return static_cast<week_day>(h);
}
