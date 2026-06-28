#include "crypto.h"

#include <cstdint>
#include <vector>
#include <sstream>
#include <random>

#include "monocypher.h"

namespace Keten {

	

	void generateKeyPair(std::string& out_publicKey, std::string& out_privateKey) 
	{
		uint8_t seed[32];
		uint8_t privateKey[64];
		uint8_t publicKey[32];

		std::random_device random;
		for (int i = 0; i < 32; i++) {
			seed[i] = static_cast<uint8_t>(random());
		}

		crypto_eddsa_key_pair(privateKey, publicKey, seed);
		
		picosha2::bytes_to_hex_string(privateKey, privateKey + 64, out_privateKey);
		picosha2::bytes_to_hex_string(publicKey, publicKey + 32, out_publicKey);
	}

	static std::vector<uint8_t> hexToBytes(const std::string& hexString) {
		std::vector<uint8_t> bytes;
		for (int i = 0; i < hexString.length(); i += 2) {
			bytes.push_back(static_cast<uint8_t>(std::stoul(hexString.substr(i, 2), nullptr, 16)));
		}
		return bytes;
	}

	std::string signMessage(const std::string& messageHash, const std::string& privateKey)
	{
		std::vector<uint8_t> msgBytes = hexToBytes(messageHash);
		std::vector<uint8_t> keyBytes = hexToBytes(privateKey);

		uint8_t signature[64];
		crypto_eddsa_sign(signature, keyBytes.data(), msgBytes.data(), msgBytes.size());

		std::string outSignature;
		picosha2::bytes_to_hex_string(signature, signature + 64, outSignature);

		return outSignature;
	}

	bool validateSignature(const std::string& messageHash, const std::string& signature, const std::string& publicKey)
	{
		std::vector<uint8_t> msgBytes = hexToBytes(messageHash);
		std::vector<uint8_t> sigBytes = hexToBytes(signature);
		std::vector<uint8_t> keyBytes = hexToBytes(publicKey);

		if (sigBytes.size() != 64 || keyBytes.size() != 32) {
			return false;
		}

		int result = crypto_eddsa_check(sigBytes.data(), keyBytes.data(), msgBytes.data(), msgBytes.size());
		return result == 0;
	}

	std::string calculateHash(const std::string& input)
	{
		std::vector<unsigned char> hash(picosha2::k_digest_size);
		picosha2::hash256(input.begin(), input.end(), hash.begin(), hash.end());
		return picosha2::bytes_to_hex_string(hash.begin(), hash.end());
	}
}