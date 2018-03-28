#ifndef _ZTOOLS_PUBLIB_LIBCRC_CRC_H_
#define _ZTOOLS_PUBLIB_LIBCRC_CRC_H_

unsigned char update_crc8(unsigned char CurCRC, const char * Buf, size_t BufSize);
unsigned char std_crc8(const char *data, size_t lens);

unsigned short update_crc16(unsigned short CurCRC,const char* Buf,size_t BufSize);
unsigned short std_crc16(const char *data,size_t lens);

unsigned long update_crc32(unsigned long CurCRC,const char* Buf,size_t BufSize);
unsigned long std_crc32(const char *data,size_t lens);

unsigned long long update_crc64(unsigned long long CurCRC,const char* Buf,size_t BufSize);
unsigned long long std_crc64(const char *data,size_t lens);

#endif
