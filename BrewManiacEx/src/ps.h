/**********************************************************************
 BrewManiac
 created by Vito Tai
 Copyright (C) 2015 Vito Tai

 This soft ware is provided as-is. Use at your own risks.
 You are free to modify and distribute this software without removing
 this statement.
 BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/

#ifndef PS_H
#define PS_H
// Persistence Storage
//  PID MENU

#define PS_SPACE   0  //	reserved
#define PS_PidBase PS_SPACE

#define PS_AddrOfPidSetting(i) (PS_PidBase+i)

#define PS_kP      1  // 	kP
#define PS_kI      2  //	kI
#define PS_kD      3  //     kD
#define PS_SampleTime      4  //     SampleTime
#define PS_WindowSize      5  //     WindowSize
#define PS_BoilHeat      6    //   Boil Heat %
#define PS_Offset     7      // Offset
#define PS_PID_Start   8      // star PID
#define PS_PID_DoughIn  9      // PID DoughIn

//  UNIT MENU
#define PS_TempUnit   10     //  Scale Temp
#define PS_UnitBase PS_TempUnit
#define PS_AddrOfUnitSetting(i) (PS_UnitBase+i)
//#define PS_SensorType     11      // Sensor Type
#define PS_NoDelayStart  11      // delaystart
#define PS_BoilTemp     12       //Temp Boil °C
//     13       Temp Boil °F
#define PS_PumpCycle     14  //     Time Pump Cycle
#define PS_PumpRest     15   //    Time Pump Rest
#define PS_PumpPreMash     16  //     Pump PreMash
#define PS_PumpOnMash     17   //    Pump on Mash
#define PS_PumpOnMashOut     18  //     Pump on MashOut
#define PS_PumpOnBoil      19     //  Pump on Boil
#define PS_TempPumpRest     20    //   Temp Pump Rest °C
//     21       Temp Pump Rest °F
#define PS_PidPipe     22     //  PID Pipe
#define PS_SkipAddMalt     23  //     Skip Add Malt
#define PS_SkipRemoveMalt     24  //     Skip Remove Malt
#define PS_SkipIodineTest     25    //   Skip Iodine Test
#define PS_IodineTime     26   //    Iodine Time
#define PS_Whirlpool     27     //  Whirlpool
//     28 -  31 foot print

#define PS_SpargeWaterEnableAddress    32
#define PS_SpargeWaterTemperatureControlAddress    33
#define PS_SpargeWaterSensorIndexAddress 34
#define PS_SpargeWaterTemperatureAddress	35
#define PS_SpargeWaterTemperatureDifferenceAddress	36

//     37 -  41 reserved

#define PS_ButtonFeedback    42
#define PS_PumpPrimeCount    43
#define PS_PumpPrimeOnTime   44
#define PS_PumpPrimeOffTime  45

//    46 -  49 reserved
#define PS_PreMashHeating    50
#define PS_MashingHeating    51
#define PS_BoilingHeating    52
#define PS_PostBoilHeating   53
// space
#define PS_kP_Secondary      55  // 	kP
#define PS_kI_Secondary      56  //	kI
#define PS_kD_Secondary      57  //     kD

#define PS_kP_AllOn      58  // 	kP
#define PS_kI_AllOn      59  //	kI
#define PS_kD_AllOn      60  //     kD
// space 


#define PS_Distill_Base      64  //     7 fields
// 64 ~ 70

//[94-98,99-103] Sensor index for stages, total 6  +1 stages
// Idle, Manual, PreMash, Mashing, Boiling, Cooling, DISTILL
#define PS_SensorUseBase    72
#define PS_SensorUseAddressOf(i)    (PS_SensorUseBase+(i))

#define PS_SensorAuxBase    80
#define PS_AuxSensorAddressOf(i)    (PS_SensorAuxBase+(i))
// 80-86

//[90- 129] [104-143] sensor addresses
#define PS_SensorAddressBase    112
#define PS_SensorAddressOf(i) ((i)*8 + PS_SensorAddressBase)

//[130-135] multi-sensor calibration
#define PS_SensorCalibrationAddressBase    130
#define CalibrationAddressOf(i) ((i) + PS_SensorCalibrationAddressBase)

#define EEPROM_SIZE 140

const unsigned char  DEFAULT_EEPROM[] PROGMEM={
0, //#define PS_UseGas   0  //	space
200, //#define PS_kP      1  // 	kP
200, //#define PS_kI      2  //	kI
200, //#define PS_kD      3  //     kD
6, //  * 250 #define PS_SampleTime      4  //     SampleTime
20, //#define PS_WindowSize      5  //     WindowSize
85, //#define PS_BoilHeat      6    //   Boil Heat %
50, //#define PS_Offset     7      // Offset
30, //#define PID Start     8   //    Hysteresi
0, //      9       [ SPACE ]
0, //#define PS_TempUnit   10     //  Scale Temp
0,  //#define NoDelay Start     11      //
99, //#define PS_BoilTemp     12       //Temp Boil °C
0, //     13       Temp Boil °F
15, //#define PS_PumpCycle     14  //     Time Pump Cycle
0, //#define PS_PumpRest     15   //    Time Pump Rest
0, //#define PS_PumpPreMash     16  //     Pump PreMash
1, //#define PS_PumpOnMash     17   //    Pump on Mash
1, //#define PS_PumpOnMashOut     18  //     Pump on MashOut
0, //#define PS_PumpOnBoil      19     //  Pump on Boil
105, //#define PS_TempPumpRest     20    //   Temp Pump Rest °C
0, //     21       Temp Pump Rest °F
0, //#define PS_PidPipe     22     //  PID Pipe
0, //#define PS_SkipAddMalt     23  //     Skip Add Malt
0, //#define PS_SkipRemoveMalt     24  //     Skip Remove Malt
1, //#define PS_SkipIodineTest     25    //   Skip Iodine Test
0, //#define PS_IodineTime     26   //    Iodine Time
0, //#define PS_Whirlpool     27     //  Whirlpool
//     28 -  31 [ SPACE ]
'B','M','E','X',
/*#define PS_SpargeWaterEnableAddress    32
#define PS_SpargeWaterTemperatureControlAddress    33
#define PS_SpargeWaterSensorIndexAddress 34
#define PS_SpargeWaterTemperatureAddress	35
#define PS_SpargeWaterTemperatureDifferenceAddress	36 */
0,0,1,78,1,
//     37 -  41 reserved
0,0,0,0,0,
1, //#define PS_ButtonFeedback    42
5, //#define PS_PumpPrimeCount    43
10, //#define PS_PumpPrimeOnTime   44
4, //#define PS_PumpPrimeOffTime  45
0,0,0,0, //    46 -  49 reserved
1, //#define PS_PreMashHeating    50
1, //#define PS_MashingHeating    51
1, //#define PS_BoilingHeating    52
1, //#define PS_PostBoilHeating   53
0,// space 54
200,//#define PS_kP_Secondary      55  // 	kP
200,//#define PS_kI_Secondary      56  //	kI
200,//#define PS_kD_Secondary      57  //     kD

200,//#define PS_kP_AllOn      58  // 	kP
200,//#define PS_kI_AllOn      59  //	kI
200,//#define PS_kD_AllOn      60  //     kD
0,0,0, // 
50,70,100,55,83,90,100 //PS_Distill_Base
};
#include "SpiffsEeprom.h"

void commitSetting(void)
{
	SpiEEPROM.commit();
}

byte readSetting(int addr)
{
	return SpiEEPROM.read(addr);
}

bool updateSetting(int addr,byte value)
{
	return SpiEEPROM.write(addr,value);
}

word readSettingWord(int addr)
{
	return word(SpiEEPROM.read(addr),SpiEEPROM.read(addr+1));
}

word updateSettingWord(int addr,word value)
{
  	SpiEEPROM.write(addr,highByte(value));
  	SpiEEPROM.write((addr+1),lowByte(value));
}

void EepromInit(void)
{
	SpiEEPROM.begin(EEPROM_SIZE);

	if(!(SpiEEPROM.read(29)=='M'
	 		&& SpiEEPROM.read(30)=='E'
	 		&& SpiEEPROM.read(31)=='X')){

//		Serial.printf("re-initialized EEPROM data\n");

		for(byte i=0;i<sizeof(DEFAULT_EEPROM);i++){
			SpiEEPROM.write(i,pgm_read_byte_near(& DEFAULT_EEPROM[i]));
		}
	}

}

#endif
