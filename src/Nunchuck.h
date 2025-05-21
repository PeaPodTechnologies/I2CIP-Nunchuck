#ifndef WIIPOD_NUNCHUCK_H
#define WIIPOD_NUNCHUCK_H

// #define I2CIP_NUNCHUCK_USE_SCREEN 1 // Uncomment to enable screen support

#include <Arduino.h>

#include <I2CIP.h>

#ifdef I2CIP_NUNCHUCK_USE_SCREEN
#include <SSD1306.h>
#endif

#define NUNCHUCK_READLEN 6
#define I2CIP_NUNCHUCK_ADDRESS 0x52
// #define NUNCHUCK_ADDRESS 0xA4
#define NUNCHUCK_DELAY 10

typedef struct {
  bool c;
  bool z;
  uint8_t x;
  uint8_t y;
  uint16_t a_x;
  uint16_t a_y;
  uint16_t a_z;
} i2cip_nunchuck_t;

#define I2CIP_NUNCHUCK_ID "NUNCHUCK"

class Nunchuck : public I2CIP::Device, public I2CIP::InputInterface<i2cip_nunchuck_t, void*> {
  I2CIP_DEVICE_CLASS_BUNDLE(Nunchuck, I2CIP_NUNCHUCK_ID);
  I2CIP_INPUT_USE_RESET(i2cip_nunchuck_t, void*, void* const);
  I2CIP_INPUT_USE_TOSTRING(i2cip_nunchuck_t, "{\"x\": %u, \"y\": %u, \"a\": {\"x\": %u, \"y\": %u, \"z\": %u}, \"c\": %u, \"z\": %u}");
  I2CIP_INPUT_ADD_PRINTCACHE(i2cip_nunchuck_t, "Joy: (%u, %u), Acc: (%u, %u, %u), C: %c, Z: %c"); // will this work? I think so printf("%...", struct) is valid

  private:
    #ifdef MAIN_CLASS_NAME
    friend class MAIN_CLASS_NAME;
    #endif
  public:
    void printToScreen(Stream& out, uint8_t width, uint8_t height, bool border = true, bool circle = true);
    #ifdef I2CIP_NUNCHUCK_USE_SCREEN
    i2cip_errorlevel_t printToScreen(SSD1306* out, uint8_t width, uint8_t height, bool border = true, bool circle = true);
    #endif

    Nunchuck(i2cip_fqa_t fqa, const i2cip_id_t& id);
    virtual ~Nunchuck() { }

    i2cip_errorlevel_t begin(bool setbus = true) override;
    static i2cip_errorlevel_t _begin(const i2cip_fqa_t& fqa, bool setbus);


    /**
     * Read from the Nunchuck.
     * @param dest Destination heap (pointer reassigned, not overwritten)
     * @param args Number of bytes to read
     **/
    i2cip_errorlevel_t get(i2cip_nunchuck_t& dest, void* const& args = nullptr) override;
};

#endif