#pragma once

#include <cstdint>
#include <memory>

typedef uint16_t float16;

struct FloatContents {
	uint32_t fraction : 23;
	uint32_t exponent : 8;
	uint32_t sign : 1;
};

inline float16 float32_to_float16(float input) {
	struct Float32Contents {
		uint32_t fraction : 23;
		uint32_t exponent : 8;
		uint32_t sign : 1;
	} float32Contents;
	memcpy(static_cast<void*>(&float32Contents), static_cast<void*>(&input), sizeof(uint32_t));

	struct Float16Contents {
		uint16_t fraction : 10;
		uint16_t exponent : 5;
		uint16_t sign : 1;
	} float16Contents;

	// Sign conversion, both one bit
	float16Contents.sign = float32Contents.sign;

	// Exponent conversion, return NAN if float32 exponent is too large
	if (float32Contents.exponent >= 32)
		float16Contents.exponent = 31;
	else
		float16Contents.exponent = float32Contents.exponent;

	// Fraction conversion, return NAN if float32 fraction is too large
	if (float32Contents.fraction >= 1024)
		float16Contents.fraction = 1023;
	else
		float16Contents.fraction = float32Contents.fraction;

	float16 output;
	memcpy(static_cast<void*>(&output), static_cast<void*>(&float16Contents), sizeof(uint16_t));

	return output;
}
