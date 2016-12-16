#include <Wire.h>

/* Debug definitions */
#define PRINT_NUNCHUK_DATA
#define WIRELESS_KAMA_NUNCHUK    /* Comment out this line if using a Wired Nunchuk */

#define READ_DELAY        5      /* (milliseconds) - Increase this number to not read the nunchuk data so fast */
#define SERIAL_BAUD_RATE  115200

unsigned int joy_x = 0;
unsigned int joy_y = 0;
unsigned int acc_x = 0;
unsigned int acc_y = 0;
unsigned int acc_z = 0;
unsigned int btn_c = 0;
unsigned int btn_z = 0;
double speed = 0;
double acceleration = 0.02;
double decay = 0.98;

unsigned long previous_read_time = 0;

void setup()
{ 
   Serial.begin(SERIAL_BAUD_RATE);

   Wire.begin();
   nunchuck_setpowerpins();
   delay(1000);
   initialize_nunchuk();
   
   previous_read_time = millis();
}

void loop()
{
   if (millis() - previous_read_time > READ_DELAY)
   {
       read_nunchuk_data();
       if (btn_c == 1 && btn_z == 1)
       {
         double accel = ((int)joy_y - 128) / 128.0;
         Serial.println(accel);
         speed *= decay;
         speed += accel * acceleration;
         if (speed > 1.0)
          speed = 1.0;
         else if (speed < -1.0)
          speed = -1.0;
         Serial.println(speed);
         int out = (int)((speed + 1.0) * 500);
         Serial.println(out);
       } else
       {
        speed = 0;
       }
       previous_read_time = millis();
   }
}

void read_nunchuk_data()
{
   unsigned int buffer[6];
   byte buffer_index = 0;
 
   Wire.beginTransmission(0x52);
   Wire.write(0x00);
   Wire.endTransmission();
   
#ifndef WIRELESS_KAMA_NUNCHUK    
   delay(1); /* This delay is required for a wired nunchuk otherwise the data will appear maxed out */
#endif
 
   Wire.requestFrom(0x52, 6);
   while(Wire.available())    
   {
       buffer[buffer_index] = Wire.read();
       buffer_index++;
   }
   
   joy_x = buffer[0];
   joy_y = buffer[1];
   acc_x = ((buffer[2] << 2) | ((buffer[5] & 0x0C) >> 2) & 0x03FF);
   acc_y = ((buffer[3] << 2) | ((buffer[5] & 0x30) >> 4) & 0x03FF);
   acc_z = ((buffer[4] << 2) | ((buffer[5] & 0xC0) >> 6) & 0x03FF);
   btn_c = !((buffer[5] & 0x02) >> 1);
   btn_z = !(buffer[5] & 0x01);
   
#ifdef PRINT_NUNCHUK_DATA
   Serial.print("    "); Serial.print(joy_x);
   Serial.print("      "); Serial.print(joy_y);
   Serial.print("      "); Serial.print(acc_x);
   Serial.print("       "); Serial.print(acc_y);
   Serial.print("       "); Serial.print(acc_z);
   Serial.print("        "); Serial.print(btn_c);
   Serial.print("          "); Serial.print(btn_z); 
   Serial.println("");
#endif
}

void initialize_nunchuk()
{
#ifdef WIRELESS_KAMA_NUNCHUK
   Wire.beginTransmission(0x52);
   Wire.write (0xF0);
   Wire.write (0x55);
   Wire.endTransmission();
   delay(30);
 
   Wire.beginTransmission (0x52);
   Wire.write (0xFB);
   Wire.write (0x00);
   Wire.endTransmission();
   delay(30);
   
   Wire.beginTransmission(0x52);
   Wire.write (0xFA);
   Wire.endTransmission();
   delay(30);
 
   Wire.requestFrom(0x52, 6);
   Serial.print("Device ID is: ");
   while(Wire.available())  
   {
       byte c = Wire.read(); 
       Serial.print(c, HEX);      
       Serial.print(" ");
   }
   delay(30);
   
#else
   Wire.beginTransmission(0x52);
   Wire.write (0x40);      
   Wire.write (0x00);      
   Wire.endTransmission();
   delay(30);
#endif

#ifdef PRINT_NUNCHUK_DATA    
   Serial.println("");  
   Serial.println("  X-axis   Y-axis   X-accel   Y-accel   Z-accel   C-button   Z-button");
#endif
}

static void nunchuck_setpowerpins()
{
#define pwrpin PORTC3
#define gndpin PORTC2
    DDRC |= _BV(pwrpin) | _BV(gndpin);
    PORTC &=~ _BV(gndpin);
    PORTC |=  _BV(pwrpin);
    delay(100);  // wait for things to stabilize        
}
