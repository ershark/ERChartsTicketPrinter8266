/*******************************************
 * see header-file for further informations
 ********************************************/

#include "Arduino.h"
#include "thermalprinter.h"

static const char LF = 0xA; // print buffer and line feed  
    
    
Epson::Epson(int rxPin, int txPin)
{
  this->_rxPin = rxPin;
  this->_txPin = txPin;
  this->start();
}

void Epson::start(){

  pinMode(this->_txPin, OUTPUT);
  pinMode(this->_rxPin, INPUT);  
  this->_printer = new SoftwareSerial (this->_rxPin, this->_txPin);
  this->_printer->begin(115200);
}

// query status of printer. when online returns value 22.
int Epson::getStatus(){
  this->write(0x10);    
  this->write(0x04);  
  this->write(1);
  int result;
  result = this->_printer->read();
  return result;
}

int Epson::read(){
    int result;
    result = this->_printer->read();
    return result;
}

// Print and feed n lines
// prints the data in the print buffer and feeds n lines
void Epson::feed(uint8_t n){
  this->write(0x1B);  
  this->write(0x64);
  this->write(n);    
}

// Print one line
void Epson::feed(){
  this->feed(1);    
}


// Set line spacing
// sets the line spacing to n/180-inch
void Epson::lineSpacing(uint8_t n){
  this->write(0x1B);  
  this->write(0x33);
  this->write(n);  
}

// Select default line spacing
// sets the line spacing to 1/6 inch (n=60). This is equivalent to 30 dots.
void Epson::defaultLineSpacing(){
  this->write(0x1B);  
  this->write(0x32);
}

// Select an international character set
//  0 = U.S.A. 
//  1 = France 
//  2 = Germany 
//  3 = U.K. 
//  4 = Denmark I 
//  5 = Sweden 
//  6 = Italy 
//  7 = Spain 
//  8 = Japan 
//  9 = Norway 
// 10 = Denmark II 
// see reference for Details! 
void Epson::characterSet(uint8_t n){
  this->write(0x1B);  
  this->write(0x52);
  this->write(n);  
}

void Epson::doubleSize(){
  this->write(0x1B);    
  this->write(0x21);  
  this->write(48);
}

void Epson::doubleWidth(){
  this->write(0x1B);    
  this->write(0x21);  
  this->write(32);
}
void Epson::magText(){
 this->write(0x1D);
  this->write(0x21);
  this->write(51);
 this->write(0x1D);
  this->write(0x62);
  this->write(11);
}
  
void Epson::normalSize(){
  this->write(0x1B);  
  this->write(0x21);    
  this->write(0);
}

void Epson::doubleHeightOn(){
  this->write(0x1B);    
  this->write(0x21);  
  this->write(16);
}

void Epson::doubleHeightOff(){
  this->write(0x1B);  
  this->write(0x21);    
  this->write(0);
}

void Epson::boldOn(){
  this->write(0x1B);  
  this->write(0x21);    
  this->write(8);
}

void Epson::boldOff(){
  this->write(0x1B);  
  this->write(0x21);    
  this->write(0);
}

void Epson::underlineOff() {
  this->write(0x1B);  
  this->write(0x21);    
  this->write(0);
}
void Epson::underlineOn() {
  this->write(0x1B);  
  this->write(0x21);    
  this->write(128);
}


// Turn white/black reverse printing mode on/off
void Epson::reverseOn() {
  this->write(0x1D);  
  this->write(0x42);    
  this->write(1);
}
  
void Epson::reverseOff() {
  this->write(0x1D);  
  this->write(0x42);    
  this->write(0);
}

void Epson::justifyLeft() {
  this->write(0x1B);  
  this->write(0x61);    
  this->write(0);
}

void Epson::justifyCenter() {
  this->write(0x1B);  
  this->write(0x61);    
  this->write(1);
}

void Epson::justifyRight() {
  this->write(0x1B);  
  this->write(0x61);    
  this->write(2);
}
//n range 1-255
void Epson::barcodeHeight(uint8_t n) {
  this->write(0x1D);  
  this->write(0x68);    
  this->write(n);
}
//n range 2-6
void Epson::barcodeWidth(uint8_t n) {
  this->write(0x1D);  
  this->write(0x77);    
  this->write(n);
}
//n range 0-3
void Epson::barcodeNumberPosition(uint8_t n) {
  this->write(0x1D);  
  this->write(0x48);    
  this->write(n);
}
//m range 65-73 (code type)
//n (digit length)
void Epson::printBarcode(uint8_t m, uint8_t n) {
  this->write(0x1D);  
  this->write(0x6B);    
  this->write(m);
  this->write(n);
}

void Epson::cut() {
  this->write(0x1D);
  this->write('V');
  this->write(65);
  this->write(0xA); // print buffer and line feed
}
void Epson::partialCut() {
  this->write(0x1D);
  this->write('V');
  this->write(66);
  this->write(0xA); // print buffer and line feed
}

void Epson::smallPaper() {
  this->write(0x1D);
  this->write(0x57);
  this->write(155);
  this->write(1);
 // right side char spacing
  //this->write(0x1B);
  //this->write(0x20);
  //this->write(10);
 
}
//void Epson::qrCodeStart(uint8_t n) {
void Epson::qrCodeStart(uint8_t n, uint8_t s) {
  //Set QR Type

  
  this->write(0x1D);//GS
  this->write(0x28);//(
  this->write(0x6B);//k
  this->write(0x04);//pL 
  this->write(0x00);//pH 
  this->write(0x31);// cn
  this->write(0x41);//fn
  this->write(50);//n1 Model 1 50 Model 2
  this->write(0);//n2   
//  Set the QR Code size of module.
  this->write(0x1D);//GS   "\x1D\x28\x6B\x03\x00\x31\x43#{qr_size}"
  this->write(0x28);//(
  this->write(0x6B);//k
  this->write(0x03);//pL
  this->write(0x00);//pH 
  this->write(0x31);// cn
  this->write(0x43);//fn
//  this->write(6);//n2?? 
  this->write(s);//n2?? 
  //Set QR Select the QR Code error correction level.
  this->write(0x1D);//GS
  this->write(0x28);//(
  this->write(0x6B);//k
  this->write(0x03);//pL 
  this->write(0x00);//pH 
  this->write(0x31);// cn 31
  this->write(0x45);//fn     escpos << "\x1D\x28\x6B\x03\x00\x31\x45\x33"
  this->write(48);//n 47 < n < 52

//  Store Data
  this->write(0x1D);//GS
  this->write(0x28);//(
  this->write(0x6B);//k
  this->write(n);//pL 0 <= pL < 256,  3 < (pL + pH* 256) < 7093 //n=22
  this->write(0);//pH 0 <= pH < 28
  this->write(0x31);// cn
  this->write(0x50);//fn 80
  this->write(0x30);//m 48     escpos << "\x1D\x28\x6B#{lsb}#{msb}\x31\x50\x30"
  //Data comes Here
  //this->print("Hello wOrld"); ///s = 14
//  Print Data

 
}
void Epson::qrCodeEnd() {
  this->write(0x1D);//GS
  this->write(0x28);//(
  this->write(0x6B);//k
  this->write(0x03);//pL
  this->write(0x00);//pH 
  this->write(0x31);// cn = 49
  this->write(0x51);//fn 81
  this->write(0x30);//m =48   "\x1D\x28\x6B\x03\x00\x31\x51\x30"
  }

size_t Epson::write(uint8_t c) {
  this->_printer->write(c);
  return 1;
}
