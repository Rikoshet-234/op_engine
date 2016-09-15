#pragma once
//#define SUN_DIR_DEBUG //вывод сообщений о положении солнца
//#define WEATHER_LOGGING //вывод сообщений о изменениях погоды
//#define MORE_SPAM // нудный спам
//#define MORE_VERIFY_PATH_MANAGER // доп проверки 
//#define MORE_VERIFY_PROP_COND //доп проверки на евалуаторы и иже с ними
//#define DEBUG_SCHEDULER //отладка шедулера
//#define DEBUG_SCHEDULER2 //отладка шедулера дополнительная :)

#define DEBUG_RESTRICTORS // вывод отладочной информации о битых рестрикторах --alpet restrictor_type==2
//#define VERIFY_RESTRICTORS //всякие проверки для рестрикторов
#define IGNORE_CRITICAL_ECONDITIONS
//#define CREATE_MDUMP //создавать дамп памяти
//#define SHOW_INCORRECT_r_tgt 


//#define PATCH_INFO_PRESENT

#define ENGINE_DESCRIPTION "OP 2.1 Engine"
#define ENGINE_MINOR "0"
#define ENGINE_MAJOR "51f"

#ifdef DEBUG
	#define ENGINE_BUILD_TYPE "debug"
#else
	#define	ENGINE_BUILD_TYPE "release"
#endif

#define PATCH_DESCRIPTION " patch"
#define PATCH_MINOR "0"
#define PATCH_MAJOR "5" 

