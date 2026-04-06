#ifndef INC_STM_BME280_H_
#define INC_STM_BME280_H_

#include "stm32f3xx_hal.h"

#define BME280_ADDR (0x76 << 1)
#define CHIP_ID_BME 0x60
#define HUM_LSB_REG_ADDR 0xFE
#define HUM_MSB_REG_ADR 0xFD
#define TEMP_XLSB_REG_ADDR 0xFC
#define TEMP_LSB_REG_ADDR 0xFB
#define TEMP_MSB_REG_ADDR 0xFA
#define RAWDATA_BASEADDR 0xF7
#define CONFIG_REG_ADDR 0xF5
#define CTRL_MEAS_REG_ADDR 0xF4
#define STATUS_REG_ADDR 0xF3
#define RESET_REG_ADDR 0xE0
#define CHIP_ID_REG_ADDR 0xD0
#define CTRL_HUM_REG_ADDR 0xF2
#define CALIB_DATA00_25_BASEADDR 0x88
#define CALIB_DATA26_41_BASEADDR 0xE1

#define SPI3_W_DISABLE 0x0
#define SPI3_W_ENABLE 0x1

#define OVERSAMPLING_OFF 0x0
#define OVERSAMPLING_1 0x1
#define OVERSAMPLING_2 0x2
#define OVERSAMPLING_4 0x3
#define OVERSAMPLING_8 0x4
#define OVERSAMPLING_16 0x5

#define BME280_SLEEP_MODE 0x0
#define BME280_FORCED_MODE 0x1
#define BME280_NORMAL_MODE 0x3

#define STANDBY_0_5 0
#define STANDBY_62_5 1
#define STANDBY_125	2
#define STANDBY_250	3
#define STANDBY_500	4
#define STANDBY_1000 5
#define STANDBY_10 6
#define STANDBY_20 7

#define FILTER_OFF 0x0
#define FILTER_2 0x1
#define FILTER_4 0x2
#define FILTER_8 0x3
#define FILTER_16 0x4

#define BME280_STATUS_IM_UPDATE (1U << 0)
#define BME280_STATUS_MEASURING (1U << 3)


typedef struct{
	int32_t raw_temp;
	int32_t raw_hum;
}Raw_Data_t;


typedef struct {
    float Temperature;
    float Humidity;
}BME280_Data_t;


typedef struct{
	I2C_HandleTypeDef *hi2c;
	uint8_t OverSampling_T;
	uint8_t OverSampling_H;
	uint8_t Mode;
	uint8_t T_StandBy;
	uint8_t Filter;
	uint8_t SPI_EnOrDıs;
}BME280_Init_t;


int BME280_Reset(uint32_t timeout_d);
int BME280_VerifyChipId(uint32_t timeout_d);
int BME280_SleepMode(void);
int BME280_Init(BME280_Init_t *BME280);
int BME280_Configuration(uint32_t timeout_d);
int BME280_RawData(Raw_Data_t *data, uint32_t timeout_d);
int32_t BME280_Temp_Fine(int32_t raw_t);
uint32_t BME280_Hum(int32_t raw_h, int32_t t_fine);
int BME280_Data(BME280_Data_t *result);


#endif /* INC_STM_BME280_H_ */
