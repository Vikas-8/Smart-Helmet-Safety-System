#include <lpc214x.h>
#include <stdio.h>

#define bit(x) (1<<x)

/* ---------- THRESHOLDS ---------- */
#define GAS_TH      800
#define RAIN_TH     700
#define VIB_TH      600
#define ALCOHOL_TH  650
#define GYRO_TH     700

/* ---------- LCD ---------- */
void lcd_cmd(unsigned char);
void lcd_data(unsigned char);
void lcd_init(void);
void lcd_show(char *);
void lcd_delay(void);

/* ---------- UART (RFID) ---------- */
void uart0_init(void);
unsigned char uart0_rx(void);

/* ---------- ADC ---------- */
void adc_init(void);
unsigned int read_adc(unsigned char);

/* ---------- PWM / SERVO ---------- */
void pwm_init(void);
void servo_0_deg(void);
void servo_90_deg(void);

/* ---------- DELAY ---------- */
void short_delay(void);

/* ---------- GLOBALS ---------- */
unsigned int gas, rain, vib, alcohol, gyro;
char buf[32];
int canopy_state = 0;
int access_granted = 0;

/* ================= MAIN ================= */
int main(void)
{
    char id;

    /* LED */
    PINSEL0 &= ~(3<<6);
    IO0DIR  |= (1<<3);

    /* LCD */
    IO1DIR |= 0xFF070000;

    uart0_init();
    adc_init();
    lcd_init();
    pwm_init();

    servo_0_deg();

    lcd_cmd(0x80);
    lcd_show("SMART BIKE SYS");
    lcd_cmd(0xC0);
    lcd_show("SHOW RFID");
    short_delay();

    while(1)
    {
        /* ===== RFID ===== */
        if(!access_granted)
        {
            if(U0LSR & 0x01)
            {
                id = uart0_rx();

                if(id=='1'||id=='2'||id=='3')
                {
                    access_granted = 1;
                    IOSET0 = (1<<3);
                    short_delay();
                    IOCLR0 = (1<<3);

                    lcd_cmd(0x01);
                    lcd_show("ACCESS GRANTED");
                }
                else
                {
                    IOSET0 = (1<<3);
                    short_delay();
                    IOCLR0 = (1<<3);

                    lcd_cmd(0x01);
                    lcd_show("ACCESS DENIED");
                }
                short_delay();
            }
            continue;
        }

        /* ===== SENSOR READ ===== */
        alcohol = read_adc(0);   /* AD0.0 */
        gyro    = read_adc(1);   /* AD0.1 (tilt, potentiometer in simulation) */
        gas     = read_adc(2);   /* AD0.2 */
        vib     = read_adc(3);   /* AD0.3 */
        rain    = read_adc(4);   /* AD0.4 */

        /* ===== NORMAL DISPLAY ===== */
        lcd_cmd(0x80);
        sprintf(buf,"R:%03d A:%03d", rain/10, alcohol/10);
        lcd_show(buf);

        lcd_cmd(0xC0);
        sprintf(buf,"G:%03d V:%03d", gas/10, vib/10);
        lcd_show(buf);

        /* ===== ALCOHOL ===== */
        if(alcohol > ALCOHOL_TH)
        {
            lcd_cmd(0x01);
            lcd_show("ALCOHOL ALERT");
            IOSET0 = (1<<3);
            short_delay();
            IOCLR0 = (1<<3);
            access_granted = 0;
            continue;
        }

        /* ===== GAS ===== */
        if(gas > GAS_TH)
        {
            lcd_cmd(0x01);
            lcd_show("GAS LEAK!");
            IOSET0 = (1<<3);
            short_delay();
            IOCLR0 = (1<<3);
        }

        /* ===== ACCIDENT (GYRO + VIBRATION) ===== */
        if(gyro > GYRO_TH && vib > VIB_TH)
        {
            lcd_cmd(0x01);
            lcd_show("ACCIDENT !!!");
            IOSET0 = (1<<3);
            short_delay();
            IOCLR0 = (1<<3);
        }
        /* ===== ONLY GYRO ===== */
        else if(gyro > GYRO_TH)
        {
            lcd_cmd(0x01);
            lcd_show("BIKE TILTED");
            IOSET0 = (1<<3);
            short_delay();
            IOCLR0 = (1<<3);
        }
        /* ===== ONLY VIBRATION ===== */
        else if(vib > VIB_TH)
        {
            lcd_cmd(0x01);
            lcd_show("HIGH VIBRATION");
            IOSET0 = (1<<3);
            short_delay();
            IOCLR0 = (1<<3);
        }

        /* ===== RAIN / CANOPY ===== */
        if(rain > RAIN_TH && canopy_state==0)
        {
            lcd_cmd(0x01);
            lcd_show("RAIN - CANOPY");
            servo_90_deg();
            canopy_state = 1;
        }
        else if(rain <= RAIN_TH && canopy_state==1)
        {
            lcd_cmd(0x01);
            lcd_show("CANOPY OFF");
            servo_0_deg();
            canopy_state = 0;
        }

        short_delay();
    }
}

/* ================= UART ================= */
void uart0_init(void){
    VPBDIV = 0x01;
    PINSEL0 |= 0x00000005;
    U0LCR = 0x83;
    U0DLL = 78;
    U0DLM = 0;
    U0LCR = 0x03;
}
unsigned char uart0_rx(void){
    while(!(U0LSR & 1));
    return U0RBR;
}

/* ================= ADC ================= */
void adc_init(void){
    PINSEL1 |= (1<<22)|(1<<24)|(1<<26)|(1<<28)|(1<<30);
    AD0CR = (1<<21)|(6<<8);
}
unsigned int read_adc(unsigned char ch){
    unsigned int v;
    AD0CR &= 0xFFFFFF00;
    AD0CR |= (1<<ch);
    AD0CR |= (1<<24);
    while(!(AD0GDR & (1U<<31)));
    v = (AD0GDR>>6)&0x3FF;
    return v;
}

/* ================= PWM ================= */
void pwm_init(void){
    PINSEL1 |= (1<<10);
    PWMPR=14; PWMMR0=20000; PWMMCR=2; PWMPCR=(1<<13); PWMTCR=9;
}
void servo_0_deg(void){ PWMMR5=1200; PWMLER=(1<<5); }
void servo_90_deg(void){ PWMMR5=1700; PWMLER=(1<<5); }

/* ================= LCD ================= */
void lcd_cmd(unsigned char a){
    IO1CLR=0xFF070000; IO1SET=a<<24;
    IO1CLR=bit(16); IO1CLR=bit(17); IO1SET=bit(18);
    lcd_delay(); IO1CLR=bit(18);
}
void lcd_data(unsigned char b){
    IO1CLR=0xFF070000; IO1SET=b<<24;
    IO1SET=bit(16); IO1CLR=bit(17); IO1SET=bit(18);
    lcd_delay(); IO1CLR=bit(18);
}
void lcd_show(char *s){ while(*s) lcd_data(*s++); }
void lcd_init(void){
    lcd_cmd(0x38); lcd_cmd(0x0C); lcd_cmd(0x01); lcd_cmd(0x06); lcd_cmd(0x80);
}
void lcd_delay(void){ int i; for(i=0;i<3000;i++); }

/* ================= DELAY ================= */
void short_delay(void){ int i; for(i=0;i<30000;i++); }
