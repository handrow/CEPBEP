#include "config_parser.h"

std::vector<std::string> ft::stringSplit(std::string str, char sign) {
    std::vector<std::string> array;
        for (size_t count = 0; str.find(sign) < str.size(); ++count) {
            array.push_back(str.substr(0, str.find(sign)));
            str = str.substr(str.find(sign) + 1, str.size());
        }
    array.push_back(str);
    return array;
}

void ft::trim(std::string &str) {
    while (str[0] == ' ' || str[0] == '	')
        str.erase(0, 1);
    while (str[str.length() - 1] == ' ' || str[str.length() - 1] == '	')
        str.erase(str.length() - 1, 1);
}

std::vector<std::string> ft::Config::FileReading(std::string file_name) {
    std::vector<std::string> conf;
    std::ifstream file(file_name);
    // Error output

    if (!file.is_open()) {
        perror("Error");
        return conf;
    }
    for (std::string buffer; getline(file, buffer); ) {
        if (buffer.size() > 0) {
            if (buffer.find(';'))
                buffer = buffer.substr(0, buffer.find(';'));
            trim(buffer);
            conf.push_back(buffer);
        }
    }
    return conf;
}

std::map<std::string, std::string> ft::Config::ParserCategory(std::vector<std::string> config, size_t start) {
    std::map<std::string, std::string> category;
    std::string first;
    std::string second;
    for (size_t i = ++start; i < config.size() && config[i][0] != '['; i++) {
        trim(first = config[i].substr(0, config[i].find('=')));
        trim(second = config[i].substr(config[i].find('=') + 1, config[i].size() - 1));
        category.insert(std::pair<std::string, std::string>(first, second));
    }
    return category;
}

void ft::Config::pushSubcategory(std::vector<std::string> conf, size_t count) {
    // std::cout << conf[count] << "\n";
    std::vector<std::string> path = stringSplit(conf[count], ':');
    // std::cout << conf[count] << "\n\n";
    categoryType::iterator it = __category.find(path[0]);
    categoryType::iterator itPrev;
    // categoryObject *location;
    if (it->first != path[0]) {
        categoryObject *newBlock = new categoryObject;
        it = __category.insert(std::pair<std::string, __interior*>(path[0], newBlock)).first;
    }
    for (size_t indexPath = 1; indexPath < path.size(); indexPath++) {
        // location = &(it->second);
        itPrev = it;
        it = it->second->subcategory.find(path[indexPath]);
        if (it->first != path[indexPath]) {
            categoryObject *newBlock = new categoryObject;
            it = itPrev->second->subcategory.insert(std::pair<std::string, __interior*>(path[indexPath], newBlock)).first;
            // location = it->second;
        }
    }
    it->second->fields = ParserCategory(conf, count);
}

ft::Config::Config(std::string __fileName) {
    std::vector<std::string> conf = FileReading(__fileName);
    categoryObject *newBlock;

    for (size_t i = 0; i < conf.size(); i++) {
        if (conf[i].find(':') < conf[i].size() && conf[i][0] == '[') {
            conf[i] = conf[i].substr(1, conf[i].size() - 2);
            pushSubcategory(conf, i);
        } else if (conf[i][0] == '[') {
            newBlock = new categoryObject;
            categoryType::iterator save = __category.insert(std::pair<std::string, categoryObject*>(conf[i].substr(1, conf[i].size() - 2), newBlock)).first;
            save->second->fields = ParserCategory(conf, i);
            // (--__category.end())->second.fields = ParserCategory(conf, i);
        }
    }
}
















ft::Config::categoryObject                *ft::Config::getCategoryObject(std::string const &category) {
    categoryObject    *objectRet;
    categoryType    location = __category;
    std::vector<std::string> path = stringSplit(category, ':');

    for (size_t count = 0; count < path.size(); count++) {
        for (categoryType::iterator start = location.begin(); start != location.end(); start++) {
            if (start->first == path[count] && count < path.size() - 1) {
                location = start->second->subcategory;
                break;
            } else if (start->first == path[count]) {
                objectRet = start->second;
            }
        }
    }
    return objectRet;
}

ft::Config::categoryObject   *ft::Config::getCategoryObject(categoryObject *object, std::string const &category) {
    categoryObject    *objectRet;
    categoryType    location = object->subcategory;
    std::vector<std::string> path = stringSplit(category, ':');

    for (size_t count = 0; count < path.size(); count++) {
        for (categoryType::iterator start = location.begin(); start != location.end(); start++) {
            if (start->first == path[count] && count < path.size() - 1) {
                location = start->second->subcategory;
                break;
            } else if (start->first == path[count]) {
                objectRet = start->second;
            }
        }
    }
    return objectRet;
}

std::vector<std::string>    ft::Config::getCategoryInteriorFields(std::string const &category) {
    std::vector<std::string>     fieldsRet;
    categoryType    location = __category;
    std::vector<std::string> path = stringSplit(category, ':');

    for (size_t count = 0; count < path.size(); count++) {
        for (categoryType::iterator start = location.begin(); start != location.end(); start++) {
            if (start->first == path[count] && count < path.size() - 1) {
                location = start->second->subcategory;
                break;
            } else if (start->first == path[count] && count == path.size() - 1) {
                for (fieldsType::iterator first = start->second->fields.begin(); first != start->second->fields.end(); first++)
                    fieldsRet.push_back(first->first);
                break;
            }
        }
    }
    return fieldsRet;
}

std::vector<std::string>    ft::Config::getCategoryInteriorFields(categoryObject *object) {
    std::vector<std::string> fieldsRet;
    for (fieldsType::iterator start = object->fields.begin(); start != object->fields.end(); start++)
        fieldsRet.push_back(start->first);
    return fieldsRet;
}

std::vector<std::string>    ft::Config::getCategoryInteriorFields(categoryObject *object, std::string const &category) {
    std::vector<std::string>     fieldsRet;
    categoryType    location = object->subcategory;
    std::vector<std::string> path = stringSplit(category, ':');

    for (size_t count = 0; count < path.size(); count++) {
        for (categoryType::iterator start = location.begin(); start != location.end(); start++) {
            if (start->first == path[count] && count < path.size() - 1) {
                location = start->second->subcategory;
                break;
            } else if (start->first == path[count] && count == path.size() - 1) {
                for (fieldsType::iterator first = start->second->fields.begin(); first != start->second->fields.end(); first++)
                    fieldsRet.push_back(first->first);
                break;
            }
        }
    }
    return fieldsRet;
}

std::vector<std::string>    ft::Config::getSubcategory(std::string const &category) {
    std::vector<std::string>     fieldsRet;
    categoryType    location = __category;
    std::vector<std::string> path = stringSplit(category, ':');

    for (size_t count = 0; count < path.size(); count++) {
        for (categoryType::iterator start = location.begin(); start != location.end(); start++) {
            if (start->first == path[count] && count < path.size() - 1) {
                location = start->second->subcategory;
                break;
            } else if (start->first == path[count] && count == path.size() - 1) {
                for (categoryType::iterator first = start->second->subcategory.begin(); first != start->second->subcategory.end(); first++)
                    fieldsRet.push_back(first->first);
                break;
            }
        }
    }
    return fieldsRet;
}

std::vector<std::string>    ft::Config::getSubcategory(categoryObject *object, std::string const &category) {
    std::vector<std::string>     fieldsRet;
    categoryType    location = object->subcategory;
    std::vector<std::string> path = stringSplit(category, ':');

    for (size_t count = 0; count < path.size(); count++) {
        for (categoryType::iterator start = location.begin(); start != location.end(); start++) {
            if (start->first == path[count] && count < path.size() - 1) {
                location = start->second->subcategory;
                break;
            } else if (start->first == path[count] && count == path.size() - 1) {
                for (categoryType::iterator first = start->second->subcategory.begin(); first != start->second->subcategory.end(); first++)
                    fieldsRet.push_back(first->first);
                break;
            }
        }
    }
    return fieldsRet;
}

std::string    ft::Config::fieldData(std::string const &category, std::string const &field) {
    categoryObject *location = getCategoryObject(category);
    return fieldData(location, field);
}

std::string    ft::Config::fieldData(categoryObject *objeck, std::string const &field) {
    std::string fieldRet;

    for (fieldsType::iterator start = objeck->fields.begin(); start != objeck->fields.end(); start++) {
        if (start->first == field)
            return fieldRet = start->second;
    }
    return fieldRet;
}

std::vector<std::string>    ft::Config::getCategory() {
    std::vector<std::string> category;

    for (categoryType::iterator start = __category.begin(); start != __category.end(); ++start)
        category.push_back(start->first);
    return category;
}
