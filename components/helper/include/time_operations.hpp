#ifndef TIME_OPERATIONS_HPP__
#define TIME_OPERATIONS_HPP__

#include <time.h>

int date_compare(const tm* date, const tm* date_cmp);
int date_between(const tm* date, const tm* date_before, const tm* date_after);
int time_compare(const tm* time, const tm* time_cmp);
int time_between(const tm* time, const tm* time_before, const tm* time_after);

// Added by thmalmeida on 202211
typedef struct {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} time_hour_format;

uint16_t timesec_to_day(uint32_t seconds);
uint8_t timesec_to_hour(uint32_t seconds);
uint8_t timesec_to_min(uint32_t seconds);
uint8_t timesec_to_sec(uint32_t seconds);
uint32_t daytime_to_timesec(uint8_t hour, uint8_t min, uint8_t sec);

#endif /* TIME_OPERATIONS_H__ */
