/* Implementing EEprom memory here
 * IRrecord: record and play back IR signals as a minimal 
 * An IR detector/demodulator must be connected to the input RECV_PIN.
 * An IR LED must be connected to the output PWM pin 3.
 * A button must be connected to the input PROGRAM_BUTTON; this is the
 * send button.
 * A visible LED can be connected to STATUS_PIN to provide status.
 *
 * The logic is:
 * If the button is pressed, send the IR code.
 * If an IR code is received, record it.
 *
 * Version 0.11 September, 2009
 * Copyright 2009 Ken Shirriff
 * http://arcfn.com
 * 
 * Edited to make a Universal Remote Code
 */
#include <BiColorLED.h>
#include <IRremote.h>
#include <EEPROM.h>
#define ONE 1
#define TWO 2
#define THREE 3
#define FOUR 4
#define PLAYBACK 1
#define RECORD 0
#define TRANSFER 2
#define GREEN 1
#define RE 2
#define BLACK 0
#define YELLOW 3


// BiColor normally expects redPin, greenPin as its argument
//however our wiring requires the pins to be reversed
//because we're sinking rather than sourcing current

// Please note the nomenclature of the bicolor led routine if should be the following:
// BiColorLED(pin(x+1),pin(x)); where green should be set to 1 and red to  2 
BiColorLED ledButton4=BiColorLED(23,22);
BiColorLED ledButton3=BiColorLED(21,20);
BiColorLED ledButton2=BiColorLED(19,18);
BiColorLED ledButton1=BiColorLED(17,16);
BiColorLED ledProgram=BiColorLED(15,14); 


int timerStart;
int timerEnd;
int STATE=1;
int RECV_PIN = 3;
int PROGRAM_BUTTON = 6;
int BUTTON_ONE = 7;
int BUTTON_TWO = 8;
int BUTTON_THREE = 9;
int BUTTON_FOUR= 10;
int STATUS_PIN = 13;
//int RECV_PINFS = 11;
struct buttonFunction{
  int codeType;
  unsigned long codeValue;
  unsigned int rawCodes[RAWBUF];
  int codeLen;
  int toggle=0;
  int count;
  int LED;
};



 //buttonFunction b[cellNumber];
 buttonFunction mButton;
 buttonFunction b[4];// defining the buttons ie we have --> 4 buttons .

IRrecv irrecv(RECV_PIN);
//IRrecv irrecvFS(RECV_PIN);
IRsend irsend;

decode_results results;

void EEPROMfetch(int EEPROMcell);


void setup()
{  

  Serial.begin(9600);
  delay(1000);
  //pinMode(5,OUTPUT);
 // while(Serial.available()!=0);
  irrecv.enableIRIn(); // Start the receiver
  //irrecvFS.enableIRIn();
  pinMode(PROGRAM_BUTTON, INPUT);
  pinMode(STATUS_PIN, OUTPUT);
  pinMode(BUTTON_ONE,INPUT);
  pinMode(BUTTON_TWO,INPUT);
  pinMode(BUTTON_THREE,INPUT);
  pinMode(BUTTON_FOUR,INPUT);
  
 
 
  //b[cellNumber].toggle= 0;// The RC5/6 toggle state
  EEPROMfetch(ONE);
 // b[0].codeLen=0;
  EEPROMfetch(TWO);
  EEPROMfetch(THREE);
  EEPROMfetch(FOUR);
  Serial.println(b[ONE-1].LED,DEC);
Serial.println(b[TWO-1].LED,DEC);
Serial.println(b[THREE-1].LED,DEC);
Serial.println(b[FOUR-1].LED,DEC);
int led1Status;
EEPROM.get(425,led1Status);
Serial.println(led1Status,DEC);
  //if(b[ONE-1].LED==GREEN){
   // ledButton1.setColor(GREEN);
 // }
 //Serial.print(b[ONE-1].codeLen,HEX);
// led1.setColor(0,2);
// if(b[0].codeLen == 0){
//  led1.setColor(0,1);
// }
 
//led1.setBlinkSpeed(1);
}

// Storage for the recorded code
//int codeType = -1; // The type of code
//unsigned long codeValue; // The code value if not raw
//unsigned int rawCodes[RAWBUF]; // The durations if raw
//int codeLen; // The length of the code

// Stores the code for later playback
// Most of this code is just logging
void storeCode(decode_results *results) {
  Serial.println("in store code");
  mButton.codeType = results->decode_type;
  mButton.count = results->rawlen;
  if (mButton.codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    mButton.codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= mButton.codeLen; i++) {
      if (i % 2) {
        // Mark
        mButton.rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        mButton.rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(mButton.rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (mButton.codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (mButton.codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (mButton.codeType == PANASONIC) {
      Serial.print("Received PANASONIC: ");
    }
    else if (mButton.codeType == JVC) {
      Serial.print("Received JVC: ");
    }
    else if (mButton.codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (mButton.codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else if (mButton.codeType ==SAMSUNG){
      Serial.printf("Recieved Samsung: ");
    }
    else 
    {
      Serial.print("Unexpected codeType ");
      Serial.print(mButton.codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    mButton.codeValue = results->value;
    mButton.codeLen = results->bits;
   // ledProgram.setColor(GREEN);
    mButton.LED=GREEN;
    
   
    
  }
}



  void storeCode(decode_results *results,int cellNumber) {
  b[cellNumber].codeType = results->decode_type;
  b[cellNumber].count = results->rawlen;
  if (b[cellNumber].codeType == UNKNOWN) {
    Serial.println("Received unknown code, saving as raw");
    b[cellNumber].codeLen = results->rawlen - 1;
    // To store raw codes:
    // Drop first value (gap)
    // Convert from ticks to microseconds
    // Tweak marks shorter, and spaces longer to cancel out IR receiver distortion
    for (int i = 1; i <= b[cellNumber].codeLen; i++) {
      if (i % 2) {
        // Mark
        b[cellNumber].rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK - MARK_EXCESS;
        Serial.print(" m");
      } 
      else {
        // Space
        b[cellNumber].rawCodes[i - 1] = results->rawbuf[i]*USECPERTICK + MARK_EXCESS;
        Serial.print(" s");
      }
      Serial.print(b[cellNumber].rawCodes[i - 1], DEC);
    }
    Serial.println("");
  }
  else {
    if (b[cellNumber].codeType == NEC) {
      Serial.print("Received NEC: ");
      if (results->value == REPEAT) {
        // Don't record a NEC repeat value as that's useless.
        Serial.println("repeat; ignoring.");
        return;
      }
    } 
    else if (b[cellNumber].codeType == SONY) {
      Serial.print("Received SONY: ");
    } 
    else if (b[cellNumber].codeType == PANASONIC) {
      Serial.print("Received PANASONIC: ");
    }
    else if (b[cellNumber].codeType == JVC) {
      Serial.print("Received JVC: ");
    }
    else if (b[cellNumber].codeType == RC5) {
      Serial.print("Received RC5: ");
    } 
    else if (b[cellNumber].codeType == RC6) {
      Serial.print("Received RC6: ");
    } 
    else if (b[cellNumber].codeType ==SAMSUNG){
      Serial.printf("Recieved Samsung: ");
      
    }
    else {
      Serial.print("Unexpected codeType ");
      Serial.print(b[cellNumber].codeType, DEC);
      Serial.println("");
    }
    Serial.println(results->value, HEX);
    b[cellNumber].codeValue = results->value;
    b[cellNumber].codeLen = results->bits;
    
    
  }
 
}
void sendCode(int repeat) {
  if (mButton.codeType == NEC) {
    if (repeat) {
      irsend.sendNEC(REPEAT, mButton.codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else {
      irsend.sendNEC(mButton.codeValue, mButton.codeLen);
      Serial.print("Sent NEC ");
      Serial.println(mButton.codeValue, HEX);
    }
  } 
  else if (mButton.codeType == SONY) {
    irsend.sendSony(mButton.codeValue, mButton.codeLen);
    Serial.print("Sent Sony ");
    Serial.println(mButton.codeValue, HEX);
  } 
  else if (mButton.codeType == PANASONIC) {
    irsend.sendPanasonic(mButton.codeValue, mButton.codeLen);
    Serial.print("Sent Panasonic");
    Serial.println(mButton.codeValue, HEX);
  }
  else if (mButton.codeType == JVC) {
    irsend.sendJVC(mButton.codeValue, mButton.codeLen, false);
    Serial.print("Sent JVC");
    Serial.println(mButton.codeValue, HEX);
  }
  else if (mButton.codeType == SAMSUNG){
    irsend.sendSAMSUNG(mButton.codeValue,mButton.codeLen);
    Serial.print("Sent Samsung");
    Serial.println(mButton.codeValue, HEX);   
   }  
  else if (mButton.codeType == RC5 || mButton.codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
      mButton.toggle = 1 - mButton.toggle;
    }
    // Put the toggle bit into the code to send
    mButton.codeValue = mButton.codeValue & ~(1 << (mButton.codeLen - 1));
    mButton.codeValue = mButton.codeValue | (mButton.toggle << (mButton.codeLen - 1));
    if (mButton.codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(mButton.codeValue, HEX);
      irsend.sendRC5(mButton.codeValue, mButton.codeLen);
    } 
    else {
      irsend.sendRC6(mButton.codeValue, mButton.codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(mButton.codeValue, HEX);
    }
  } 
  else if (mButton.codeType == UNKNOWN /* i.e. raw */) {
    // sending 56khz to mButton the module 
  //if(digitalRead(freqSwitchpin))
    //irsend.sendRaw(mButton.rawCodes, mButton.codeLen,38);
    irsend.sendRaw(mButton.rawCodes, mButton.codeLen,56);
    Serial.println("Sent raw");
  }
}


void sendCode(int repeat,int cellNumber) {
  Serial.println("Hey i am in sendCode");
  if (b[cellNumber].codeType == NEC) {
    if (repeat) {
      irsend.sendNEC(REPEAT, b[cellNumber].codeLen);
      Serial.println("Sent NEC repeat");
    } 
    else {
      irsend.sendNEC(b[cellNumber].codeValue, b[cellNumber].codeLen);
      Serial.print("Sent NEC ");
      Serial.println(b[cellNumber].codeValue, HEX);
    }
  } 
  else if (b[cellNumber].codeType == SONY) {
    irsend.sendSony(b[cellNumber].codeValue, b[cellNumber].codeLen);
    Serial.print("Sent Sony ");
    Serial.println(b[cellNumber].codeValue, HEX);
  } 
  else if (b[cellNumber].codeType == PANASONIC) {
    irsend.sendPanasonic(b[cellNumber].codeValue, b[cellNumber].codeLen);
    Serial.print("Sent Panasonic");
    Serial.println(b[cellNumber].codeValue, HEX);
  }
  else if (b[cellNumber].codeType == JVC) {
    irsend.sendJVC(b[cellNumber].codeValue, b[cellNumber].codeLen, false);
    Serial.print("Sent JVC");
    Serial.println(b[cellNumber].codeValue, HEX);
  }

   else if (b[cellNumber].codeType == SAMSUNG){
    irsend.sendSAMSUNG(b[cellNumber].codeValue,b[cellNumber].codeLen);
    Serial.print("Sent Samsung");
    Serial.print(b[cellNumber].codeValue,HEX);
   }
  else if (b[cellNumber].codeType == RC5 || b[cellNumber].codeType == RC6) {
    if (!repeat) {
      // Flip the toggle bit for a new button press
     b[cellNumber].toggle = 1 - b[cellNumber].toggle;
    }
    // Put the toggle bit into the code to send
    b[cellNumber].codeValue = b[cellNumber].codeValue & ~(1 << (b[cellNumber].codeLen - 1));
    b[cellNumber].codeValue = b[cellNumber].codeValue | (b[cellNumber].toggle << (b[cellNumber].codeLen - 1));
    if (b[cellNumber].codeType == RC5) {
      Serial.print("Sent RC5 ");
      Serial.println(b[cellNumber].codeValue, HEX);
      irsend.sendRC5(b[cellNumber].codeValue,b[cellNumber].codeLen);
    } 
    else {
      irsend.sendRC6(b[cellNumber].codeValue, b[cellNumber].codeLen);
      Serial.print("Sent RC6 ");
      Serial.println(b[cellNumber].codeValue, HEX);
    }
  } 
  else if (b[cellNumber].codeType == UNKNOWN /* i.e. raw */) {
    // sending 56khz to b[cellNumber] the module 
   
    irsend.sendRaw(b[cellNumber].rawCodes, b[cellNumber].codeLen, 38);
    Serial.println("Sent raw");
    for (int i = 1; i <= b[cellNumber].codeLen; i++) {
      Serial.print(b[cellNumber].rawCodes[i-1],DEC);
      
      if(i%2){
      Serial.print('m');
      }
      else{
      Serial.print('s');
      }
      Serial.print(" ");
    }
    }
}

void EEPROMstore(int EEPROMcell){
  int eeAddress=1;
 int fetchCycle=1;
 for(;fetchCycle<EEPROMcell;eeAddress+=sizeof(buttonFunction),++fetchCycle){
 }
 EEPROM.put(eeAddress,mButton);
 
 b[EEPROMcell-1].LED=GREEN;
 Serial.println(b[EEPROMcell-1].LED,DEC);
 // case statements decide where the statements will go*
 // EEPROM.put(adrressOne,mbutton);*
 //Led logic to set to turn the lights green(store) from orange 
}

void EEPROMfetch(int EEPROMcell){
  int eeAddress=1;
  int fetchCycle=1;
 for(;fetchCycle<EEPROMcell;eeAddress+=sizeof(buttonFunction),++fetchCycle){
 }

Serial.println(eeAddress,DEC);
 EEPROM.get(eeAddress,b[EEPROMcell-1]);
 if(b[ONE-1].LED == GREEN)
  ledButton1.setColor(GREEN);

  else
  ledButton1.setColor(YELLOW);

  if(b[TWO-1].LED == GREEN)
  ledButton2.setColor(GREEN);

  else
  ledButton2.setColor(RE);
  
  if(b[THREE-1].LED == GREEN)
  ledButton3.setColor(GREEN);

  else
  ledButton3.setColor(RE);
  
  if(b[FOUR-1].LED == GREEN)
  ledButton4.setColor(GREEN);

  else
  ledButton4.setColor(RE);

  
  
  
 
 Serial.print("I am in fetch\n");
  
}



int lastButtonState;
int lastButtonState1;
int lastButtonState2;
int lastButtonState3;
int lastButtonState4;

void progButton();
void firstButton();
int buttonState;

void loop() {
 
  ledProgram.drive();
  ledButton1.drive();
  ledButton2.drive();
   ledButton3.drive();
    ledButton4.drive();

   if (irrecv.decode(&results)) {
    digitalWrite(STATUS_PIN, HIGH);
    storeCode(&results);
    irrecv.resume();// resume receiver
    digitalWrite(STATUS_PIN, LOW);    
  } 
//Serial.println(STATE,DEC);
switch(STATE){

    case RECORD:
    Serial.println("NOw in record");

    if(b[ONE-1].LED==GREEN)
       ledButton1.setColor(GREEN);
      // else if(b[ONE-1].LED==RE)
      
       else if (b[TWO-1].LED==GREEN)
       ledButton2.setColor(GREEN);

       else if (b[THREE-1].LED==GREEN)
       ledButton3.setColor(GREEN);

       else if (b[FOUR-1].LED==GREEN)
       ledButton4.setColor(GREEN);

        else {
       ledButton1.setColor(RE);
       ledButton2.setColor(RE);
       ledButton3.setColor(RE);
       ledButton4.setColor(RE);     
        }      
    //led sequence to record
   // ledProgram.setColor(2);
    STATE=TRANSFER;
    break;

    case TRANSFER:
    //led glow sequence to acknowledge what is free and stored  etc.
   // progButton();
   
 // If button pressed, send the code.
   buttonState = digitalRead(PROGRAM_BUTTON);
  if (lastButtonState == HIGH && buttonState == LOW) {
    Serial.println("Released");
    irrecv.enableIRIn(); // Re-enable receiver
  }

  else if (buttonState) {
    Serial.println("Pressed program button, sending");
    digitalWrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState == buttonState);
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions
  } 
  
   
  lastButtonState = buttonState;
  
     if(digitalRead(BUTTON_ONE)==HIGH){
      //transfer sequence: write this to EEPROM cell
      Serial.println("IN state transfer if loop");
      EEPROMstore(ONE);
      delay(500);
      EEPROMfetch(ONE);
      ledButton1.setColor(GREEN);
      ledButton1.setBlinkSpeed(100, 400);
      
      ledProgram.setColor(BLACK);
      
      //delay(100);
     // ledProgram.setColor(RE);
     
      STATE=PLAYBACK;
     }

     if(digitalRead(BUTTON_TWO)==HIGH){
      //transfer sequence: write this to EEPROM cell
      Serial.println("IN state transfer if loop");
      EEPROMstore(TWO);
      delay(500);
      EEPROMfetch(TWO);
      //ledButton[TWO-1].setColor(GREEN);
       ledButton2.setColor(GREEN);
      ledButton2.setBlinkSpeed(100, 400);
      ledProgram.setColor(BLACK);
      
      //delay(100);
     // ledProgram.setColor(RE);
     
      STATE=PLAYBACK;
     }

if(digitalRead(BUTTON_THREE)==HIGH){
      //transfer sequence: write this to EEPROM cell
      Serial.println("IN state transfer if loop");
      EEPROMstore(THREE);
      delay(500);
      EEPROMfetch(THREE);
      //ledButton[THREE-1].setColor(GREEN);
      ledButton3.setColor(GREEN);
      ledButton3.setBlinkSpeed(100, 400);
      ledProgram.setColor(BLACK);
      
      //delay(100);
     // ledProgram.setColor(RE);
     
      STATE=PLAYBACK;
     }

if(digitalRead(BUTTON_FOUR)==HIGH){
      //transfer sequence: write this to EEPROM cell
      Serial.println("IN state transfer if loop");
      EEPROMstore(FOUR);
      delay(500);
      EEPROMfetch(FOUR);
       ledButton4.setColor(GREEN);
      ledButton4.setBlinkSpeed(100, 400);
      ledProgram.setColor(BLACK);
      
      //delay(100);
     // ledProgram.setColor(RE);
     
      STATE=PLAYBACK;
     }
    
    break;
    
    case PLAYBACK:
    if(digitalRead(PROGRAM_BUTTON)==HIGH){
      timerStart=millis();
     //ledProgram.setColor(BLACK, RE);
    // ledProgram.setBlinkSpeed(450, 50);
      while(digitalRead(PROGRAM_BUTTON)==HIGH){}
     // ledProgram.setBlinkSpeed(1);
      
      timerEnd=millis();
      if(timerEnd - timerStart > 2000){
        STATE=RECORD;
        ledProgram.setColor(BLACK,RE);
     ledProgram.setBlinkSpeed(450, 50);
     
     
        //ledProgram.setBlinkSpeed(0,0);
        Serial.println("Change to record now");
      }
    }
  firstButton();  
break;
  }
//  // If button pressed, send the code.
//  int buttonState = digitalRead(PROGRAM_BUTTON);
//  int buttonOne= digitalRead(BUTTON_ONE);
//  if (lastButtonState == HIGH && buttonState == LOW) {
//    Serial.println("Released");
//    irrecv.enableIRIn(); // Re-enable receiver
//  // irrecvFS.enableIRIn();
//  }
//  else if (lastButtonState1 == HIGH && buttonOne == LOW){
//    
//    Serial.println("Released");
//    irrecv.enableIRIn(); // Re-enable receiver
//  
//  }
//  
//
//  if (buttonState) {
//    Serial.println("Pressed, sending");
//    digitalWrite(STATUS_PIN, HIGH);
//    sendCode(lastButtonState == buttonState);
//    digitalWrite(STATUS_PIN, LOW);
//    delay(50); // Wait a bit between retransmissions
//  } 
//
//  else if(buttonOne){
//    Serial.println("Pressed button 1, sending");
//    digitalWrite(STATUS_PIN, HIGH);
//    sendCode(lastButtonState1 == buttonOne,ONE);
//    digitalWrite(STATUS_PIN, LOW);
//    delay(50); // Wait a bit between retransmissions
//  }
//  
//  else if (irrecv.decode(&results)) {
//    digitalWrite(STATUS_PIN, HIGH);
//    storeCode(&results);
//    irrecv.resume();// resume receiver
//    digitalWrite(STATUS_PIN, LOW);
//  }
//  /* else if(irrecvFS.decode(&results)){
//    digitalWrite(STATUS_PIN, HIGH);
//    storeCode(&results);
//    irrecvFS.resume();// resume receiver
//    digitalWrite(STATUS_PIN, LOW);
//  } */
//  lastButtonState = buttonState;
//  lastButtonState1=buttonState;
//  
}




void firstButton(){

//    ledButton1.setBlinkSpeed(BLACK);
//    ledButton1.setColor(GREEN);
//    
//    ledButton4.setBlinkSpeed(BLACK);
//    ledButton4.setColor(GREEN);
//    
//    ledButton4.setBlinkSpeed(BLACK);
//    ledButton4.setColor(GREEN);
//    
//    ledButton4.setBlinkSpeed(BLACK);
//    ledButton4.setColor(GREEN);
  

  // If button pressed, send the code.
  int buttonOne= digitalRead(BUTTON_ONE);
  int buttonTwo= digitalRead(BUTTON_TWO);
  int buttonThree= digitalRead(BUTTON_THREE);
  int buttonFour= digitalRead(BUTTON_FOUR);
  
   if (lastButtonState1 == HIGH && buttonOne == LOW){
    
    ledButton1.setBlinkSpeed(BLACK);
    Serial.println("Released 1");
    irrecv.enableIRIn(); // Re-enable receiver
  
  }

  else if (lastButtonState2 == HIGH && buttonTwo == LOW){
    
     ledButton2.setBlinkSpeed(BLACK);
    Serial.println("Released 2");
    irrecv.enableIRIn(); // Re-enable receiver
  
  }

  else if (lastButtonState3 == HIGH && buttonThree == LOW){
    
     ledButton3.setBlinkSpeed(BLACK);
    Serial.println("Released 3");
    irrecv.enableIRIn(); // Re-enable receiver
  
  }

  else if (lastButtonState4 == HIGH && buttonFour == LOW){
    
    ledButton4.setBlinkSpeed(BLACK);
    Serial.println("Released 4");
    irrecv.enableIRIn(); // Re-enable receiver
  
  }
  
   else if(buttonOne){
    
    Serial.println("Pressed button first, sending");
    digitalWrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState1 == buttonOne,ONE-1);
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions
  }

else if(buttonTwo){
 
    Serial.println("Pressed button two, sending");
    digitalWrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState2 == buttonTwo,TWO-1);
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions
  }

  else if(buttonThree){
    
    Serial.println("Pressed button third, sending");
    digitalWrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState3 == buttonThree,THREE-1);
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions
  }

  else if(buttonFour){
    
    Serial.println("Pressed button fourth, sending");
    digitalWrite(STATUS_PIN, HIGH);
    sendCode(lastButtonState4 == buttonFour,FOUR-1);
    digitalWrite(STATUS_PIN, LOW);
    delay(50); // Wait a bit between retransmissions
  }

  

  lastButtonState1=buttonOne;
  lastButtonState2=buttonTwo;
  lastButtonState3=buttonThree;
  lastButtonState4=buttonFour;
  
}

