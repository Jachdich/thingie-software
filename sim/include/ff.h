#ifndef __FF_H
#define __FF_H

typedef struct {
} hw_spi;

typedef struct {
  hw_spi hw_inst;
  int sck_gpio;
  int mosi_gpio;
  int miso_gpio;
  int baud_rate;

} spi_t;

typedef struct {
  spi_t spi;
  int ss_gpio;
} sd_spi_if_t;

typedef struct sd_card_t {
  int type;
  sd_spi_if_t spi_if_p;
  
} sd_card_t;

typedef int FATFS;
typedef int FRESULT;
#define FR_OK 0
typedef int FIL;

#define spi0 (hw_spi){}
#define SD_IF_SPI 0
#define FA_OPEN_APPEND 0
#define FA_WRITE 0
#define FR_EXIST 0

#endif
