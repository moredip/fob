#include "key.h"
#include "Time.h"
#include "base32.h"

Key::Key(String name, uint8_t* hmacKey, int keyLength) 
  : _name(name), _keyLength(keyLength)
{
  _hmacKey = new uint8_t[_keyLength];
  memcpy(_hmacKey, hmacKey, _keyLength);
};

Key::~Key(){
  delete[] _hmacKey;
}

Key Key::fromBase32Encoding( String name, String base32Key )
{
  const uint8_t *encoded = (const uint8_t *)base32Key.c_str();
  uint8_t *decoded = (uint8_t *)malloc( sizeof(uint8_t)*(base32Key.length()+1) );
  
  base32_decode( encoded, decoded, base32Key.length());
  Key key = Key(name,decoded,base32Key.length());

  free(decoded);
  return key;
}

String Key::getCurrentCode() {
  TOTP totp(_hmacKey,_keyLength);
  return String( totp.getCode(now()) );
}
