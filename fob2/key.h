#ifndef _KEY_H
#define _KEY_H

#include "TOTP.h"
#include <Arduino.h>

class Key {
  public: 
    static Key fromBase32Encoding( String name, String base32Key );
    
	public:
		Key(String name, uint8_t* hmacKey, int keyLength);
                ~Key();
		String getCurrentCode();
	
	private:
		const String _name;
                uint8_t* _hmacKey;
                int _keyLength;
};

#endif
