#include <Encoder.h>
#include <LiquidCrystal.h>

Encoder SetKnob(3, 4);
long KnobPosition=-999;

int KnobPush=5;
int RelayFeed=2;
int SetTemperature=100;
int LockTemperature=100;
int SensedTemperature=150;
int Hysteresis=3;
int tmem=0;
unsigned long IdleTimer=0;

enum SystemState {
  COAST,
  HEATING,
  SET,
  LOCKED
} State;

LiquidCrystal lcd(12,11,7,8,9,10);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(KnobPush, INPUT);  
  pinMode(RelayFeed,OUTPUT);
  pinMode(A0, INPUT);
  lcd.begin(16,2);
  lcd.print("Sous-Viduino");
}

void loop() {

  int x = analogRead(A0);
  SensedTemperature=map(x,0,1024,100,210);
//  SensedTemperatureerature=x;
  
  
if (State != SET)
{  
    if ((LockTemperature-Hysteresis)>=SensedTemperature && SensedTemperature>=(LockTemperature+Hysteresis) && tmem==1) {
      HeatingMode(true);
    }
    
     if (SensedTemperature<=LockTemperature-Hysteresis) {
      tmem=1;
      HeatingMode(true);
    }
     
     if (SensedTemperature>LockTemperature+Hysteresis) {
      tmem=0;
      HeatingMode(false);
    }
}
  
  long NewKnobPosition;
  NewKnobPosition = SetKnob.read();
   int pushButton=digitalRead(KnobPush);
   if (IdleTimer!=0)
   {
     if (millis()-IdleTimer>5000)
     {
       State=LOCKED;
       lcd.clear();
       IdleTimer=0;
     }
   }
   if (pushButton==HIGH) {
    LockTemperature=SetTemperature;
    do
    {
      pushButton=digitalRead(KnobPush);
    } while (pushButton==HIGH);
      State=LOCKED;
    IdleTimer=0;
    lcd.clear();
  }

  if (abs(NewKnobPosition-KnobPosition)>2) {
    IdleTimer=millis();
    State=SET;
    lcd.clear();
    if (NewKnobPosition > KnobPosition)
    {
      SetTemperature+=5;
    }
    else
    {
      SetTemperature-=5;
    }

    if (SetTemperature<100)
      SetTemperature=100;

    if (SetTemperature > 210)
      SetTemperature=210;
    KnobPosition = NewKnobPosition;
  }
  writelcd();
}

void writelcd()
{
  //1234567890123456
  //1234567890123456
  //C:XXXF S:XXXF 
  //Set:XXXF
  //Idle         Confirm  
  
  char stateline[20];
  char templine[20];
  
  switch(State)
  {
   case LOCKED:
   case COAST:
   {
     sprintf(templine,"C:%3.0dF L:%3.0dF", SensedTemperature, LockTemperature);
     sprintf(stateline,"Idle");  
     break;
   }
   case HEATING:
   {
     sprintf(templine,"C:%3.0dF L:%3.0dF", SensedTemperature, LockTemperature);
     sprintf(stateline,"Heating...");
     break;
   }
   case SET:
   {
     sprintf(templine,"Set Temp: %3.0dF", SetTemperature);
     sprintf(stateline,"Push knob to set");
     break;
   }
  }
  lcd.setCursor(0,0);
  lcd.write(templine);
  lcd.setCursor(0,1);
  lcd.write(stateline);
}
  

void HeatingMode(bool status)
{
  if (status)
  {
    if (State!=HEATING)
    {
       State=HEATING;
      digitalWrite(RelayFeed,HIGH);
      lcd.clear();
    }
  }
  else
  {
    if (State !=COAST)
    {
      State=COAST;
      digitalWrite(RelayFeed,LOW);
      lcd.clear();
    }
  }
}
