#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <util/delay.h>
#define	USCK	PB7
#define	MOSI	PB5
#define	MISO	PB6
#define DEBUG	0
#define TIMER_ON()	TCCR0B |= 0b100;
#define TIMER_OFF()	TCCR0B =0;

char GPRMC[5]={'C','M','R','P','G'};
char stack[5];
char rawdata[20];
int status=0;
int count=0;
int j=0;
unsigned char UTC_time_H[7]={0};
unsigned char UTC_time_L[4]={0};
unsigned char latitude_H[5]={0};
unsigned char latitude_L[5]={0};
unsigned char longitude_H[6]={0};
unsigned char longitude_L[5]={0};

uint8_t command=0;
volatile uint8_t triggered = 0;
int counter =0;

struct GPS{
	unsigned long UTC_time;
	unsigned long latitude;
	unsigned long longitude;
};

struct GPS now;
void USART_initialize();
void USART_Transmit(unsigned char data);
uint8_t	spi_slave_receive(uint8_t send);
void	spi_slave_init();
unsigned long Ctol();
unsigned int Ctoi();


ISR(INT0_vect)
{
		triggered = 1;
}

ISR(TIMER0_OVF_vect)  //�^�C�}���荞��
{
	if (counter==10)
	{
		USART_Transmit('t');
		counter=0;
		TIMER_OFF();
		status=0;
		count=0;
		command=0;
		j=0;
		PORTD|=(1<<PD3);
	}else
	{
		counter++;
	}
}


ISR(USART_RX_vect){
	//�X�^�b�N��UDR����f�[�^���i�[
	for (int i=4;i>0;i--)
	{
		stack[i]=stack[i-1];
	}
	stack[0]=UDR;
	
	//USART_Transmit(stack[0]);
	//�X�^�b�N���g���K�[�����񂩂ǂ����𔻒�(status��ԕϐ���False���̂�)
	if (status!=1)
	{
		int c=0;
		for (int i=0;i<5;i++)
		{
			if (stack[i]==GPRMC[i])
			{
				c++;
			}
		}
		if(c==5){
			status=1;
		}
	}

	//status��ԕϐ���True�̂Ƃ��A�f�[�^�̉�͂��s��
	if (status==1)
	{
		TIMER_ON();
		rawdata[j]=stack[0];
		count++;
		j++;
		//UTC�f�[�^�̎擾
		if (count==14)
		{
			//USART_Transmit(rawdata[13]);
			if (rawdata[13]=='A')
			{
				for (int i=0;i<6;i++)
				{
					UTC_time_H[i]=rawdata[i+2];
				}
				
				for (int i=0;i<3;i++)
				{
					UTC_time_L[i]=rawdata[i+9];
				}
				
				j=0;
			}
			else if (rawdata[13]=='V'){
				for (int i=0;i<6;i++)
				{
					UTC_time_H[i]=rawdata[i+2];
				}
				
				for (int i=0;i<3;i++)
				{
					UTC_time_L[i]=rawdata[i+9];
				}

				now.UTC_time=(Ctol(UTC_time_H,6))*1000;
				now.UTC_time+=Ctol(UTC_time_L,3);
				now.latitude=0;
				now.longitude=0;
								
				PORTD&=~(1<<PD3);

				status=0;
				count=0;
				j=0;

				////debug////////
				#if (DEBUG==1)
				for (int i=0;i<6;i++)
				{
					USART_Transmit(UTC_time_H[i]);
				}
				for (int i=0;i<3;i++)
				{
					USART_Transmit(UTC_time_L[i]);
				}
				USART_Transmit('\n');
				#endif
				#if (DEBUG==2)
				char UTC_time_1 = (now.UTC_time>>24)|(1<<7);
				char UTC_time_2 = now.UTC_time>>16;
				char UTC_time_3 = now.UTC_time>>8;
				char UTC_time_4 = now.UTC_time;
				USART_Transmit(UTC_time_1);
				USART_Transmit(UTC_time_2);
				USART_Transmit(UTC_time_3);
				USART_Transmit(UTC_time_4);
				USART_Transmit('\n');
				#endif
				////end debug////////
				
				
			}
			else{
				status=0;
				count=0;
				j=0;
			}
		}
		//�ܓx�f�[�^�̎擾
		if (count==24)
		{
			for (int i=0;i<4;i++)
			{
				latitude_H[i]=rawdata[i+1];
			}
			for (int i=0;i<4;i++)
			{
				latitude_L[i]=rawdata[i+6];
			}
			j=0;
		}
		//�o�x�f�[�^�̎擾
		if (count==38)
		{
			for (int i=0;i<5;i++)
			{
				longitude_H[i]=rawdata[i+3];
			}
			for (int i=0;i<4;i++)
			{
				longitude_L[i]=rawdata[i+9];
			}
			//�ŏI����
			//�������琔���ւ̕ϊ�
			now.UTC_time=(Ctol(UTC_time_H,6)*1000);
			now.UTC_time+=Ctol(UTC_time_L,3);
			now.latitude=(Ctol(latitude_H,4)*10000);
			now.latitude+=Ctol(latitude_L,4);
			now.longitude=(Ctol(longitude_H,5)*10000);
			now.longitude+=Ctol(longitude_L,4);
			
			//TODO:��肱�݂��悤��
			
			PORTD&=~(1<<PD3);
			
			//��ԕϐ��B�̏�����
			status=0;
			count=0;
			j=0;
			
			////debug////////
			#if (DEBUG==1)
			for (int i=0;i<6;i++)
			{
				USART_Transmit(UTC_time_H[i]);
			}
			USART_Transmit(' ');
			for (int i=0;i<3;i++)
			{
				USART_Transmit(UTC_time_L[i]);
			}
			USART_Transmit(' ');
			for (int i=0;i<4;i++)
			{
				USART_Transmit(latitude_H[i]);
			}
			USART_Transmit(' ');
			for (int i=0;i<4;i++)
			{
				USART_Transmit(latitude_L[i]);
			}
			USART_Transmit(' ');
			for (int i=0;i<5;i++)
			{
				USART_Transmit(longitude_H[i]);
			}
			USART_Transmit(' ');
			for (int i=0;i<4;i++)
			{
				USART_Transmit(longitude_L[i]);
			}
			USART_Transmit('\n');
			#endif
			#if (DEBUG==2)
			char data[12];
			data[0] = now.UTC_time>>24;
			data[1] = now.UTC_time>>16;
			data[2] = now.UTC_time>>8;
			data[3] = now.UTC_time;
			//data[4] = now.latitude>>24;
			//data[5] = now.latitude>>16;
			//data[6] = now.latitude>>8;
		//	data[7] = now.latitude;
		//	data[8] = now.longitude>>24;
		//	data[9] = now.longitude>>16;
		//	data[10] = now.longitude>>8;
		//	data[11] = now.longitude;
			for (int i = 0;i</*12*/3;i++)
			{
				USART_Transmit(data[i]);
			}
			USART_Transmit('\n');
			#endif
			
			////end debug////////
			
		}
	}
}


int main(void)
{
	USART_initialize();
	//�̍����������̂ŕK���}�C�R�����ƂɃf�t�H���g�l���m�F���邱�ƁB�K�����g����8.29MHz�ł���B
	//���������f�t�H���g�l�v���X6���炢���Ǝv����B
	//Fuse bit Low = E4
	OSCCAL=0x59;
	PORTD|=(1<<PD3);
	DDRD|=(1<<PD3);
	spi_slave_init();
	sei();
	USART_Transmit('R');
	while(1)
	{
		while (!triggered)
		{
		}
		triggered = 0;
		
		if (command==0)
		{
			command = spi_slave_receive(0x57);
		}else
		{
			uint8_t data=0x32;
			switch(command){
				case 3:
				data= now.UTC_time>>24;
				break;
				case 4:
				data = now.UTC_time>>16;
				break;
				case 5:
				data= now.UTC_time>>8;
				break;
				case 6:
				data= now.UTC_time;
				break;
				case 7:
				data= now.latitude>>24;
				break;
				case 8:
				data = now.latitude>>16;
				break;
				case 9:
				data = now.latitude>>8;
				break;
				case 10:
				data = now.latitude;
				break;
				case 11:
				data = now.longitude>>24;
				break;
				case 12:
				data = now.longitude>>16;
				break;
				case 13:
				data = now.longitude>>8;
				break;
				case 14:
				data = now.longitude;
				break;
			}
			uint8_t r = spi_slave_receive(data);
			USART_Transmit(command);
			if ((command>=3)&&(command<=13)&&(r==0x03))
			{
				command++;
			}else
			{
				command=0;
			}
		}
	}
}
void USART_Transmit(unsigned char data)
{
	while(!(UCSRA&(1<<UDRE))); /*���M�ޯ̧�󂫑ҋ@ */
	UDR=data; /*�ް����M(���M�J�n)*/
}
void USART_initialize(){
	UBRRH=0;
	UBRRL=8;
	UCSRA|=0b00000010;
	UCSRC=(1<<USBS)|(3<<UCSZ0);
	UCSRB=(1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
}

uint8_t	spi_slave_receive(uint8_t send)
{
	USIDR = send;
	USISR = (1<<USIOIF);
	while (!(USISR & (1<<USIOIF)));
	uint8_t	receive = USIDR;

	return	receive;
}

void	spi_slave_init()
{
	USICR = (1<<USIWM0)|(1<<USICS1);

	// MISO(PB6)�͏o�͂�
	DDRB |= _BV(MISO);
	// MOSI(PB5)/USCK�͓��͂�
	DDRB &= ~(_BV(MOSI) | _BV(USCK));

	// MOSI(PB5)/USCK(PB7)�v���A�b�v
	PORTB |= (_BV(MOSI) | _BV(USCK));
	
	//INT0
	
	MCUCR = (1<<ISC01); // INT0����������
	GIMSK = (1<<INT0);  // INT0�̊��荞�݋���
	
	TIMSK  |= (1<<TOIE0);
	TIMER_OFF();
}

unsigned long Ctol(unsigned char *num,int length){
	unsigned long ret=0;
	for (int i=0;i<length;i++){
		ret*=10;
		ret+=(num[i]-48);
	}
	return ret;
}
