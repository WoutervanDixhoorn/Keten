#pragma once

#include <string>
#include <vector>

#include "monocypher.h"
#include "picosha2.h"

#include "block.h"

namespace Keten {

	void generateKeyPair(std::string& out_publicKey, std::string& out_privateKey);
	std::string signMessage(const std::string& messageHash, const std::string& privateKey);
	bool validateSignature(const std::string& messageHash, const std::string& signature, const std::string& publicKey);

	std::string calculateHash(const std::string& input);

}