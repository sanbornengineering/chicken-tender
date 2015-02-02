
/*
Chicken Tender is an open source chicken coop controller developed by Sanborn Engineering LLC
Copyright (C) 2014  James Sanborn

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/************************************************************************/
/* Project: Chicken Tender		                                        */
/* Program: MasterController                                            */
/* Version: 1.0                                                         */
/* Date: 01/20/2015                                                    */
/* Company: Sanborn Engineering                                         */
/* Author: James Sanborn                                                */
/* Description: see latest date ProductDescriptionMasterController.odt  */
/* Description: see latest date ProductDescriptionMasterController.odt  */
/************************************************************************/ 
#include "Arduino.h"
#include "ChickenTender/ChickenTender.h"
#include "EEPROMEx/EEPROMex.h"
#include "avr/pgmspace.h"
#include "Wire/Wire.h"
#include "LiquidCrystal_I2C.h"
#include "M2tk.h"
#include "utility/m2ghnlc.h"
#include "Time/Time.h"
#include "DS1307RTC/DS1307RTC.h"
#include "TimeAlarms/TimeAlarms.h"
#include "Narcoleptic/Narcoleptic.h"
#include "MemoryFree/MemoryFree.h" // library to determine how much ram is used


// Defines
#define _DEBUG
#define _DEBUGEEPROM
#define blinkInterval 1000	//blink pin 13 LED every second This needs to be changed to a const uint8_t
//#define sleepInterval 6000 //go to sleep after ten minutes of no user button press
#define sleepTime 100 // sleep for 500ms then wake up and check the sensors and buttons This needs to be changed to a const uint8_t
#define sensorInterval 6000 // check sensors ever 10 minutes This needs to be changed to a const uint8_t
#define alarmInterval 60000 // check alarms ever 1 minute This needs to be changed to a const uint8_t

// Global variables
const uint8_t displayID_u8 = 0;
const uint8_t sleepModeID_u8 = 1;
const uint8_t userMenuID_u8 = 2;
const uint8_t openDoorID_u8 = 3;
const uint8_t closeDoorID_u8 = 4;
uint8_t stateID = displayID_u8;

// IDs used for user display
const uint8_t doNothingkDisplayID_u8 = 0;
const uint8_t clockDisplayID_u8 = 1;
const uint8_t alarmDisplayID_u8 = 2;
const uint8_t foodDisplayID_u8 = 3;
const uint8_t gritDisplayID_u8 = 4;
const uint8_t calciumDisplayID_u8 = 5;
const uint8_t waterDisplayID_u8 = 6;
const uint8_t environmentDisplayID_u8 = 7;
uint8_t userDisplayID=clockDisplayID_u8;
unsigned long displayRefreshRate = 1000; 
uint8_t displayChangeRate_u8 = 3;				// time that the display will switch between screens time = displayRefreshRate*displayChangeRate

//////////////////////////////////////////////////////////////////////////
unsigned long gotoSleepTimer=0;		// where is this used?
uint8_t sleepInterval_u8 = 10; //go to sleep after interval. In minutes
unsigned int checkAlarmsCounter_u16 = 0;
unsigned int checkAlarmsSleepInterval_u16 = 120;	// Every minute
unsigned int checkSensorCounter_u16 = 0;
unsigned int checkSensorsSleepInterval_u16 = 1200;	// Every 10 minutes
uint8_t displayInterval_u8 = 10; // This is not used. What is it for? 
uint8_t displayTimerInterval_u8=10; // switch from user menu to user display after time. In minutes  
// user buttons
const uint8_t uiKeySelectPin_U8 = 2;
const uint8_t uiKeyNextPin_u8 = 4;
const uint8_t uiKeyDataUpPin_u8 = 7;
const uint8_t uiKeyDataDownPin_u8 = 8;

ChickenTender CT;			// create instance of chicken tender library. This should only by created once.

//MemoryFree Memory;		// create instance of ram checker

//////////////////////////////////////////////////////////////////////////
// Setup LCD 
//////////////////////////////////////////////////////////////////////////
// set the LCD address to 0x3f for a 20 chars and 4 line display
LiquidCrystal_I2C lcd(0x3f, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

uint8_t lcdPowerPin_u8 = 52; 


// Variables used in the Main menu
uint8_t el_strlist_mainMenu_first_u8 = 0;
uint8_t el_strlist_mainMenu_cnt_u8 = 7;

// Variables used in the time menu
uint8_t el_strlist_timeMenu_first_u8 = 0;
uint8_t el_strlist_timeMenu_cnt_u8 = 5;

// Variables used in the set date menu
uint8_t date_day_u8 = 1;
uint8_t date_month_u8 = 1;
uint8_t date_year_u8 = 14;
uint8_t lastDayMonth_u8 = 31;

// Variables used in the set time menu
uint8_t time_second_u8 = 1;
uint8_t time_minute_u8 = 1;
uint8_t time_hour_u8 = 1;
uint8_t el_strlist_AMPMMenu_first_u8 = 0;
uint8_t el_strlist_AMPMMenu_cnt_u8 = 1;

// used to determine if it is morning or night for time function set.
uint8_t meriden_flg = 0; // 0 = AM, 1 = PM
const char *meriden = "AM"; 


// Variables used in the set open time menu
uint8_t timeOpen_second_u8 = 0;
uint8_t timeOpen_minute_u8 = 0;
// 
uint8_t timeOpen_hour_u8 = 8;
uint8_t timeOpen_second_temp_u8 = 0; // used to store current alarm values in the even the user cancels the selection
uint8_t timeOpen_minute_temp_u8 = 0;
uint8_t timeOpen_hour_temp_u8 = 8;

// Variables used in the set open time menu
uint8_t timeClose_second_u8 = 0;
uint8_t timeClose_minute_u8 = 0;
uint8_t timeClose_hour_u8 = 8;
uint8_t timeClose_second_temp_u8 = 0; // used to store current alarm values in the even the user cancels the selection
uint8_t timeClose_minute_temp_u8 = 0;
uint8_t timeClose_hour_temp_u8 = 8;

// variables used in the food menu
uint8_t foodLevel_u8 = 10;			//Delete later food level in cm for current food level this should come from main program.
uint8_t foodLevelMin_u8 = 30;		// Food level in cm when feeder is empty
uint8_t foodLevelMax_u8 = 2;		// Food level in cm when feeder is full
uint8_t foodLevelYesterday_u8 = 4; // food level in cm stored each day at night alarm time. 

// variables used in the grit menu
uint8_t gritLevel_u8 = 5;			// Grit level in cm for current grit level this shall come from main program.
uint8_t gritLevelMin_u8 = 20;		// Grit level in cm when feeder is empty
uint8_t gritLevelMax_u8 = 2;		// Grit level in cm when feeder is full

// variables used in the calcium menu
uint8_t calciumLevel_u8 = 5;			// Calcium level in cm for current calcium level this shall come from main program.
uint8_t calciumLevelMin_u8 = 20;		// Calcium level in cm when feeder is empty
uint8_t calciumLevelMax_u8 = 2;		// Calcium level in cm when feeder is full

// variables used in the water menu
uint8_t waterLevel_u8 = 5;			// Water level in cm for current water level this shall come from main program.
uint8_t waterLevelMin_u8 = 30;		// Water level in cm when feeder is empty
uint8_t waterLevelMax_u8 = 2;		// Water level in cm when feeder is full


// Variables used in the temperature menu
uint8_t el_strlist_temperatureMenu_first_u8 = 0;
uint8_t el_strlist_temperatureMenu_cnt_u8 = 6;
float temperature_flt = 23;
int8_t temperatureMin_s8 = -40;
int8_t temperatureMax_s8 = 127;
int8_t temperatureOffset_s8 = 0;
int8_t temperatureOffsetMin_s8 = -100;
int8_t temperatureOffsetMax_s8 = 100;
int8_t lowTemperatureAlarmValue_s8 = 0;
int8_t lowTemperatureAlarmMin_s8 = -40;
int8_t lowTemperatureAlarmMax_s8 = 98;
uint8_t highTemperatureAlarmValue_u8 = 85;
uint8_t highTemperatureAlarmMin_u8 = 0;
uint8_t highTemperatureAlarmMax_u8 = 120;
uint8_t humidityOffset_u8 = 0;
uint8_t temperatureUnits = 0; // 0 = Celsius, 1 = Fahrenheit


// EEPROM support values

const int maxAllowedWrites = 1000;
const int memBase          = 350;
const uint8_t firstTime_u8 = 0xAC;	// check flag for EEPROM data

// EEPROM memory map used later in program to store and read EEPROM data
/* Must be sequential and no number can be used twice. All variables associated with these addresses 
   must be 8 bits.*/ 
// unsigned EEPROM data
const uint8_t addressFirstTime_u8 = 0;
const uint8_t addressTimeOpen_hour_u8 = 1;
const uint8_t addressTimeOpen_minute_u8 = 2;
const uint8_t addressTimeClose_hour_u8 = 3;
const uint8_t addressTimeClose_minute_u8 = 4;
const uint8_t addressSleepInterval_u8 = 5;
const uint8_t addressDisplayInterval_u8 = 6;
const uint8_t addressDisplayTimerInterval_u8 = 7;
const uint8_t addressFoodLevelMin_u8 = 8;
const uint8_t addressFoodLevelMax_u8 = 9;
const uint8_t addressGritLevelMin_u8 = 10;
const uint8_t addressGritLevelMax_u8 = 11;
const uint8_t addressCalciumLevelMin_u8 = 12;
const uint8_t addressCalciumLevelMax_u8 = 13;
const uint8_t addressWaterLevelMin_u8 = 14;
const uint8_t addressWaterLevelMax_u8 = 15;
const uint8_t addressHighTemperatureAlarmValue_u8 = 16;
const uint8_t addressHighTemperatureAlarmMin_u8 = 17;
const uint8_t addressHighTemperatureAlarmMax_u8 = 18;
const uint8_t addressHumidityOffset_u8 = 19;
const uint8_t addressTemperatureUnits_u8 = 20;
// signed EEPROM data
const uint8_t addressTemperatureMin_s8 = 21;
const uint8_t addressTemperatureMax_s8 = 22;
const uint8_t addressTemperatureOffset_s8 = 23;
const uint8_t addressTemperatureOffsetMin_s8 = 24;
const uint8_t addressTemperatureOffsetMax_s8 = 25;
const uint8_t addressLowTemperatureAlarmValue_s8 = 26;
const uint8_t addressLowTemperatureAlarmMin_s8 = 27;
const uint8_t addressLowTemperatureAlarmMax_s8 = 28;



//uint8_t userSetting_u8[] = {firstTime_u8, timeOpen_hour_u8, timeOpen_minute_u8, timeClose_hour_u8, timeClose_minute_u8, 
							//sleepInterval_u8, displayInterval_u8, displayTimerInterval_u8, foodLevelMin_u8, foodLevelMax_u8,
							//gritLevelMin_u8, gritLevelMax_u8, calciumLevelMin_u8, calciumLevelMax_u8, waterLevelMin_u8,
							//waterLevelMax_u8, highTemperatureAlarmValue_u8, highTemperatureAlarmMin_u8, 
							//highTemperatureAlarmMax_u8, humidityOffset_u8, temperatureUnits};
							//
					//
//uint8_t userSetting_s8[] = {temperatureMin_s8, temperatureMax_s8, temperatureOffset_s8, temperatureOffsetMin_s8, 
							//temperatureOffsetMax_s8, lowTemperatureAlarmValue_s8, lowTemperatureAlarmMin_s8,
							//lowTemperatureAlarmMax_s8};



uint8_t userSettingEepromAddressesAll[] = {addressFirstTime_u8,addressTimeOpen_hour_u8,addressTimeOpen_minute_u8,
									addressTimeClose_hour_u8,addressTimeClose_minute_u8,addressSleepInterval_u8,
									addressDisplayInterval_u8,addressDisplayTimerInterval_u8,
									addressFoodLevelMin_u8,addressFoodLevelMax_u8,addressGritLevelMin_u8,
									addressGritLevelMax_u8,addressCalciumLevelMin_u8,
									addressCalciumLevelMax_u8,addressWaterLevelMin_u8,
									addressWaterLevelMax_u8,addressHighTemperatureAlarmValue_u8,
									addressHighTemperatureAlarmMin_u8,addressHighTemperatureAlarmMax_u8,
									addressHumidityOffset_u8,addressTemperatureUnits_u8,
									addressTemperatureMin_s8,addressTemperatureMax_s8,
									addressTemperatureOffset_s8,addressTemperatureOffsetMin_s8,
									addressTemperatureOffsetMax_s8,addressLowTemperatureAlarmValue_s8,
									addressLowTemperatureAlarmMin_s8,addressLowTemperatureAlarmMax_s8};

//uint8_t userSettingEepromAddressesUnsigned[] = {addressFirstTime_u8,addressTimeOpen_hour_u8,addressTimeOpen_minute_u8,addressTimeClose_hour_u8,
										//addressTimeClose_minute_u8,addressSleepInterval_u8,
										//addressDisplayInterval_u8,addressDisplayTimerInterval_u8,
										//addressFoodLevelMin_u8,addressFoodLevelMax_u8,addressGritLevelMin_u8,
										//addressGritLevelMax_u8,addressCalciumLevelMin_u8,
										//addressCalciumLevelMax_u8,addressWaterLevelMin_u8,
										//addressWaterLevelMax_u8,addressHighTemperatureAlarmValue_u8,
										//addressHighTemperatureAlarmMin_u8,addressHighTemperatureAlarmMax_u8,
										//addressHumidityOffset_u8,addressTemperatureUnits_u8};
										//
										//
//uint8_t userSettingEepromAddressesSigned[]={addressTemperatureMin_s8,addressTemperatureMax_s8,
										//addressTemperatureOffset_s8,addressTemperatureOffsetMin_s8,
										//addressTemperatureOffsetMax_s8,addressLowTemperatureAlarmValue_s8,
										//addressLowTemperatureAlarmMin_s8,addressLowTemperatureAlarmMax_s8};


//*********************
// Displays - Prototypes
//*********************

M2_EXTERN_VLIST(vlist_sanbornEngineering_toplevel);


void displays();	// function used to display screen information

// date/time display
// this is needed because dateTimeDisplay is called before it is declared
M2_EXTERN_VLIST(vlist_dateTimeDisplay_toplevel);

// alarm display
// this is needed because alarmDisplay is called before it is declared
M2_EXTERN_VLIST(vlist_alarmDisplay_toplevel);


// Food display
// this is needed because food display is called before it is declared
M2_EXTERN_VLIST(vlist_foodDisplay_toplevel);

// Grit display
// this is needed because grit display is called before it is declared
M2_EXTERN_VLIST(vlist_gritDisplay_toplevel);

// Calcium display
// this is needed because calcium display is called before it is declared
M2_EXTERN_VLIST(vlist_calciumDisplay_toplevel);

// Water display
// this is needed because water display is called before it is declared
M2_EXTERN_VLIST(vlist_waterDisplay_toplevel);

// Environment display
// this is needed because environment display is called before it is declared
M2_EXTERN_VLIST(vlist_environmentDisplay_toplevel);
//****************************************************************




//*********************
// User menus - Prototypes this needs to move to header file eventually
//*********************

// Main menu list
// this is needed because Main Menu is called before it is declared
M2_EXTERN_VLIST(vlist_mainMenu_toplevel);

// date/time menu list
// this is needed because timeMenu is called before it is declared
M2_EXTERN_VLIST(vlist_dateTimeMenu_toplevel);

// set date menu list
// this is needed because temperatureMenu is called before it is declared
M2_EXTERN_VLIST(vlist_dateMenu_toplevel);

// set time menu list
// this is needed because temperatureMenu is called before it is declared
M2_EXTERN_VLIST(vlist_timeMenu_toplevel);

// set open time menu list
// this is needed because temperatureMenu is called before it is declared
M2_EXTERN_VLIST(vlist_timeOpenMenu_toplevel);

// set close time menu list
// this is needed because temperatureMenu is called before it is declared
M2_EXTERN_VLIST(vlist_timeCloseMenu_toplevel);

// Food menu list
// this is needed because foodMenu is called before it is declared
M2_EXTERN_VLIST(Vlist_foodMenu_toplevel);

// Grit menu list
// this is needed because gritMenu is called before it is declared
M2_EXTERN_VLIST(Vlist_gritMenu_toplevel);

// Calcium menu list
// this is needed because calciumMenu is called before it is declared
M2_EXTERN_VLIST(Vlist_calciumMenu_toplevel);

// Water menu list
// this is needed because waterMenu is called before it is declared
M2_EXTERN_VLIST(Vlist_waterMenu_toplevel);

// set temperature menu list
// this is needed because temperatureMenu is called before it is declared
M2_EXTERN_VLIST(vlist_temperatureMenu_toplevel);

// set temperature offset
// this is needed because calibrateTemperatureMenu is called before it is declared
M2_EXTERN_VLIST(vlist_calibrateTemperatureMenu_toplevel);

// set temperature alarms
// this is needed because calibrateTemperatureMenu is called before it is declared
M2_EXTERN_VLIST(vlist_temperatureAlarm_toplevel);

// constructor that defines the very top element, also micro controller type, how many buttons, and lcd library to use
M2tk m2(&vlist_sanbornEngineering_toplevel, m2_es_arduino, m2_eh_6bs, m2_gh_nlc);






//////////////////////////////////////////////////////////////////////////
// Setup, run through one time
//////////////////////////////////////////////////////////////////////////
void setup() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	delay(1000);			// Wait to allow system to setup
	
	// Stores user settings in EEPROM 
	initEEPROM(); 
	
	Serial.println("--------------------------------");
	Serial.println("Chicken Tender Master Controller V1.0");
	Serial.println("by Sanborn Engineering");
	Serial.println("--------------------------------");
	
	CT.setupRTC();	
	pinMode(lcdPowerPin_u8, OUTPUT);
	digitalWrite(lcdPowerPin_u8, true);	// turn on LCD screen
	m2_SetNewLiquidCrystal(&lcd, 20, 4);	// define LCD and screen size
	m2_Draw();								// place splash screen on LCD
	CT.setupSensors();
	CT.setupUserButtons(uiKeySelectPin_U8, uiKeyNextPin_u8, uiKeyDataUpPin_u8, uiKeyDataDownPin_u8);
	m2.setPin(M2_KEY_SELECT, uiKeySelectPin_U8); 
	m2.setPin(M2_KEY_NEXT, uiKeyNextPin_u8);
	m2.setPin(M2_KEY_DATA_UP, uiKeyDataUpPin_u8);
	m2.setPin(M2_KEY_DATA_DOWN, uiKeyDataDownPin_u8);
	//Alarm.alarmRepeat(timeOpen_hour,timeOpen_minute, timeOpen_second, openAlarm);
	//Alarm.alarmRepeat(timeClose_hour,timeClose_minute, timeClose_second, closeAlarm);
	CT.setupWirelessLink();
	Serial.println("Setup Finished");
	Serial.print("Ram Free ");
	Serial.println(freeMemory());
	delay(3000); // delay for user to see sanborn engineering screen
}

void loop() {
	// put your main code here, to run repeatedly:

	
	switch (stateID)
	{
		case displayID_u8:
			CT.blinkLED(true,blinkInterval);					// activate heartbeat LED					
			displays(0); 								
			// check to see if the user has pressed up or down button 
			if(!CT.debounceButton(uiKeyDataUpPin_u8))
			{
				while(!CT.debounceButton(uiKeyDataUpPin_u8))
				{
					#ifdef _DEBUG
						Serial.println("Up Button Pressed");	// Hold until button is released
					#endif // _DEBUG
				}
				CT.sleepTimer(sleepInterval_u8, true);		// reset sleep timer
				displays(-1);
				
			}
			if(!CT.debounceButton(uiKeyDataDownPin_u8))
			{
				while(!CT.debounceButton(uiKeyDataDownPin_u8))
				{
					#ifdef _DEBUG
						Serial.println("Down Button Pressed");	// Hold until button is released
					#endif // _DEBUG
				}
				CT.sleepTimer(sleepInterval_u8, true);		// reset sleep timer
				displays(1);			
			}
			// check to see if the user has pressed select or next button 
			if(!CT.debounceButton(uiKeySelectPin_U8)|| !CT.debounceButton(uiKeyNextPin_u8))
			{
				stateID = userMenuID_u8;		// jump to user menu
				#ifdef _DEBUG
					Serial.println("Select/Next Button Pressed");	// Hold until button is released
					Serial.println("User Menu");	// Hold until button is released
				#endif // _DEBUG
				
				CT.sleepTimer(sleepInterval_u8, true);		// reset sleep timer
				m2_SetRoot(&vlist_mainMenu_toplevel);
				m2_Draw();
			}		
			else
			{
				Alarm.delay(0);		// check alarms
				if(CT.sleepTimer(sleepInterval_u8, false))
				{
					stateID = sleepModeID_u8;				
					// turn off LCD
									
					#ifdef _DEBUG
						Serial.println("Go to sleep");
					#endif // _DEBUG
				}
			}

			break;	// end display ID mode
	
	case sleepModeID_u8:
			
			
			//implement a sleep flag to save time in the future
			
			CT.blinkLED(false,blinkInterval); // Shut down heart beat LED
			// turn off LCD
			lcd.noDisplay();
			digitalWrite(lcdPowerPin_u8, LOW);		// turn off power to LCD screen
			// check to see if the user has pressed up or down button 
			if(!CT.debounceButton(uiKeyDataUpPin_u8) ||!CT.debounceButton(uiKeyDataDownPin_u8))
			{
				digitalWrite(lcdPowerPin_u8, HIGH);
				CT.sleepTimer(sleepInterval_u8, true);		// reset sleep timer
				// add delay?
				lcd.display();							// Activate LCD
				stateID = displayID_u8;					// jump to user display state
				#ifdef _DEBUG
					Serial.println("Goto user display");
				#endif // _DEBUG
				
			}
			// check to see if the user has pressed select or next button 
			else if (!CT.debounceButton(uiKeySelectPin_U8)|| !CT.debounceButton(uiKeyNextPin_u8))
			{
				digitalWrite(lcdPowerPin_u8, HIGH);
				// add delay?
				CT.sleepTimer(sleepInterval_u8, true);			// reset sleep timer
				lcd.display();								// Activate LCD
				CT.displayTimer(displayTimerInterval_u8,true); // reset display timer
				m2_SetRoot(&vlist_mainMenu_toplevel);		// goto main menu
				stateID = userMenuID_u8;
				#ifdef _DEBUG
					Serial.println("Goto User Menus");				
				#endif // _DEBUG
			}
			else
			{
				//CT.checkSensors(sensorInterval, true);
				//CT.checkAlarms(sensorInterval, true);
				//Narcoleptic(100);			// go into low power mode for xx ms
				//setSyncProvider(RTC.get); // sync system time with RTC time 
			}
			
		break;	// end sleep mode
		
	case userMenuID_u8:
			CT.blinkLED(true,blinkInterval);
			CT.sleepTimer(sleepInterval_u8, true);		// reset sleep timer
			m2.checkKey();
			m2.checkKey();
			if ( m2.handleKey() )
			{
				#ifdef _DEBUG
					Serial.println("Button Pressed");
				#endif // _DEBUG
				m2.draw();
				CT.displayTimer(displayTimerInterval_u8, true);
			}
			m2.checkKey();
			if(CT.displayTimer(displayTimerInterval_u8, false))
			{
				stateID = displayID_u8;
				#ifdef _DEBUG
					Serial.println("Go to user display");
				#endif // _DEBUG
			}
			
		break;	// end user menu mode
		
		
		
		
	case openDoorID_u8:

		break;	// end user menu mode
	
	case closeDoorID_u8:

		break;	// end user menu mode		
		
	}
}



//**********************
// main menu index setup
//**********************

// setup menu tree and assign function based on selection
const char *el_strlist_mainMenu_getstr(uint8_t idx, uint8_t msg) {
  const char *m = "";
  if  ( idx == 0 )
  {
    m = "Time/Date Menu";
  }
  else if ( idx == 1 )
  {
    m = "Food";
  }
  else if ( idx == 2 )
  {
	  m = "Grit";
  }  
  else if ( idx == 3 )
  {
	  m = "Calcium";
  }
  else if ( idx == 4 )
  {
    m = "Water";
  }
  else if ( idx == 5 )
  {
    m = "Temperature";
  }
  else if ( idx == 6 )
  {
	m = "User Displays";
  }
  
  else if (idx >6)
  {
    m= "Time/Date Menu";
  }    
  
  if (msg == M2_STRLIST_MSG_SELECT) 
  {
      Serial.print("idx = ");
      Serial.println(idx);
      Serial.print("m = ");
      Serial.println(m);
      
      switch(idx) 
      {
        case 0:
          Serial.println("goto time menu");
          m2_SetRoot(&vlist_dateTimeMenu_toplevel); // goto time menu
          break;
        case 1:
          Serial.println("goto food menu");
          m2_SetRoot(&Vlist_foodMenu_toplevel);		// goto food menu
          break;
        case 2:
		  Serial.println("goto grit menu");
          m2_SetRoot(&Vlist_gritMenu_toplevel);		// goto grit menu
          break;
        case 3:
          Serial.println("goto calcium menu");
          m2_SetRoot(&Vlist_calciumMenu_toplevel);		// goto calcium menu
        break;
        case 4:
          Serial.println("goto water menu");
          m2_SetRoot(&Vlist_waterMenu_toplevel);	// goto water menu
          break;    
        case 5:
          Serial.println("goto temperature menu");
          m2_SetRoot(&vlist_temperatureMenu_toplevel);
          break; 
		case 6:
		  Serial.println("goto user displays menu");
		  stateID=displayID_u8;
		  break;
        default :
          Serial.print("no menu change was initiated");
          break;  
      }     
  }
  return m;
}


//**********************
// food menu function setup
//**********************





//**********************
// time menu function for index setup
//**********************

// setup menu tree and assign function based on selection
const char *el_strlist_timeMenu_getstr(uint8_t idx, uint8_t msg) {
  const char *t = "";
  if  ( idx == 0 )
  {
    t = "Set Date";
  }
  else if ( idx == 1 )
  {
    t = "Set Time";
  }
  else if ( idx == 2 )
  {
    t = "Door open time";
  }
  else if ( idx == 3 )
  {
    t = "Door close time";
  }
  else if ( idx == 4 )
  {
    t = "Goto Main Menu";
  }
  else if ( idx > 4 )
  {
    t = "Set Date";
  }
  
  if (msg == M2_STRLIST_MSG_SELECT) 
  {
      Serial.print("time idx = ");
      Serial.println(idx);
      Serial.print("time t = ");
      Serial.println(t);
      
      switch(idx) 
      {
        case 0:
          //set date
          Serial.println("set date");
		  date_year_u8 = year()-2000;
		  date_month_u8 = month();
		  date_day_u8 = day();
          m2_SetRoot(&vlist_dateMenu_toplevel);
          break;
        case 1:
          //set time
          Serial.println("set time");
		  time_hour_u8 = hourFormat12(); 
		  time_minute_u8 = minute();
          m2_SetRoot(&vlist_timeMenu_toplevel);
          break;
        case 2:
          //"Door open time";
          Serial.println("Door open time");
          timeOpen_second_temp_u8 = timeOpen_second_u8; // used to store current alarm values in the even the user cancels the selection
          timeOpen_minute_temp_u8 = timeOpen_minute_u8;
          timeOpen_hour_temp_u8 = timeOpen_hour_u8;
          m2_SetRoot(&vlist_timeOpenMenu_toplevel);
          break;    
        case 3:
          //"Door close time";
          Serial.println("Door close time");
          timeClose_second_temp_u8 = timeClose_second_u8; // used to store current alarm values in the even the user cancels the selection
          timeClose_minute_temp_u8 = timeClose_minute_u8;
          timeClose_hour_temp_u8 = timeClose_hour_u8;
          m2_SetRoot(&vlist_timeCloseMenu_toplevel);
          break;
        case 4:
          //"Go back to main menu";
          Serial.println("go back to main menu");
          m2_SetRoot(&vlist_mainMenu_toplevel);
          break;  
        default :
          Serial.print("Error undeclared menu selected");
          break;
      }       
  }
  return t;
}



//*******************
// set date functions
//*******************

void date_ok_fn(m2_el_fnarg_p fnarg)  {

  setTime(hour(), minute(), second(),date_day_u8,date_month_u8,date_year_u8);
  displayDate();
  RTC.set(now());
  m2_SetRoot(&vlist_dateTimeMenu_toplevel);
}

void date_cancel_fn(m2_el_fnarg_p fnarg)  {
  date_month_u8 = month();
  date_day_u8 = day();
  date_year_u8 = year();
  displayDate();
  m2_SetRoot(&vlist_dateTimeMenu_toplevel);
}

//***********************
// set AM or PM depending on user selection
//************************



uint8_t time_hour_fn(m2_rom_void_p element, uint8_t msg, uint8_t val)
{
	#ifdef _DEBUG
	Serial.print("Incoming VAL = ");	
	Serial.println(val);
	#endif

	if ( msg == M2_U8_MSG_GET_VALUE )
	{
		if (isPM())
		{
			val = time_hour_u8-12;
		} 
		else
		{
			val = time_hour_u8;
		}
	}
	#ifdef _DEBUG
	Serial.print("Returning VAL = ");	
	Serial.println(val);
	#endif
	return val;
}

// Function to handle AM/PM selection in set time menu
void meridenSelect_fn(m2_el_fnarg_p fnarg)  {
	// when the button is clicked the following is executed.
	if (0 == meriden_flg )
	{
		
		meriden = "PM";
		meriden_flg = 1;
	}
	else
	{
		meriden = "AM";
		meriden_flg = 0;
	}
	#ifdef _DEBUG
	Serial.println(meriden);
	#endif

}

//*******************
// set time functions
//*******************

void time_ok_fn(m2_el_fnarg_p fnarg)  {
	Serial.println("set time ok");
	uint8_t temp_hour_u8;
	if (1==meriden_flg)
	{
		temp_hour_u8 = time_hour_u8+12;
	}
	else
	{
		temp_hour_u8 = time_hour_u8;
	}
	setTime(temp_hour_u8, time_minute_u8, time_second_u8,day(),month(),year());
	displayDate();
	RTC.set(now());
	m2_SetRoot(&vlist_dateTimeMenu_toplevel);
}

void time_cancel_fn(m2_el_fnarg_p fnarg)  {
  time_second_u8 = second();
  time_minute_u8 = minute();
  time_hour_u8 = hour();
  displayDate();
  m2_SetRoot(&vlist_dateTimeMenu_toplevel);
}

//*******************
// set open time function
//*******************

void timeOpen_ok_fn(m2_el_fnarg_p fnarg)  {
  //Alarm.alarmRepeat(timeOpen_hour,timeOpen_minute,timeOpen_second, MorningAlarm); 
  Serial.print("Time open");
  Serial.print(timeOpen_hour_u8);
  Serial.print(":");
  Serial.print(timeOpen_minute_u8);
  m2_SetRoot(&vlist_dateTimeMenu_toplevel);
}

void timeOpen_cancel_fn(m2_el_fnarg_p fnarg)  {
  // store the previous value of the alarm in the event the user cancels their input
  timeOpen_second_u8= timeOpen_second_temp_u8 ; // used to store current alarm values in the even the user cancels the selection
  timeOpen_minute_u8 = timeOpen_minute_temp_u8;
  timeOpen_hour_u8 = timeOpen_hour_temp_u8;
  m2_SetRoot(&vlist_dateTimeMenu_toplevel);
}


//*******************
// set close time function
//*******************

void timeClose_ok_fn(m2_el_fnarg_p fnarg)  {
  //Alarm.alarmRepeat(timeClose_hour+12,timeClose_minute,timeClose_second, EveningAlarm); 
  Serial.print("Time Close");
  Serial.print(timeClose_hour_u8);
  Serial.print(":");
  Serial.print(timeClose_minute_u8);
  m2_SetRoot(&vlist_dateTimeMenu_toplevel);
}

void timeClose_cancel_fn(m2_el_fnarg_p fnarg)  {
  // store the previous value of the alarm in the event the user cancels their input
  timeClose_second_u8 = timeClose_second_temp_u8; 
  timeClose_minute_u8 = timeClose_minute_temp_u8;
  timeClose_hour_u8 = timeClose_hour_temp_u8;
  m2_SetRoot(&vlist_dateTimeMenu_toplevel);
}

//**********************
// temperature menu index setup
//**********************

// setup menu tree and assign function based on selection
const char *el_strlist_temperatureMenu_getstr(uint8_t idx, uint8_t msg) 
{  
  const char *e = "";
  if  ( idx == 0 )
  {
	  if (1==temperatureUnits)
	  {
		  e = "Units = *F";
	  }
	  else
	  {
		   e = "Units = *C";
	  }
  }
  else if ( idx == 1 )
  {
    e = "Temperature Adj";
  }
  else if ( idx == 2 )
  {
    e = "Temp Alarm";
  }
  else if ( idx == 3 )
  {
    e = "Frostbite alarm";
  }
  else if ( idx == 4 )
  {
    e = "Humidity Adj.";
  }
  else if ( idx == 5 )
  {
    e = "Go back";
  }
  else if ( idx > 5 )
  {
    if (1==temperatureUnits)
    {
	    e = "Units = *F";
    }
    else
    {
	    e = "Units = *C";
    }
  }
  if (msg == M2_STRLIST_MSG_SELECT) 
  {
      Serial.print("Temperature idx = ");
      Serial.println(idx);
      Serial.print("Temperature e = ");
      Serial.println(e);
      
      switch(idx) 
      {
        case 0:
          if (1==temperatureUnits)
          {
			temperatureUnits = 0;
			Serial.println("Units = *C");
		  }
          else
		  {
			 temperatureUnits = 1;
			 Serial.println("Units = *F");
		  }
          break;
        case 1:
          Serial.println("Temperature Adjust");
		  m2_SetRoot(&vlist_calibrateTemperatureMenu_toplevel);
		  break;
        case 2:
          Serial.println("Temp alarm");
		  m2_SetRoot(&vlist_temperatureAlarm_toplevel);
          break;    
        case 3:
          Serial.println("Frostbite alarm");
          //m2_SetRoot(&list_label_strlist_waterMenu_vlist_toplevel);
          break;       
        case 4:
          Serial.println("Calibrate humidity");
          //m2_SetRoot(&list_label_strlist_waterMenu_vlist_toplevel);
          break;
        case 5:
          Serial.println("Go back");
          m2_SetRoot(&vlist_mainMenu_toplevel);
          break;            
        default :
          Serial.print("no menu change was initiated");
          break;
      }  
  }
  return e;
}

void displayDate()
{
	Serial.print(hour());
	Serial.print(":");
	Serial.print(minute());
	Serial.print(":");
	Serial.print(second());
	Serial.print(" - ");
	Serial.print(month());
	Serial.print("/");
	Serial.print(day());
	Serial.print("/");
	Serial.println(year());
}


void displays(int8_t direction)
{
	switch(userDisplayID)
	{
		
		case doNothingkDisplayID_u8:
			break;
						
		case clockDisplayID_u8:
			m2_SetRoot(&vlist_dateTimeDisplay_toplevel);
			m2_Draw();
			m2_HandleKey();
			break;
				
		case alarmDisplayID_u8:	
			m2_SetRoot(&vlist_alarmDisplay_toplevel);
			m2_Draw();
			m2_HandleKey();
			break;
				
		case foodDisplayID_u8:
			m2_SetRoot(&vlist_foodDisplay_toplevel);
			m2_Draw();
			m2_HandleKey();
			break;
				
		case gritDisplayID_u8:
			m2_SetRoot(&vlist_gritDisplay_toplevel);
			m2_Draw();
			m2_HandleKey();
			break;
				
		case calciumDisplayID_u8:
			m2_SetRoot(&vlist_calciumDisplay_toplevel);
			m2_Draw();
			m2_HandleKey();
			break;
				
				
		case waterDisplayID_u8:
			m2_SetRoot(&vlist_waterDisplay_toplevel);
			m2_Draw();
			m2_HandleKey();
			break;
				
		case environmentDisplayID_u8:
			m2_SetRoot(&vlist_environmentDisplay_toplevel);
			m2_Draw();
			m2_HandleKey();
			break;
	}
			
	userDisplayID = CT.advanceDisplays(displayRefreshRate, displayChangeRate_u8, environmentDisplayID_u8, direction);
	
	#ifdef _DEBUG
		if(0!=userDisplayID)
		{
			Serial.print("user display = ");
			Serial.println(userDisplayID);
		}

	#endif // _DEBUG
}


//void openAlarm()
//{
	////Serial.println("Morning alarm trigger open door");
//}
//
//void closeAlarm()
//{
	//Serial.println("Evening alarm trigger open door");
//}


uint8_t getYear(m2_rom_void_p element, uint8_t msg, uint8_t _year)
{
	_year = year()-2000;		// remove top two digits of year and return last two digits
	return _year;
	
}

uint8_t getMonth(m2_rom_void_p element, uint8_t msg, uint8_t _month)
{
	
	_month = month();
	return _month;
}

uint8_t getDay(m2_rom_void_p element, uint8_t msg, uint8_t _day)
{
	
	_day = day();
	return _day;
}

uint8_t getHour(m2_rom_void_p element, uint8_t msg, uint8_t _hour)
{
	
	_hour = hourFormat12();
	return _hour;
}

uint8_t getMinute(m2_rom_void_p element, uint8_t msg, uint8_t _minute)
{
	
	_minute = minute();
	return _minute;
}

uint8_t getSeconds(m2_rom_void_p element, uint8_t msg, uint8_t _seconds)
{
	
	_seconds = second();
	return _seconds;
}

const char *ampmDisplay(m2_rom_void_p element)
{
	if(isAM())
	{
		return "AM";
	}
	else if (isPM())
	{
		return "PM";	
	}
	else
	{
		return "xx";
	}
}



//////////////////////////////////////////////////////////////////////////
// Food display functions
//////////////////////////////////////////////////////////////////////////
uint8_t foodLevel_fn(m2_rom_void_p element, uint8_t msg, uint8_t foodLevelPerc) {
	
	
	float food = CT.foodSensor(temperature_flt);
	foodLevelPerc = map(food, foodLevelMin_u8, foodLevelMax_u8,1,99);
	
	#ifdef _DEBUG
		Serial.print("Food level = ");
		Serial.println(food);
	#endif // _DEBUG
	
	if(foodLevelMin_u8<food || food<foodLevelMax_u8)	// if sensor is out of range return 0
	{
		return NULL;
	}
		
	return foodLevelPerc;
	
}


uint8_t foodLevelChange_fn(m2_rom_void_p element, uint8_t msg, uint8_t foodLevelYesterday) {
	
	
	int8_t foodLevelChange = foodLevel_u8 - foodLevelYesterday;
	
	return foodLevelChange;
	
	
	
}


//////////////////////////////////////////////////////////////////////////
// Grit display functions
//////////////////////////////////////////////////////////////////////////

uint8_t gritLevel_fn(m2_rom_void_p element, uint8_t msg, uint8_t gritLevelPerc) {
	
	
	float grit = CT.gritSensor(temperature_flt);
	gritLevelPerc = map(grit, gritLevelMin_u8, gritLevelMax_u8,1,99);
	#ifdef _DEBUG
		Serial.print("Grit level = ");
		Serial.println(grit);
	#endif // _DEBUG
	
	if(gritLevelMin_u8<grit || grit<gritLevelMax_u8)	// if sensor is out of range return 0
	{
		return NULL;
	}

	return gritLevelPerc;
	
}


uint8_t gritLevelChange_fn(m2_rom_void_p element, uint8_t msg, uint8_t gritLevelYesterday) {
	
	
	int8_t gritLevelChange = gritLevel_u8 - gritLevelYesterday;
	
	return gritLevelChange;
	
}


//////////////////////////////////////////////////////////////////////////
// Calcium display functions
//////////////////////////////////////////////////////////////////////////
uint8_t calciumLevel_fn(m2_rom_void_p element, uint8_t msg, uint8_t calciumLevelPerc) {
	
	float calcium = CT.calciumSensor(temperature_flt);
	calciumLevelPerc = map(calcium, calciumLevelMin_u8, calciumLevelMax_u8,1,99);
	#ifdef _DEBUG
		Serial.print("Calcium level = ");
		Serial.println(calcium);
	#endif // _DEBUG
	if(calciumLevelMin_u8<calcium || calcium<calciumLevelMax_u8)	// if sensor is out of range return 0
	{
		return NULL;
	}
	return calciumLevelPerc;
	
}


uint8_t calciumLevelChange_fn(m2_rom_void_p element, uint8_t msg, uint8_t calciumLevelYesterday) {
	
	
	int8_t calciumLevelChange = calciumLevel_u8 - calciumLevelYesterday;
	
	return calciumLevelChange;
	
}

//////////////////////////////////////////////////////////////////////////
// water display functions
//////////////////////////////////////////////////////////////////////////

uint8_t waterLevel_fn(m2_rom_void_p element, uint8_t msg, uint8_t waterLevelPerc) {
	
	unsigned int water = 15;			// replace with sensor call function
	waterLevelPerc = map(water, waterLevelMin_u8, waterLevelMax_u8,0,99);
	return waterLevelPerc;
	
}


uint8_t waterLevelChange_fn(m2_rom_void_p element, uint8_t msg, uint8_t waterLevelYesterday) {
	
	
	int8_t waterLevelChange = waterLevel_u8 - waterLevelYesterday;
	
	return waterLevelChange;
	
}


/************************************************************************/
/* Temperature display functions                                        */
/************************************************************************/

uint8_t getTemperature_fn(m2_rom_void_p element, uint8_t msg, uint8_t temp) {
	
	float temperature = CT.temperatureSensor(temperatureUnits); 

	temp = round(temperature);
	return temp;
}

const char *tempUnits_fn(m2_rom_void_p element)
{
	if (1==temperatureUnits)
	{
		static const char f[] = "*F";
		return f;
	} 
	else
	{
		static const char c[] = "*C";
		return c;
	}
}

/************************************************************************/
/*  End of display functions                                            */
/************************************************************************/


void goBack2Main(m2_el_fnarg_p fnarg) {
	
	m2_SetRoot(&vlist_mainMenu_toplevel);
	
}

void goBack2TempMenu(m2_el_fnarg_p fnarg) {
	
	m2_SetRoot(&vlist_temperatureMenu_toplevel);
	
}

boolean lowTemperatureAlarm(int8_t actualTemperature, int8_t lowTemperatureThreshold)
{
	static boolean lowTemperatureFlag = LOW;
	
	if(actualTemperature<lowTemperatureThreshold)
	{
		lowTemperatureFlag = HIGH;
	}
	else
	{
		
		lowTemperatureFlag = LOW;
	}
	return lowTemperatureFlag;
}

//***********************
// EEPROM Functions
//***********************

// Function used to store EEPROM values first time program is stored
boolean initEEPROM()
{
	if(EEPROM.read(0) == firstTime_u8)
	{
		Serial.println("EEPROM Already contains information");
		// read unsigned variables from EEPROM
		timeOpen_hour_u8 = EEPROM.read(addressTimeOpen_hour_u8);
		timeOpen_minute_u8 = EEPROM.read(addressTimeOpen_minute_u8);
		timeClose_hour_u8 = EEPROM.read(addressTimeClose_hour_u8);
		timeClose_minute_u8 = EEPROM.read(addressTimeClose_minute_u8);
		sleepInterval_u8 = EEPROM.read(addressSleepInterval_u8);
		displayInterval_u8 = EEPROM.read(addressDisplayInterval_u8);
		displayTimerInterval_u8 = EEPROM.read(addressDisplayTimerInterval_u8);
		foodLevelMin_u8 = EEPROM.read(addressFoodLevelMin_u8);
		foodLevelMax_u8 = EEPROM.read(addressFoodLevelMax_u8);
		gritLevelMin_u8 = EEPROM.read(addressGritLevelMin_u8);
		gritLevelMax_u8 = EEPROM.read(addressGritLevelMax_u8);
		calciumLevelMin_u8 = EEPROM.read(addressCalciumLevelMin_u8);
		calciumLevelMax_u8 = EEPROM.read(addressCalciumLevelMax_u8);
		waterLevelMin_u8 = EEPROM.read(addressWaterLevelMin_u8);
		waterLevelMax_u8 = EEPROM.read(addressWaterLevelMax_u8);
		highTemperatureAlarmValue_u8 = EEPROM.read(addressHighTemperatureAlarmValue_u8);
		highTemperatureAlarmMin_u8 = EEPROM.read(addressHighTemperatureAlarmMin_u8);
		highTemperatureAlarmMax_u8 = EEPROM.read(addressHighTemperatureAlarmMax_u8);
		humidityOffset_u8 = EEPROM.read(addressHumidityOffset_u8);
		temperatureUnits = EEPROM.read(addressTemperatureUnits_u8);

		// read unsigned variables from EEPROM
		temperatureMin_s8 = EEPROM.read(addressTemperatureMin_s8);
		temperatureMax_s8 = EEPROM.read(addressTemperatureMax_s8);
		temperatureOffset_s8 = EEPROM.read(addressTemperatureOffset_s8);
		temperatureOffsetMin_s8 = EEPROM.read(addressTemperatureOffsetMin_s8);
		temperatureOffsetMax_s8 = EEPROM.read(addressTemperatureOffsetMax_s8);
		lowTemperatureAlarmValue_s8 = EEPROM.read(addressLowTemperatureAlarmValue_s8);
		lowTemperatureAlarmMin_s8 = EEPROM.read(addressLowTemperatureAlarmMin_s8);
		lowTemperatureAlarmMax_s8 = EEPROM.read(addressLowTemperatureAlarmMax_s8);	
	}
	else // First time storing data in EEPROM
	{
		Serial.println("EEPROM Empty place data inside");
		// write unsigned variables to EEPROM
		EEPROM.write(addressFirstTime_u8,firstTime_u8);
		EEPROM.write(addressTimeOpen_hour_u8,timeOpen_hour_u8);
		EEPROM.write(addressTimeOpen_minute_u8,timeOpen_minute_u8);
		EEPROM.write(addressTimeClose_hour_u8,timeClose_hour_u8);
		EEPROM.write(addressTimeClose_minute_u8,timeClose_minute_u8);
		EEPROM.write(addressSleepInterval_u8,sleepInterval_u8);
		EEPROM.write(addressDisplayInterval_u8,displayInterval_u8);
		EEPROM.write(addressDisplayTimerInterval_u8,displayTimerInterval_u8);
		EEPROM.write(addressFoodLevelMin_u8,foodLevelMin_u8);
		EEPROM.write(addressFoodLevelMax_u8,foodLevelMax_u8);
		EEPROM.write(addressGritLevelMin_u8,gritLevelMin_u8);
		EEPROM.write(addressGritLevelMax_u8,gritLevelMax_u8);
		EEPROM.write(addressCalciumLevelMin_u8,calciumLevelMin_u8);
		EEPROM.write(addressCalciumLevelMax_u8,calciumLevelMax_u8);
		EEPROM.write(addressWaterLevelMin_u8,waterLevelMin_u8);
		EEPROM.write(addressWaterLevelMax_u8,waterLevelMax_u8);
		EEPROM.write(addressHighTemperatureAlarmValue_u8,highTemperatureAlarmValue_u8);
		EEPROM.write(addressHighTemperatureAlarmMin_u8,highTemperatureAlarmMin_u8);
		EEPROM.write(addressHighTemperatureAlarmMax_u8,highTemperatureAlarmMax_u8);
		EEPROM.write(addressHumidityOffset_u8,humidityOffset_u8);
		EEPROM.write(addressTemperatureUnits_u8,temperatureUnits);

		// write signed variables to EEPROM
		EEPROM.write(addressTemperatureMin_s8,temperatureMin_s8);
		EEPROM.write(addressTemperatureMax_s8,temperatureMax_s8);
		EEPROM.write(addressTemperatureOffset_s8,temperatureOffset_s8);
		EEPROM.write(addressTemperatureOffsetMin_s8,temperatureOffsetMin_s8);
		EEPROM.write(addressTemperatureOffsetMax_s8,temperatureOffsetMax_s8);
		EEPROM.write(addressLowTemperatureAlarmValue_s8,lowTemperatureAlarmValue_s8);
		EEPROM.write(addressLowTemperatureAlarmMin_s8,lowTemperatureAlarmMin_s8);
		EEPROM.write(addressLowTemperatureAlarmMax_s8,lowTemperatureAlarmMax_s8);
		
	}
	
	#ifdef _DEBUG
	Serial.println("initialize EEPROM Complete");
	for(uint8_t k = 0; k<sizeof(userSettingEepromAddressesAll); k++)
	{
		Serial.print("EEPROM location " );
		Serial.print(userSettingEepromAddressesAll[k]);
		Serial.print(" = " );
		Serial.println(EEPROM.read(userSettingEepromAddressesAll[k]));
	}
	
	#endif // _DEBUG
	return NULL;
}


//***********************
// Labels used in program
//***********************
M2_LABEL(el_mainMenu_label, NULL,"Main Menu");
M2_LABEL(el_timeMenu_label, NULL,"Time Menu");
M2_LABEL(el_setTime_label, NULL,"Set Time");
M2_LABEL(el_setDate_label, NULL, "Set Date");
M2_LABEL(el_openTime_label, NULL, "Open Time");
M2_LABEL(el_closeTime_label, NULL, "Close Time");
M2_LABEL(el_AM_label, NULL, "AM");
M2_LABEL(el_PM_label, NULL, "PM");
M2_LABEL(el_dateStructure_label, NULL, " mn. : day: year");
M2_LABEL(el_timeStructure_label, NULL, " Hr : min: sec-AM/PM");
M2_LABEL(el_slash_label, NULL, "/");
M2_LABEL(el_colon_label, NULL, ":");  // colon separator
M2_LABEL(el_dash_label, NULL, "-");   // dash separator
M2_LABEL(el_percent_label, NULL, "%");   // percent symbol
M2_LABEL(el_cm_label, NULL, "cm");   // centimeter symbol
M2_SPACE(el_space,NULL); // space element
M2_LABEL(el_change_label, NULL,"Change since");
M2_LABEL(el_yesterday_label, NULL,"yesterday:");
M2_BUTTON(el_mainMenuGoBack_button, NULL, "Goto Main Menu", goBack2Main);
M2_BUTTON(el_tempMenuGoBack_button, NULL, "Goto Temp Menu", goBack2TempMenu);


//****************************************************************************************
// Splash screen
// ***************************************************************************************

M2_LABEL(el_sanborn_label, NULL,"Sanborn Engineering");
M2_LABEL(el_chicken_label, NULL,"Chicken Tender");
M2_LABEL(el_master_label, NULL,"Master Controller");
M2_LABEL(el_version_label, NULL,"Version: 1.0");
M2_LIST(list_sanbornEngineering) = { &el_sanborn_label, &el_chicken_label, &el_master_label, &el_version_label};
M2_VLIST(vlist_sanbornEngineering_toplevel, NULL, list_sanbornEngineering);



//*******************
// date/time display
//*******************

M2_LABEL(el_date_label, NULL, "Date"); 
M2_U8NUMFN(el_day_label, "c2r1", 1,31, getDay); // need to create a function to return current day
M2_U8NUMFN(el_month_label,"c2r1", 1,12, getMonth); // need to create a function to return current month
M2_U8NUMFN(el_year_label, "c2r1", 0,99, getYear); // // need to create a function to return current year
M2_LIST(list_dateDisplay) = { &el_month_label, &el_slash_label, &el_day_label, &el_slash_label, &el_year_label};
M2_HLIST(hlist_dateDisplay, NULL, list_dateDisplay);
M2_LABEL(el_time_label, NULL, "Time"); 
M2_U8NUMFN(el_hour_label, "c2r1", 1,12, getHour); // need to create a function to return current hour
M2_U8NUMFN(el_minute_label, "c2r1", 0,59, getMinute); // need to create a function to return current minute
M2_U8NUMFN(el_second_label, "c2r1", 0,59, getSeconds); // need to create a function to return current second
M2_LABELFN(el_AMPMDisplay_labelfn, NULL, ampmDisplay);
M2_LIST(list_timeDisplay) = { &el_hour_label, &el_colon_label, &el_minute_label, &el_colon_label, &el_second_label, &el_AMPMDisplay_labelfn};
M2_HLIST(hlist_timeDisplay, NULL, list_timeDisplay);
M2_LIST(list_dateTimeDisplay) = {&el_date_label, &hlist_dateDisplay, &el_time_label, &hlist_timeDisplay};
M2_VLIST(vlist_dateTimeDisplay_toplevel, NULL, list_dateTimeDisplay);


//*******************
// Alarms display
//*******************
M2_U8NUM(el_openHour_label, "c2r1", 1,12, &timeOpen_hour_u8); // Hour When door opens
M2_U8NUM(el_openMinute_label, "c2r1", 0,59, &timeOpen_minute_u8); // Minute When door opens
M2_U8NUM(el_closeHour_label, "c2r1", 1,12, &timeClose_hour_u8); // Hour When door closes
M2_U8NUM(el_closeMinute_label, "c2r1", 0,59, &timeClose_minute_u8); // Minute When door closes
M2_LIST(list_openTime) = {&el_openHour_label, &el_colon_label, &el_openMinute_label, &el_AM_label};
M2_HLIST(hlist_openTime, NULL, list_openTime);
M2_LIST(list_closeTime) = {&el_closeHour_label, &el_colon_label, &el_closeMinute_label, &el_PM_label};
M2_HLIST(hlist_closeTime, NULL, list_closeTime);
M2_LIST(list_alarmDisplay) = {&el_openTime_label, &hlist_openTime,  &el_closeTime_label, &hlist_closeTime};
M2_VLIST(vlist_alarmDisplay_toplevel, NULL, list_alarmDisplay);

//*******************
// Food display
//*******************


M2_LABEL(el_foodLevel_label, NULL,"Food Level Now");
M2_U8NUMFN(el_foodLevelValue_U8FN, "c2r1", 0,99, foodLevel_fn);
M2_LIST(list_displayFoodLevel) = {&el_foodLevelValue_U8FN, &el_percent_label};
M2_HLIST(hlist_displayFoodLevel, NULL, list_displayFoodLevel);
M2_S8NUMFN(el_foodLevelChange_S8, "c2r1", -99,99, foodLevelChange_fn);
M2_LIST(list_displayFoodLevelChange) = {&el_yesterday_label, &el_foodLevelChange_S8, &el_cm_label};
M2_HLIST(hlist_displayFoodLevelChange, NULL, list_displayFoodLevelChange);
M2_LIST(list_foodUserDisplay) = {&el_foodLevel_label, &hlist_displayFoodLevel, &el_change_label, &hlist_displayFoodLevelChange};
M2_VLIST(vlist_foodDisplay_toplevel, NULL, list_foodUserDisplay);

//*******************
// Grit display
//*******************


M2_LABEL(el_gritLevel_label, NULL,"Grit Level Now");
M2_U8NUMFN(el_gritLevelValue_U8FN, "c2r1", 0,99, gritLevel_fn);
M2_LIST(list_displayGritLevel) = {&el_gritLevelValue_U8FN, &el_percent_label};
M2_HLIST(hlist_displayGritLevel, NULL, list_displayGritLevel);
M2_S8NUMFN(el_gritLevelChange_S8, "c2r1", 0,99, gritLevelChange_fn);
M2_LIST(list_displayGritLevelChange) = {&el_yesterday_label,&el_gritLevelChange_S8, &el_cm_label};
M2_HLIST(hlist_displayGritLevelChange, NULL, list_displayGritLevelChange);
M2_LIST(list_gritUserDisplay) = {&el_gritLevel_label, &hlist_displayGritLevel, &el_change_label, &hlist_displayGritLevelChange};
M2_VLIST(vlist_gritDisplay_toplevel, NULL, list_gritUserDisplay);

//*******************
// Calcium display
//*******************


M2_LABEL(el_calciumLevel_label, NULL,"Calcium Level Now");
M2_U8NUMFN(el_calciumLevelValue_U8FN, "c2r1", 0,99, calciumLevel_fn);
M2_LIST(list_displayCalciumLevel) = {&el_calciumLevelValue_U8FN, &el_percent_label};
M2_HLIST(hlist_displayCalciumLevel, NULL, list_displayCalciumLevel);
M2_S8NUMFN(el_calciumLevelChange_S8, "c2r1", -99,99, calciumLevelChange_fn);
M2_LIST(list_displayCalciumLevelChange) = {&el_yesterday_label, &el_calciumLevelChange_S8, &el_cm_label};
M2_HLIST(hlist_displayCalciumLevelChange, NULL, list_displayCalciumLevelChange);
M2_LIST(list_calciumUserDisplay) = {&el_calciumLevel_label, &hlist_displayCalciumLevel, &el_change_label, &hlist_displayCalciumLevelChange};
M2_VLIST(vlist_calciumDisplay_toplevel, NULL, list_calciumUserDisplay);


//*******************
// Water display
//*******************


M2_LABEL(el_waterLevel_label, NULL,"H20 Level Now");
M2_U8NUMFN(el_waterLevelValue_U8FN, "c2r1", 0,99, waterLevel_fn);
M2_LIST(list_displayWaterLevel) = {&el_waterLevelValue_U8FN, &el_percent_label};
M2_HLIST(hlist_displayWaterLevel, NULL, list_displayWaterLevel);
M2_S8NUMFN(el_waterLevelChange_S8, "c2r1", -99,99, waterLevelChange_fn);
M2_LIST(list_displayWaterLevelChange) = {&el_yesterday_label, &el_waterLevelChange_S8, &el_cm_label};
M2_HLIST(hlist_displayWaterLevelChange, NULL, list_displayWaterLevelChange);
M2_LIST(list_waterUserDisplay) = {&el_waterLevel_label, &hlist_displayWaterLevel, &el_change_label, &hlist_displayWaterLevelChange};
M2_VLIST(vlist_waterDisplay_toplevel, NULL, list_waterUserDisplay);

//*******************
// Environment display
//*******************
M2_LABEL(el_temperature_label, NULL,"Temperature Now");
M2_S8NUMFN(el_temperatureValue_S8, "c2r1", temperatureMin_s8,temperatureMax_s8, getTemperature_fn);
M2_LABELFN(el_tempUnits_label, NULL, tempUnits_fn); 
M2_LIST(list_displaytemperatureValue) = {&el_temperatureValue_S8, &el_tempUnits_label};
M2_HLIST(hlist_displaytemperatureValue, NULL, list_displaytemperatureValue);
//M2_LABEL(el_foodLevelChange_label, NULL,"Change since yesterday");
//M2_S8NUMFN(el_foodLevelChange_S8, "c2r1", 0,99, foodLevelChange_fn);
//M2_LIST(list_displayFoodLevelChange) = {&el_foodLevelChange_S8, &el_cm_label};
//M2_HLIST(hlist_foodLevelChange, NULL, list_displayFoodLevelChange);
M2_LIST(list_environmentUserDisplay) = {&el_temperature_label, &hlist_displaytemperatureValue, /*&el_foodLevelChange_label, &hlist_displayFoodLevelChange*/};
M2_VLIST(vlist_environmentDisplay_toplevel, NULL, list_environmentUserDisplay);





//****************************************************************************************
// User Menu setup elements
// ***************************************************************************************


//*******************
// Main Menu setup
//*******************
M2_STRLIST(el_strlist_mainMenu, "l3w15", &el_strlist_mainMenu_first_u8, &el_strlist_mainMenu_cnt_u8, el_strlist_mainMenu_getstr);
M2_VSB(el_strlist_mainMenu_vsb, "l3w1", &el_strlist_mainMenu_first_u8, &el_strlist_mainMenu_cnt_u8);
M2_LIST(list_strlist_mainMenu) = { &el_strlist_mainMenu_vsb , &el_strlist_mainMenu };
M2_HLIST(el_strlist_mainMenu_hlist, NULL, list_strlist_mainMenu);
M2_LIST(list_label_strlist_mainMenu) = {&el_mainMenu_label, &el_strlist_mainMenu_hlist};
M2_VLIST(vlist_mainMenu_toplevel, NULL, list_label_strlist_mainMenu);


//*******************
// Time menu setup
//*******************
M2_STRLIST(el_strlist_timeMenu, "l3w15", &el_strlist_timeMenu_first_u8, &el_strlist_timeMenu_cnt_u8, el_strlist_timeMenu_getstr);
M2_VSB(el_strlist_timeMenu_vsb, "l3w1", &el_strlist_timeMenu_first_u8, &el_strlist_timeMenu_cnt_u8);
// determines the horizontal position of the elements vs the scroll bar
M2_LIST(list_strlist_timeMenu) = { &el_strlist_timeMenu_vsb , /*&el_space,*/ &el_strlist_timeMenu };
M2_HLIST(el_strlist_timeMenu_hlist, NULL, list_strlist_timeMenu);
M2_LIST(list_label_strlist_timeMenu) = {&el_timeMenu_label, &el_strlist_timeMenu_hlist};
M2_VLIST(vlist_dateTimeMenu_toplevel, NULL, list_label_strlist_timeMenu);

//*******************
// Set Date menu setup
//*******************
M2_U8NUM(el_date_day, "a0c2", 1,31, &date_day_u8);
M2_U8NUM(el_date_month, "a0c2", 1,12,&date_month_u8);
M2_U8NUM(el_date_year, "a0c2", 0,99,&date_year_u8);
M2_LIST(list_date) = { &el_date_month, &el_slash_label, &el_date_day, &el_slash_label, &el_date_year };
M2_HLIST(hlist_date, NULL, list_date);
M2_BUTTON(el_date_cancel, NULL, "CANCEL", date_cancel_fn);
M2_BUTTON(el_date_ok, NULL, "OK", date_ok_fn);
M2_LIST(list_date_buttons) = {&el_date_ok, &el_date_cancel };
M2_HLIST(el_date_buttons, NULL, list_date_buttons);
M2_LIST(list_dateMenu) = {&el_setDate_label, &el_dateStructure_label, &hlist_date, &el_date_buttons };
M2_VLIST(vlist_dateMenu_toplevel, NULL, list_dateMenu);

//*******************
// Set time menu
//*******************

M2_U8NUM(el_time_hour, "c2", 1,12,&time_hour_u8);
M2_U8NUM(el_time_minute, "c2", 0,59,&time_minute_u8);
M2_U8NUMFN(el_time_second, "c2r1", 0,59,getSeconds);
M2_BUTTONPTR(el_meriden_button,NULL, &meriden, meridenSelect_fn);
M2_LIST(list_time) = { &el_time_hour, &el_colon_label, &el_time_minute, &el_colon_label, &el_time_second, &el_dash_label, &el_meriden_button};
M2_HLIST(hlist_time, NULL, list_time);
M2_BUTTON(el_time_cancel, NULL, "CANCEL", time_cancel_fn);
M2_BUTTON(el_time_ok, NULL, "OK", time_ok_fn);
M2_LIST(list_time_buttons) = {&el_time_ok, &el_time_cancel };
M2_HLIST(el_time_buttons, NULL, list_time_buttons);
M2_LIST(list_timeMenu) = {&el_setTime_label, &el_timeStructure_label, &hlist_time, &el_time_buttons };
M2_VLIST(vlist_timeMenu_toplevel, NULL, list_timeMenu);



//*******************
// Open time menu
//*******************

M2_U8NUM(el_timeOpen_hour, "c2", 1,12,&timeOpen_hour_u8);
M2_U8NUM(el_timeOpen_minute, "c2", 0,59,&timeOpen_minute_u8);
M2_U8NUM(el_timeOpen_second, "c2r1", 0,59,&timeOpen_second_u8);
M2_LIST(list_timeOpen) = { &el_timeOpen_hour, &el_colon_label, &el_timeOpen_minute, &el_colon_label, &el_timeOpen_second};
M2_HLIST(hlist_timeOpen, NULL, list_timeOpen);
M2_BUTTON(el_timeOpen_cancel, NULL, "CANCEL", timeOpen_cancel_fn);
M2_BUTTON(el_timeOpen_ok, NULL, "OK", timeOpen_ok_fn);
M2_LIST(list_timeOpen_buttons) = {&el_timeOpen_ok, &el_timeOpen_cancel };
M2_HLIST(el_timeOpen_buttons, NULL, list_timeOpen_buttons);
M2_LIST(list_timeOpenMenu) = {&el_openTime_label, &el_timeStructure_label, &hlist_timeOpen, &el_timeOpen_buttons };
M2_VLIST(vlist_timeOpenMenu_toplevel, NULL, list_timeOpenMenu);



//*******************
// Close time
//*******************

M2_U8NUM(el_timeClose_hour, "c2", 1,12,&timeClose_hour_u8);
M2_U8NUM(el_timeClose_minute, "c2", 0,59,&timeClose_minute_u8);
M2_U8NUM(el_timeClose_second, "c2r1", 0,59,&timeClose_second_u8);
M2_LIST(list_timeClose) = { &el_timeClose_hour, &el_colon_label, &el_timeClose_minute, &el_colon_label, &el_timeClose_second};
M2_HLIST(hlist_timeClose, NULL, list_timeClose);
M2_BUTTON(el_timeClose_cancel, NULL, "CANCEL", timeClose_cancel_fn);
M2_BUTTON(el_timeClose_ok, NULL, "OK", timeClose_ok_fn);
M2_LIST(list_timeClose_buttons) = {&el_timeClose_ok, &el_timeClose_cancel };
M2_HLIST(el_timeClose_buttons, NULL, list_timeClose_buttons);
M2_LIST(list_timeCloseMenu) = {&el_closeTime_label, &el_timeStructure_label, &hlist_timeClose, &el_timeClose_buttons };
M2_VLIST(vlist_timeCloseMenu_toplevel, NULL, list_timeCloseMenu);


//*******************
// Food menu setup
//*******************
M2_U8NUMFN(el_foodLevelValue_U8, "c2r1",0,99, foodLevel_fn);
M2_LIST(list_foodLevel) = {&el_foodLevel_label, &el_foodLevelValue_U8, &el_percent_label};
M2_HLIST(hlist_foodLevel, NULL, list_foodLevel);
M2_LABEL(el_foodLevelMax_label, NULL,"Food Max");
M2_U8NUM(el_foodLevelMax_U8, "c2", 0, 99,&foodLevelMax_u8);
M2_LIST(list_foodLevelMax) = {&el_foodLevelMax_label, &el_foodLevelMax_U8, &el_cm_label};
M2_HLIST(hlist_foodLevelMax, NULL, list_foodLevelMax);
M2_LABEL(el_foodLevelMin_label, NULL,"Food Min");
M2_U8NUM(el_foodLevelMin_U8, "c2", 0, 99, &foodLevelMin_u8);
M2_LIST(list_foodLevelMin) = {&el_foodLevelMin_label, &el_foodLevelMin_U8, &el_cm_label};
M2_HLIST(hlist_foodLevelMin, NULL, list_foodLevelMin);
M2_LIST(list_foodMenu) = {&hlist_foodLevel, &hlist_foodLevelMin, &hlist_foodLevelMax, &el_mainMenuGoBack_button};
M2_VLIST(Vlist_foodMenu_toplevel, NULL, list_foodMenu);


//*******************
// Grit menu setup
//*******************
//M2_LABEL(el_gritLevel_label, NULL,"Grit Level Now");
M2_U8NUMFN(el_gritLevelValue_U8, "c2r1", 0,99, gritLevel_fn);
M2_LIST(list_gritLevel) = {&el_gritLevel_label, &el_gritLevelValue_U8, &el_percent_label};
M2_HLIST(hlist_gritLevel, NULL, list_gritLevel);
M2_LABEL(el_gritLevelMax_label, NULL,"Grit Max");
M2_U8NUM(el_gritLevelMax_U8, "c2", 0, 99,&gritLevelMax_u8);
M2_LIST(list_gritLevelMax) = {&el_gritLevelMax_label, &el_gritLevelMax_U8, &el_cm_label};
M2_HLIST(hlist_gritLevelMax, NULL, list_gritLevelMax);
M2_LABEL(el_gritLevelMin_label, NULL,"Grit Min");
M2_U8NUM(el_gritLevelMin_U8, "c2", 0, 99, &gritLevelMin_u8);
M2_LIST(list_gritLevelMin) = {&el_gritLevelMin_label, &el_gritLevelMin_U8, &el_cm_label};
M2_HLIST(hlist_gritLevelMin, NULL, list_gritLevelMin);
M2_LIST(list_gritMenu) = {&hlist_gritLevel, &hlist_gritLevelMin, &hlist_gritLevelMax, &el_mainMenuGoBack_button};
M2_VLIST(Vlist_gritMenu_toplevel, NULL, list_gritMenu);
//

//*******************
// Calcium menu setup
//*******************
//M2_LABEL(el_calciumLevel_label, NULL,"Calcium Level");
M2_U8NUMFN(el_calciumLevelValue_U8, "c2r1", 0,99, calciumLevel_fn);
M2_LIST(list_calciumLevel) = {&el_calciumLevel_label, &el_calciumLevelValue_U8, &el_percent_label};
M2_HLIST(hlist_calciumLevel, NULL, list_calciumLevel);
M2_LABEL(el_calciumLevelMax_label, NULL,"Cal. Max");
M2_U8NUM(el_calciumLevelMax_U8, "c2", 0, 99, &calciumLevelMax_u8);
M2_LIST(list_calciumLevelMax) = {&el_calciumLevelMax_label, &el_calciumLevelMax_U8, &el_cm_label};	
M2_HLIST(hlist_calciumLevelMax, NULL, list_calciumLevelMax);
M2_LABEL(el_calciumLevelMin_label, NULL,"Cal. Min");
M2_U8NUM(el_calciumLevelMin_U8, "c2", 0, 99, &calciumLevelMin_u8);
M2_LIST(list_calciumLevelMin) = {&el_calciumLevelMin_label, &el_calciumLevelMin_U8, &el_cm_label};
M2_HLIST(hlist_calciumLevelMin, NULL, list_calciumLevelMin);
M2_LIST(list_calciumMenu) = {&hlist_calciumLevel, &hlist_calciumLevelMin, &hlist_calciumLevelMax, &el_mainMenuGoBack_button};
M2_VLIST(Vlist_calciumMenu_toplevel, NULL, list_calciumMenu);
//
//*******************
// Water menu setup
//*******************

//M2_LABEL(el_waterLevel_label, NULL,"H2O Level Now");
M2_U8NUM(el_waterLevelValue_U8, "c2r1", 0, 99, &waterLevel_u8);
M2_LIST(list_waterLevel) = {&el_waterLevel_label, &el_waterLevelValue_U8, &el_percent_label};
M2_HLIST(hlist_waterLevel, NULL, list_waterLevel);
M2_LABEL(el_waterLevelMax_label, NULL,"Water Max");
M2_U8NUM(el_waterLevelMax_U8, "c2", 0, 300, &waterLevelMax_u8);
M2_LIST(list_waterLevelMax) = {&el_waterLevelMax_label, &el_waterLevelMax_U8, &el_cm_label};
M2_HLIST(hlist_waterLevelMax, NULL, list_waterLevelMax);
M2_LABEL(el_waterLevelMin_label, NULL,"Water Min");
M2_U8NUM(el_waterLevelMin_U8, "c2", 0, 300, &waterLevelMin_u8);
M2_LIST(list_waterLevelMin) = {&el_waterLevelMin_label, &el_waterLevelMin_U8, &el_cm_label};
M2_HLIST(hlist_waterLevelMin, NULL, list_waterLevelMin);
M2_LIST(list_waterMenu) = {&hlist_waterLevel, &hlist_waterLevelMin, &hlist_waterLevelMax, &el_mainMenuGoBack_button};
M2_VLIST(Vlist_waterMenu_toplevel, NULL, list_waterMenu);


//*******************
// Temperature menu setup
//*******************
M2_LABEL(el_title_temperatureMenu_label, NULL,"Temperature Menu");
M2_STRLIST(el_strlist_temperatureMenu, "l3w15", &el_strlist_temperatureMenu_first_u8, &el_strlist_temperatureMenu_cnt_u8, el_strlist_temperatureMenu_getstr);
M2_VSB(el_strlist_temperatureMenu_vsb, "l3w1", &el_strlist_temperatureMenu_first_u8, &el_strlist_temperatureMenu_cnt_u8);
M2_LIST(list_strlist_temperatureMenu) = { &el_strlist_temperatureMenu_vsb , &el_strlist_temperatureMenu};
M2_HLIST(el_strlist_temperatureMenu_hlist, NULL, list_strlist_temperatureMenu);
M2_LIST(list_label_strlist_temperatureMenu) = {&el_title_temperatureMenu_label, &el_strlist_temperatureMenu_hlist};
M2_VLIST(vlist_temperatureMenu_toplevel, NULL, list_label_strlist_temperatureMenu);

//"Calibrate temp";
M2_LABEL(el_title_temperatureCalibrateMenu_label, NULL,"Temperature Adjust");
M2_LABEL(el_actualTemperature_label, NULL, "Actual Temp =");
M2_S8NUMFN(el_actualTemperatureNumber_label, "+c3r1", temperatureMin_s8, temperatureMax_s8, getTemperature_fn);
//M2_S8NUM(el,fmt,min,max,number)
M2_LIST(list_actualTemperature) = {&el_actualTemperature_label, &el_actualTemperatureNumber_label};
M2_HLIST(hlist_actualTemperature, NULL, list_actualTemperature);
M2_LABEL(el_temperatureOffset_label, NULL, "Temp Offset =");
M2_S8NUM(el_temperatureOffset_S8NUM, "+c2", temperatureOffsetMin_s8, temperatureOffsetMax_s8, &temperatureOffset_s8);
M2_LIST(list_temperatureOffset) = {&el_temperatureOffset_label, &el_temperatureOffset_S8NUM};
M2_HLIST(hlist_temperatureOffset, NULL, list_temperatureOffset);
M2_LIST(list_calibrateTemperatureMenu) = {&el_title_temperatureCalibrateMenu_label, &hlist_actualTemperature, &hlist_temperatureOffset, &el_tempMenuGoBack_button};
M2_VLIST(vlist_calibrateTemperatureMenu_toplevel, NULL, list_calibrateTemperatureMenu);

//"Temperature alarms";
M2_LABEL(el_title_TemperatureAlarm_label, NULL,"Temperature Alarms");
M2_LABEL(el_lowTemperatureAlarm_label, NULL , "Low Temp Alarm");
M2_S8NUM(el_lowTemperatureAlarm_S8NUM,"+c2", -40, 98, &lowTemperatureAlarmValue_s8);
M2_LIST(list_lowTemperatureAlarm) = {&el_lowTemperatureAlarm_label, &el_lowTemperatureAlarm_S8NUM};
M2_HLIST(hlist_lowTemperatureAlarm, NULL, list_lowTemperatureAlarm);
M2_LABEL(el_highTemperatureAlarm_label, NULL, "High Temp Alarm");
M2_U8NUM(el_highTemperatureAlarm_U8NUM, "+c2", 0, 120, &highTemperatureAlarmValue_u8);
M2_LIST(list_highTemperatureAlarm) = {&el_highTemperatureAlarm_label, &el_highTemperatureAlarm_U8NUM};
M2_HLIST(hlist_highTemperatureAlarm, NULL, list_highTemperatureAlarm);
M2_LIST(list_temperatureAlarm) = {&el_title_TemperatureAlarm_label, &hlist_lowTemperatureAlarm, &hlist_highTemperatureAlarm, &el_tempMenuGoBack_button};
M2_VLIST(vlist_temperatureAlarm_toplevel, NULL, list_temperatureAlarm);


//"Frostbite alarm"
// Enabled
// Generate linear function for humidity/temp


//"Calibrate humidity";
// Add offset


//"Axillary lighting
// Enabled
// Turn on time morning
// Turn off time morning
// Turn on time night
// Turn off time night

// RFID tracking
// Enabled
// Name chickens

// Wireless link to home
// Enabled
// key










