
#include"drivespiadxl.h"


void SPI_gpio(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	//_______________SPI______________________________________________
	// GPIOB14 - SDO (SPI2_MISO)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// GPIOB15 - SDA (SPI2_MOSI)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// GPIOB13 - SCL (SPI2_SCK)
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// GPIOB12 - CS (Chip Select)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}
void SPI_config(void){
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_AFIO, ENABLE );
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Direction= SPI_Direction_2Lines_FullDuplex;//Comunicação Bidirecional
	SPI_InitStructure.SPI_Mode= SPI_Mode_Master;// Master(3SO) -> Slave(Sensor)
	SPI_InitStructure.SPI_DataSize= SPI_DataSize_8b;// 8bits
	SPI_InitStructure.SPI_CPOL= SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA= SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS= SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler= SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_FirstBit= SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial= 7;

	SPI_Init(SPI2, &SPI_InitStructure);
	SPI_Cmd(SPI2, ENABLE);
}

void ADXL345(void){
	SPI_write(0x2d, 0x00);  //Reset registo 0x2d
	SPI_write(0x31, 0x00);  //Reset registo 0x31
	SPI_write(0x2d, 0x08);  //Measure Mode ON e Wake Up a 8Hz
	SPI_write(0x31, 0x00);  //Range +/-16g
}

void SPI_write(uint8_t address, uint8_t data){
  /*1-pull the CS low to enable the slave
	2-transmit the address to which we want to write data
	3-transmit the data
	4-pull the CS pin high to disable the slave*/
  GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);


  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)== RESET);
  SPI_I2S_SendData(SPI2,address);
  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)== RESET);
  SPI_I2S_ReceiveData(SPI2);

  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)== RESET);
  SPI_I2S_SendData(SPI2,data);
  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)== RESET);
  SPI_I2S_ReceiveData(SPI2);

  GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_SET);
}

uint8_t SPI_read(uint8_t address){
  /*1-pull the CS low to enable the slave
	2-transmit the address from where we want to read data
	3-receive data.
	4-pull the CS pin high to disable the slave */
  GPIO_WriteBit(GPIOB,GPIO_Pin_12,Bit_RESET);

  address |= 0x80;  // read operation
  /*That address is OR with 0x80. That’s because according to ADXL datasheet, If we want to read data, we need to set the last bit HIGH*/

  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)== RESET);
  SPI_I2S_SendData(SPI2,address);
  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)== RESET);
  SPI_I2S_ReceiveData(SPI2);
  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_TXE)== RESET);
  SPI_I2S_SendData(SPI2,0x00);
  while(SPI_I2S_GetFlagStatus(SPI2,SPI_I2S_FLAG_RXNE)== RESET);



  return SPI_I2S_ReceiveData(SPI2);
}
