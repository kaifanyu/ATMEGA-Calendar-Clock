/*
 * GccApplication3.c
 *
 * Created: 1/26/2023 5:23:23 PM
 * Author : Kai
 
 This is the program for the micro controller in controlling a 16 keypad input to modify a clock 
 that displays date and time. The simple logic is that we have a variable called set up.
 While setup is not 0, we are in configuration mode, where the clock will freeze in time. We will
 then displays a blinking '_', and the user can use keys 'A', 'C', '*' to change the location of the cursor.
 When hover overing a number, say year, we can use a keypad number to change the number properly. If the clock
 was set to 25:90:90, we take the invalid input and changes it to 23:00:00, which is the closest valid time. 
 If the date was set to something invalid, it will also self adjust accordingly. If the month was 13/02, it will be reset
 to 01/02. If the month is 13/35, it will be reset to 01/01.It can also detect leap years, February will have 29 days 
 if the year is divisible by 4. To exit set up, we press '#', and to reenter configuration, we can press 'D'. 
 */ 

#include <avr/io.h>
#include <stdio.h>
#include "avr.h"
#include "lcd.h"

//to check for setup; 
int setUp = 1;

//define struct for DateTime
typedef struct {
	int year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	int cursorX;
	int cursorY;
} DateTime;

//Sets initial variables
void init_dt(DateTime *dt) {
	dt->year = 2001;
	dt->month = 2;
	dt->day = 28;
	dt->hour = 23;
	dt->minute = 59;
	dt->second = 57;
	dt->cursorX = 15;
	dt->cursorY = 0;
	
}
//helper function to reset Month and day
void resetMonth(DateTime *dt)
{
	dt->day = 1;
	dt->month++;
}


//prints Date on top row, time on bottom row.
void print_dt(const DateTime *dt) {
	
	char pDate[17];
	// Print date on top row.
	lcd_pos(0,0);
	sprintf(pDate, "Date: %04d-%02d-%02d",	dt->year,	dt->month,	dt->day);
	lcd_puts2(pDate);
	
	char pTime[17];
	// Print Time on bottom row.
	lcd_pos(1,0);
	sprintf(pTime, "Time: %02d:%02d:%02d  ",	dt->hour,	dt->minute,	dt->second);
	lcd_puts2(pTime);
}


//increments time, and checks for proper timing 
void advance_dt(DateTime *dt) {
	
	//increment a second
	++dt->second;
	
	if (dt->second > 59) {
		dt->second = 0;
		++dt->minute;
	}
	if(dt->minute > 59){
		dt->minute = 0;
		++dt->hour;
	}
	if(dt->hour > 23)
	{
		dt->hour = 0;
		++dt->day;
	}
	//if it is a month with 31 days
	if((dt ->month == 1 || dt ->month == 3 || dt ->month == 5 || dt ->month == 7 || dt ->month == 8 || dt ->month == 10) && dt->day == 32 )
	{
		resetMonth(dt);
	}
	else if((dt ->month == 4 || dt ->month == 6 || dt ->month == 9 || dt ->month == 11) && dt->day == 31)
	{
		resetMonth(dt);
	}
	//if year is a leap year and day is 29
	else if((dt->year % 4 == 0) && dt->month == 2 && dt->day == 30 )
	{
		resetMonth(dt);		
	}
	else if((dt->year % 4 != 0) && dt->month == 2 && dt->day == 29 )
	{
		resetMonth(dt);
	}
	
	if(dt->month == 12 && dt->day == 32)
	{
		dt->day = 1;
		dt->month = 1;
		++dt->year;
	}
}
void validate(DateTime *dt)
{
			
	//check if hour is greater than 23
	//hour becomes 23
	//minute is 0
	if(dt->hour > 23)
	{
		dt->hour = 23;
		dt->minute = 0;
		dt->day++;
	}

	//if month with 31 days, and days are > 31, reset month. Same logic for following
	//if it is a month with 31 days

	//if month is greater than 12
	if(dt->month > 12 && dt->day > 31)
	{
		dt->day	= 1;
		dt->month = 1;
		dt->year++;
	}
	else if(dt->month >12 && dt->day < 31)
	{
		dt->month = 1;
		dt->year++;
	}
	
	
	//if month is less or equal to 12
	if (dt->month <= 12)
	{
		//if it is a month of 31 days
			if((dt ->month == 1 || dt ->month == 3 || dt ->month == 5 || dt ->month == 7 || dt ->month == 8 || dt ->month == 10)  )
			{
				//if days are over 31 aka 13/34
				if(dt->day > 31)
				{
					resetMonth(dt);
				}
			}
			// if it is a month of 30 days
			else if((dt ->month == 4 || dt ->month == 6 || dt ->month == 9 || dt ->month == 11))
			{
				//if days are above 30
				if(dt->day > 30)
				{
					resetMonth(dt);
					//reset days to 1; month to 1, year++
				}
			}
			//if year is a leap year and month is 2
			else if((dt->year % 4 == 0) && dt->month == 2)
			{
				//if in a leap year, days are over 29, reset month
				if(dt->day > 29)
				{
					resetMonth(dt);
				}
			}
			else if((dt->year % 4 != 0) && dt->month == 2)
			{
				// if not in a leap year, days are over 28, reset month
				if(dt->day > 28)
				{
					//set day back to 1
					resetMonth(dt);
				}
			}
		}
		
		

			
}
	

	


int get_key()
{
	int i, j;	
		for(i = 0; i < 4; ++i){
			for(j = 0; j < 4; ++j){
				if(is_pressed(i,j)){
					return i*4+j+1;
					}	
				}
		}
	return 0;
			
}



int is_pressed(int r, int c)
{ 
	
	DDRC = 0x00;  // set lower 4 bits as input
	PORTC = 0x00;

	
	SET_BIT(DDRC, r);	
	CLR_BIT(PORTC, r);
	
	SET_BIT(PORTC, c + 4);
	
	avr_wait(10);
	if(GET_BIT(PINC, c+4) == 0)
	{
		
		return 1;
	}

	return 0;
}

/*
if user is in set up mode
user can press '*' to move cursor left  13
user can press 'C' to move cursor right 12
user can press 'A' to move cursor up 4
user can press 'B' to move cursor down 8

when user is moving cursor, display '_' on that position, and resume to normal afterwards
*/
void checkInput(DateTime *dt)
{
	int k = get_key();
	//if key is 13 (*)

	lcd_pos(dt->cursorY,dt->cursorX);
	//display underline
	char underline = '_';
	lcd_put(underline);
	
	//if we press 'C'
	if(k == 12)
	{
		//increment cursor x position by 1 to the right
		dt->cursorX++;
	}
	
	//if we press '*'
	if(k == 13)
	{
		//if curosrX is greater than 0, we can move it left
		if(dt->cursorX > 0)
		{
			//decrement cursor x, move 1 to the left
			dt->cursorX--;
		}
		else
		{
			dt->cursorX = 15;
		}
	}

	//if we press 'A'
	if(k == 4)
	{
		//change the position from 0 to 1 or 1 to 0, moving it up and down
		if(dt->cursorY == 0)
		{
			dt->cursorY = 1;
		}
		else
		{
			dt->cursorY = 0;
		}
	}
	
	
	// if k is a number input
	if(k == 1 || k == 2 || k == 3 || k == 5 || k == 6 || k == 7 || k == 9 || k == 10 || k == 11 || k == 14)
	{
		//if we are on the top row
		if(dt->cursorY == 0)
		{
			//if our x position is at 6
			if(dt->cursorX == 6)
			{
				//we can change the year
				dt->year = (dt->year % 1000) + (actualNumber(k) * 1000);

			}
			//if we are modifying hundredth digit of year
			else if(dt->cursorX == 7)
			{
				
				dt->year = dt->year - (dt->year % 1000) + (actualNumber(k) * 100) + (dt->year % 100);	//year + k * 100
				
			}
			//if we are modifying tenth digit of year
			else if(dt->cursorX == 8)
			{
				dt->year = dt->year - (dt->year % 100) + (actualNumber(k) * 10) + (dt->year % 10);
			}
			//modify the last digit of year
			else if(dt->cursorX == 9)
			{
				dt->year = dt->year - (dt->year % 10) + actualNumber(k);
			}
			else if(dt->cursorX == 11)
			{
				dt->month = dt->month - (dt->month % 100) + (actualNumber(k) * 10) + (dt->month % 10);
			}
			else if(dt->cursorX == 12)
			{
				dt->month = dt->month - (dt->month % 10) + actualNumber(k);
			}
			else if(dt->cursorX == 14)
			{
				dt->day = dt->day - (dt->day % 100) + (actualNumber(k) * 10) + (dt->day % 10);
			}
			else if(dt->cursorX == 15)
			{
				dt->day = dt->day - (dt->day % 10) + actualNumber(k);
			}
			
		}
		//if we are on the bottom row
		else if(dt->cursorY == 1)	
		{
			if(dt->cursorX == 6)
			{
				dt->hour = dt->hour - (dt->hour % 100) + (actualNumber(k) * 10) + (dt->hour % 10);
			}
			//if we are modifying hundredth digit of year
			else if(dt->cursorX == 7)
			{
				dt->hour = dt->hour- (dt->hour% 10) + actualNumber(k);
			}
			else if(dt->cursorX == 9)
			{
				dt->minute = dt->minute - (dt->minute % 100) + (actualNumber(k) * 10) + (dt->minute% 10);
			}
			else if(dt->cursorX == 10)
			{
				dt->minute = dt->minute - (dt->minute % 10) + actualNumber(k);
			}
			else if(dt->cursorX == 12)
			{
				dt->second = dt->second - (dt->second % 100) + (actualNumber(k) * 10) + (dt->second % 10);
			}
			else if(dt->cursorX == 13)
			{
				dt->second = dt->second- (dt->second% 10) + actualNumber(k);
			}
		}
		
		
	}

	
}

int actualNumber(int k)
{
	if(k == 1 || k ==2 || k ==3)
	{
		return k;
	}
	else if(k == 5 || k == 6 || k == 7)
	{
		return k-1;
	}
	else if(k == 9 || k == 10 || k == 11)
	{
		return k-2;
	}
	else if(k == 14)
	{
		return 0;
	}
	
	return 0;
}



void checkSetUp()
{
	//if we press '#' we resume the clock
	if(get_key() == 15)
	{
		setUp = 0;	
	}	
	//if we press 'D' we stop the clock
	if(get_key() == 16)
	{
		setUp = 1;
	}	
}

int main(void)
{
	DateTime dt;
	lcd_init();
	init_dt(&dt);
	while (1) {
			
		//check if setup is pressed
		checkSetUp();
		//if setUp is 0, then the clock should run normally
		while (setUp == 0)
		{
			validate(&dt);
			//pause, advance 1 second, print out Date and Time
			avr_wait(5);
			advance_dt(&dt);
			print_dt(&dt);
			//check for setup again to see if it has changed to 0
			checkSetUp();
			
		}
		//if setUp is not 0, we display the Date and Time as is without increments
		print_dt(&dt);
		//check for Input and update accordingly
		checkInput(&dt);

}
			

}

