// Include LibAPRS
#include <LibAPRS.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x3f,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

// You must define what reference voltage the ADC
// of your device is running at. If you bought a
// MicroModem from unsigned.io, it will be running
// at 3.3v if the "hw rev" is greater than 2.0.
// This is the most common. If you build your own
// modem, you should know this value yourself :)
#define ADC_REFERENCE REF_3V3
// OR
//#define ADC_REFERENCE REF_5V

// You can also define whether your modem will be
// running with an open squelch radio:
#define OPEN_SQUELCH false

char diag_mode = 0;
 
 int rotation;  
 int value;
 int pressed = 1;
 boolean LeftRight;

// Rotary Encoder Module connections
const char PinSW=2;   // Rotary Encoder Switch
const char PinDT=11;    // DATA signal
const char PinCLK=12;    // CLOCK signal
//int displaycounter=0; // Store current counter value
//boolean refresh_display = false;


// Store previous Pins state
int PreviousCLK;   
int PreviousDATA;
int NowCLK;   
int NowDT;




int menu_level = 0;

char menu [][7] = { "Xmit", "Freq", "Mode", "Summit", "Call" };
int menu_cnt = 5;
int menu_select = 0;

char tran_select = 0;

char summit [][6] = { "Assoc", "Alph1", "Alph2", "Code", "Done" };
short summit_cnt = 5;
short summit_select = 0;

char mode [][5] = { "FM", "SSB", "CW", "AM", "DATA" };
int mode_cnt = 5;
int mode_select = 0;

char freq [][5] = { "MHz", "kHz", "Done" };
char freq_cnt = 3;
char freq_select = 0;

short mhz [] = { 
  1,
  3,
  7,
  10,
  14,
  18,
  21,
  24,
  28,
  29,
  50,
  144, 
  420 
  };
  
short mhz_plus [] = { 
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  3, 
  3, 
  29 
  };
  
short mhz_cnt = 13;
short mhz_select = 0;
short mhz_offset = 0;

const short khz_lower[] = {
  800, // 1
  500, // 3
  000, // 7
  100, // 10
  000, // 14
  68,  // 18
  000, // 21
  890, // 24
  000, // 28
  000, // 29
  000, // 50 - 53
  000, // 144 - 147
  000 // 420 - 449
};

const short khz_upper[] = {
  999,  // 1
  999,  // 3
  300,  // 7
  150,  // 10
  350,  // 14
  168,  // 18
  450,  // 21
  990,  // 24
  999,  // 28
  699,  // 29
  999,  // 50 - 53
  999,  // 144 - 147
  999   // 420 - 449
};

short khz_cnt = 48;
short khz_select = khz_lower[0];

char call [5*7];

char call_cnt = 5;
char call_select = 0;

char special[3];
char special_check[] = "123";

char szfreq[12];
char *freq_fmt = "%d.%03d";
char sota_us_assocs[][3] =
{
  {"0C"},
  {"0D"},
  {"0I"},
  {"0M"},
  {"0N"},
  {"1"},
  {"2"},
  {"3"},
  {"4A"},
  {"4C"},
  "4G",
  "4K",
  "4T",
  "4V",
  "5A",
  "5M",
  "5N",
  "5O",
  "5T",
  "6",
  "7A",
  "7I",
  "7M",
  "7N",
  "7O",
  "7U",
  "7W",
  "7Y",
  "8M",
  "8O",
  "8V",
  "9"
};

short summit_assoc_cnt=32;

short summit_assoc_select=0;
char summit_group1='A', summit_group2='A';
short summit_code=0;
char szSummit[12];
char summit_fmt[] = "W%s/%c%c-%03d";


char msg_buffer[34];





// You always need to include this function. It will
// get called by the library every time a packet is
// received, so you can process incoming packets.
//
// If you are only interested in transmitting, you
// should just leave this function empty.
// 
// IMPORTANT! This function is called from within an
// interrupt. That means that you should only do things
// here that are FAST. Don't print out info directly
// from this function, instead set a flag and print it
// from your main loop, like this:

boolean gotPacket = false;
AX25Msg incomingPacket;
uint8_t *packetData;
void aprs_msg_callback(struct AX25Msg *msg) {
  // If we already have a packet waiting to be
  // processed, we must drop the new one.
  if (!gotPacket) {
    // Set flag to indicate we got a packet
    gotPacket = true;

    // The memory referenced as *msg is volatile
    // and we need to copy all the data to a
    // local variable for later processing.
    memcpy(&incomingPacket, msg, sizeof(AX25Msg));

    // We need to allocate a new buffer for the
    // data payload of the packet. First we check
    // if there is enough free RAM.
    if (freeMemory() > msg->len) {
      packetData = (uint8_t*)malloc(msg->len);
      memcpy(packetData, msg->info, msg->len);
      incomingPacket.info = packetData;
    } else {
      // We did not have enough free RAM to receive
      // this packet, so we drop it.
      gotPacket = false;
    }
  }
}

void setup() {
  // Set up serial port

    lcd.init();                      // initialize the lcd 
    //lcd.init();                      // initialize the lcd 
    lcd.backlight();
    lcd.clear();


  //lcd.setCursor(1,0);
  //lcd.print("hello everyone");
  //lcd.setCursor(1,1);
  //lcd.print("konichiwaa");

   pinMode (PinCLK,INPUT);
   pinMode (PinDT,INPUT);
   pinMode (PinSW,INPUT_PULLUP);

   
  // Initialise APRS library - This starts the modem
  APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
  
  // You must at a minimum configure your callsign and SSID
  //APRS_setCallsign("K4SDM", 0);
  
  // You don't need to set the destination identifier, but
  // if you want to, this is how you do it:
////APRS_setDestination("TEST  ", 0);
  
  // Path parameters are set to sensible values by
  // default, but this is how you can configure them:
  // APRS_setPath1("WIDE1", 1);
  // APRS_setPath2("WIDE2", 2);
  
  // You can define preamble and tail like this:
  APRS_setPreamble(600);
  APRS_setTail(100);
  
  // You can use the normal or alternate symbol table:
  // APRS_useAlternateSymbolTable(false);
  
  // And set what symbol you want to use:
  // APRS_setSymbol('n');
  
  // We can print out all the settings
  APRS_printSettings();
  //Serial.print(F("Free RAM:     ")); Serial.println(freeMemory());


    //lcd_print_line("W4G/NG-010 SSB", "144.000 K4SDM" );

    lcd_print_line( "Turn dial and", "Press to select" );

    //while( check_rotary(1) == 0 )
      ;

    if( check_press() == -1 )
    {
      diag_mode = 1;
      lcd_print_line( "*", "" );
    }

    /*
    memset( call, 0, sizeof(call) );
    memcpy( &(call[0]), "K4SDM", 7 );
    memcpy( &(call[7]), "K4KHO", 7 );
    memcpy( &(call[14]), "KO4YLZ", 7 );
    memcpy( &(call[21]), "N4NFM", 7 );
    memcpy( &(call[28]), "KO4VRW", 7 );
    */
    
    strncpy( &(call[0]), "K4SDM", 7 );
    strncpy( &(call[7]), "K4KHO", 7 );
    strncpy( &(call[14]), "KO4YLZ", 7 );
    strncpy( &(call[21]), "N4NFM", 7 );
    strncpy( &(call[28]), "K4HET", 7 );

}


void messageExample( char* msg, const char* callsign ) {
  // We first need to set the message recipient
  APRS_setCallsign( callsign, 0);
//  APRS_setDestination("SOTA  ", 0);

  if( diag_mode == 1 )
    APRS_setMessageDestination("TEST  ", 0);
  else
    APRS_setMessageDestination("SOTA  ", 0);
  
  APRS_sendMsg(msg, strlen(msg));
  
}

// Here's a function to process incoming packets
// Remember to call this function often, so you
// won't miss any packets due to one already
// waiting to be processed
void processPacket() {
  if (gotPacket) {
    gotPacket = false;

    /*
    Serial.print(F("Received APRS packet. SRC: "));
    Serial.print(incomingPacket.src.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.src.ssid);
    Serial.print(F(". DST: "));
    Serial.print(incomingPacket.dst.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.dst.ssid);
    Serial.print(F(". Data: "));
    
    for (int i = 0; i < incomingPacket.len; i++) {
      Serial.write(incomingPacket.info[i]);
    }
    Serial.println("");
    */
    
    // Remeber to free memory for our buffer!
    free(packetData);

    // You can print out the amount of free
    // RAM to check you don't have any memory
    // leaks
    // Serial.print(F("Free RAM: ")); Serial.println(freeMemory());
  }
}

void eeprom_load()
{
  short eAddr = 0;
  EEPROM.get(eAddr, special); eAddr+=sizeof(special);
  EEPROM.get(eAddr, call); eAddr+=sizeof(call);
  EEPROM.get(eAddr, call_select); eAddr+=sizeof(call_select);
  EEPROM.get(eAddr, mhz_select); eAddr+=sizeof(mhz_select);
  EEPROM.get(eAddr, mhz_offset); eAddr+=sizeof(mhz_offset);
  EEPROM.get(eAddr, khz_select); eAddr+=sizeof(khz_select);
  EEPROM.get(eAddr, summit_assoc_select); eAddr+=sizeof(summit_assoc_select);
  EEPROM.get(eAddr, summit_group1); eAddr+=sizeof(summit_group1);
  EEPROM.get(eAddr, summit_group2); eAddr+=sizeof(summit_group2);
  EEPROM.get(eAddr, summit_code); eAddr+=sizeof(summit_code);  
}

void eeprom_store()
{
  short eAddr = 0;
  EEPROM.put(eAddr, special_check); eAddr+=sizeof(special);
  EEPROM.put(eAddr, call); eAddr+=sizeof(call);
  EEPROM.put(eAddr, call_select); eAddr+=sizeof(call_select);
  EEPROM.put(eAddr, mhz_select); eAddr+=sizeof(mhz_select);
  EEPROM.put(eAddr, mhz_offset); eAddr+=sizeof(mhz_offset);
  EEPROM.put(eAddr, khz_select); eAddr+=sizeof(khz_select);
  EEPROM.put(eAddr, summit_assoc_select); eAddr+=sizeof(summit_assoc_select);
  EEPROM.put(eAddr, summit_group1); eAddr+=sizeof(summit_group1);
  EEPROM.put(eAddr, summit_group2); eAddr+=sizeof(summit_group2);
  EEPROM.put(eAddr, summit_code); eAddr+=sizeof(summit_code);  
}

void system_init()
{

  memset( special, 0, sizeof(special) );
  memcpy( special, special_check, sizeof(special) );


  memset( call, 0, sizeof(call) );
  strncpy( &(call[0]), "K4SDM", 7 );
  strncpy( &(call[7]), "K4KHO", 7 );
  strncpy( &(call[14]), "KO4YLZ", 7 );
  strncpy( &(call[21]), "N4NFM", 7 );
  strncpy( &(call[28]), "K4HET", 7 );

  
  call_select = 0;
  mhz_select = 0;
  mhz_offset = 0;
  khz_select = 0;
  summit_assoc_select = 0;
  summit_group1 = 'A';
  summit_group2 = 'A';
  summit_code = 0;
}

void loop() {

  //if ((millis() - TimeOfLastDebounce) > DelayofDebounce) {
    
    NowCLK=digitalRead(PinCLK);
    NowDT=digitalRead(PinDT);

    if( NowCLK == 0 )
    {
        //diag_mode = -1;
        
        short ret = check_rotary(10);  // Rotary Encoder check routine below


        if( memcmp( special, special_check, sizeof(special) ) != 0 )
        {
          system_init();
          eeprom_load();
          
          if( memcmp( special, special_check, sizeof(special) ) != 0 )
          {
            system_init();
            eeprom_store();
          }
        }


        if( ret != 0 )
        {
           if( menu_level == 0 ) // Top level menu
           {
             menu_select += ret;
             if( menu_select < 0 )
              menu_select = menu_cnt-1;
              
             menu_select %= menu_cnt;
             if( 1 )
             { 
                lcd_print_line( "Main Menu", menu[menu_select] );
             }
           }
           else if( menu_level == 1 ) // Freq select
           {
             freq_select += ret;
             if( freq_select < 0 )
              freq_select = freq_cnt-1;
              
             freq_select %= freq_cnt;
             if( 1 )
             { 
                lcd_print_line( menu[menu_select], freq[freq_select] );
             }
           }
           else if( menu_level == 2 ) // Freq set
           {
             if( freq_select == 0 ) // MHz
             {

               if( mhz_select >= 0 && mhz_select < mhz_cnt )
               {
                  if( ret > 0 )
                    mhz_offset++;
                  if( ret < 0 )
                    mhz_offset--;

                  if( mhz_offset > mhz_plus[mhz_select] )
                  {
                      mhz_offset = 0;
                      mhz_select++;               
                  } 
                  else if( mhz_offset < 0 )
                  {
                      mhz_offset = 0;
                      mhz_select--;               
                  } 
               }

               
               if( mhz_select < 0 )
               {
                mhz_select = mhz_cnt-1;
                mhz_offset = mhz_plus[mhz_select]; 
               }
               else if( mhz_select >= mhz_cnt )
               {
                mhz_select = 0;
                mhz_offset = 0; 
               }
               
               khz_select = khz_lower[mhz_select];
               
               //mhz_select %= mhz_cnt;
             }
             else if( freq_select == 1 ) // KHz
             {
               khz_select += ret;
               if( khz_select < khz_lower[mhz_select] )
                khz_select = khz_upper[mhz_select];
               else if( khz_select > khz_upper[mhz_select] )
                khz_select = khz_lower[mhz_select];
             }
             
             if( 1 )
             {  
                sprintf( szfreq, freq_fmt, mhz[mhz_select]+mhz_offset, khz_select );
                lcd_print_line( freq[freq_select], szfreq );
             }
           }
           else if( menu_level == 3 ) // Mode select
           {
             mode_select += ret;
             if( mode_select < 0 )
              mode_select = mode_cnt-1;
              
             mode_select %= mode_cnt;
             
             if( 1 )
             { 
                lcd_print_line( menu[menu_select], mode[mode_select] );
             }
           }
           else if( menu_level == 4 ) // Summit select
           {
             summit_select += ret;
             if( summit_select < 0 )
              summit_select = summit_cnt-1;
              
             summit_select %= summit_cnt;
             if( 1 )
             { 
                lcd_print_line( menu[menu_select], summit[summit_select] );
             }
           }
           else if( menu_level == 5 ) // Summit set
           {
             if( summit_select == 0 ) // Summit Assoc
             {
               summit_assoc_select += ret;
               
               if( summit_assoc_select < 0 )
                 summit_assoc_select = summit_assoc_cnt-1;
                
               summit_assoc_select %= summit_assoc_cnt;
             }
             else if( summit_select == 1 ) // Group part 1
             {
               summit_group1 += ret;
               if( summit_group1 < '0' )
                summit_group1 = 'Z';
               else if( summit_group1 > '9' && summit_group1 < 'A' )
                summit_group1 = 'A';
               else if( summit_group1 > '9' && summit_group1 > 'Z' )
                summit_group1 = '0';
             }
             else if( summit_select == 2 ) // Group part 2
             {
               summit_group2 += ret;
               if( summit_group2 < '0' )
                summit_group2 = 'Z';
               else if( summit_group2 > '9' && summit_group2 < 'A' )
                summit_group2 = 'A';
               else if( summit_group1 > '9' && summit_group2 > 'Z' )
                summit_group2 = '0';
             }
             else if( summit_select == 3 ) // Summit Code
             {
               summit_code += ret;
               if( summit_code < 0 )
                summit_code = 999;
               else if( summit_code > 999 )
                summit_code = 0;
             }
             
             if( 1 )
             {  
                sprintf( szSummit, summit_fmt, sota_us_assocs[summit_assoc_select], summit_group1, summit_group2, summit_code );
                lcd_print_line( menu[menu_select], szSummit );
             }

           }
           else if( menu_level == 6 ) // Xmit select
           {
             tran_select += ret;
             if( tran_select < 0 )
              tran_select = 1;
             else if( tran_select > 1 )
              tran_select = 0;
              
              
             // summit_select %= 2;
             if( tran_select == 0 )
             { 
                sprintf( szfreq, freq_fmt, mhz[mhz_select]+mhz_offset, khz_select );
                sprintf( szSummit, summit_fmt, sota_us_assocs[summit_assoc_select], summit_group1, summit_group2, summit_code );
                sprintf( &msg_buffer[0], "%s %s", szSummit, mode[mode_select] );
                sprintf( &msg_buffer[17], "%s %s", szfreq, &(call[7*call_select]) );
                lcd_print_line( &msg_buffer[0], &msg_buffer[17] );
             }
             else if(tran_select == 1 )
             {
                lcd_print_line( menu[menu_select], "Done" );
             }
           }
           else if( menu_level == 7 ) // Call select
           {
             call_select += ret;
             if( call_select < 0 )
              call_select = call_cnt-1;
              
             call_select %= call_cnt;
             
             if( 1 )
             { 
                lcd_print_line( menu[menu_select], &(call[7*call_select]) );
             }
           }
            
        }
        
    }
    PreviousCLK = NowCLK;
 
    
  // Check if Rotary Encoder switch was pressed
  char ret = check_press();
  if ( ret != 0 ) {

      
    if( menu_level == 0 )
    {
      if( menu_select == 0 ) // Transmit
      {
        menu_level = 6; // Transmit select
        lcd_print_line( menu[menu_select], "Xmit" );
      }
      else if( menu_select == 1 ) // Frequency
      {
        menu_level = 1; // Frequency select
        lcd_print_line( menu[menu_select], freq[freq_select] );
      }
      else if( menu_select == 2 ) // Mode
      {
        menu_level = 3; // Mode select
        lcd_print_line( menu[menu_select], mode[mode_select] );
      }
      else if( menu_select == 3 ) // Summit
      {
        menu_level = 4; // Summit select
        lcd_print_line( menu[menu_select], summit[summit_select] );
      }
      else if( menu_select == 4 ) // Call
      {
        menu_level = 7; // Call select
        lcd_print_line( menu[menu_select], &(call[7*call_select]) );
      }
    }
    else if( menu_level == 1 ) // Frequency select
    {
      if( freq_select == 0 ) // MHz
      {
        menu_level = 2; // Frequency set
        sprintf( szfreq, freq_fmt, mhz[mhz_select], khz_select );
        lcd_print_line( freq[freq_select], szfreq );
      }
      else if( freq_select == 1 ) // kHz
      {
        menu_level = 2; // Frequency set
        sprintf( szfreq, freq_fmt, mhz[mhz_select], khz_select );
        lcd_print_line( freq[freq_select], szfreq );
      }
      else if( freq_select == 2 ) // Done
      {
        menu_level = 0; // Main Menu
        lcd_print_line( "Main", menu[menu_select] );
      }
    }
    else if( menu_level == 2 ) // Frequency set
    {
      if( freq_select == 0 ) // MHz
      {
        menu_level = 1; // Frequency Select
      }
      else if( freq_select == 1 ) // KHz
      {
        menu_level = 1; // Frequency Select
      }
      sprintf( szfreq, freq_fmt, mhz[mhz_select], khz_select );
      lcd_print_line( menu[menu_select], freq[freq_select] );
    }
    else if( menu_level == 3 ) // Mode select
    {
        menu_level = 0; // Main Menu
        lcd_print_line( menu[menu_select], mode[mode_select] );
    }
    else if( menu_level == 4 ) // Summit select
    {
        menu_level = 5; // Summit set
        lcd_print_line( menu[menu_select], summit[summit_select] );
        //sprintf( szSummit, summit_fmt, sota_us_assocs[summit_assoc_select], summit_group1, summit_group2, summit_code );
        //lcd_print_line( menu[menu_select], szSummit );
    }
    else if( menu_level == 5 ) // Summit set
    {
      if( summit_select == 0 ) // Summit Association
      {
        menu_level = 4; // Summit Assoc set
      }
      else if( summit_select == 1 ) // Summit Group1
      {
        menu_level = 4; // Frequency set
      }
      else if( summit_select == 2 ) // Summit Group2
      {
        menu_level = 4; // Frequency set
      }
      else if( summit_select == 3 ) // Summit Code
      {
        menu_level = 4; // Frequency set
      }
      else if( summit_select == 4 ) // Summit Done
      {
        menu_level = 0; // Main Menu
        //lcd_print_line( "Main", menu[menu_select] );
      }
    }
    else if( menu_level == 6 ) // Xmit
    {
      if( tran_select == 0 )  
      {

        if( ret < 0 )
        {
          eeprom_store();
          sprintf( szfreq, freq_fmt, mhz[mhz_select]+mhz_offset, khz_select );
          sprintf( szSummit, summit_fmt, sota_us_assocs[summit_assoc_select], summit_group1, summit_group2, summit_code );
          sprintf( msg_buffer, "%s %s %s %s", szSummit, &(call[7*call_select]), szfreq, mode[mode_select] );
          messageExample(msg_buffer, &(call[7*call_select]));
          delay(500);
          processPacket();
        }
      }
      else if( tran_select == 1 )
      {
        menu_level = 0; // Main Menu
        lcd_print_line( "Main", menu[menu_select] );
      }
        
    }
    else if( menu_level == 7 ) // Mode select
    {
        menu_level = 0; // Main Menu
        lcd_print_line( menu[menu_select], &(call[7*call_select]) );
    }
  }

  
}

void lcd_print_line( char* title, const char* value )
{
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(title);
        lcd.setCursor(0,1);
        lcd.print(value);
  
}

void lcd_print_line( char* title, long value )
{
        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print(title);
        lcd.setCursor(1,1);
        lcd.print(value);
  
}

long last_press = millis();

char check_press()
{
  if (digitalRead(PinSW) == LOW)
  {
    long start_press = millis();
    while( digitalRead(PinSW) == LOW )
      ;
    long last_press = millis();
    //long press_time = last_press - start_press;
    
    if( (last_press - start_press) > 1000 && (last_press - start_press) < 10000 )
      return -1;
    return 1;
  }
  return 0;
}



long last_turn = millis();

int check_rotary( int step_value ) {
    
    int turn_step = 1;

    if( step_value == 0 )
      step_value = 1;
     
    if( (NowCLK != PreviousCLK) )
    {
      int tick = millis();

      if( last_turn > tick-50 )
        turn_step = step_value*2;
      else if( last_turn > tick-80 )
        turn_step = step_value;
      
      last_turn = tick;
      if ( NowDT == 0 ) {
        return -turn_step;
      }
      else if ( NowDT == 1 ) {
        return turn_step;
      }
    }    
  
    return 0;

 }
