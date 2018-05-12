/*
 * comm.h
 *
 *  Created on: 2015/07/19
 *      Author: Yusuke
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#define BAUDRATE	57600

#define IER_RBR         (0x01<<0)
#define IER_THRE        (0x01<<1)
#define IER_RLS         (0x01<<2)
#define IER_ABEO        (0x01<<8)
#define IER_ABTO        (0x01<<9)

#define IIR_PEND        0x01
#define IIR_RLS         0x03
#define IIR_RDA         0x02
#define IIR_CTI         0x06
#define IIR_THRE        0x01
#define IIR_ABEO        (0x01<<8)
#define IIR_ABTO        (0x01<<9)
#define SSP_BUFSIZE     16
#define SSPSR_TFE       (0x1<<0)
#define SSPSR_TNF       (0x1<<1)
#define SSPSR_RNE       (0x1<<2)
#define SSPSR_RFF       (0x1<<3)
#define SSPSR_BSY       (0x1<<4)

#define  READ_FLAG         0x80
#define  WRITE_FLAG        0x00

#define SERVO_ON()	(LPC_TMR16B0->MR0 = 100)
#define SERVO_OFF()	(LPC_TMR16B0->MR0 = 80)
#define VALVE_OPEN()	(LPC_GPIO2->DATA |= (1 << 6))
#define VALVE_CLOSE()	(LPC_GPIO2->DATA &= ~(1 << 6))	//TODO:待機中は電磁弁閉める操作しないとなーっている

extern void UART_initialize();
extern void UART_SendChar(char data);
extern void UART_SendString(char *str);
extern uint8_t UART_Read();
extern void I2C_Initialization();
extern void I2C_Write(uint8_t address, uint8_t data);
extern void I2C_Read(uint8_t address, uint8_t length, uint8_t *data);
extern void SPI0_Initialization();
extern void SPI0_Read(uint8_t address, uint8_t length, uint8_t *data);
void ReadRegs(uint8_t ReadAddr, uint8_t *ReadBuf, uint16_t Bytes);
uint8_t WriteReg(uint8_t WriteAddr, uint8_t WriteData);
extern void SPI0_Write(uint8_t address, uint8_t data);
extern void SPI0_Send(uint8_t *buf, uint32_t Length);
extern void SPI0_Receive(uint8_t *buf, uint32_t Length);
extern void Servo_initialize();
extern void Valve_initialize();
void TIMER16_0_IRQHandler(void);

#endif /* INTERFACE_H_ */
