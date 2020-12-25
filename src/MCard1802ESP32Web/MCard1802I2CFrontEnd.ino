/*
 * Support for reading and writing data to the 
 * 1802 Membership Card using MCP23008 and MCP23017 GPIO Extenders
 */
// Import required libraries
#include "Adafruit_MCP23008.h"
#include "MCP23017.h" 

//Define mask for data byte
#define BYTE_MASK  0x00FF  

//Defines for setting GPIO Control bits
#define WAIT_BIT  0x01
#define CLEAR_BIT 0x02
#define WR_BIT    0x04
#define EF4_BIT   0x08

//Define for the GPIO Control input pin
#define Q_GPIO     7

//EF2 is free
#define EF2_BIT   0x10

//Offset for Control MCP23008 GPIO Expander Address 0x20
#define MCP_CTRL  0

//Address for Data MCP23017 GPIO Expander
#define MCP_DATA 0x21

//Define MCP23008 port
Adafruit_MCP23008 mcpCtrl;

//Define MCP23017 
MCP23017 mcpData = MCP23017(MCP_DATA);


//Set up the MCP GPIO extenders to talk to the Arduino
void setupMcpCommunication() {
  //Set up MCP23017 for data lines
  mcpData.init();
  mcpData.portMode(MCP23017Port::A, 0);         //Port A as ouput
  mcpData.portMode(MCP23017Port::B, 0b11111111);//Port B as input

  //Initialize GPIO ports
  mcpData.writeRegister(MCP23017Register::GPIO_A, 0x00);
  mcpData.writeRegister(MCP23017Register::GPIO_B, 0x00);
    
  //Set up MCP23008 for control lines
  mcpCtrl.begin(MCP_CTRL);

  //Set all but last of the GPIO pins to outputs
  for (int i = 0; i < Q_GPIO; i++) {
    mcpCtrl.pinMode(i, OUTPUT);
  }//for
  //Set last GPIO pin to input
  mcpCtrl.pinMode(Q_GPIO, INPUT);
  // turn on a 100K pullup internally
  mcpCtrl.pullUp(Q_GPIO, HIGH);  
} //setupMcpCommunication

//Write control data byte to MCP23008
void writeControlData(byte ctrl_data) {
  mcpCtrl.writeGPIO(ctrl_data);
  //Wait a bit after writing
  delay(10);
} //writeControlData

//Write data to MCard1802 Data In lines
void writeDataIn(byte d_in) {
  mcpData.writeRegister(MCP23017Register::GPIO_A, d_in);
  //wait a bit after writing
  delay(10);  
} //writeDataIn

//Read data from MCard1802 Data out lines
byte readDataOut() {
  byte result = mcpData.readPort(MCP23017Port::B);
  //wait a bit after reading
  delay(10);
  return result;
}

//Read the Q bit from the MCP23008
boolean readQBit() {
  boolean result = mcpCtrl.digitalRead(Q_GPIO);
  //wait a bit after reading
  delay(10); 
  return result;
}

//Set the control flags based on button status and return data
byte getControlData(boolean input) {
  byte control_flags = 0x00;

  //Negative Logic: If run is on, set /CLEAR hgh
  if (runState) {
    control_flags |= CLEAR_BIT;
  }

  //Negative Logic: If load is off, set /WAIT high
  if (!loadState) {
    control_flags |= WAIT_BIT;    
  }

  //If M/P is not down, set the write bit on
  if (!memProtState) {
    control_flags |= WR_BIT;
  }

  //Set the /EF2 input
  if (assert_ef2) {
    //Negative Logic: /EF2 is LOW when TRUE
    control_flags &= ~EF2_BIT;
  } else {
    //Negative Logic: /EF2 is HIGH when FALSE
    control_flags |= EF2_BIT;
  } //if-else assert_ef2
    
  //Set /EF4 bit if needed (Negative Logic: 1 = not pressed)
  if (!input) {
    control_flags |= EF4_BIT;
  } //if !input
  
  return control_flags;
} //getControlData

//Character routines
// Pretty print two hex digits for display
void print2Hex(uint8_t v) {  
  //If single Hex digit
  if (v < 0x10) {
   Serial.print(F("0"));
  } // if v < 0x10
  Serial.print(v, HEX);
}

//Get the numeric value of a hexadecimal character
byte getHexValue(char d) {
  byte value = 0x00;
  // check to see what range value is in
  if (isDigit(d)) {
    value = d - '0';   
  } else {
      value = 10 + toupper(d) - 'A';
  } // if else
  return value;
} // getHexValue

//Put a hex character in the key buffer and set the data lines
void processHexChar(char h) {
  //shift previous digit into high nibble and clear rest of bits
  key_buffer = (key_buffer << 4) & HI_NIBBLE;
  key_buffer |= getHexValue(h);

  //set data value for mcp23017
  data_in = key_buffer & BYTE_MASK;
  
  #if DEBUG
    Serial.print(F("Key Buffer: "));
    print2Hex(data_in);
    Serial.println();
  #endif      
} //processHexChar

//Process a character from the keypad        
void processChar(char c) {
  #if DEBUG
    Serial.print(F("Key Input: "));
    Serial.println(c);
  #endif

  switch(c) {        
    //Input
    case 'I':
      //Set ef4 true
      input_pressed = true;
      //Stop holding input true
      hold_input = false;
      //Set timestamp for holding flag for duration of keypress
      t_input_down = millis();  
      #if DEBUG
        Serial.println(F("Input Up"));           
      #endif
    break;

    
    //Simulate holding the input button down
    case 'H':
        hold_input = true;
        #if DEBUG
          Serial.println(F("Hold Input button."));
        #endif
    break;

    //Set the control flags for Run (Go) state
    case 'G':
      runState  = true;
      loadState = false; 
    break;
    

    //Set the control flags for Load state
    case 'L':
      runState = false;
      loadState = true;
    break;

    //Set the control flags for Reset state
    case 'R':
      runState  = false;
      loadState = false;
      //Q is off and Key buffer is cleared
      old_q_bit = q_bit;
      q_bit = false;
      key_buffer = 0x00;      
    break;
    
    //Set the control flags for Wait state
    case 'W':
      runState = true;
     loadState = true;
    break;

    //Toggle the memory protect flag
    case 'M':
      if (memProtState) {
        memProtState = false;   
      } else {
        memProtState = true;
      } //if-else memProtState
    break;

    //Process Hex characters
    default:         
      if (isHexadecimalDigit(c)) {
        #if DEBUG
          Serial.println(c);                  
        #endif
        processHexChar(c);
      }
       break;
  } // switch    
} //processChar
