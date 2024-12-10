#ifndef WIIPOD_NUNCHUCK_H
#define WIIPOD_NUNCHUCK_H

#include <Arduino.h>

#include <I2CIP.h>

#define NUNCHUCK_READLEN 6
#define NUNCHUCK_ADDRESS 0x52

typedef struct {
  uint8_t joy_x;
  uint8_t joy_y;
  uint16_t accel_x;
  uint16_t accel_y;
  uint16_t accel_z;
  bool c;
  bool z;
} wiipod_nunchuck_t;

const char wiipod_nunchuck_id_progmem[] PROGMEM = {"NUNCHUCK"};

class Nunchuck : public I2CIP::Device, public I2CIP::InputInterface<wiipod_nunchuck_t, void*> {
  private:
      static bool _id_set;
      static char _id[]; // to be loaded from progmem

      bool initialized = false;

      void* const isnull = nullptr;
      Nunchuck(const i2cip_fqa_t& fqa);

      #ifdef MAIN_CLASS_NAME
      friend class MAIN_CLASS_NAME;
      #endif
    public:
      void printToScreen(Stream& out, uint8_t width, uint8_t height, bool border = true, bool circle = true);

      Nunchuck(const i2cip_fqa_t& fqa, const i2cip_id_t& id);

      static Device* nunchuckFactory(const i2cip_fqa_t& fqa, const i2cip_id_t& id);
      static Device* nunchuckFactory(const i2cip_fqa_t& fqa);

      static void loadID(void);

      virtual ~Nunchuck() { }

      // static const char* getStaticIDBuffer() { return Nunchuck::_id_set ? Nunchuck::_id : nullptr; } // Harsh but fair
      static const char* getStaticIDBuffer() { return Nunchuck::_id; }

      /**
       * Read from the Nunchuck.
       * @param dest Destination heap (pointer reassigned, not overwritten)
       * @param args Number of bytes to read
       **/
      i2cip_errorlevel_t get(wiipod_nunchuck_t& dest, void* const& args = nullptr) override;

      void clearCache(void) override;
      void* const& getDefaultA(void) const override { return this->isnull; };
};

#endif