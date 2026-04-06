#include "STM_BME280.h"
#include <stdint.h>

static I2C_HandleTypeDef *bme280_i2c_port;

static uint16_t dig_T1;
static int16_t dig_T2, dig_T3;
static uint8_t dig_H1, dig_H3;
static int16_t dig_H2, dig_H4, dig_H5;
static int8_t dig_H6;


void BME280_Assign_I2C(I2C_HandleTypeDef *hi2c) {
    bme280_i2c_port = hi2c;
}

int BME280_Reset(uint32_t timeout_d){
	uint8_t cmd = 0xB6;

	if (HAL_I2C_Mem_Write(bme280_i2c_port, BME280_ADDR, RESET_REG_ADDR, I2C_MEMADD_SIZE_8BIT, &cmd, 1, timeout_d) != HAL_OK) {
        return -1;
    }

	uint32_t start = HAL_GetTick();
	uint8_t status = 0;

	while (1){
		if (HAL_I2C_Mem_Read(bme280_i2c_port, BME280_ADDR, STATUS_REG_ADDR, I2C_MEMADD_SIZE_8BIT, &status, 1, timeout_d) != HAL_OK) {
            return -1;
        }

		if ((status & BME280_STATUS_IM_UPDATE) == 0){
			if ((status & BME280_STATUS_MEASURING) == 0) return 0;
	    }

	    if ((HAL_GetTick() - start) >= timeout_d) return -2;
	    HAL_Delay(1);
	}
}


int BME280_VerifyChipId(uint32_t timeout_d){
    uint8_t id = 0;
    if (HAL_I2C_Mem_Read(bme280_i2c_port, BME280_ADDR, CHIP_ID_REG_ADDR, I2C_MEMADD_SIZE_8BIT, &id, 1, timeout_d) != HAL_OK) {
        return -1;
    }

    if (id != CHIP_ID_BME) return -2;
    return 0;
}


int BME280_SleepMode(void){
	uint8_t cmd = BME280_SLEEP_MODE;
	if (HAL_I2C_Mem_Write(bme280_i2c_port, BME280_ADDR, CTRL_MEAS_REG_ADDR, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 1000) != HAL_OK) {
        return -1;
    }
    return 0;
}


int BME280_Init(BME280_Init_t *BME280) {
    if (BME280 == NULL) return -1;
    if (bme280_i2c_port == NULL) return -1;

    if (BME280_SleepMode() != 0) return -1;

    uint8_t init = (((BME280->T_StandBy & 0x07) << 5) | ((BME280->Filter & 0x07) << 2) | ((BME280->SPI_EnOrDıs & 0x01) << 0));
    if (HAL_I2C_Mem_Write(bme280_i2c_port, BME280_ADDR, CONFIG_REG_ADDR, I2C_MEMADD_SIZE_8BIT, &init, 1, 1000) != HAL_OK) return -1;

    init = (BME280->OverSampling_H & 0x07);
    if (HAL_I2C_Mem_Write(bme280_i2c_port, BME280_ADDR, CTRL_HUM_REG_ADDR, I2C_MEMADD_SIZE_8BIT, &init, 1, 1000) != HAL_OK) return -1;

    init = ((BME280->OverSampling_T & 0x07) << 5) | (0 << 2) | (BME280->Mode &0x03);
    if (HAL_I2C_Mem_Write(bme280_i2c_port, BME280_ADDR, CTRL_MEAS_REG_ADDR, I2C_MEMADD_SIZE_8BIT, &init, 1, 1000) != HAL_OK) return -1;

    return 0;
}


int BME280_Configuration(uint32_t timeout_d){
	HAL_StatusTypeDef err;
    uint8_t tcal[6];   // 0x88..0x8D
    uint8_t h1;        // 0xA1
    uint8_t hcal[7];   // 0xE1..0xE7

    err = HAL_I2C_Mem_Read(bme280_i2c_port, BME280_ADDR, 0x88, I2C_MEMADD_SIZE_8BIT, tcal, sizeof(tcal), timeout_d);
    if (err != HAL_OK) return -1;

    err = HAL_I2C_Mem_Read(bme280_i2c_port, BME280_ADDR, 0xA1, I2C_MEMADD_SIZE_8BIT, &h1, 1, timeout_d);
    if (err != HAL_OK) return -1;

    err = HAL_I2C_Mem_Read(bme280_i2c_port, BME280_ADDR, 0xE1, I2C_MEMADD_SIZE_8BIT, hcal, sizeof(hcal), timeout_d);
    if (err != HAL_OK) return -1;

    dig_T1 = (uint16_t)((tcal[1] << 8) | tcal[0]);
    dig_T2 = (int16_t)((tcal[3] << 8) | tcal[2]);
    dig_T3 = (int16_t)((tcal[5] << 8) | tcal[4]);
    dig_H1 = h1;
    dig_H2 = (int16_t)((hcal[1] << 8) | hcal[0]);
    dig_H3 = hcal[2];
    dig_H4 = (int16_t)((hcal[3] << 4) | (hcal[4] & 0x0F));
    dig_H5 = (int16_t)((hcal[5] << 4) | (hcal[4] >> 4));
    dig_H6 = (int8_t)hcal[6];

    return 0;
}


int BME280_RawData(Raw_Data_t *data, uint32_t timeout_d){
	HAL_StatusTypeDef err;
	uint8_t raw[8];

	err = HAL_I2C_Mem_Read(bme280_i2c_port, BME280_ADDR, RAWDATA_BASEADDR, I2C_MEMADD_SIZE_8BIT, raw, 8, timeout_d);
	if (err != HAL_OK) return -1;

	//raw[3]
	//uint8_t - (11001100)
	//uint32_t - (00000000 00000000 00000000 11001100)
	//uint32_t << 12 - (00000000 00001100 11000000 00000000)

	//raw[4]
	//uint8_t - (11010001)
	//uint32_t - (00000000 00000000 00000000 11010001)
	//uint32_t << 4 - (00000000 00000000 00001101 00010000)

	//raw[5]
	//uint8_t - (10011001)
	//uint32_t - (00000000 00000000 00000000 10011001)
	//uint32_t >> 4 - (00000000 00000000 00000000 00001001)

	data->raw_temp = ((uint32_t)raw[3]<<12)|((uint32_t)raw[4]<<4)|((uint32_t)raw[5]>>4);
	data->raw_hum = ((uint32_t)raw[6]<<8)|((uint32_t)raw[7]);

	return 0;
}


int32_t BME280_Temp_Fine(int32_t raw_t){
	int32_t data1, data2;

	data1 = ((((raw_t>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11;
	data2 = (((((raw_t>>4) - ((int32_t)dig_T1)) * ((raw_t>>4) - ((int32_t)dig_T1)))>> 12) *((int32_t)dig_T3)) >> 14;
	return data1 + data2;
}


uint32_t BME280_Hum(int32_t raw_h, int32_t t_fine){

    int32_t x;
    x = t_fine - 76800;
    int32_t term1 = (raw_h << 14) - ((int32_t)dig_H4 << 20) - ((int32_t)dig_H5 * x) + 16384;
    term1 >>= 15;
    int32_t h6 = (int32_t)(int8_t)dig_H6;
    int32_t h3 = (int32_t)(uint8_t)dig_H3;
    int32_t term2a = (x * h6) >> 10;
    int32_t term2b = ((x * h3) >> 11) + 32768;
    int32_t term2c = (term2a * term2b) >> 10;
    int32_t term2d = term2c + 2097152;
    int32_t term2 = (term2d * (int32_t)dig_H2 + 8192) >> 14;
    int32_t v = term1 * term2;
    int32_t h1 = (int32_t)(uint8_t)dig_H1;
    int32_t v15 = v >> 15;
    int32_t correction = (((v15 * v15) >> 7) * h1) >> 4;

    v = v - correction;

    if (v < 0) v = 0;
    if (v > 419430400) v = 419430400;

    return (uint32_t)(v >> 12);
}

int BME280_Data(BME280_Data_t *result){
	if (result == NULL) return -2;

	Raw_Data_t raw;
	if (BME280_RawData(&raw, 100) != 0) return -1;

	int32_t t_fine = BME280_Temp_Fine(raw.raw_temp);
	result->Temperature = (((t_fine * 5 + 128) >> 8) / 100.0f);
	result->Humidity = (BME280_Hum(raw.raw_hum, t_fine))/1024.0;

	return 0;
}
