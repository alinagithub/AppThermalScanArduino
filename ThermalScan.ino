
#include <Wire.h>
#include <Servo.h>
#include <SoftwareSerial.h>

#include <Adafruit_MLX90614.h> // see https://github.com/adafruit/Adafruit-MLX90614-Library    
#include <libserlcdarduino.h>  // see https://github.com/alinagithub/LibSerLcdArduino
#include <LibComArduino.h>     // see https://github.com/alinagithub/LibComArduino
#include <MemoryFree.h>        // see https://github.com/maniacbug/MemoryFree

// CABLING
const int PinLcd   = 7;
const int PinYaw   = 9;
const int PinPitch = 10;

// DECLARE OBJETCS
SerLcd16x2Arduino LcdObj(PinLcd); // 16x2 LCD
Adafruit_MLX90614 ThermObj;       // Temperature sensor
ComArduino        ComObj;         // Communication obj
Servo             ServoY;         // Yaw servo
Servo             ServoP;         // Pitch servo

// DECLARE GLOBAL VARIABLES
String  Text;
boolean Connected  = false;
boolean ParamErr   = false;
boolean EoScan     = false;
int     ScanSizeX  = 10;
int     ScanSizeY  = 10;
int     YawStart   = -45;
int     YawEnd     = +45;
int     PitchStart = -20;
int     PitchEnd   = +20;

// FUNCTIONS
void DisplayMemUsage(int delayval)
   {
   const int mem_total = 2048;
   int mem_usage = int((mem_total-freeMemory())*100.0/float(mem_total));
   Text = "(MEM::"; Text += mem_usage; Text += "%)";
   LcdObj.Print(1,7,Text);
   delay(delayval);
   LcdObj.ClearRange(1,7,10);
   }
   
void DisplayMeasure(int Yaw, int Pitch, float Val)
   {
   Text = "Y:"; 
   Text += Yaw;
   Text += " P:"; 
   Text += Pitch;
   LcdObj.Print(1,7,Text);
  
   Text = ""; 
   Text += Val; 
   Text += "_C";
   LcdObj.Print(2,7,Text);
   }

void ReadParam(int& Param, String Name)
   {
   int Value;
   String Text(".");
   Text = Name; Text+="...";
   LcdObj.ClearLine(2);
   LcdObj.Print(2,1,Text);
   Text = Name; Text+="::";
   
   if(ComObj.ReadInt(&Value, true))
      {
      Param=Value;
      Text+=Param;
      }
   else
      {
      Text+="FAIL!";
      ParamErr=true;
      }

   LcdObj.Print(2,1,Text);
   delay(1000);
   }

void MoveServo(Servo& servo, int deg, int iter=10, int delval=20)
   {
   for(int i=0; i<iter; i++)
      {
      servo.write(deg+90);
      delay(delval);
      }
   }

void setup()
   { 
   // Start sensor
   ThermObj.begin();
   
   // Init Lcd and splash screen
   LcdObj.Clear();
   LcdObj.SetBrightness(50);
   LcdObj.SetDisplay(true, false, false);
   LcdObj.Print(1,1,"--THERMAL SCAN--", 16);
  
   // Init com. and connect to host
   LcdObj.Print(2,1,".Connection...  ", 16);
   Serial.begin(9600);
   ComObj.Lcd = &LcdObj;
   ComObj.verbose=false;
   Connected = ComObj.Open(true);
  
   if(Connected)
      {
      LcdObj.Print(2,1,".Connection...OK", 16);
      delay(3000);
     
      // Receiving parameters from host
      LcdObj.Print(2,1,".Parameters...  ", 16);
      delay(3000);
      
      ReadParam(ScanSizeX,  "ScanSx");
      ReadParam(ScanSizeY,  "ScanSy");
      ReadParam(YawStart,   "YawS");  
      ReadParam(YawEnd,     "YawE");  
      ReadParam(PitchStart, "PitchS");
      ReadParam(PitchEnd,   "PitchE");
      
      if(!ParamErr) 
         LcdObj.Print(2,1,".Parameters...OK", 16);
      else
         LcdObj.Print(2,1,".Parameters...NO", 16);
      delay(2000);
      
      // Init servo
      LcdObj.Print(2,1,".Test servo...     ", 16);
       
      ServoY.attach(PinYaw, 544, 2400);
      ServoP.attach(PinPitch, 544, 2400);
      delay(200);
       
      for(int i =YawStart; i<YawEnd; i++)
         MoveServo(ServoY, i, 1);
          
      for(int i =PitchStart; i<PitchEnd; i++)
         MoveServo(ServoP, i, 1);
       
      MoveServo(ServoY, 0, 10);
      MoveServo(ServoP, 0, 10);
       
      ServoY.detach();
      ServoP.detach();
       
      LcdObj.Print(2,1,".Test servo...   OK", 16);
      delay(3000);
     
      // Display scan layout
      LcdObj.Clear();
      LcdObj.Print(1,1,"Scan:",5);
      LcdObj.Print(2,1,"Meas:",5);
      }
   else
      {
      LcdObj.Print(2,1,".Connection...NO", 16);
      }
    }

// MAIN LOOP
void loop() 
   {
   if(!Connected)
      {
      LcdObj.Print(1,1,"--THERMAL SCAN--", 16);
      LcdObj.Print(2,1,".Connection...NO", 16); 
      delay(1000); return;
      }
  
   if(ParamErr)
      {
      LcdObj.Print(1,1,"--THERMAL SCAN--", 16);
      LcdObj.Print(2,1,".Parameter error", 16); 
      delay(1000); return;
      }
  
   if(EoScan)
      {
      LcdObj.Print(1,1,"--THERMAL SCAN--", 16);
      LcdObj.Print(2,1,".Scan finished! ", 16); 
      delay(1000); return;
      }
  
   String  DataTx; // Data container to transmit to host
   DataTx.reserve(ScanSizeX*6); // ScanSizeX x "XX.XX "
  
   // Init servo positions
   ServoP.attach(PinPitch,  544, 2400);
   ServoY.attach(PinYaw,  544, 2400);
   delay(100);
   MoveServo(ServoP, PitchStart, 10); 
   MoveServo(ServoY, YawStart, 10);   
   ServoP.detach();
   ServoY.detach();
   delay(20);
      
   for(int j=0; j<ScanSizeY; j++)
      {
      // Display memory usage %
      LcdObj.ClearRange(1,7,10);
      LcdObj.ClearRange(2,7,10);
      DisplayMemUsage(500);
      
      // Set Pitch
      int pitch=(int)(PitchStart+j*(PitchEnd-PitchStart)/(float)ScanSizeY);
      
      // Set servo to new scan position
      ServoP.attach(PinPitch,  544, 2400);
      ServoY.attach(PinYaw,  544, 2400);
      delay(50);
      MoveServo(ServoP, pitch, 10);
      MoveServo(ServoY, YawStart, 10);  
      ServoP.detach();
      ServoY.detach();
      delay(20);

      // Init data container
      DataTx="";
    
      for(int i=0; i<ScanSizeX; i++)
         {
         // Inc. scanning servo position
         int yaw=(int)(YawStart+i*(YawEnd-YawStart)/(float)ScanSizeX);
         ServoY.attach(PinYaw,  544, 2400); delay(50);
         MoveServo(ServoY, yaw, 10);
         ServoY.detach();
      
         // Read sensor value
         float Value = ThermObj.readObjectTempC();
         
         // Accumulate data
         DataTx+=Value;
         DataTx+=" ";
         
         // Display measure info
         DisplayMeasure(i, j, Value);
         }
    
      if(ComObj.WriteString(DataTx, true))
         {
         LcdObj.Print(2,15,":)",2);
         delay(500);
         }
      }
   
   // Terminate 
   MoveServo(ServoY, 0, 20);
   MoveServo(ServoP, 0, 20);
   EoScan=true;
   }


