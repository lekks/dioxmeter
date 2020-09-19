
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

int calc_ppm(unsigned long period,unsigned long imp, int MAX_PPM)
{
    int dead_time = period/251;
    return MAX_PPM*(imp-dead_time/2)/(period-dead_time);
}