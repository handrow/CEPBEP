#ifndef CONFIG_PARSER_CONFIG_PARSER_H_
# define CONFIG_PARSER_CONFIG_PARSER_H_

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <cstdio>

namespace ft {

std::vector<std::string> stringSplit(std::string str, char sign);
void trim(std::string &str);

class Config {
 private:
    struct __interior {
        std::map<std::string, std::string> fields;
        std::map<std::string, __interior*> subcategory;
    };

    std::map<std::string, __interior*> __category;

    std::vector<std::string>            FileReading(std::string file_name);
    std::map<std::string, std::string>  ParserCategory(std::vector<std::string> config, size_t start);
    void                                pushSubcategory(std::vector<std::string> conf, size_t count);

 public:
    typedef __interior                            categoryObject;
    typedef std::map<std::string, __interior*>    categoryType;
    typedef std::map<std::string, std::string>    fieldsType;

    explicit Config(std::string __fileName);
    explicit Config(Config const &other);

    std::vector<std::string>    getCategory();
    categoryObject              *getCategoryObject(std::string const &category);
    categoryObject              *getCategoryObject(categoryObject *object, std::string const &category);

    std::vector<std::string>    getCategoryInteriorFields(std::string const &category);  // category("logger:http")
    std::vector<std::string>    getCategoryInteriorFields(categoryObject *object);
    std::vector<std::string>    getCategoryInteriorFields(categoryObject *object, std::string const &category);

    std::vector<std::string>    getSubcategory(std::string const &category);
    std::vector<std::string>    getSubcategory(categoryObject *object);
    std::vector<std::string>    getSubcategory(categoryObject *object, std::string const &category);

    std::string                  fieldData(std::string const &category,  std::string const &field);
    std::string                  fieldData(categoryObject *objeck, std::string const &field);
};

};  // namespace ft

#endif  // CONFIG_PARSER_CONFIG_PARSER_H_
