#include "Utilities.h"

PLAYBACK_TIME Utilities::toPlaybackTimeStruct(uint32_t totalMillis){
  PLAYBACK_TIME ret;
  ret.totalMillisElapsedPlayback = totalMillis;
  ret.minutesElapsedPlayback = totalMillis / 60000;
  uint32_t temp = totalMillis - ret.minutesElapsedPlayback*60000;
  ret.secondsElapsedPlayback = temp / 1000;
  ret.millisElapsedPlayback = temp - ret.secondsElapsedPlayback*1000;
  return ret;
}

String Utilities::playBackTimeToString(PLAYBACK_TIME t){
  String min = String(t.minutesElapsedPlayback);
  String sec = String(t.secondsElapsedPlayback);
  String millis = String(t.millisElapsedPlayback);
  String ret = min;
  ret.concat(":");
  if(sec.length() == 1) ret.concat("0");
  ret.concat(sec);
  ret.concat(":");
  if(millis.length() == 1) ret.concat("00");
  else if(millis.length() == 2) ret.concat("0");
  ret.concat(millis);
  return ret;
}

void Utilities::enterErrorState(){
  pinMode(ERROR_LED, OUTPUT);
  xTaskCreatePinnedToCore(
    errorTask, "ErrorTask", 2048, NULL, 5,
    NULL, 1);
}

void Utilities::errorTask(void *funcParams){
  Serial.println("There has been a fatal error!");
  for(;;){
    delay(1000);
    digitalWrite(ERROR_LED, 1);
    delay(100);
    digitalWrite(ERROR_LED, 0);
  }
}

void Utilities::debug(const char* x, ... ) {
  if(!GLOBAL_DEBUG) return;
  va_list args;
  va_start(args, x);
  ESP_LOGI("[D]", x, args);
  va_end(args );
}

void Utilities::debug(const char* x, va_list args) {
  if(!GLOBAL_DEBUG) return;
  ESP_LOGI("[D]", x, args);
}

void Utilities::error(const char* x, ... ) {
  va_list args;
  //fprintf(stderr, "AF: ");
  va_start(args, x);
  ESP_LOGI("[E]", x, args);
  va_end(args );
  enterErrorState();
}

void Utilities::error(const char* x, va_list args) {
  ESP_LOGI("[E]", x, args);
  enterErrorState();
}
