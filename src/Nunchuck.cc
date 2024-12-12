#include <Nunchuck.h>

bool Nunchuck::_id_set = false;
char Nunchuck::_id[I2CIP_ID_SIZE];

using namespace I2CIP;

void Nunchuck::loadID(void) {
  uint8_t idlen = strlen_P(wiipod_nunchuck_id_progmem);

  // Read in PROGMEM
  for (uint8_t k = 0; k < idlen; k++) {
    char c = pgm_read_byte_near(wiipod_nunchuck_id_progmem + k);
    Nunchuck::_id[k] = c;
  }

  Nunchuck::_id[idlen] = '\0';
  Nunchuck::_id_set = true;

  #ifdef I2CIP_DEBUG_SERIAL
    DEBUG_DELAY();
    I2CIP_DEBUG_SERIAL.print(F("Nunchuck ID Loaded: '"));
    I2CIP_DEBUG_SERIAL.print(Nunchuck::_id);
    I2CIP_DEBUG_SERIAL.print(F("' @"));
    I2CIP_DEBUG_SERIAL.println((uintptr_t)(&Nunchuck::_id[0]), HEX);
    DEBUG_DELAY();
  #endif
}

// Handles ID pointer assignment too
// NEVER returns nullptr, unless out of memory
Device* Nunchuck::nunchuckFactory(const i2cip_fqa_t& fqa, const i2cip_id_t& id) {
  if(!Nunchuck::_id_set || id == nullptr) {
    loadID();

    (Device*)(new Nunchuck(fqa, id == nullptr ? _id : id));
  }

  return (Device*)(new Nunchuck(fqa, id));
}

Device* Nunchuck::nunchuckFactory(const i2cip_fqa_t& fqa) { return nunchuckFactory(fqa, Nunchuck::getStaticIDBuffer()); }

Nunchuck::Nunchuck(const i2cip_fqa_t& fqa, const i2cip_id_t& id) : Device(fqa, id), InputInterface<wiipod_nunchuck_t, void*>((Device*)this) { }
Nunchuck::Nunchuck(const i2cip_fqa_t& fqa) : Nunchuck(fqa, Nunchuck::_id) { }

// Nunchuck::Nunchuck(const uint8_t& wire, const uint8_t& module, const uint8_t& addr) : Nunchuck(I2CIP_FQA_CREATE(wire, module, I2CIP_MUX_BUS_DEFAULT, addr)) { }

i2cip_errorlevel_t Nunchuck::get(wiipod_nunchuck_t& dest, void* const& args) {
  // 0. Check args
  if(args != nullptr) {
    return I2CIP_ERR_SOFT;
  }

  i2cip_errorlevel_t errlev;
  #ifdef I2CIP_DEBUG_SERIAL
    DEBUG_DELAY();
    I2CIP_DEBUG_SERIAL.print(F("[NUNCHUCK] "));
    I2CIP_DEBUG_SERIAL.print(I2CIP_FQA_SEG_I2CBUS(fqa), HEX);
    I2CIP_DEBUG_SERIAL.print(':');
    I2CIP_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MODULE(fqa), HEX);
    I2CIP_DEBUG_SERIAL.print(':');
    I2CIP_DEBUG_SERIAL.print(I2CIP_FQA_SEG_MUXBUS(fqa), HEX);
    I2CIP_DEBUG_SERIAL.print(':');
    I2CIP_DEBUG_SERIAL.print(I2CIP_FQA_SEG_DEVADR(fqa), HEX);

    DEBUG_DELAY();
  #endif

  if(!initialized) {
    errlev = writeRegister((uint8_t)0xF0, (uint8_t)0x55, false);
    I2CIP_ERR_BREAK(errlev);

    // delay(NUNCHUCK_DELAY);

    errlev = writeRegister((uint8_t)0xFB, (uint8_t)0x00, false);
    I2CIP_ERR_BREAK(errlev);
    // initialized = true;
    delay(NUNCHUCK_DELAY);
  }
  //  else {
  //   // Ping
  //   errlev = ping(this->getFQA(), false, false);
  //   I2CIP_ERR_BREAK(errlev);
  // }

  // delay(NUNCHUCK_DELAY);

  uint8_t temp[NUNCHUCK_READLEN];
  size_t len = NUNCHUCK_READLEN;
  // errlev = readRegister((uint8_t)0x00, temp, len, false, true, false);
  // I2CIP_ERR_BREAK(errlev);

  I2CIP_FQA_TO_WIRE(fqa)->beginTransmission(I2CIP_FQA_SEG_DEVADR(fqa));

  // write internal register address - most significant byte first
  if(I2CIP_FQA_TO_WIRE(fqa)->write((uint8_t)0) != 1) return I2CIP_ERR_SOFT;
  
  if(I2CIP_FQA_TO_WIRE(fqa)->endTransmission(false) != 0) return I2CIP_ERR_HARD;

  delayMicroseconds(275);

  errlev = read((uint8_t*)temp, len, false, true, false);

  if(len != NUNCHUCK_READLEN || (temp[0] == 0xFF && temp[1] == 0xFF && temp[2] == 0xFF && temp[3] == 0xFF && temp[4] == 0xFF && temp[5] == 0xFF)) {
    return I2CIP_ERR_SOFT;
  }

  // Serial.print("0b");
  // Serial.println(temp[0], BIN);
  // Serial.print("0b");
  // Serial.println(temp[1], BIN);
  // Serial.print("0b");
  // Serial.println(temp[2], BIN);
  // Serial.print("0b");
  // Serial.println(temp[3], BIN);
  // Serial.print("0b");
  // Serial.println(temp[4], BIN);
  // Serial.print("0b");
  // Serial.println(temp[5], BIN);

  dest.joy_x = temp[0];
  dest.joy_y = temp[1];
  dest.accel_x = (temp[2] << 2) | ((temp[5] & 0b11000000) >> 6);
  dest.accel_y = (temp[3] << 2) | ((temp[5] & 0b00110000) >> 4);
  dest.accel_z = (temp[4] << 2) | ((temp[5] & 0b00001100) >> 2);
  dest.c = !(temp[5] & 0b00000010);
  dest.z = !(temp[5] & 0b00000001);

  return errlev;
}

// G - Getter type: char* (null-terminated; writable heap)
void Nunchuck::clearCache(void) {
  this->setCache({ 0, 0, 0, 0, 0, false, false });

  #ifdef I2CIP_DEBUG_SERIAL
    DEBUG_DELAY();
    I2CIP_DEBUG_SERIAL.print(F("Nunchuck Cache Cleared (Zeroed)\n"));
    DEBUG_DELAY();
  #endif
}

void Nunchuck::printToScreen(Stream& out, uint8_t width, uint8_t height, bool border, bool circle) {
  // width *= 2; // accounts for character aspect ratio; trying to make it square
  wiipod_nunchuck_t data = this->getCache();

  // Pixel position
  int _x = ((double)data.joy_x / 255.0) * width;
  int _y = ((255.0 - (double)data.joy_y) / 255.0) * height; // Y-invert

  if(border){ out.print('|'); for(int x = 0; x < width; x++) { out.print('-'); } out.print('|'); }
  out.print('\n');

  for(int iy = 0; iy < height; iy++) {
    if(border) { out.print('|'); }
    for(int ix = 0; ix < width; ix++) {
      if(ix == _x && iy == _y) {
        // out.print(use_bold ? 'X' : (use_z ? 'N' : (circle ? '#' : '@')));
        out.print('X'); // "Crosshair" cursor
      } else if(circle) {
        // Unit Circle
        double unit_x = 1.0 - ((2.0 * ix) / width);
        double unit_y = 1.0 - ((2.0 * iy) / height);
        out.print(((unit_x * unit_x) + (unit_y * unit_y) <= 0.9) ? ' ' :  '+');
      } else {
        // Empty
        out.print(' ');
      }
    }
    if(border) out.print('|');
    out.print('\n');
  }
  
  if(border){ out.print('|'); for(int x = 0; x < width; x++) { out.print('-'); } out.print('|'); }
}