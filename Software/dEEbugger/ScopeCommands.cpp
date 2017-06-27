#include "ScopeCommands.h"
#include <Q2HX711.h>
const byte hx711_data_pin = D5;
const byte hx711_clock_pin = D6 ;
Q2HX711 hx711(SDA, SCL); //rcc wire interface defaults

String channelModeOutput1;
String channelModeOutput2;
bool toggledChannelOffFlag1;
bool toggledChannelOffFlag2;
byte ADCAddress = 54;  adc address on 12c

long CH1Scale =1 ; //RCc new variables for scaling everything
long CH2Scale =1 ;
void scopeInit(void)
{
  
  setChannelMode1(""); // nb this does not set the initial settings, use "SELECTED" IN THE WEBSITE HTML CODES 
  setChannelMode2("");
  toggledChannelOffFlag1 = false;
  toggledChannelOffFlag2 = false;
  channelModeOutput1 = "";
  channelModeOutput2 = "";
  // changed and webserver to defauult to var yPlotScaleFactor = 10;
 
  setUartScopeData("0");
  ADCInit();
  
}
void scopeHandler(WebSocketsServer &WEBSOCKETOBJECT)
{
  //Channel 1
  if( (getChanneMode1()=="4V ADC")||(getChanneMode1()=="64V ADC")||(getChanneMode1()=="INT ADC")||(getChanneMode1()=="DIG") )
  {
    toggledChannelOffFlag1 = false;
    channelModeOutput1 = "SCOPE ADC DATACHANNEL1 ";  // RCC ADDED SPACE TO WORK WITH SCALE FACTOR string IN NEXT LINE
    channelModeOutput1 += String ( (((getADCScopeData1().toInt()*4096/64)/CH1Scale)),DEC);    //rcc  to scale,(4096 =64v)/CH1Scale) order is to ensure INT stays big for longer in the sum to avoid truncation..
    WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
    if(getDataLog())
    {
      Serial.print("CHANNEL1 mV, ");
     // Serial.println(getADCScopeData1()); 
      Serial.println(getADCScopeData1().toInt()*1000/CH1Scale);
    }
    clearADCScopeData1();
  }
  if(getChanneMode1()=="UART")
  {
    toggledChannelOffFlag1 = false;
    channelModeOutput1 = "SCOPE UART DATACHANNEL1";
    channelModeOutput1 += getUartScopeData();
    WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
    clearUartScopeData();
  }
  if(getChanneMode1()=="OFF")
  {
    if(!toggledChannelOffFlag1)
    {
      toggledChannelOffFlag1 = true;
      channelModeOutput1 = "SCOPE OFF DATACHANNEL1";
      channelModeOutput1 += " 0";
      WEBSOCKETOBJECT.broadcastTXT(channelModeOutput1);
    }
  }
  //Channel 2
  if( (getChanneMode2()=="4V ADC")||(getChanneMode2()=="64V ADC")||(getChanneMode2()=="INT ADC")||(getChanneMode2()=="DIG")  )
  {
    toggledChannelOffFlag2 = false;
    channelModeOutput2 = "SCOPE ADC DATACHANNEL2 ";
    channelModeOutput2 += String ( (((getADCScopeData2().toInt()*4096/64)/CH2Scale)),DEC);    //rcc trying to scale,(4096 =64v)/CHxScale) order is to ensure INT stays big for longer in the sum to avoid truncation..

    WEBSOCKETOBJECT.broadcastTXT(channelModeOutput2);
    if(getDataLog())
    {
      Serial.print("CHANNEL2 mv, ");
      Serial.println(getADCScopeData2().toInt()*1000/CH2Scale);
    }
    clearADCScopeData2();
  }
  if(getChanneMode2()=="UART")
  {
    toggledChannelOffFlag2 = false;
    channelModeOutput2 = "SCOPE UART DATACHANNEL2";
    channelModeOutput2 += getUartScopeData();
    WEBSOCKETOBJECT.broadcastTXT(channelModeOutput2);
    clearUartScopeData();
  }
  if(getChanneMode2()=="OFF")
  {
    if(!toggledChannelOffFlag2)
    {
      toggledChannelOffFlag2 = true;
      channelModeOutput2 = "SCOPE OFF DATACHANNEL2";
      channelModeOutput2 += " 0";
      WEBSOCKETOBJECT.broadcastTXT(channelModeOutput2);
    }
  }
}
void ADCInit(void)
{
  	byte internalError;
    byte ADCSetupByte = 210;
	  byte ADCConfigByte = 97;
    Wire.beginTransmission(ADCAddress);
    Wire.write(ADCSetupByte);
    Wire.write(ADCConfigByte);
    internalError = Wire.endTransmission();
	if (internalError == 0)
	{
		Serial.println("i2c ADC Initialized");
	}
}
void setADCChannel(int CHANNEL)
{   
    byte internalError, ADCConfigByte;
    //Select correct channel
    switch(CHANNEL)
    {
        case 0:
            ADCConfigByte = 97;
            break;
        case 1:
            ADCConfigByte = 99;
            break;
        default:
            ADCConfigByte = 97;
            break;
    }
    //Send channel selection
    Wire.beginTransmission(ADCAddress);
    Wire.write(ADCConfigByte);
    internalError = Wire.endTransmission();
    if (internalError != 0)
    {
        Serial.println("Error setting ADC channel");
    }
}
int ADCRead(void)
{
	//Read channel
	Wire.requestFrom(ADCAddress, 2); 
    if (Wire.available() > 0)
    { 
        byte ADCResultMSB = Wire.read();
        byte ADCResultLSB = Wire.read();
        uint16_t ADCResult = (((ADCResultMSB<<8)|ADCResultLSB)&0x0FFF);
        return ADCResult;
	}
}
void ADCHandler(void)
{
byte DigPin = D_Input;
  if(getChanneMode1()=="DIG")
    {
       if  (digitalRead(DigPin)==1) {  
            addADCScopeData1("1") ;}
       else {
        addADCScopeData1("0");
               }
        CH1Scale=1;
    }
  if(getChanneMode1()=="INT ADC")
    {
        addADCScopeData1(String((analogRead(0)) ));  //3.3v ref, output in mv1024 not 2048
        CH1Scale=1024/3.3;
    }
    
    if(getChanneMode1()=="4V ADC")
    {
        setADCChannel(0);
        addADCScopeData1(String(ADCRead()));
        CH1Scale=2048/4;
    }
    if(getChanneMode1()=="64V ADC")
    {
        setADCChannel(1);
        addADCScopeData1(String(ADCRead()));
        CH1Scale=2048/64;
    } 
     if(getChanneMode1()=="SCALES")
    {
        setADCChannel(1);
        addADCScopeData1(String(hx711.read()) ) ;
        CH1Scale=2^24/5; //rcc 24 bit scale!
    } 
   
    if(getChanneMode2()=="DIG")
    {
     // Serial.println(digitalRead(13));  //test
       if  (digitalRead(DigPin)==1){  
        addADCScopeData2("1") ;
        }
       else {
        addADCScopeData2("0");
               }
        CH2Scale=1;
     }
     if(getChanneMode2()=="INT ADC")
    {
        addADCScopeData2(String((analogRead(0)) ));  //3.3v ref, output in mv1024 not 2048
        CH2Scale=1024/3.3;
    }
    if(getChanneMode2()=="4V ADC")
    {
        setADCChannel(0);
        addADCScopeData2(String(ADCRead()));
        CH2Scale=2048/4;
    }
    if(getChanneMode2()=="64V ADC")
    {
        setADCChannel(1);
        addADCScopeData2(String(ADCRead()));
        CH2Scale=2048/64;
    }
}

