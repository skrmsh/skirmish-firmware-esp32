/*
Skirmish ESP32 Firmware

Infrared Driver

Copyright (C) 2023 Ole Lange
*/

#include <Arduino.h>
#include <conf.h>

#include "inc/infrared.h"
#include "inc/log.h"

/**
 * sends an NEC IR code. No library required. Just copy this function into your sketch.
 * 
 * code:  the code to send e.g. 0x00FDA857
 * irLedPin: the output pin e.g. 3
 * freqKhz: the carrier frequency in KHz
 * inverted: (default false). Default case : led wired low side so pin HIGH = on. otherwise pin LOW = on.
 * invertedBitOrder (default false). Default case: MSB as used in ver <= 2  of the Ken Shirriff IR library. Otherwise mirrored for V3+
 *
 * sample call: irSendNECblk( 0x00FF1A9B, 3, 38 )  // pin 3, 38kHz carrier, pin HIGH puts led on, original (non mirrored) hex code format.
 * Note: blocks the loop() for approx 70 ms .
 * Author: 6v6gt 04.05.2021 https://forum.arduino.cc/t/extreme-lightweight-ir-sender-for-nec-protocol/858910
 */
void irSendNECblk( uint32_t code, uint8_t irLedPin, uint8_t freqKhz, bool inverted = false, bool invertedBitOrder = false ) {
	uint16_t NecBurstUnit = (freqKhz * 562L) / 1000L; // IR carrier waves for 1 NEC mark or 1 NEC space( bit 0).
	uint8_t carrierPeriodUs = (int16_t)1000 / freqKhz;
	uint8_t * codeSplit;
	codeSplit = (uint8_t*) &code;  // treat uint32_t as byte array ;

	auto xmit = [irLedPin, inverted, carrierPeriodUs](bool isOn, uint16_t waves) {
		// send carrier burst. on = IR mark, otherwise IR space (filler)
		uint32_t burstStartUs = micros() ;

		for ( uint16_t i = 0 ; i < waves ; i ++ ) {
			digitalWrite(irLedPin, isOn != inverted ? HIGH : LOW) ; 
			delayMicroseconds(carrierPeriodUs / 3) ;  // 33% duty cycle
			digitalWrite(irLedPin, inverted ? HIGH : LOW) ;   // carrier space
			while (micros() - ( burstStartUs + ( i * carrierPeriodUs ) ) < carrierPeriodUs)  ;  // idle until end of wave(i)
		}
	};

	xmit(true, NecBurstUnit * 16) ; // header mark 9000 us
	xmit(false, NecBurstUnit * 8) ; // header space 4500 us
	for (uint8_t i = 0; i < 32; i ++ ) { // 32 bits
		xmit(true, NecBurstUnit ) ; // NEC mark
		uint8_t codeByte = !invertedBitOrder ? 3 - i / 8 /*MSB*/ : i/8 /*LSB*/;
		uint8_t codeBit = !invertedBitOrder ? 7 - i % 8 /*MSB*/ : i % 8 /*LSB*/;
		xmit(false, bitRead(*(codeSplit + codeByte), codeBit) == 1 ? NecBurstUnit * 3 : NecBurstUnit);  // NEC space(0) 562us or NEC space(1) ~1675us
	}
	xmit(true, NecBurstUnit ); // terminator
}

void infraredInit() {
    logInfo("Init: Infrared TX");
	pinMode(PIN_IR_LED, OUTPUT) ;
    logInfo(">     Init Done!");
}

/*
Transmits the given pid + sid + a checksum
via the infrared led using the nec protocol
*/
void infraredTransmitShot(uint8_t pid, uint32_t sid) {
    uint32_t irpack = (pid << 16) | (sid & 0xffff);
    uint8_t checksum = irpack % 0xff;
    irpack = (checksum << 24) | irpack;

    logDebug("Transmitting Shot:");
    logDebug("  playerID: %02x", pid);
    logDebug("  shotID:   %05x", sid);
    logDebug("  checksum: %02x", checksum);
    logDebug("  packet:   %08x", irpack);

    irSendNECblk(irpack, PIN_IR_LED, 38);
}