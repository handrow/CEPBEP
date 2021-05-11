#include "config_parser.h"

std::vector<std::string> FileReading(std::string file_name) {
	std::vector<std::string> config;
	std::ifstream file(file_name);
	// Error output

	if (!file.is_open()) {
		perror("Error");
		return config;
	}
	for (std::string buffer; getline(file, buffer); ) {
		if (buffer.size() > 0)
			config.push_back(buffer);
	}
	return config;
}

std::map<std::string, std::string> ParserCategory(std::vector<std::string> config, size_t start) {
	std::map<std::string, std::string> category;
	for (size_t i = ++start; i < config.size() && config[i][0] != '['; i++) {
		category.insert(std::pair<std::string, std::string>(config[i].substr(0, config[i].find('=')), config[i].substr(config[i].find('=') + 1, config[i].size() - 1)));
	}
	return category;
}

std::map<std::string, std::map<std::string, std::string> > Parser(std::vector<std::string> config) {
	std::map<std::string, std::map<std::string, std::string> > data;
	std::map<std::string, std::string> category;

	for (size_t i = 0; i < config.size(); i++) {
		if (config[i][0] == '[') {
			category = ParserCategory(config, i);
			data.insert(std::pair<std::string, std::map<std::string, std::string> >(config[i].substr(1, config[i].size() - 2), category));
		}
	}
	return data;
}