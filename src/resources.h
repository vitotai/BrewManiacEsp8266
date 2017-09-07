/**********************************************************************
 BrewManiac
 created by Vito Tai
 Copyright (C) 2015 Vito Tai

 This soft ware is provided as-is. Use at your own risks.
 You are free to modify and distribute this software without removing
 this statement.
 BrewManiac by Vito Tai is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
***********************************************************************/

#ifndef RESOURCES_H
#define RESOURCES_H

#define StringConst(v,c) const char C_##v []  PROGMEM = c
#define STR(v) C_##v

#define DefineButtonLabel(N,S,v) const byte BtnLabelId_##N=v; const char BtnLabel_##N[] PROGMEM = S
#define ButtonLabel(N) BtnLabelId_##N,BtnLabel_##N

#define _makeVERSION_INFORMATION(a) "BrewManiacEx " a
#define _makeVERSION_indirect(v) _makeVERSION_INFORMATION(v)
#define VERSION_INFORMATION  _makeVERSION_indirect(BME8266_VERSION)

typedef const char* str_t;
#define str_t const char*

StringConst(IpAddress,"IP:");
StringConst(HS_Session_Number,"HS Session");
StringConst(HS_Start,"Start HS");
StringConst(HS_Keep,"Keep HS");
StringConst(PBH_x,"PBH #");

StringConst(min,"min");

StringConst( welcome,VERSION_INFORMATION);

StringConst( Initialization,"Connecting..");
StringConst( SetupNetwork,"Setup Network");

#if NoDelayStart == false
StringConst(To_be_started_in,"To be started in");
StringConst( Setting_Delay,"Setting Delay");
StringConst( Delay_Start,"Delay Start?");
#endif

StringConst( No_Delay_Start,"No Delay Start");

StringConst( Resume_Process,"Resume Process?");

StringConst( In_Pause,"In Pause");
StringConst( IODINE_TEST,"IODINE  TEST");
StringConst( AutomaticMode, "AUTOMATIC MODE");
StringConst( Pump_Prime, "Pump Prime");
StringConst( Auto_Label, "AUTO -->");

#if MaximumNumberOfSensors > 1
// less than 11 chars, avoid overwrite 2nd temp display
StringConst( TemperatureReached,"Temp Reach");
StringConst(Add_Malt, "Add Malt");
StringConst(Remove_Malt, "Remove Malt");

#if EnableExtendedMashStep
StringConst(Skip_Or_Extend,"Ext./Skip?");
#else
StringConst(Go_to_next_step,"Next step?");
#endif

#else
StringConst( TemperatureReached,"Temp. Reached!");
StringConst(Add_Malt, "Add Malt");
StringConst(Remove_Malt, "Remove Malt");

#if EnableExtendedMashStep
StringConst(Skip_Or_Extend,"Extend, or Skip?");
#else
StringConst(Go_to_next_step,"Go to Next step?");
#endif

#endif

StringConst(START_COOLING,"START  COOLING");
StringConst(WHIRLPOOL,"WHIRLPOOL");
StringConst(Timeing_Whirlpool,"Timing Whirlpool");

StringConst(Brewing_Process,"Brewing Process");
StringConst(Finished,"Finished");

//button labels

#if NoDelayStart == false
DefineButtonLabel( x_x_Quit_Go,       "          Quit  Go",0);
#endif
DefineButtonLabel( x_x_x_Ok,          "---  ---  ---   Ok",1);
DefineButtonLabel( x_x_Edit_Ok,       "---  ---  Edit  Ok",2);
DefineButtonLabel( Up_Down_Next_x,    "UP   DWN   >>  ---",3);
DefineButtonLabel( Up_Down_Done_x,    "UP   DWN  Done ---",4);

DefineButtonLabel( No_Yes,            "            No Yes",5);
#if SupportDistilling
DefineButtonLabel( Manual_Auto_Setup, "DST MAN AUTO SETUP",6);
#else
DefineButtonLabel( Manual_Auto_Setup, "--- MAN AUTO SETUP",6);
#endif

#if UsePaddleInsteadOfPump
DefineButtonLabel( _Pump_Rest_,       " -  Stir  Rest  - ",7);
#else
DefineButtonLabel( _Pump_Rest_,       " -  Pump  Rest  - ",7);
#endif


#if UsePaddleInsteadOfPump
DefineButtonLabel( Continue_Yes_Pmp,    "Continue: Yes Stir",8);
#else
DefineButtonLabel( Continue_Yes_Pmp,    "Continue: Yes  Pmp",8);
#endif

DefineButtonLabel( Continue_Yes_No,   "Continue: Yes  No",9);

#if UsePaddleInsteadOfPump
DefineButtonLabel( x_x_x_Pmp,         "--* *--  ---  Stir",10);
DefineButtonLabel( x_x_Time_Pmp,      "--* *--  Tme  Stir",11);
DefineButtonLabel( Up_Down_x_Pmp,     "UP* *DWN  --- Stir",12);
#else
DefineButtonLabel( x_x_x_Pmp,         "--* *--  ---  Pmp",10);
DefineButtonLabel( x_x_Time_Pmp,      "--* *--  Tme  Pmp",11);
DefineButtonLabel( Up_Down_x_Pmp,     "UP* *DWN  ---  Pmp",12);
#endif

DefineButtonLabel( Up_Down_x_x,       "UP* *DWN  ---  ---",13);
DefineButtonLabel( x_x_Exit_x,        "---  ---  Exit ---",14);
DefineButtonLabel( x_x_Ok_x,          "---  ---   Ok  ---",15);

#if UsePaddleInsteadOfPump
DefineButtonLabel( Up_Down_END_Pmp,   "UP* *DWN  END Stir",16);
DefineButtonLabel( Up_Down_Heat_Pmp,  "UP* *DWN Heat Stir",17);
#else
DefineButtonLabel( Up_Down_END_Pmp,   "UP* *DWN  END  Pmp",16);
DefineButtonLabel( Up_Down_Heat_Pmp,  "UP* *DWN Heat Pmp",17);
#endif

DefineButtonLabel( Up_Down_Pause_STP, "UP* *DWN Pause STP",18);

DefineButtonLabel( Up_Down_PmPus_STP, "UP* *DWN PmPus STP",19);

DefineButtonLabel( Up_Down_Pause_x,   "UP* *DWN Pause ---",20);
DefineButtonLabel( Up_Down_Skip_Ok,   "UP* *DWN Skip   Ok",21);
DefineButtonLabel( Up_Down_x_Ok,      "UP* *DWN  ---   Ok",22);

DefineButtonLabel( Up_Down_Quit_Ok,   "Up   Dwn  Quit  Ok",23);
DefineButtonLabel( x_Down_Quit_Ok,    "---  Dwn  Quit  Ok",24);
DefineButtonLabel( Up_x_Quit_Ok,      "Up   ---  Quit  Ok",25);

#if UsePaddleInsteadOfPump
DefineButtonLabel( Up_Down_Pause_Pmp, "UP* *DWN Pause Stir",26);
DefineButtonLabel( Up_Down_RUN_Pmp,   "UP* *DWN  RUN  Stir",27);
#else
DefineButtonLabel( Up_Down_Pause_Pmp, "UP* *DWN Pause Pmp",26);
DefineButtonLabel( Up_Down_RUN_Pmp,   "UP* *DWN  RUN  Pmp",27);
#endif

DefineButtonLabel( Up_Down_End_More,  "UP* *DWN Done More",28);

DefineButtonLabel( Up_x_x_Ok,  		  "Up   ---  ---   Ok",29);
DefineButtonLabel( x_Down_x_Ok,   	  "---  Dwn  ---   Ok",30);

#if UsePaddleInsteadOfPump
DefineButtonLabel( x_x_Exit_Pmp,      "---  --- Exit Stir",31);
#else
DefineButtonLabel( x_x_Exit_Pmp,      "---  ---  Exit Pmp",31);
#endif

DefineButtonLabel( Tune_PID_No_Yes,   "Tune PID?  No  Yes",32);
DefineButtonLabel( Stop_No_Yes,       " Stop?     No  Yes",33);

DefineButtonLabel( AccessPoint_Yes,   " Run AP Mode?  Yes",34);

DefineButtonLabel( Extend_Skip_Back,   "Extend - Skip Back",35);

#if UsePaddleInsteadOfPump
DefineButtonLabel( Up_Down_Skip_Pmp,   "UP* *DWN Skip Stir",36);
#else
DefineButtonLabel( Up_Down_Skip_Pmp,   "UP* *DWN Skip  Pmp",36);
#endif
DefineButtonLabel( x_x_Mashout_Extend,  "        M.Out  Ext",37);
DefineButtonLabel( DistillConfirm,  "    MAN AUTO CANCL",38);
// end of menu

StringConst( SensorResolution ,"Sensor Res.");

StringConst( Manual_Mode ,"MANUAL MODE");
StringConst( Water_Added ,"Water  Added?");

#if SpargeHeaterSupport == true
StringConst( SpargeWater_Added ,"SpargeWater Added?");
StringConst( Sparge_Heater,"Sparge Heater");
StringConst( Enable,"Sparge Heatine");
StringConst( Temp_Ctrl,"Temp. Ctrl");
StringConst( Sparge_Sensor,"Sparge Sensor");
StringConst( Sparge_Temp,"Sparge Temp");
StringConst( Temp_Diff,"Temp Diff");
#endif

StringConst( PWM_Is, "PWM=   %");

// Setting strings
StringConst( Setup, "SETUP MENU");
StringConst( PID_PWM, "PID -- PWM");
StringConst( Misc_Setting, "Misc Setting");

StringConst( Button_Buzz, "Button Buzz");
StringConst( PumpPrime, "PumpPrime");
StringConst( PrimeOn, "Prime On");
StringConst( PrimeOff, "Prime Off");

StringConst( PID_AUTOTUNE, "PID Auto Tune");

//StringConst( Use, "Use");
//StringConst( Electric, "Electric");
//StringConst( Gas, "Gas");
#if SecondaryHeaterSupport == true
StringConst( kP_1, "1st Only kP");
StringConst( kI_1, "1st Only kI");
StringConst( kD_1, "1st Only kD");

StringConst( kP_2, "2nd Only kP");
StringConst( kI_2, "2nd Only kI");
StringConst( kD_2, "2nd Only kD");

StringConst( kP_both, "Both: kP");
StringConst( kI_both, "Both: kI");
StringConst( kD_both, "Both: kD");

StringConst(HeaterPreMash, "Heat PreMash");
StringConst(HeaterMashing,"Heat on Mash");
StringConst(HeaterBoiling,"Heat on Boil");
StringConst(HeaterPostBoil,"Heat PostBoil");

StringConst(PrimaryHeater,"#1");
StringConst(SecondaryHeater,"#2");
StringConst(BothHeater,"Both");

#else
StringConst( kP, "Constant kP");
StringConst( kI, "Constant kI");
StringConst( kD, "Constant kD");
#endif

StringConst( SampleTime, "SampleTime");
StringConst( WindowSet_ms, "WindowSet ms");
StringConst( Heat_in_Boil, "Heat in Boil");

#if MaximumNumberOfSensors > 1
StringConst( Calibration, "Cal.#");
#else
StringConst( Calibration, "Calibration");
#endif

//StringConst( Hysteresi, "Hysteresi");
StringConst( Start_PID_In, "Start PID In");
StringConst( PID_Dough_In, "PID DoughIn");

StringConst( Unit_Parameters, "Unit Parameters");
StringConst( Set_Degree, "Set Degrees");
StringConst( Sensor, "Sensor");
StringConst( Inside, "Inside");
StringConst( Outside, "Outside");
StringConst( Temp_Boil, "Temp Boil");

#if UsePaddleInsteadOfPump
StringConst( Pump_Cycle, "Stir Cycle");
StringConst( Pump_Rest, "Stir Rest");
StringConst( Pump_PreMash, "Stir PreMash");
#else
StringConst( Pump_Cycle, "Pump Cycle");
StringConst( Pump_Rest, "Pump Rest");
StringConst( Pump_PreMash, "Pump PreMash");
#endif
StringConst( On, "On");
StringConst( Off, "Off");
#if UsePaddleInsteadOfPump
StringConst( Pump_On_Mash, "Stir on Mash");
StringConst( Pump_Mashout, "Stir Mashout");
StringConst( Pump_On_Boil, "Stir on Boil");
StringConst( Pump_Stop, "Stir Stop");
#else
StringConst( Pump_On_Mash, "Pmp on Mash");
StringConst( Pump_Mashout, "Pmp Mashout");
StringConst( Pump_On_Boil, "Pmp on Boil");
StringConst( Pump_Stop, "Pump Stop");
#endif
StringConst( PID_MaltOut, "PID Malt Out");
StringConst( Active, "Active");
StringConst( Passive, "Passive");

StringConst( Skip_Add, "Skip Add");
StringConst( Yes, "Yes");
StringConst( No, "No");
StringConst( Skip_Remove, "Skip Remove");
StringConst( Skip_Iodine, "Skip Iodine");
StringConst( IodineTime, "IodineTime");
StringConst( Whirlpool_e, "Whirlpool");
StringConst( Hot, "Hot");
StringConst( Cold, "Cold"); // and off

StringConst( Set_Automation, "Set Automation");
StringConst( Mash_In, "Mash In");
StringConst( Phytase, "Phytase");
StringConst( Glucanase, "Glucanase");
StringConst( Protease, "Protease");
StringConst( bAmylase, "\xE2""Amylase");
StringConst( aAmylase1, "\xE0""Amylase1");
StringConst( aAmylase2, "\xE0""Amylase2");
StringConst( Mash_out, "Mash Out");
StringConst( Boil, "Boil");
StringConst( Cooling, "Cooling");
StringConst( Whirlpool, "Whirlpool");
StringConst( HopStandChilling,"Chilling");
StringConst( HopStand, "HopStand");
StringConst( Number_Of_Hops, "Number of Hops");
StringConst( Hops_Number_leftPara, "Hop nmbr#");
StringConst( right_Para, ")");

StringConst( Hops_Number_x, "Hop #");
StringConst( Pbh_Number_x,"PBH #");

StringConst( Manage_Recipes, "Manage Recipes");
StringConst(Mash_x,"Mash ");

#if MaximumNumberOfSensors > 1
StringConst(Sensor_Setting, "Sensor Setting");
StringConst(No_Sensor_Found, "No Sensor Found");

StringConst(Sensor_PreMash, "PreMash Primary");
StringConst(Sensor_Mash, 	"Mash Primary");
StringConst(Sensor_MashOut, "MashOut Primary");
StringConst(Sensor_Boil,    "Boil Primary");
StringConst(Sensor_PostBoil,    "PostBoil Primary");

StringConst(AuxSensor_PreMash, "PreMash Aux");
StringConst(AuxSensor_Mash,    "Mash Aux");
StringConst(AuxSensor_MashOut, "MashOut Aux");
StringConst(AuxSensor_Boil,    "Boil Aux");
StringConst(AuxSensor_PostBoil,    "PostBoil Aux");

StringConst(Sensor_Idle, "Idle Primary");
StringConst(AuxSensor_Idle,   "Idle Aux");
StringConst(Sensor_Manual, "Manual Primary");
StringConst(AuxSensor_Manual,   "Manual Aux");

#endif // MaximumNumberOfSensors > 1

#if SupportDistilling
StringConst(Manual_Distill,"Manual Distill");
StringConst(Sensor_Distill, "Distill Primary");
StringConst(AuxSensor_Distill,   "Distill Aux");

StringConst(StartTemp,   "Head Temp");
StringConst(HeadPwm,   "Head PWM");
StringConst(HeartTemp,   "Heart Temp");
StringConst(HeartPwm,   "Heart PWM");
StringConst(TailTemp,   "Tail Temp");
StringConst(TailPwm,   "Tail PWM");
StringConst(EndTemp,   "End Temp");

StringConst(Distill_Recipe,"Distill Recipe");
StringConst(StartDistilling,"Start Distilling?");
StringConst(Distilling,"Distilling");
StringConst(TurnOnCoolWater,"Start Cooling");
StringConst(Distill_finished,"Distilling Finished");

StringConst(DistillTitle,"Distill-");
StringConst(Start,"Preheat");
StringConst(Head,"Head");
StringConst(Heart,"Heart");
StringConst(Tail,"Tail");
#endif

#endif //RESOURCES_H
