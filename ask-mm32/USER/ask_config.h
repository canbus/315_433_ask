#ifndef _ASK_CONFIG_H_
#define _ASK_CONFIG_H_

#define _ASK_MIN_BYTE_LEN_              3       //  byte
#define _ASK_MAX_BYTE_LEN_              3       //  byte

//同步高脉冲宽度约为360us，那么CLK频率约为4*90us,低脉冲10.46ms
#define _ASK_BASE_CLOCK_TIME            90  //us
#define _ASK_MIN_NEW_FRAM_DETECT_TIME_  90 * 2    //  us  
#define _ASK_TOLERANCE_                 40      //  %
//#define _ASK_SYNC_HEAD_LEN_             10960   //us 315M
#define _ASK_SYNC_HEAD_LEN_             11219   //us		433M


#if (_ASK_MAX_BYTE_LEN_ < _ASK_MIN_BYTE_LEN_)
#error SELECT CORRECT ASK MIN/MAX BYTE LENGHT
#endif

#endif
