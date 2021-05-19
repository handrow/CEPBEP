#ifndef CONFIG_PARSER_H_
# define CONFIG_PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cstdio>

std::vector<std::string> stringSplit(std::string str, char sign);

namespace ft {

class config
{
 private:

	struct __interior
	{
		std::map<std::string, std::string> fields;
		std::map<std::string, __interior> subcategory;
	};

	std::map<std::string, __interior> __category;

	void pushSubcategory(std::vector<std::string> conf, size_t count);

 public:

	typedef __interior							categoryObject;
	typedef std::map<std::string, __interior>	categoryType;

	config(std::string __fileName);
	config(config const &other);

	categoryObject				getCategoryObject(std::string category);
	categoryObject				getCategoryObject(categoryObject object, std::string category);

	std::vector<std::string>	getCategoryInteriorFields(std::string category); // category("logger/http")
	std::vector<std::string>	getCategoryInteriorFields(categoryObject object, std::string category);

	std::vector<std::string>	getSubcategory(std::string category);
	std::vector<std::string>	getSubcategory(categoryObject object, std::string category);

	std::vector<std::string>	getCategory();
	std::string					fieldData(categoryObject const &objeck, std::string field);
};

};

#endif