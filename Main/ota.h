#ifndef _OTA_H_
#define _OTA_H_

String reset_reason(int);
String processor(const String&);
void setupAsyncServer();
void notFound(AsyncWebServerRequest*);
String convertFileSize(const size_t);

#endif