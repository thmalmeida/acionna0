/* Created by rnascunha
*
* Edited by thmalmeida on 20240415
*/

#ifndef	DATE_TIME_HPP__
#define DATE_TIME_HPP__

#include <stdio.h>
// #include <stdint.h>
#include <cstdint>

// #include <time.hpp>

/* Refs.:
 * 	1- https://howardhinnant.github.io/date_algorithms.html
 *	2- https://en.wikipedia.org/wiki/ISO_week_date
 *	3- https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html
 *	4- https://www.geeksforgeeks.org/measure-execution-time-function-cpp/
 */

enum class week_day {
	MONDAY = 1,
	TUESDAY,
	WEDNEDAY,
	THURSDAY,
	FRIDAY,
	SATURDAY,
	SUNDAY
};

enum class time_format : int {
	H24 = 0,
	H12
};

enum class day_period {
	AM = 0,
	PM,
	ND						// Not Define for 24 hours format
};


class Date_Time {
	public:

		Date_Time(time_format mode = time_format::H24, int fuse = -3);

		// Set functions
		void year(uint16_t yr);
		void month(uint8_t mo);
		void day(uint8_t da);

		void hour(uint8_t hor, day_period per = day_period::ND);
		void minute(uint8_t min);
		void second(uint8_t sec);

		void date(uint16_t yr, uint8_t mt, uint8_t dy);
		void time(uint8_t hor, uint8_t min, uint8_t sec, day_period per = day_period::ND);
		void date_time(uint16_t yr, uint8_t mo, uint8_t da, uint8_t hor, uint8_t min, uint8_t sec, day_period per = day_period::ND);

		void unix_time(uint32_t ut);


		// Get functions
		uint16_t year(void);
		uint8_t month(void);
		uint8_t day(void);

		uint8_t hour(void);
		uint8_t minute(void);
		uint8_t second(void);

		uint32_t unix_time(void);


		// Setting using this class
		void date(Date_Time *date);
		void time(Date_Time *time);
		void date_time(Date_Time *dt);

	
		// Other features
		day_period period(void);
		time_format time_mode(void);

		int fuse(void);
		void fuse(int fs);

		bool leap_year(void); 					/* ano bissexto */

		week_day day_of_week(void);
		static week_day day_of_week(uint16_t year, uint8_t month, uint8_t day) noexcept;

	protected:
		// Date
		uint8_t day_;				/*0-31*/
		uint8_t month_;				/*1-12*/
		uint16_t year_;				/* XXXX */

		// Time
		uint8_t hour_;				/*0-11 / 0-23*/
		uint8_t minute_;			/*0-59*/
		uint8_t second_;			/*0-59*/
		//uint16_t _miliseconds;	/*0-999*/
		//uint16_t _microseconds; 	/*0-999*/

		time_format time_format_;		/* Modo 12 ou 24 horas */
		day_period period_;			/* ND,AM,PM */
		int fuse_;					/* Fuso in hours */
		int fuse_sec_;
};

#endif /* DATE_TIME_HPP__ */
