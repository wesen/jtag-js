#ifndef UTILS_H__
#define UTILS_H__

#define fill_b4(u) \
{ ((u) & 0xffUL), (((u) & 0xff00UL) >> 8), \
  (((u) & 0xff0000UL) >> 16), (((u) & 0xff000000UL) >> 24) }
#define fill_b2(u) \
{ ((u) & 0xff), (((u) & 0xff00) >> 8) }


unsigned long b4_to_u32(unsigned char *b);
void u32_to_b4(unsigned char *b, unsigned long l);
unsigned short b2_to_u16(unsigned char *b);
void u16_to_b2(unsigned char *b, unsigned short l);

char *byteToHex(uint8_t x, char *buf);
int hex(unsigned char ch);
int hexToInt(char **ptr, int *intValue);
char *mem2hex(uint8_t *mem, char *buf, int count);
uint8_t *hex2mem(char *buf, uint8_t *mem, int count);
uint8_t *bin2mem(char *buf, uint8_t *mem, int count);

#endif /* UTILS_H__ */
