/* 
	Editor: http://www.visualmicro.com
	        visual micro and the arduino ide ignore this code during compilation. this code is automatically maintained by visualmicro, manual changes to this file will be overwritten
	        the contents of the Visual Micro sketch sub folder can be deleted prior to publishing a project
	        all non-arduino files created by visual micro and all visual studio project or solution files can be freely deleted and are not required to compile a sketch (do not delete your own code!).
	        note: debugger breakpoints are stored in '.sln' or '.asln' files, knowledge of last uploaded breakpoints is stored in the upload.vmps.xml file. Both files are required to continue a previous debug session without needing to compile and upload again
	
	Hardware: Arduino Mega 2560 or Mega ADK, Platform=avr, Package=arduino
*/

#define __AVR_ATmega2560__
#define ARDUINO 101
#define ARDUINO_MAIN
#define F_CPU 16000000L
#define __AVR__
#define __cplusplus
extern "C" void __cxa_pure_virtual() {;}

//
//
const char *el_strlist_mainMenu_getstr(uint8_t idx, uint8_t msg);
const char *el_strlist_timeMenu_getstr(uint8_t idx, uint8_t msg);
void date_ok_fn(m2_el_fnarg_p fnarg);
void date_cancel_fn(m2_el_fnarg_p fnarg);
void meridenSelect_fn(m2_el_fnarg_p fnarg);
uint8_t time_hour_fn(m2_rom_void_p element, uint8_t msg, uint8_t _hour);
void time_ok_fn(m2_el_fnarg_p fnarg);
void time_cancel_fn(m2_el_fnarg_p fnarg);
void timeOpen_ok_fn(m2_el_fnarg_p fnarg);
void timeOpen_cancel_fn(m2_el_fnarg_p fnarg);
void timeClose_ok_fn(m2_el_fnarg_p fnarg);
void timeClose_cancel_fn(m2_el_fnarg_p fnarg);
const char *el_strlist_temperatureMenu_getstr(uint8_t idx, uint8_t msg);
void displayDate();
void displays(int8_t direction);
uint8_t getYear(m2_rom_void_p element, uint8_t msg, uint8_t _year);
uint8_t getMonth(m2_rom_void_p element, uint8_t msg, uint8_t _month);
uint8_t getDay(m2_rom_void_p element, uint8_t msg, uint8_t _day);
uint8_t getHour(m2_rom_void_p element, uint8_t msg, uint8_t _hour);
uint8_t getMinute(m2_rom_void_p element, uint8_t msg, uint8_t _minute);
uint8_t getSeconds(m2_rom_void_p element, uint8_t msg, uint8_t _seconds);
const char *ampmDisplay(m2_rom_void_p element);
uint8_t foodLevel_fn(m2_rom_void_p element, uint8_t msg, uint8_t foodLevelPerc);
uint8_t foodLevelChange_fn(m2_rom_void_p element, uint8_t msg, uint8_t foodLevelYesterday);
uint8_t gritLevel_fn(m2_rom_void_p element, uint8_t msg, uint8_t gritLevelPerc);
uint8_t gritLevelChange_fn(m2_rom_void_p element, uint8_t msg, uint8_t gritLevelYesterday);
uint8_t calciumLevel_fn(m2_rom_void_p element, uint8_t msg, uint8_t calciumLevelPerc);
uint8_t calciumLevelChange_fn(m2_rom_void_p element, uint8_t msg, uint8_t calciumLevelYesterday);
uint8_t waterLevel_fn(m2_rom_void_p element, uint8_t msg, uint8_t waterLevelPerc);
uint8_t waterLevelChange_fn(m2_rom_void_p element, uint8_t msg, uint8_t waterLevelYesterday);
uint8_t getTemperature_fn(m2_rom_void_p element, uint8_t msg, uint8_t temp);
const char *tempUnits_fn(m2_rom_void_p element);
void goBack2Main(m2_el_fnarg_p fnarg);
void goBack2TempMenu(m2_el_fnarg_p fnarg);
boolean lowTemperatureAlarm(int8_t actualTemperature, int8_t lowTemperatureThreshold);
unsigned int searchIndex(int value, byte lookUpTable[]);

#include "C:\Program Files (x86)\Arduino\hardware\arduino\variants\mega\pins_arduino.h" 
#include "C:\Program Files (x86)\Arduino\hardware\arduino\cores\arduino\arduino.h"
#include "C:\Users\James\Documents\Arduino\Sketches\MasterController\MasterController.ino"
