#ifndef _FIXEDPOINT_H_
#define _FIXEDPOINT_H_

template <int decimal_bits>
double fixed64_to_double(long long fixed)
{
	long long i = fixed >> decimal_bits;
	unsigned long long d = fixed & ((1LL << decimal_bits) - 1);
	d <<= 52 - decimal_bits;
	d |= 0x3FF0000000000000ULL;
	return *(double*)&d + ( i - 1 );
}


template <int decimal_bits>
double fixed32_to_double(long fixed)
{
	long i = fixed >> decimal_bits;
	unsigned long long d = fixed & ((1L << decimal_bits) - 1);
	d <<= 52 - decimal_bits;
	d |= 0x3FF0000000000000ULL;
	return *(double*)&d + ( i - 1 );
}

template <int decimal_bits>
double fixed28_to_double(long fixed)
{
	return fixed32_to_double<decimal_bits>((fixed << 4) >> 4);
}

template <int decimal_bits>
double fixed16_to_double(short fixed)
{
	short i = fixed >> decimal_bits;
	unsigned long long d = fixed & ((1 << decimal_bits) - 1);
	d <<= 52 - decimal_bits;
	d |= 0x3FF0000000000000ULL;
	return *(double*)&d + ( i - 1 );
}

#endif
