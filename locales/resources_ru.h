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
StringConst(HS_Session_Number,"HS cecc\xB8\xC7");//HS Session
StringConst(HS_Start,"C\xBF""ap\xBF""HS");//Start HS
StringConst(HS_Keep,"\xE0""ep\xB6""a\xBF\xC4""HS");//Keep HS
StringConst(PBH_x,"X\xBC""e\xBB\xC4 \xA8""K #");//PBH # X\xBC""e\xBB\xC4 #

StringConst(min,"\xBC\xB8\xBD");//min

StringConst( welcome,VERSION_INFORMATION);

StringConst( Initialization,"Coe\xE3\xB8\xBD""e\xBD\xB8""e..");//Connecting..
StringConst( SetupNetwork,"Hac\xBF""po\xB9\xBA""a ce\xBF\xB8");//Setup Network

#if NoDelayStart == false
StringConst(To_be_started_in,"\xA4""a\xBE""yc\xBA \xC0""epe\xB7"); //To be started in
StringConst( Setting_Delay,"Ta\xB9\xBC""ep \xB7""a\xBE""yc\xBA""a"); //Setting Delay
StringConst( Delay_Start,"O\xBF\xBB""o\xB6\xB8\xBF\xC4"" \xB7""a\xBE""yc\xBA""?"); //Delay Start?
#endif

StringConst( No_Delay_Start,"C\xBF""ap\xBF\xA0""e\xB7\xA4""a\xE3""ep\xB6");//No Delay Start

StringConst( Resume_Process,"\xA8""po\xE3""o\xBB\xB6\xB8\xBF\xC4?"); //Resume Process?

StringConst( In_Pause,"HA \xA8""A\xA9\xA4""E"); //"In Pause"
StringConst( IODINE_TEST,"\xA6""O\xE3""H\xAE\xA6  TECT"); //IODINE  TEST
StringConst( AutomaticMode, "ABTOMAT. PE\xA3\xA5""M"); //"AUTOMATIC MODE"
StringConst( Pump_Prime, "\xA8""po\xBA""a\xC0\xBA""a"); //Pump Prime
StringConst( Auto_Label, "ABTO -->"); //"AUTO -->"

#if MaximumNumberOfSensors > 1
// less than 11 chars, avoid overwrite 2nd temp display
StringConst( TemperatureReached,"Te\xBC\xBE"". \xE3""oc\xBF\xB8\xB4\xBD""y\xBF""a"); //Temp.  Reached
StringConst(Add_Malt, "\xE0""o\xB2""a\xB3\xC4\xBF""e co\xBB""o\xE3"); //Add Malt
StringConst(Remove_Malt, "\xA5\xB7\xB3\xBB""e\xBA\xB8\xBF""e  co\xBB""o\xE3"); //"Remove Malt"

#if EnableExtendedMashStep
StringConst(Skip_Or_Extend,"Pac\xC1\xB8""p/\xA8""po\xBE""yc\xBF\xB8\xBF\xC4?"); //Ext./Skip?
#else
StringConst(Go_to_next_step,"K c\xBB""e\xE3""y\xC6\xE6""e\xBC""y \xC1""a\xB4""y?"); //Next step?
#endif

#else
StringConst( TemperatureReached,"Te\xBC\xBE"". \xE3""oc\xBF\xB8\xB4\xBD""y\xBF""a"); //Temp.  Reached
StringConst(Add_Malt, "\xE0""o\xB2""a\xB3\xC4\xBF""e co\xBB""o\xE3"); //Add Malt
StringConst(Remove_Malt, "\xA5\xB7\xB3\xBB""e\xBA\xB8\xBF""e  co\xBB""o\xE3"); //"Remove Malt"

#if EnableExtendedMashStep
StringConst(Skip_Or_Extend,"Pac\xC1\xB8""p/\xA8""po\xBE""yc\xBF\xB8\xBF\xC4?"); //Extend, or Skip?
#else
StringConst(Go_to_next_step,"K c\xBB""e\xE3""y\xC6\xE6""e\xBC""y \xC1""a\xB4""y?"); //Go to Next step?
#endif

#endif

StringConst(START_COOLING,"Ox\xBB""a\xB6\xE3""e\xBD\xB8""e"); //START  COOLING
StringConst(WHIRLPOOL,"B\xA5""P\xA8\xA9\xA7"); //WHIRLPOOL
StringConst(Timeing_Whirlpool,"Timing Whirlpool");

StringConst(Brewing_Process,"\xA8""PO\xE1""ECC  \xA8\xA5""BOBAPEH\xA5\xB1"); //Brewing Process
StringConst(Finished,"\xA4""ABEP\xAC\xA2""H !"); //Finished

//button labels

#if NoDelayStart == false
DefineButtonLabel( x_x_Quit_Go,       "        B\xAE""XO\xE0 \xA8\xA9""CK",0); //Quit  Go
#endif
DefineButtonLabel( x_x_x_Ok,          "---  ---  ---   OK",1);
DefineButtonLabel( x_x_Edit_Ok,       "---  ---  \xA5\xA4""MEH OK",2); //---  ---  Edit  OK
DefineButtonLabel( Up_Down_Next_x,    " \xD9    \xDA""  >>>  ---",3); //UP   DWN   >>  ---
DefineButtonLabel( Up_Down_Done_x,    " \xD9    \xDA""   OK  ---",4); //UP   DWN  Done ---

DefineButtonLabel( No_Yes,            "           HET  \xE0""A",5);
#if SupportDistilling
DefineButtonLabel( Manual_Auto_Setup, "\xE0""CT P\xA9\xAB"" ABTO HACTP",6); //"DST MAN AUTO SETUP"
#else
DefineButtonLabel( Manual_Auto_Setup, "--- P\xA9\xAB"" ABTO HACTP",6); //"--- MAN AUTO SETUP"
#endif

#if UsePaddleInsteadOfPump
DefineButtonLabel( _Pump_Rest_,       " -  Stir  Rest  - ",7);
#else
DefineButtonLabel( _Pump_Rest_,       " - HACOC  OT\xE0\xAE""X - ",7); // -  Pump  Rest  -
#endif


#if UsePaddleInsteadOfPump
DefineButtonLabel( Continue_Yes_Pmp,    "Continue: Yes Stir",8);
#else
DefineButtonLabel( Continue_Yes_Pmp,    "\xA8""po\xE3""o\xBB\xB6\xB8\xBF\xC4"": \xE0""A ---",8); //Continue: Yes  Pmp
#endif

DefineButtonLabel( Continue_Yes_No,   "\xA8""po\xE3""o\xBB\xB6\xB8\xBF\xC4"": \xE0""A HET",9); //"Continue: Yes  No"

#if UsePaddleInsteadOfPump
DefineButtonLabel( x_x_x_Pmp,         "--* *--  ---  Stir",10);
DefineButtonLabel( x_x_Time_Pmp,      "--* *--  Tme  Stir",11);
DefineButtonLabel( Up_Down_x_Pmp,     "UP* *DWN  --- Stir",12);
#else
DefineButtonLabel( x_x_x_Pmp,         "--* *--  ---  HAC",10); //--* *--  ---  Pmp
DefineButtonLabel( x_x_Time_Pmp,      "---* *--- BPEM/xB1 HAC",11); //--* *--  Tme  Pmp
DefineButtonLabel( Up_Down_x_Pmp,      " \xD9""* *\xDA    ---  HAC",12); //"UP* *DWN  ---  Pmp"
#endif

DefineButtonLabel( Up_Down_x_x,       " \xD9""* *\xDA    ---  ---",13); //"UP* *DWN  ---  ---"
DefineButtonLabel( x_x_Exit_x,        "---  --- B\xAE""XO\xE0"" ---",14); //"---  ---  Exit ---"
DefineButtonLabel( x_x_Ok_x,          "---  ---   OK  ---",15);

#if UsePaddleInsteadOfPump
DefineButtonLabel( Up_Down_END_Pmp,   "UP* *DWN  END Stir",16);
DefineButtonLabel( Up_Down_Heat_Pmp,  "UP* *DWN Heat Stir",17);
#else
DefineButtonLabel( Up_Down_END_Pmp,   " \xD9""* *\xDA  ""OCTAHOB HAC",16); //"UP* *DWN  END  Pmp"
DefineButtonLabel( Up_Down_Heat_Pmp,  " \xD9""*  *\xDA  T\xAF""H HACOC",17); //"UP* *DWN Heat Pmp"
#endif

DefineButtonLabel( Up_Down_Pause_STP, " \xD9""* *\xDA   \xA8""A\xA9\xA4""A \xAC""A\xA1",18); //"UP* *DWN Pause STP"

DefineButtonLabel( Up_Down_PmPus_STP, " \xD9""* *\xDA   \xA8""A\xA9\xA4""A \xAC""A\xA1",19); //"UP* *DWN PmPus STP"

DefineButtonLabel( Up_Down_Pause_x,   " \xD9""* *\xDA   \xA8""A\xA9\xA4""A ---",20); //"UP* *DWN Pause ---"
DefineButtonLabel( Up_Down_Skip_Ok,   " \xD9""*  *\xDA""   >>>   OK",21); //"UP* *DWN Skip   Ok"
DefineButtonLabel( Up_Down_x_Ok,      " \xD9""*  *\xDA""   ---   OK",22); //"UP* *DWN  ---   Ok"

DefineButtonLabel( Up_Down_Quit_Ok,   " \xD9""    \xDA""  B\xAE""XO\xE0""  OK",23); //"Up   Dwn  Quit  Ok"
DefineButtonLabel( x_Down_Quit_Ok,    "---   \xDA""  B\xAE""XO\xE0""  OK",24); //"---  Dwn  Quit  Ok"
DefineButtonLabel( Up_x_Quit_Ok,      " \xD9""   --- B\xAE""XO\xE0""  OK",25); //"Up   ---  Quit  Ok"

#if UsePaddleInsteadOfPump
DefineButtonLabel( Up_Down_Pause_Pmp, "UP* *DWN Pause Stir",26);
DefineButtonLabel( Up_Down_RUN_Pmp,   "UP* *DWN  RUN  Stir",27);
#else
DefineButtonLabel( Up_Down_Pause_Pmp, " \xD9""* *\xDA   \xA8""A\xA9\xA4""A HAC",26); //"UP* *DWN Pause Pmp"
DefineButtonLabel( Up_Down_RUN_Pmp,   " \xD9""* *\xDA  ""\xA8""PO\xE0""O\xA7\xA3"" HAC",27); //"UP* *DWN  RUN  Pmp"
#endif

DefineButtonLabel( Up_Down_End_More,  " \xD9""*  *\xDA \xA1""OTOBO >>>",28); //"UP* *DWN Done More"

DefineButtonLabel( Up_x_x_Ok,  		  " \xD9   ---  ---   OK",29); //Up   ---  ---   Ok // \xD9   ---  ---   OK
DefineButtonLabel( x_Down_x_Ok,   	  "---   \xDA   ---   OK",30);//---  DWN  ---   Ok //---   \xDA   ---   OK

#if UsePaddleInsteadOfPump
DefineButtonLabel( x_x_Exit_Pmp,      "---  --- Exit Stir",31);
#else
DefineButtonLabel( x_x_Exit_Pmp,      "---  --- B\xAE""XO\xE0"" HAC",31); //---  ---  Exit Pmp
#endif

DefineButtonLabel( Tune_PID_No_Yes,   "ABTO\xA8\xA5\xE0""?  HET  \xE0""A",32); //Tune PID?  No  Yes
DefineButtonLabel( Stop_No_Yes,       "\xA4""a\xBA""o\xBD\xC0\xB8\xBF\xC4""?  HET  \xE0""A",33); //Stop?     No  Yes

DefineButtonLabel( AccessPoint_Yes,   " Run AP Mode?  Yes",34);

DefineButtonLabel( Extend_Skip_Back,   "Pac\xC1"" -- \xA8""po\xBE"" Ha\xB7""a\xE3",35);//Extend - Skip Back

#if UsePaddleInsteadOfPump
DefineButtonLabel( Up_Down_Skip_Pmp,   "UP* *DWN Skip Stir",36);
#else
DefineButtonLabel( Up_Down_Skip_Pmp,   "UP* *DWN Skip  Pmp",36);
#endif
DefineButtonLabel( x_x_Mashout_Extend,  "        Ha\xB7""a\xE3"" Pac\xC1",37); //        M.Out  Ext
DefineButtonLabel( DistillConfirm,  "    P\xA9\xAB"" ABTO OTMEH",38);//MAN AUTO CANCL
// end of menu

StringConst( SensorResolution ,"Pa\xB7""pe\xC1""e\xBD\xB8""e t\xEF""C"); //Sensor Res.

StringConst( Manual_Mode ,"P\xA9\xAB""HO\xA6"" PE\xA3\xA5""M"); //MANUAL MODE
StringConst( Water_Added ,"Bo\xE3""a  \xE3""o\xB2""a\xB3\xBB""e\xBD""a?"); //Water  Added?

#if SpargeHeaterSupport == true
StringConst( SpargeWater_Added ,"SpargeWater Added?");
StringConst( Sparge_Heater,"Sparge Heater");
StringConst( Enable,"Sparge Heatine");
StringConst( Temp_Ctrl,"Temp. Ctrl");
StringConst( Sparge_Sensor,"Sparge Sensor");
StringConst( Sparge_Temp,"Sparge Temp");
StringConst( Temp_Diff,"Temp Diff");
#endif

StringConst( PWM_Is, "MO\xE2=   %"); //PWM=   %

// Setting strings
StringConst( Setup, "MEH\xB0  HACTPOEK"); //SETUP MENU
StringConst( PID_PWM, "\xA8\xA5\xE0"" -- MO\xE2"); //PID -- PWM
StringConst( Misc_Setting, "\xA8""PO\xAB\xA5""E HACTPO\xA6""K\xA5"); //Misc Setting

StringConst( Button_Buzz, "\xA4\xB3""y\xBA \xBA\xBD""o\xBE""o\xBA"); //Button Buzz
StringConst( PumpPrime, "\xA8""po\xBA""a\xC0\xBA""a \xBD""acoca"); //PumpPrime
StringConst( PrimeOn, "\xA8""po\xBA""a\xC0\xBA""aBK\xA7"); //Prime On
StringConst( PrimeOff, "\xA8""po\xBA""a\xC0\xBA""aB\xAE""K\xA7"); //Prime Off

StringConst( PID_AUTOTUNE, "ABTOHACTPO\xA6""KA \xA8\xA5\xE0"); //PID Auto Tune

//StringConst( Use, "Use");
//StringConst( Electric, "Electric");
//StringConst( Gas, "Gas");
StringConst( PoM, "PonM");
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
StringConst( kP, "Ko\xBD""c\xBF""a\xBD\xBF""a kP"); //Constant kP
StringConst( kI, "Ko\xBD""c\xBF""a\xBD\xBF""a kI"); //Constant kI
StringConst( kD, "Ko\xBD""c\xBF""a\xBD\xBF""a kD"); //Constant kD
#endif

StringConst( SampleTime, "Bpe\xBC\xC7""B\xC3\xB2""op."); //SampleTime
StringConst( WindowSet_ms, "O\xBA\xBD""oB\xC3\xB2""op\xBA\xB8"); //WindowSet ms
StringConst( Heat_in_Boil, "Mo\xE6\xBD"".\xBA\xB8\xBE""e\xBD\xB8\xC7"); //Heat in Boil

#if MaximumNumberOfSensors > 1
StringConst( Calibration, "Ka\xBB.#"); //Cal.#
#else
StringConst( Calibration, "Ka\xBB\xB8\xB2""p. t\xEF""C"); //Calibration
#endif

//StringConst( Hysteresi, "Hysteresi");
StringConst( Start_PID_In, "C\xBF""ap\xBF \xA8\xA5\xE0"" c"); //Start PID In
StringConst( PID_Dough_In, "\xA8\xA5\xE0""Ha\xA4""a\xB4""py\xB7\xBA""y"); //PID DoughIn

StringConst( Unit_Parameters, "\xA8""APAMETP\xAE"); //Unit Parameters
StringConst( Set_Degree, "Te\xBC\xBE""epa\xBF""ypa"); //Set Degrees
StringConst( Sensor, "Ce\xBD""cop \xBF""e\xBC\xBE"); //Sensor
StringConst( Inside, "\xB3\xBD""y\xBF""p\xB8"); //Inside
StringConst( Outside, "c\xBD""apy\xB6\xB8"); //Outside
StringConst( Temp_Boil, "Te\xBC\xBE \xBA\xB8\xBE""e\xBD""."); //Temp Boil

#if UsePaddleInsteadOfPump
StringConst( Pump_Cycle, "Stir Cycle");
StringConst( Pump_Rest, "Stir Rest");
StringConst( Pump_PreMash, "Stir PreMash");
#else
StringConst( Pump_Cycle, "Hacoc pa\xB2""o\xBF""ae\xBF"); //Pump Cycle
StringConst( Pump_Rest, "Hacoc o\xBF\xE3\xC3""xae\xBF"); //Pump Rest
StringConst( Pump_PreMash, "HacHaHa\xB4""pe\xB3"); //Pump PreMash
#endif
StringConst( On, "\xE3""a"); //On
StringConst( Off, "\xBD""e\xBF"); //Off
#if UsePaddleInsteadOfPump
StringConst( Pump_On_Mash, "Stir on Mash");
StringConst( Pump_Mashout, "Stir Mashout");
StringConst( Pump_On_Boil, "Stir on Boil");
StringConst( Pump_Stop, "Stir Stop");
#else
StringConst( Pump_On_Mash, "HacHa\xA4""a\xBF\xB8""pa\xBD\xB8\xB8"); //Pmp on Mash
StringConst( Pump_Mashout, "HacHaM\xC5\xC1""Ay\xBF"); //Pmp Mashout
StringConst( Pump_On_Boil, "HacHaK\xB8\xBE""e\xBD\xB8""e"); //Pmp on Boil
StringConst( Pump_Stop, "HacTe\xBC\xBE""Oc\xBF"); //Pump Stop
#endif
StringConst( PID_MaltOut, "\xA8\xA5\xE0""Ha\xA9\xE3""a\xBB""e\xBD\xB8""e"); //PID Malt Out
StringConst( Active, "a\xBA\xBF\xB8\xB3"); //Active
StringConst( Passive, "\xBE""acc\xB8\xB3"); //Passive
StringConst( HeatOnPump,"T\xAF""H@Hacoc"); //"Heat@Pump"

StringConst( Skip_Add, "\xA8""po\xBE\xE0""o\xB2""a\xB3\xB8\xBF\xC4"); //Skip Add
StringConst( Yes, "\xE3""a"); //Yes
StringConst( No, "\xBD""e\xBF"); //No
StringConst( Skip_Remove, "\xA8""po\xBE\xA5\xB7\xB3\xBB""e\xC0\xC4"); //Skip Remove
StringConst( Skip_Iodine, "\xA8""po\xBE\xA5""o\xE3""Tec\xBF"); //Skip Iodine
StringConst( IodineTime, "\xA8""po\xE3\xA5""o\xE3""Tec\xBF"); //IodineTime
StringConst( Whirlpool_e, "B\xA5""P\xA8\xA9\xA7"); //Whirlpool
StringConst( Hot, "\xB4""op"); //Hot
StringConst( Cold, "xo\xBB"); // "Cold" and off

StringConst( Set_Automation, "HACTP.ABTOMAT\xA5""K\xA5"); //Set Automation
StringConst( Mash_In, "M\xC5\xC1""-\xB8\xBD"); //Mash In
StringConst( Phytase, "\xAA\xB8\xBF""a\xB7""a"); //Phytase
StringConst( Glucanase, "\xA1\xBB\xC6\xBA""a\xBD""a\xB7""a"); //Glucanase
StringConst( Protease, "\xA8""po\xBF""ea\xB7""a"); //Protease
StringConst( bAmylase, "B-A\xBC\xB8\xBB""a\xB7""a"); //\xE2""Amylase
StringConst( aAmylase1, "A-A\xBC\xB8\xBB""a\xB7""a1"); //\xE0""Amylase1
StringConst( aAmylase2, "A-A\xBC\xB8\xBB""a\xB7""a2"); //\xE0""Amylase2
StringConst( Mash_out, "M\xC5\xC1""-ay\xBF"); //Mash Out
StringConst( Boil, "K\xB8\xBE\xC7\xC0""e\xBD\xB8""e"); //Boil
StringConst( Cooling, "Ox\xBB""a\xB6\xE3""e\xBD\xB8""e"); //Cooling
StringConst( Whirlpool, "B\xB8""p\xBE""y\xBB"); //Whirlpool
StringConst( HopStandChilling,"Chilling");
StringConst( HopStand, "HopStand");
StringConst( Number_Of_Hops, "Ko\xBB""-\xB3""o \xE3""o\xB2"".x\xBC""e\xBB\xC7"); //Number of Hops
StringConst( Hops_Number_leftPara, "x\xBC""e\xBB\xC4 \xBD""o\xBC"); //Hop nmbr#
StringConst( right_Para, ")");

StringConst( Hops_Number_x, "X\xBC""e\xBB\xC4 #"); //Hop #
StringConst( Pbh_Number_x,"PBH #");

StringConst( Manage_Recipes, "Pe\xE5""e\xBE\xBF\xC3:"); //Manage Recipes
StringConst(Mash_x,"\xA8""ay\xB7""a");
StringConst(Pump_Inverse,"Pump Inverted");

#if MaximumNumberOfSensors > 1
StringConst(Sensor_Setting, "HACTPO\xA6""K\xA5 \xE0""AT\xAB\xA5""KOB");//Sensor Setting
StringConst(No_Sensor_Found, "HET \xE0""AT\xAB\xA5""KOB");//No Sensor Found

StringConst(Sensor_PreMash, "\xA1\xBB""a\xB3""HaHa\xB4""pe\xB3");//PreMash Primary
StringConst(Sensor_Mash, 	"\xA1\xBB""a\xB3""Ha\xA8""ay\xB7""ax");//Mash Primary
StringConst(Sensor_MashOut, "\xA1\xBB""a\xB3""HaM\xC5\xC1""Ay\xBF");//MashOut Primary
StringConst(Sensor_Boil,    "\xA1\xBB""a\xB3""HaK\xB8\xBE""e\xBD\xB8""e");//Boil Primary
StringConst(Sensor_PostBoil,    "\xA1\xBB""a\xB3\xA8""oc\xBB""K\xB8\xBE""e\xBD\xB8\xC7");//PostBoil Primary

StringConst(AuxSensor_PreMash, "Bc\xBE\xBC""HaHa\xB4""pe\xB3");//PreMash Aux
StringConst(AuxSensor_Mash,    "Bc\xBE\xBC""Ha\xA8""ay\xB7""ax");//Mash Aux
StringConst(AuxSensor_MashOut, "Bc\xBE\xBC""HaM\xC5\xC1""Ay\xBF");//MashOut Aux
StringConst(AuxSensor_Boil,    "Bc\xBE\xBC""HaK\xB8\xBE""e\xBD\xB8""e");//Boil Aux
StringConst(AuxSensor_PostBoil,    "Bc\xBE\xBC\xA8""oc\xBB""K\xB8\xBE""e\xBD\xB8\xC7");//PostBoil Aux

StringConst(Sensor_Idle, "\xA1\xBB""a\xB3\xBD\xC3\xA6");//Idle Primary
StringConst(AuxSensor_Idle,   "Bc\xBE""o\xBC""o\xB4""a\xBF""e\xBB\xC4\xBD\xC3\xA6");//Idle Aux
StringConst(Sensor_Manual, "\xA1\xBB""a\xB3HaPy\xC0""Pe\xB6\xB8\xBC""e");//Manual Primary
StringConst(AuxSensor_Manual,   "Bc\xBE\xBCHaPy\xC0""Pe\xB6\xB8\xBC""e");//Manual Aux

#endif // MaximumNumberOfSensors > 1

#if SupportDistilling
StringConst(Manual_Distill,"P\xA9\xAB""HA\xB1"" \xE0\xA5""CT\xA5\xA7\xA7\xB1\xE1\xA5\xB1");//Manual Distill
StringConst(Sensor_Distill, "\xA1\xBB""a\xB3""Ha\xE0\xB8""c\xBF\xB8\xBB\xBB");//Distill Primary
StringConst(AuxSensor_Distill,   "Bc\xBE\xBC""Ha\xE0\xB8""c\xBF\xB8\xBB\xBB");//Distill Aux

StringConst(StartTemp,   "Te\xBC\xBE\xA1""o\xBB""o\xB3");//Head Temp
StringConst(HeadPwm,   "Mo\xE6\xA1""o\xBB""o\xB3");//Head PWM
StringConst(HeartTemp,   "Te\xBC\xBE""Te\xBB""a");//Heart Temp
StringConst(HeartPwm,   "Mo\xE6""Te\xBB""a");//Heart PWM
StringConst(TailTemp,   "Te\xBC\xBE""X\xB3""oc\xBF""o\xB3");//Tail Temp
StringConst(TailPwm,   "Mo\xE6""X\xB3""oc\xBF""o\xB3");//Tail PWM
StringConst(EndTemp,   "\xA4""a\xBA""o\xBD\xC0\xB8\xBF\xC4");//End Temp

StringConst(Distill_Recipe,"\xE0\xA5""CT\xA5\xA7\xA7\xB1\xE1\xA5\xB1");//Distill Recipe
StringConst(StartDistilling,"\xA8""po\xE3""o\xBB\xB6\xB8\xBF\xC4?"); //Start Distilling?
StringConst(Distilling,"\xE0\xA5""CT\xA5\xA7\xA7\xB1\xE1\xA5\xB1");//Distilling
StringConst(TurnOnCoolWater,"Ox\xBB""a\xB6\xE3""e\xBD\xB8""e"); //Start Cooling
StringConst(Distill_finished,"\xA8""po\xE5""cc \xB7""a\xB3""ep\xC1""e\xBD");//Distilling Finished

StringConst(DistillTitle,"\xE0\xA5""CT\xA5\xA7\xA7-"); //Distill-
StringConst(Start,"\xA8""pe\xE3\xBD""a\xB4""pe\xB3"); //Preheat
StringConst(Head,"\xA1""o\xBB""o\xB3\xC3"); //Head
StringConst(Heart,"Te\xBB""o"); //Heart
StringConst(Tail,"X\xB3""oc\xBF\xC3"); //Tail
#endif

#if EnableLevelSensor
StringConst(Enable_Level_Sensor,"\xE0""a\xBF\xC0\xB8\xBA ""ypo\xB3\xBD\xC7");//Enable LV Sen
StringConst(Lv_Trig,"Bpe\xBC\xC7""O\xBE""poca ms");//LV Trig. ms
StringConst(Ext_Pump_Rest,"Bpe\xBC\xC7""O\xB6\xB8\xE3""a\xBD\xB8\xC7");//Ext Pump Rest
#endif


#endif //RESOURCES_H
