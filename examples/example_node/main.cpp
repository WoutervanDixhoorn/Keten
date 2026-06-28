#include <print>
#include <string>

#include "json.hpp"
using json = nlohmann::json;

int main() {
	std::println("Hello, Node!");

	std::string jsonString = R"(
		{
			"Test": 10.0,
			"Kimy": "Mopje"
		}	
	)";
	json data = json::parse(jsonString);

	double testValue = data["Test"].get<double>();
	std::string kimyValue = data["Kimy"].get<std::string>();
	std::println("Test: {}, Kimy: {}", testValue, kimyValue);

	return 0;
}