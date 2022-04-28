#include <Wire.h>
#include <LiquidCrystal_I2C.h> 
#include <OneWire.h>
#include "SIM900.h"
#include <SoftwareSerial.h>
#include "sms.h"
SMSGSM sms;

LiquidCrystal_I2C lcd(0x3F, 16, 2);

OneWire  ds(4);

int mark = 0;

const int AOUTpin1=1;
int adc0;
int suhu;
float h;
float T;
int output;
float dimming;

  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
 
 
float dingin, hangat, panas;
float sedikit, sedang, banyak;
float lb, sd, ke;
float hasillb, hasilsd, hasilke;
float Coglb, Cogsd, Cogke;
float Cogxlb, Cogxsd, Cogxke;
float Komlb, Komsd, Komke;
float Penlb, Pensd, Penke;
float Totalkom, Totalpen;
float Cog;
float lb1, lb2, lb3;
float sd1, sd2, sd3;
float ke1, ke2, ke3;

int buzzer = 7; 
int relay1 = 14;
int relay2 = 15;
int relay3 = 16;

int numdata;
boolean started=false;
char smsbuffer[160];
char n[20];


void setup()
{
   digitalWrite(relay1,HIGH);
   digitalWrite(relay2,HIGH);
   digitalWrite(relay3,HIGH);
  
  Serial.begin(9600);
 
       Serial.println("GSM Shield testing.");
     if (gsm.begin(2400)) {
          Serial.println("\nstatus=READY");
          started=true;
     } else Serial.println("\nstatus=IDLE");

     if(started) {
          //gsm.call("085726496789",10000); //miss call
          //delay (3000);
          if (sms.SendSMS("085726496789", "Alat Ready"))
          Serial.println("\nSMS sent OK");
     }
 
 
  lcd.begin();

  pinMode(buzzer,OUTPUT);
  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
 
}



void loop()
{
    
   if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }
 

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
 
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
 
  present = ds.reset();
  ds.select(addr);   
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
 
  celsius = (float)raw / 16.0;
  T = celsius;
  lcd.setCursor(0,0);
  lcd.print("T=");
  lcd.print(celsius,1);


   h = 1.953125 *(analogRead(AOUTpin1));
   lcd.setCursor (0,1);
   lcd.print ("G=");
   lcd.print (h,1);
   lcd.print ("   ");




   //======================
   //   fuzzyfikasi suhu
   //======================
  
   if(T <= 25){
   dingin = 1;
   hangat = 0;
   panas = 0;
   }
   else if(T >= 35){
   panas = 1;
   hangat = 0;
   dingin = 0;
   }
   else if(T == 30){
   hangat = 1;
   dingin = 0;
   panas = 0;
   }
   else if((T > 25)&&(T < 30)){
   hangat = (T - 25)/5;
   dingin = 1-(T - 25)/5;
   panas = 0;
   }
   else if((T > 30)&&(T < 35)){
   panas = (T - 30)/5;
   hangat = 1-(T - 30)/5;
   dingin = 0;
   }
  
  
 //  Serial.print("panas= ");  
 //  Serial.println(panas);
 //  Serial.print("hangat= ");  
 //  Serial.println(hangat);
 //  Serial.print("dingin= ");  
 //  Serial.println(dingin);
     
   //===============================
   //       fuzzyfikasi gas
   //===============================
  
   if(h <= 150){
   sedikit = 1;
   sedang = 0;
   banyak = 0;
   }
   else if(h >= 300){
   banyak = 1;
   sedang = 0;
   sedikit = 0;
   }
   else if(h == 200){
   banyak = 0;
   sedang = 1;
   sedikit = 0;
   }
   else if((h > 150)&&(h < 200)){
   sedang = (h - 150)/50;
   sedikit = 1-(h - 150)/50;
   banyak = 0;
   }
   else if((h > 200)&&(h < 300)){
   sedang = (h - 200)/100;
   banyak = 1-(h - 200)/100;
   sedikit = 0;
   }
  
 //  Serial.print("Sedikit= ");  
 //  Serial.println(sedikit);
 //  Serial.print("sedang= ");  
 //  Serial.println(sedang);
 //  Serial.print("banyak= ");  
 //  Serial.println(banyak);
   
  
  
  
   //==================================
   //           fuzzy rule
   //===================================
     
   //============================================================================
   if((T <= 25 )&&(h <= 150)){ //1A
    if(dingin < sedikit){
    lb = dingin;
    }
    else if(dingin > sedikit){
    lb = sedikit;
    }
    else if(dingin == sedikit){
    lb = sedikit;
    }
   
    hasillb = lb;
    hasilsd = 0;
    hasilke = 0;
    
   }
  
   //=========================================================================
  
   else if((T <= 25 )&&(h > 150)&&(h < 200)){ //1AB
    if(dingin < sedikit){
    lb1 = dingin;
    }
    else if(dingin > sedikit){
    lb1 = sedikit;
    }
    else if(dingin == sedikit){
    lb1 = sedikit;
    }
   
    if(dingin < sedang){
    lb2 = dingin;
    }
    else if(dingin > sedang){
    lb2 = sedang;
    }
    else if(dingin == sedang){
    lb2 = sedang ;
    }
   
    if(lb1 < lb2){   
     hasillb = lb2;  
    }
    else if(lb1 > lb2){   
     hasillb = lb1;  
    }
    else if(lb1 == lb2){   
     hasillb = lb1;  
    }
   
    hasilsd = 0;
    hasilke = 0;
    
   }
  
   //========================================================================
    else if((T <= 25 )&&(h > 200)&&(h < 300)){ //1BC
    if(dingin < sedang){
    lb = dingin;
    }
    else if(dingin > sedang){
    lb = sedang;
    }
    else if(dingin == sedang){
    lb = sedang;
    }
   
    if(dingin < banyak){
    sd = dingin;
    }
    else if(dingin > banyak){
    sd = banyak;
    }
    else if(dingin == banyak){
    sd = banyak;
    }
   
    hasillb = lb;  
    hasilsd = sd;
    hasilke = 0;
    
   }
  
   //=======================================================================
    else if((T <= 25 )&&(h >= 300)){ //1C

    if(dingin < banyak){
    sd = dingin;
    }
    else if(dingin > banyak){
    sd = banyak;
    }
    else if(dingin == banyak){
    sd = banyak;
    }
   
    hasillb = 0;  
    hasilsd = sd;
    hasilke = 0;
    
   }
  
  
   //==========================================================================
  
    else if((T > 25 )&&(T < 30 )&&(h <= 150)){ //12A

    if(dingin < sedikit){
    lb1 = dingin;
    }
    else if(dingin > sedikit){
    lb1 = sedikit;
    }
    else if(dingin == sedikit){
    lb1 = sedikit;
    }
   
   
    if(hangat > sedikit){
    lb2 = sedikit;
    }
    else if(hangat < sedikit){
    lb2 = hangat;
    }
    else if(hangat == sedikit){
    lb2 = hangat;
    }
   
   
    if(lb1 < lb2){   
     hasillb = lb2;  
    }
    else if(lb1 > lb2){   
     hasillb = lb1;  
    }
    else if(lb1 == lb2){   
     hasillb = lb1;  
    }
   
    hasilsd = 0;
    hasilke = 0;
    
   }
  
   //=========================================================================
  
    else if((T > 25 )&&(T < 30 )&&(h > 150)&&(h < 200)){ //12AB

    if(dingin < sedikit){
    lb1 = dingin;
    }
    else if(dingin > sedikit){
    lb1 = sedikit;
    }
    else if(dingin == sedikit){
    lb1 = sedikit;
    }
   
   
    if(dingin < sedang){
    lb2 = dingin;
    }
    else if(dingin > sedang){
    lb2 = sedang;
    }
    else if(dingin == sedang){
    lb2 = sedang;
    }
   
   
    if(hangat < sedikit){
    lb3 = dingin;
    }
    else if(hangat > sedikit){
    lb3 = sedikit;
    }
    else if(hangat == sedikit){
    lb3 = sedikit;
    }
   
   
    if(hangat < sedang){
    sd = hangat;
    }
    else if(hangat > sedang){
    sd = sedang;
    }
    else if(hangat == sedang){
    sd = sedang;
    }
   
   
   if((lb1 > lb2)&&(lb1 > lb3)){                         
   hasillb = lb1;
   }
   else if((lb1 > lb2)&&(lb1 == lb3)){
   hasillb = lb1;
   }
   else if((lb1 == lb2)&&(lb1 > lb3)){
   hasillb = lb1;
   }
   else if((lb1 == lb2)&&(lb1 == lb3)){
   hasillb = lb1;
   }
   else if((lb2 > lb1)&&(lb2 > lb3)){
   hasillb = lb2;
   }
   else if((lb2 > lb1)&&(lb2 == lb3)){
   hasillb = lb2;
   }
   else if((lb2 == lb1)&&(lb2 > lb3)){
   hasillb = lb2;
   }
   else if((lb2 == lb1)&&(lb2 == lb3)){
   hasillb = lb2;
   }
   else if((lb3 > lb1)&&(lb3 > lb2)){
   hasillb = lb3;
   }
   else if((lb3 > lb1)&&(lb3 == lb2)){
   hasillb = lb3;
   }
   else if((lb3 == lb1)&&(lb3 > lb2)){
   hasillb = lb3;
   }
   else if((lb3 == lb1)&&(lb3 == lb2)){
   hasillb = lb3;
   }

   hasilsd = sd;
   hasilke = 0;
   
   }
  
   //===============================================================================
  
   else if((T > 25)&&(T < 30)&&(h > 200)&&(h < 300)){ //12BC
   if (dingin < sedang){
   lb = dingin;
   }
   else if(dingin > sedang){
   lb = sedang;
   }
   else if(dingin == sedang){
   lb = sedang;
   }

   if(dingin < banyak){
   sd1 = dingin;
   }
   else if(dingin > banyak){
   sd1 = banyak;
   }
   else if(dingin == banyak){
   sd1 = banyak;
   }

   if(hangat < sedang){
   sd2 = hangat;
   }
   else if(hangat > sedang){
   sd2 = sedang;
   }
   else if(hangat == sedang){
   sd2 = sedang;
   }

   if(hangat < banyak){
   ke = hangat;
   }
   else if(hangat > banyak){
   ke = banyak;
   }
   else if(hangat == banyak){
   ke = banyak;
   }

   if(sd1 > sd2){
   hasilsd = sd1;
   }
   else if(sd2 > sd1){
   hasilsd = sd2;
   }
   else if(sd2 == sd1){
   hasilsd = sd2;
   }

   hasillb = lb;
   hasilke = ke;

   }

   //=======================================================================

   else if((T > 25)&&(T < 30)&&(h >= 300)){ //12C
    
   if(dingin < banyak){
   sd = dingin;
   }
   else if(dingin > banyak){
   sd = banyak;
   }
   else if(dingin == banyak){
   sd = banyak;
   }

   if(hangat > banyak){
   ke = banyak;
   }
   else if(hangat < banyak){
   ke = hangat;
   }
   else if(hangat == banyak){
   ke = hangat;
   }
  
   hasilsd = sd;
   hasilke = ke;
   hasillb = 0;
   }

   //===============================================================
   else if ((T == 30) && (h <= 150)){ //2A
   if (hangat > sedikit){    
   lb = sedikit;
   }
   else if (hangat < sedikit){
   lb = hangat;
   }
   else if (hangat == sedikit){
   lb = hangat;
   }

   hasillb = lb;
   hasilsd = 0;
   hasilke = 0;
   }
  
   //============================================================
   else if ((T == 30) && (h > 150) && (h < 200)){ //2AB
   if (hangat < sedikit){
   lb = hangat;
   }
   else if (hangat > sedikit){
   lb = sedikit;
   }
   else if (hangat == sedikit){
   lb = sedikit;
   }
  

   if (hangat < sedang){
   sd = hangat;
   }
   else if (hangat > sedikit){
   sd = sedikit;
   }
   else if (hangat == sedikit){
   sd = sedikit;
   }

   hasillb = lb;
   hasilsd = sd;
   hasilke = 0;
  
   }

//===================================================================
   else if ((T == 30) && (h > 200) && (h < 300)){ //2BC
   if (hangat < sedang){
   sd = hangat;
   }
   else if (hangat > sedang){
   sd = sedang;
   }
   else if (hangat == sedang){
   sd = sedang;
   }

   if (hangat < banyak){
   ke = hangat;
   }
   else if (hangat > banyak){
   ke = banyak;
   }
   else if (hangat == banyak){
   ke = banyak;
   }

   hasilsd = sd;
   hasilke = ke;
   hasillb = 0;
  
   }

 //=============================================================
   else if ((T == 30) && (h >= 300)){  //2C
   if (hangat < banyak){
   ke = hangat;
   }
   else if (hangat > banyak){
   ke = banyak;
   }
   else if (hangat == banyak){
   ke = banyak;
   }

   hasilke = ke;
   hasilsd = 0;
   hasillb = 0;

}


//====================================================================

   else if ((T > 30) && (T < 35) && (h <= 150)){ //23A
   if (hangat < sedikit){
   lb = hangat;
   }
   else if (hangat > sedikit){
   lb = sedikit;
   }
   else if (hangat == sedikit){
   lb = sedikit;
   }
  
   if (panas < sedikit){
   sd = panas;
   }
   else if (panas > sedikit){
   sd = sedikit;
   }
   else if (panas == sedikit){
   sd = sedikit;
   }

   hasillb = lb;
   hasilsd = sd;
   hasilke = 0;
   }

//=======================================================================

   else if ((T > 30) && (T < 35) && (h > 150) && (h < 200)){  //23AB
   if (hangat < sedikit){
   lb = hangat;
   }
   else if (hangat > sedikit){
   lb = sedikit;
   }
   else if (hangat == sedikit){
   lb = sedikit;
   }

   if (hangat < sedang){
   sd1 = hangat;
   }
   else if (hangat > sedang){
   sd1 = sedang;
   }
   else if (hangat == sedang){
   sd1 = sedang;
   }
  
   if (panas < sedikit){
   sd2 = panas;
   }
   else if (panas > sedikit){
   sd2 = sedikit;
   }
   else if (panas == sedikit){
   sd2 = sedikit;
   }

   if (panas < sedang){
   ke = panas;
   }
   else if (panas > sedang){
   ke = sedang;
   }
   else if (panas == sedang){
   ke = sedang;
   }

   if (sd1 < sd2){
   hasilsd = sd2;
   }
   else if (sd1 > sd2){
   hasilsd = sd1;
   }
   else if (sd1 == sd2){
   hasilsd = sd1;
   }

   hasilke = ke;
   hasillb = lb;
   }

//=========================================================================

   else if ((T > 30) && (T < 35) && (h > 200) && (h < 300)){ //23BC
   if (hangat < sedang){
   sd = hangat;
   }
   else if (hangat > sedang){
   sd = sedang;
   }
   else if (hangat == sedang){
   sd = sedang;
   }

   if (hangat < banyak){
   ke1 = hangat;
   }
   else if (hangat > banyak){
   ke1 = banyak;
   }
   else if (hangat == banyak){
   ke1 = banyak;
   }

   if (panas < sedang){
   ke2 = panas;
   }
   else if (panas > sedang){
   ke2 = sedang;
   }
   else if (panas == sedang){
   ke2 = sedang;
   }

   if (panas < banyak){
   ke3 = panas;
   }
   else if (panas > banyak){
   ke3 = banyak;
   }
   else if (panas == banyak){
   ke3 = banyak;
   }
  
   if ((ke1 > ke2) && (ke1 > ke3)){
   hasilke = ke1;
   }
   else if ((ke1 > ke2) && (ke1 == ke3)){
   hasilke = ke1;
   }
   else if ((ke1 == ke2) && (ke1 > ke3)){
   hasilke = ke1;
   }
   else if ((ke1 == ke2) && (ke1 == ke3)){
   hasilke = ke1;
   }
   else if ((ke2 > ke1) && (ke2 > ke3)){
   hasilke = ke2;
   }
   else if ((ke2 > ke1) && (ke2 == ke3)){
   hasilke = ke2;
   }
   else if ((ke2 == ke1) && (ke2 > ke3)){
   hasilke = ke2;
   }
   else if ((ke2 == ke1) && (ke2 == ke3)){
   hasilke = ke2;
   }

   else if ((ke3 > ke1) && (ke3 > ke2)){
   hasilke = ke3;
   }
   else if ((ke3 > ke1) && (ke3 == ke2)){
   hasilke = ke3;
   }
   else if ((ke3 == ke1) && (ke3 > ke2)){
   hasilke = ke3;
   }
   else if ((ke3 == ke1) && (ke3 == ke2)){
   hasilke = ke3;
   }

   hasilsd = sd;
   hasillb = 0;
   }

//===========================================================================

   else if ((T > 30) && (T < 35) && (h >= 300)){ //23C
   if (hangat < banyak){
   ke1 = hangat;
   }
   else if (hangat > banyak){
   ke1 = banyak;
   }
   else if (hangat == banyak){
   ke1 = banyak;
   }

   if (panas < banyak){
   ke2 = panas;
   }
   else if (panas > banyak){
   ke2 = banyak;
   }
   else if (panas == banyak){
   ke2 = banyak;
   }

   if (ke1 < ke2){
   hasilke = ke2;
   }
   else if (ke1 > ke2){
   hasilke = ke1;
   }
   else if (ke1 == ke2){
   hasilke = ke1;
   }

   hasilsd = 0;
   hasillb = 0;
   }

//===========================================================================

  else if ((T >= 35) && (h <= 150)){  //3A
   if (panas < sedikit){
   sd = panas;
   }
   else if (panas > sedikit){
   sd = sedikit;
   }
   else if (panas == sedikit){
   sd = sedikit;
   }

   hasilsd = sd;
   hasillb = 0;
   hasilke = 0;
   }

//============================================================================

else if ((T >= 35) && (h > 150) && (h < 200)){  //3AB
   if (panas < sedikit){
   sd = panas;
   }
   else if (panas > sedikit){
   sd = sedikit;
   }
   else if (panas == sedikit){
   sd = sedikit;
   }

   if (panas < sedang){
   ke = panas;
   }
   else if (panas > sedang){
   ke = sedang;
   }
   else if (panas == sedang){
   ke = sedang;
   }

   hasilsd = sd;
   hasilke = ke;
   hasillb = 0;
   }
  
  
//======================================================================  
  
else if ((T >= 35) && (h > 200) && (h < 300)){ //3BC
   if (panas < sedang){
   ke1 = panas;
   }
   else if (panas > sedang){
   ke1 = sedang;
   }
   else if (panas == sedang){
   ke1 = sedang;
   }

   if (panas < banyak){
   ke2 = panas;
   }
   else if (panas > banyak){
   ke2 = banyak;
   }
   else if (panas == banyak){
   ke2 = banyak;
   }

   if (ke1 < ke2){
   hasilke = ke2;
   }
   else if (ke1 > ke2){
   hasilke = ke1;
   }
   else if (ke1 == ke2){
   hasilke = ke1;
   }

   hasillb = 0;
   hasilsd = 0;
   }

//=========================================================================

else if ((T >= 35) && (h >= 300)){ //3C
   if (panas < banyak){
   ke = panas;
   }
   else if (panas > banyak){
   ke = banyak;
   }
   else if (panas == banyak){
   ke = banyak;
   }

   hasilke = ke;
   hasilsd = 0;
   hasillb = 0;
}


//======================================================================

else if ((T <= 25) && (h == 200)){ //1B
   if (dingin < sedang){
   lb = dingin;
   }
   else if (dingin > sedang){
   lb = sedang;
   }
   else if (dingin == sedang){
   lb = sedang;
   }

   hasillb = lb;
   hasilsd = 0;
   hasilke = 0;
}

//===================================================================

else if ((T > 25) && (T < 30) && (h == 200)){ //12B
   if (dingin < sedang){
   lb = dingin;
   }
   else if (dingin > sedang){
   lb = sedang;
   }
   else if (dingin == sedang){
   lb = sedang;
   }

   if (hangat < sedang){
   sd = hangat;
   }
   else if (hangat > sedang){
   sd = sedang;
   }
   else if (hangat == sedang){
   sd = sedang;
   }

   hasillb = lb;
   hasilsd = sd;
   hasilke = 0;
}

//==========================================================

else if ((T == 30) && (h == 200)){ //2B
   if (hangat < sedang){
   sd = hangat;
   }
   else if (hangat > sedang){
   sd = sedang;
   }
   else if (hangat == sedang){
   sd = sedang;
   }

   hasilsd = sd;
   hasillb = 0;
   hasilke = 0;
  
}


//================================================================


else if ((T > 30) && (T < 35) && (h == 200)){ //23B
   if (hangat < sedang){
   sd = hangat;
   }
   else if (hangat > sedang){
   sd = sedang;
   }
   else if (hangat == sedang){
   sd = sedang;
   }

   if (panas < sedang){
   ke = panas;
   }
   else if (panas > sedang){
   ke = sedang;
   }
   else if (panas == sedang){
   ke = sedang;
   }
  
   hasilsd = sd;
   hasilke = ke;
   hasillb = 0;
}



//================================================================

else if ((T >= 35) && (h == 200)){  //3B
   if (panas < sedang){
   ke = panas;
   }
   else if (panas > sedang){
   ke = sedang;
   }
   else if (panas == sedang){
   ke = sedang;
   }

   hasilke = ke;
   hasilsd = 0;
   hasillb = 0;

}



//==================================================
//               Defuzzifikasi
//==================================================


//0-10-20-30
Coglb = 60;
Cogxlb = 4;

//40-50-60-70
Cogsd = 220;
Cogxsd = 4;

//80-90-100-110-120
Cogke = 500;
Cogxke = 5;


Totalkom = (Coglb * hasillb) + (Cogsd * hasilsd) + (Cogke * hasilke);

Totalpen = (hasillb * Cogxlb) + (hasilsd * Cogxsd) + (hasilke * Cogxke);

Cog = Totalkom / Totalpen;
     
   //Serial.print("Totalkom= ");
   //Serial.println(Totalkom);
   //Serial.print("Totalpen= ");
   //Serial.println(Totalpen);
   //Serial.print("COG= ");
   //Serial.println(Cog);
  
   lcd.setCursor (9,0);
   lcd.print ("C=");
   lcd.print (Cog,1);
   lcd.print ("  ");
  
   int adcapi = analogRead(A2);
  
   lcd.setCursor (9,1);
   lcd.print ("A=");
   lcd.print (adcapi);
   lcd.print ("    ");
  
   delay(100);
  
   if(adcapi < 100){
   digitalWrite(buzzer,HIGH);
   }
   if(adcapi > 100){
   digitalWrite(buzzer,LOW);
   }
  
   if((Cog <= 80)&&(mark == 0)){
   digitalWrite(relay1,LOW);
   digitalWrite(relay2,HIGH);
   digitalWrite(relay3,HIGH); 
   //mark = 0;
   }
  
   if((Cog > 80)&&(Cog <= 90)&&(mark == 0)){
   digitalWrite(relay1,HIGH);
   digitalWrite(relay2,LOW);
   digitalWrite(relay3,HIGH);
       
   mark = 0;
   }
  
   if((Cog > 80)&&(Cog <= 90)&&(mark == 1)){
   digitalWrite(relay1,HIGH);
   digitalWrite(relay2,HIGH);
   digitalWrite(relay3,HIGH);
   delay(2000);
   digitalWrite(relay1,HIGH);
   digitalWrite(relay2,LOW);
   digitalWrite(relay3,HIGH);
       
   mark = 0;
   }
  
  
   if((Cog > 90)&&(mark == 0)){
   digitalWrite(relay1,HIGH);
   digitalWrite(relay2,HIGH);
   digitalWrite(relay3,HIGH);
  
     Serial.println("kirim sms kondisi bahaya");
     if (gsm.begin(2400)) {
          Serial.println("\nstatus=READY");
          started=true;
     } else Serial.println("\nstatus=IDLE");

     if(started) {
          gsm.call("+6285726496789",10000); //Set Number & Dial Duration
          delay (3000);
          if (sms.SendSMS("085726496789", "KONDISI BAHAYA"))
          Serial.println("\nSMS sent OK");
     }
    
   digitalWrite(relay1,HIGH);
   digitalWrite(relay2,HIGH);
   digitalWrite(relay3,LOW);
   mark = 1;
   
   }
  
 
}
