#include "utils.hpp"
#include <limits.h>
#include <stdio.h>

int makeNmeaSentense(char* buffer, int size, const char * sentense, ...)
{
  int ret;
  if (*sentense != '$') return -1;
  va_list arg; 
  va_start(arg, sentense);
  ret = vsnprintf(buffer, size, sentense,arg);
  va_end(arg);
  if(buffer[ret-1] != '*') return -1;
  if(ret+5>size) return -1;
  unsigned char crc=0;
  char *data = buffer+1;
  while (*data && *data!='*') {
    crc ^=*data++;
  }
  ret+=snprintf(&buffer[ret],5,"%02X\r\n",crc);
  return ret;
}


unsigned long time_diff(long unsigned time1, long unsigned time2)
{
  if(time2>=time2)
    return time2-time1;
  else
    return (ULONG_MAX-time1)+time2+1;
}
