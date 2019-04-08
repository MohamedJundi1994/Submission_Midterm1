#define F_CPU 16000000UL
#define BAUD 9600
#define NEWuussaarrtt F_CPU/16/BAUD-1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


void initialization_usart( unsigned int uussaarrtt );     // Used to initialize USART
void prnt_strng(char *info);               // Used to print String USART

volatile unsigned int temp_info = 0;           // Used for the temperature value recorded

char outputs[256];                  // Array for outputs

unsigned char checkok[] = "AT\r\n";      // Check if okay, send AT cmds
unsigned char mod[] = "AT+CWMODE=1";     // Enables the WiFi
unsigned char namepass[] = "AT+CWJAP=\"MOE_Wifi\",\"moo123456\"\r\n"; // Hotspot used for transfer
unsigned char cmux[] = "AT+CIPMUX=0\r\n";   // Used to select the mux
unsigned char start[] = "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n";  // Used to start a TCP connection
unsigned char sent[] = "AT+CIPSEND=100\r\n";    // Used to send data


int main(void) {

	sei(); // Enable Global Interrupt

	/** Setup and enable ADC **/
	ADMUX = (0<<REFS1)|    // Reference Selection Bits
	(1<<REFS0)|    // AVcc - external cap at AREF
	(0<<ADLAR)|    // ADC Left Adjust Result
	(1<<MUX2)|     // Analog Channel Selection Bits
	(0<<MUX1)|     // ADC4 (PC5 PIN28)
	(1<<MUX0);
	ADCSRA = (1<<ADEN)|    // ADC ENable
	(0<<ADSC)|     // ADC Start Conversion
	(0<<ADATE)|    // ADC Auto Trigger Enable
	(0<<ADIF)|     // ADC Interrupt Flag
	(0<<ADIE)|     // ADC Interrupt Enable
	(1<<ADPS2)|    // ADC Prescaler Select Bits
	(0<<ADPS1)|
	(1<<ADPS0);

	TCCR1B |= (1 << CS12 | (1 << CS10)) ;   // Used to set prescaler of 1024
	TIMSK1 = (1 << TOIE1);                  // TOIE1 will be set high and enable overflow
	TCNT1 = 49911;                          // Set to count from 49911 to count up to 65535 to achieve 1 second
	
	initialization_usart(NEWuussaarrtt); // Used to call function to initialize the USART
	_delay_ms(1000);   // Take time, wait a second
	
	while(1)
	{
                      // Infinite loop, wait here for interrupt
	}
}


ISR(TIMER1_OVF_vect) //timer overflow interrupt to delay for 1 second
{
	_delay_ms(1000);	   // Don't rush, wait 1 sec
	prnt_strng(checkok);   // Used to send AT cmds
	_delay_ms(1000);	   // Don't rush, wait 1 sec
	prnt_strng(mod);       // Used to set the mode
	_delay_ms(1000);       // Don't rush, wait 1 sec
	prnt_strng(namepass);  // Used to connect to WiFi
	_delay_ms(3000);       // Don't rush, wait 3 sec
	prnt_strng(cmux);      // Used the select mux values
	_delay_ms(2000);       // Don't rush, wait 2 sec
	prnt_strng(start);     // Used to start a TCP connection
	_delay_ms(2000);       // Don't rush, wait 2 sec
	prnt_strng(sent);      // Used to send data
	_delay_ms(1000);       // Don't rush, wait 1 sec
	
	unsigned char i = 4;  // Start loop from four, down to zero
	temp_info = 0;       // Setting the temp info to zero
	
	while (i--)   // Decrement from 4
	{
		ADCSRA |= (1<<ADSC);    // Start conversion
		while(ADCSRA & (1<<ADSC));  // While ADC control and status register set to high on start conversion, add temp
		temp_info+= ADC; // add temp_info to ADC and set equal
		_delay_ms(100);   // Don't rush wait 0.1 sec
	}
	temp_info = temp_info / 8; // Used to average out sample
	
	
	snprintf(outputs,sizeof(outputs),"GET https://api.thingspeak.com/update?api_key=29PX5VOXJLHTDGBS&field1=%3d\r\n", temp_info);  // Used to Print
	prnt_strng(outputs);   // Used to sent outputs
	_delay_ms(4000);    // Wait 4 sec
	
	TCNT1 = 49911; // Used to reset the TCNT1 value
	
}

void initialization_usart( unsigned int uussaarrtt ) 
{
	UBRR0L = (unsigned char)uussaarrtt;
	UBRR0H = (unsigned char)(uussaarrtt >> 8);
	UCSR0B |= (1 << TXEN0) | (1 << RXEN0)| ( 1 << RXCIE0); 
	UCSR0C |= (1<<UCSZ01) | (1 << UCSZ00);
}

void prnt_strng( char *info ) 
{
	while ((*info != '\0')) 
	{
		while (!(UCSR0A & (1 <<UDRE0)));
		UDR0 = *info;
		info++;
	}
}


