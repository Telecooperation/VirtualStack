
#pragma once

/**
 * Provides commonly used methods used for checksum calculation
 */
class ChecksumExtensions
{
public:
	/**
	 * Provides a c-style fast checksum generation for ip and udp header checksums
	 * @param buf The data to generate a checksum for
	 * @param len The length of the data based on uint16_t
	 * @return The checksum of the given data
	 */
	static inline uint32_t  generateChecksum(const uint16_t *buf, size_t len){
		uint32_t sum = 0;
		while (len > 1)
		{
			sum += *buf;
			if (sum & 0x80000000)
				sum = (sum & 0xFFFF) + (sum >> 16);
			buf++;
			len -= 2;
		}

		if ( len & 1 )
			// Add the padding if the packet lenght is odd          //
			sum += *(reinterpret_cast<const uint8_t *>(buf));

		return sum;
	}

	/**
	 * Provides a c-style fast checksum generation for ip and udp header checksums IF: (len * 1500) < (2^16 - 1)
	 * @param buf The data to generate a checksum for
	 * @param len The length of the data based on uint16_t
	 * @return The checksum of the given data
	 */
	static inline uint32_t  generateChecksumWithoutWrap(const uint16_t *buf, size_t len){
		uint32_t sum = 0;
		while (len > 1)
		{
			sum += *buf;
			//we need no wrap as we only generate checksum for len <= 1500 and therefore max(uint16_t) * (len / 2) => 2^16 * (1500 / 2) = 49152000 => ... < 2^26
			//so we never get true for sum&0x8000000 as sum would need to be > 2^31
			buf++;
			len -= 2;
		}

		if ( len & 1 )
			// Add the padding if the packet lenght is odd          //
			sum += *(reinterpret_cast<const uint8_t *>(buf));

		return sum;
	}
};

