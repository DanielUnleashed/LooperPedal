#ifndef Utilities_h
#define Utilities_h

#include <Arduino.h>
// For eliptic arguments (...) in debug() or error()
#include <stdarg.h>

#define GLOBAL_DEBUG true

#define ERROR_LED 2

struct PLAYBACK_TIME{
  uint32_t totalMillisElapsedPlayback;
  uint8_t minutesElapsedPlayback;
  uint8_t secondsElapsedPlayback;
  uint8_t millisElapsedPlayback;
};

class Utilities{
  public:
    static PLAYBACK_TIME toPlaybackTimeStruct(uint32_t totalMillis);
    static String playBackTimeToString(PLAYBACK_TIME t);
    static String millisToString(uint32_t in);

    static void enterErrorState();
    static void debug(const char* x, ... );
    static void debug(const char* x, va_list args);
    static void error(const char* x, ... );
    static void error(const char* x, va_list args);

  private:
    static void errorTask(void *funcParams);
};

#endif
