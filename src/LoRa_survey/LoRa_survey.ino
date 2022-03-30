  #include <Adafruit_GPS.h>
  #include <SoftwareSerial.h>
  #include <Wire.h>
  #include "arduinoUtils.h"

  // Include the SX1272 and SPI library:
  #include "sx1272_INSAT.h"
  #include <SPI.h>

  #define freq_centrale CH_868v1
  #define BW BW_125
  #define CR CR_5
  #define SF SF_12
  #define OutPower POW_14
  #define PreambLong 12
  #define RX_Addr 8
  #define MaxNbRetries 3
  #define WaitRxMax 7000 //en ms

  #define mySerial Serial1
  Adafruit_GPS GPS(&mySerial);

  //période de rafraichissement des données GPS
  #define PeriodUpdateGPS 2000 //en ms


  // Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
  // Set to 'true' if you want to debug and listen to the raw GPS sentences. 
  //#define GPSECHO  true
  #define GPSECHO  false //pour ne pas avoir les données brutes

  // this keeps track of whether we're using the interrupt
  // off by default!
  boolean usingInterrupt = true;
  void useInterrupt(boolean); // Func prototype keeps Arduino 0023 happy


  uint8_t rx_address = RX_Addr;

  // status variables
  int8_t e;
  boolean ConfigOK = true; //passe à false si problème d'allumage, de config de la fréquence ou de la puissance de sortie



  void setup()  
  {
      
    // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
    // also spit it out
    Serial.begin(115200);
    //pas de serial.print en dehors des datas pour le csv!

    // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
    GPS.begin(9600);
    
    // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    
    // Set the update rate
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
    // For the parsing code to work nicely and have time to sort thru the data, and
    // print it out we don't suggest using anything higher than 1 Hz

    // Request updates on antenna status, comment out to keep quiet
    GPS.sendCommand(PGCMD_ANTENNA);

    // autorisation des interruptions.
    useInterrupt(true);

    delay(1000);
    // Ask for firmware version




    // Power ON the module
    e = sx1272.ON();
    if (e != 0)
    {
      ConfigOK = false;
    }

    // Select frequency channel
    e = sx1272.setChannel(freq_centrale);
    if (e != 0)
    {
      ConfigOK = false;
    }

    // Select output power
    e = sx1272.setPower(OutPower);
    if (e != 0)
    {
      ConfigOK = false;
    }
    
    if (ConfigOK == true) {
      // Set header
      e = sx1272.setHeaderON();
      // Set transmission mode
      e = sx1272.setCR(CR_5);    // CR = 4/5
      e = sx1272.setSF(SF_12);   // SF = 12
      e = sx1272.setBW(BW_125);    // BW = 125 KHz
      // Set CRC
      e = sx1272.setCRC_ON();
      // Set the node address
      e = sx1272.setNodeAddress(rx_address);
      // Set the length of preamble
      e = sx1272.setPreambleLength(PreambLong);
      // Set the number of transmission retries
      sx1272._maxRetries = MaxNbRetries; 

    }
    else
    {
      exit(-2); 
    }

  }


  // Interrupt is called once a millisecond, looks for any new GPS data, and stores it
  SIGNAL(TIMER0_COMPA_vect) {
    char c = GPS.read();
    // if you want to debug, this is a good time to do it!
  }

  void useInterrupt(boolean v) {
    if (v) {
      // Timer0 is already used for millis() - we'll just interrupt somewhere
      // in the middle and call the "Compare A" function above
      OCR0A = 0xAF;
      TIMSK0 |= _BV(OCIE0A);
      usingInterrupt = true;
    } else {
      // do not call the interrupt function COMPA anymore
      TIMSK0 &= ~_BV(OCIE0A);
      usingInterrupt = false;
    }
  }

  uint32_t timer = millis();
  void loop()                     // run over and over again
  {
        
    if (ConfigOK == true) {
      e = sx1272.receivePacketTimeout(WaitRxMax);
    }
        // in case you are not using the interrupt above, you'll
    // need to 'hand query' the GPS, not suggested :(
    if (! usingInterrupt) {
      // read data from the GPS in the 'main loop'
      char c = GPS.read();
      // if you want to debug, this is a good time to do it!
      if (GPSECHO)
        if (c) Serial.print(c);
    }

    
    // if a sentence is received, we can check the checksum, parse it...
    if (GPS.newNMEAreceived()) {
      // a tricky thing here is if we print the NMEA sentence, or data
      // we end up not listening and catching other sentences! 
      // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
      //Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false

    
      if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
        return;  // we can fail to parse a sentence in which case we should just wait for another
    }

    // if millis() or timer wraps around, we'll just reset it
    if (timer > millis())  timer = millis();


    // approximately every PeriodUpdateGPS ms or so, print out the current stats
    if (millis() - timer > PeriodUpdateGPS) { 
      timer = millis(); // reset the timer
          
     if (GPS.fix) {
        Serial.print(GPS.latitudeDegrees, 4);
        Serial.print(", "); 
        Serial.print(GPS.longitudeDegrees, 4);
        Serial.print(", "); 
        // RSSI, RSSIpacket, SNR, IDsource
        e = sx1272.getRSSI();
        Serial.print(sx1272._RSSI, DEC); 
        Serial.print(F(", "));
        e = sx1272.getRSSIpacket();
        Serial.print(sx1272._RSSIpacket, DEC); 
        Serial.print(F(", "));
        e = sx1272.getSNR();
        Serial.print(sx1272._SNR, DEC); 
        Serial.print(F(", "));
        Serial.println(sx1272.packet_received.src,DEC);
        }else{          
          Serial.println(F("0.0000, 0.0000, 0, 0, 0, 0"));
        }
      }
    }
  
