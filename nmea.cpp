#include "nmea.h"

int nmea_time_set(char* src, int cbSrc) {// <= входящее от UART
	char b[128]; int cb;
	char* la; char la_s; char* lo; char lo_s; char* time; char* date; bool statusValid;
    char* src0 = src; char* srcOver = src+cbSrc;
    while (src<srcOver) {
      char c = *src++; if (cb>=sizeof(b)-1) return -1;
      if (c<' ') continue;
      if (!cb && c!='$') {return -1; continue;}
      if (c=='$') return -1;
      b[cb++] = c;
      if (cb<12 || b[cb-3]!='*') continue; b[cb] = 0;
      // проверка контрольной суммы
      U32 c1; if (!sscanf(b+cb-2, "%02x", &c1)) {return -1;}
      U32 c2 = 0; for (int i=1; i<cb-3; i++) c2^=U8(b[i]); if (c1!=c2) {return -1;}
      //print("NMEA LINE: [%s]", b);
      // проверка типа сообщения
      if (!!memcmp(b+3, "RMC", 3)) {return -1;}
      int i = 0; char* s = b+7; char* sOver = b+cb-3;//print("NMEA: [%s]", b);
      while (s<sOver) {
        char* t = s; while (t<sOver && *t!=',') t++; *t++ = 0;
        switch (i) {
          case 0 : time = s; break;                // 82400.00
          case 1 : statusValid = (*s=='A'); break; // A
          case 2 : la = s; break;                  // 5552.25725
          case 3 : la_s = *s; break;               // N
          case 4 : lo = s; break;                  // 03737.03485
          case 5 : lo_s = *s; break;               // E
          case 8 : date = s; break;                // 180821
        }
        i++; s = t;
      }
      if (i<9) {return -1;}
    }
	set_utc_from_datetime(date, time);
    return src-src0;
  }