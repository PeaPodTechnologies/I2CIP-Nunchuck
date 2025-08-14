#include <Nunchuck.h>

using namespace I2CIP;

#define NUNCHUCK_DEFAULT_CACHE { .c = false, .z = false, .x = 0, .y = 0, .a_x = 0, .a_y = 0, .a_z = 0 }

I2CIP_DEVICE_INIT_STATIC_ID(Nunchuck, I2CIP_NUNCHUCK_ID);
I2CIP_INPUT_INIT_RESET(Nunchuck, i2cip_nunchuck_t, NUNCHUCK_DEFAULT_CACHE, void*, nullptr);

void Nunchuck::parseJSONArgs(I2CIP::i2cip_args_io_t& argsDest, JsonVariant argsA, JsonVariant argsS, JsonVariant argsB) { } // Takes no args
void Nunchuck::deleteArgs(I2CIP::i2cip_args_io_t& args) { } // No args to delete

Nunchuck::Nunchuck(i2cip_fqa_t fqa, const i2cip_id_t& id) : I2CIP::Device(fqa, id), I2CIP::InputInterface<i2cip_nunchuck_t, void*>((I2CIP::Device*)this) { }

i2cip_errorlevel_t Nunchuck::begin(bool setbus) { return Nunchuck::_begin(this->fqa, setbus); }
i2cip_errorlevel_t Nunchuck::_begin(const i2cip_fqa_t& fqa, bool setbus) {
  i2cip_errorlevel_t errlev = writeRegister(fqa, (uint8_t)0xF0, (uint8_t)0x55, setbus);
  I2CIP_ERR_BREAK(errlev);

  errlev = writeRegister(fqa, (uint8_t)0xFB, (uint8_t)0x00, false);
  I2CIP_ERR_BREAK(errlev);

  delay(NUNCHUCK_DELAY);
  return errlev;
}

i2cip_errorlevel_t Nunchuck::get(i2cip_nunchuck_t& dest, void* const& args) {
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

  uint8_t temp[NUNCHUCK_READLEN];
  size_t len = NUNCHUCK_READLEN;
  // errlev = readRegister((uint8_t)0x00, temp, len, false, true, false);
  // I2CIP_ERR_BREAK(errlev);

  I2CIP_FQA_TO_WIRE(fqa)->beginTransmission(I2CIP_FQA_SEG_DEVADR(fqa));

  // write internal register address - most significant byte first
  if(I2CIP_FQA_TO_WIRE(fqa)->write((uint8_t)0) != 1) return I2CIP_ERR_SOFT;
  
  if(I2CIP_FQA_TO_WIRE(fqa)->endTransmission() != 0) return I2CIP_ERR_HARD;

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

  dest.x = temp[0];
  dest.y = temp[1];
  dest.a_x = (temp[2] << 2) | ((temp[5] & 0b11000000) >> 6);
  dest.a_y = (temp[3] << 2) | ((temp[5] & 0b00110000) >> 4);
  dest.a_z = (temp[4] << 2) | ((temp[5] & 0b00001100) >> 2);
  dest.z = !(temp[5] & 0b00000001);
  dest.c = !(temp[5] & 0b00000010);

  return errlev;
}

void Nunchuck::printToScreen(Stream& out, uint8_t width, uint8_t height, bool border, bool circle) {
  // width *= 2; // accounts for character aspect ratio; trying to make it square
  i2cip_nunchuck_t data = this->getCache();

  // Pixel position
  int _x = ((double)data.x / 255.0) * width;
  int _y = ((255.0 - (double)data.y) / 255.0) * height; // Y-invert

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

#ifdef I2CIP_NUNCHUCK_USE_SCREEN
i2cip_errorlevel_t Nunchuck::printToScreen(SSD1306* out, uint8_t width, uint8_t height, bool border, bool circle) {
  if(out == nullptr || out->getOutput() == nullptr) return I2CIP_ERR_NONE; // No harm no foul
  // width *= 2; // accounts for character aspect ratio; trying to make it square
  i2cip_nunchuck_t data = this->getCache();

  // Pixel position
  int _x = ((double)data.x / 255.0) * width;
  int _y = ((255.0 - (double)data.y) / 255.0) * height; // Y-invert

  bool bits[height][width] = {0};

  if(border) {
    for(int x = 0; x < width; x++) { bits[0][x] = true; bits[height-1][x] = true; }
    for(int y = 0; y < height; y++) { bits[y][0] = true; bits[y][width-1] = true; }
  }

  bits[_y][_x] = true;

  if(circle) {
    for(int x = 0; x < width; x++) {
      double unit_x = 1.0 - ((2.0 * x) / width); // -1.0 to 1.0
      uint8_t fx = sqrt(1.0 - (unit_x * unit_x)); // 0 to 1
      uint8_t y = (height*(1-fx))/2.0; // .5*height to 0
      bits[y][x] = true;
      y = (height*(1+fx))/2.0; // height to .5height
      bits[y][x] = true;
    }
  }

  size_t len = (uint8_t)((width*height/8)+0.5);
  uint8_t buf[len] = {0};
  for(int i = 0; i < width*height; i++) {
    buf[i/8] <<= 1;
    buf[i/8] |= (bits[i/width][i%width] ? 1 : 0);
  }

  return out->getOutput()->failSet(buf);
}
#endif