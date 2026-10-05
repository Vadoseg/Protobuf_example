#ifndef CRC_H
#define CRC_H

#include <cstdlib>

namespace crc {

  template<unsigned N>
  struct lut_t
  {
    constexpr lut_t() : value()
    {
      for ( auto i = 0u; i < N; i++ ) {
        value[i] = i;
        for ( auto j = 0u; j < 8u; j++ )
          value[i] = value[i] & 1 ? (value[i] >> 1) ^ 0xEDB88320ul : value[i] >> 1;
      }
    }
    unsigned value[N];
  };

  // Calculate CRC with polynom 0x04C11DB7
  static unsigned calculate(const unsigned char data[], size_t size) // TODO make universal crc calculaton
  {
    constexpr lut_t<256> lut = lut_t<256>();
    unsigned crc = 0xFFFFFFFFul;
    const unsigned char* p = data;
    while ( size-- )
      crc = lut.value[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
    return crc ^ 0xFFFFFFFFul;
  }

}

#endif // CRC_H
