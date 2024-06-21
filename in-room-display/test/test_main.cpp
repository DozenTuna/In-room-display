#include <SPI.h>
#include <Arduino.h>
//自動で VSPI となる。SCLK= #18 , MOSI= #23 は自動アサイン。
const uint8_t cs_OLED = 17; //CS (Chip Select)
const uint8_t DCpin =  16; //OLED DC(Data/Command)
const uint8_t RSTpin =  4; //OLED Reset


void DataWrite(uint8_t b){  
  digitalWrite(cs_OLED, LOW);
  digitalWrite(DCpin, HIGH);//DC
  SPI.write(b);
  digitalWrite(cs_OLED, HIGH);
}
void CommandWrite(uint8_t b){  
  digitalWrite(cs_OLED, LOW);
  digitalWrite(DCpin, LOW);//DC
  SPI.write(b);
  digitalWrite(cs_OLED, HIGH);
}
void SSD1331_Init(uint8_t CS, uint8_t DC, uint8_t RST){  
  pinMode(RST, OUTPUT);
  pinMode(DC, OUTPUT);
  pinMode(CS, OUTPUT);
 
  digitalWrite(RST, HIGH);
  digitalWrite(RST, LOW);
  delay(1);
  digitalWrite(RST, HIGH);
 
  digitalWrite(CS, HIGH);
  digitalWrite(DC, HIGH);
 
  SPI.begin(); //VSPI
  SPI.setFrequency(5000000); //SSD1331 のSPI Clock Cycle Time 最低150ns
  SPI.setDataMode(SPI_MODE2); //オシロで測ると、ESP32のSPI_MODE2はMODE3だったので要注意
 
  CommandWrite(0xAE); //Set Display Off
  CommandWrite(0xA0); //Remap & Color Depth setting　
    CommandWrite(0b00110010); //A[7:6] = 00; 256 color. A[7:6] = 01; 65k color format
  CommandWrite(0xA1); //Set Display Start Line
    CommandWrite(0);
  CommandWrite(0xA2); //Set Display Offset
    CommandWrite(0);
  CommandWrite(0xA4); //Set Display Mode (Normal)
  CommandWrite(0xA8); //Set Multiplex Ratio
    CommandWrite(63); //15-63
  CommandWrite(0xAD); //Set Master Configration
    CommandWrite(0b10001110); //a[0]=0 Select external Vcc supply, a[0]=1 Reserved(reset)
  CommandWrite(0xB0); //Power Save Mode
    CommandWrite(0x1A); //0x1A Enable power save mode. 0x00 Disable
  CommandWrite(0xB1); //Phase 1 and 2 period adjustment
    CommandWrite(0x74);
  CommandWrite(0xB3); //Display Clock DIV
    CommandWrite(0xF0);
  CommandWrite(0x8A); //Pre Charge A
    CommandWrite(0x81);
  CommandWrite(0x8B); //Pre Charge B
    CommandWrite(0x82);
  CommandWrite(0x8C); //Pre Charge C
    CommandWrite(0x83);
  CommandWrite(0xBB); //Set Pre-charge level
    CommandWrite(0x3A);
  CommandWrite(0xBE); //Set VcomH
    CommandWrite(0x3E);
  CommandWrite(0x87); //Set Master Current Control
    CommandWrite(0x06);
  CommandWrite(0x15); //Set Column Address
    CommandWrite(0);
    CommandWrite(95);
  CommandWrite(0x75); //Set Row Address
    CommandWrite(0);
    CommandWrite(63);
  CommandWrite(0x81); //Set Contrast for Color A
    CommandWrite(255);
  CommandWrite(0x82); //Set Contrast for Color B
    CommandWrite(255);
  CommandWrite(0x83); //Set Contrast for Color C
    CommandWrite(255);
  CommandWrite(0xAF); //Set Display On
  delay(110); //0xAFコマンド後最低100ms必要
}

void setup() {
  int i, j;
  uint8_t R, G, B, Dot1, Dot2;
   
  SSD1331_Init(cs_OLED, DCpin, RSTpin);
 
  for(j=0; j<64; j++){ //画面黒塗りつぶし
    for(i=0; i<96; i++){
      DataWrite(0);
    }
  }
 
  R = 7; G = 0; B = 0; //256 color : R (0-7), G (0-7), B (0-3) 
  Dot1 = (R << 5) | (G << 2) | B;
   
  R = 0; G = 0; B = 3; //256 color : R (0-7), G (0-7), B (0-3) 
  Dot2 = (R << 5) | (G << 2) | B;
   
  for(j=0; j<64; j++){
    for(i=0; i<96; i++){
      if(j<8 && i<16) {
        DataWrite(Dot1);
      }else{
        DataWrite(Dot2);
      }
    }
  }
}
 
void loop() {
   
}
 
//*********** SSD1331 初期化 ****************************

//********** SPI コマンド出力 ****************************
//********** SPI データ出力 ****************************
