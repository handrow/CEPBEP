#include "config_parser.h"

std::vector<std::string> ConfigParser::stringSplit(std::string str, char sign) {
    std::vector<std::string> array;
        for (size_t count = 0; str.find(sign) < str.size(); ++count) {
            array.push_back(str.substr(0, str.find(sign)));
            str = str.substr(str.find(sign) + 1, str.size());
        }
    array.push_back(str);
    trim(array[0], '[');
    trim(array[array.size() - 1], ']');
    return (array);
}

void ConfigParser::trim(std::string &str, char sing) {
    while (str[0] == sing || str[0] == '	')
        str.erase(0, 1);
    while (str[str.length() - 1] == sing || str[str.length() - 1] == '	')
        str.erase(str.length() - 1, 1);
}

std::vector<std::string> ConfigParser::Category::FileReading(std::string file_name) {
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
            trim(buffer, ' ');
            if (buffer.find(';') != 0)
                conf.push_back(buffer);
        }
    }
    return (conf);
}

std::map<std::string, std::string> ConfigParser::Category::ParserFields(std::vector<std::string> config, size_t start) {
    std::map<std::string, std::string> fields;
    std::string first;
    std::string second;
    for (size_t i = ++start; i < config.size() && config[i][0] != '['; i++) {
        trim(first = config[i].substr(0, config[i].find('=')), ' ');
        trim(second = config[i].substr(config[i].find('=') + 1, config[i].size() - 1), ' ');
        fields.insert(std::pair<std::string, std::string>(first, second));
    }
    return (fields);
}

ConfigParser::Category::Category(std::string &__fileName) : __name("global") {
    std::vector<std::string> conf = FileReading(__fileName);
    std::vector<std::string> path;

    if (conf[0][0] != '[') {
        __fields = ParserFields(conf, 0);
    }

    for (size_t i = 0; i < conf.size(); i++) {
        if (conf[i].find(':') < conf[i].size() && conf[i][0] == '[') {
            path = stringSplit(conf[i], ':');
            Category *subcategory = &GetCategoryRef(path[0]);
            for (size_t count = 1; count < path.size() - 1; count++) {
                subcategory = &subcategory->GetCategoryRef(path[count]);
            }
                Category new_category(conf, i);
                subcategory->__categories.push_back(new_category);
            //    std::cout << i << "\n";
        } else if (conf[i][0] == '[') {
            Category new_category(conf, i);
            AddCategory(Category(conf, i));
        }
    }
}

ConfigParser::Category::Category(std::vector<std::string> &conf, int start) {
    std::vector<std::string> path = stringSplit(conf[start], ':');
    __name = path[path.size() - 1];
    __fields = ParserFields(conf, start);
}

ConfigParser::Category::~Category() { }

// ConfigParser::Category &ConfigParser::Category::operator=(const Category &other) {
//     __name = other.__name;
//     __fields = other.__fields;
//     __categories = other.__categories;
//     return (*this);
// }

void ConfigParser::Category::SetField(const std::string &key, const std::string &value) {
    __fields.insert(std::pair<std::string, std::string>(key, value));
}

void ConfigParser::Category::RemoveField(const std::string &key) {
    __fields.erase(key);
}

ConfigParser::Category::Category &ConfigParser::Category::category(const std::string &key) {
    std::list<Category>::iterator start = __categories.begin();
    while (start != __categories.end() && start->__name != key) {
        start++;
    }
    return (*start);
}

bool ConfigParser::Category::HasCategory(const std::string &key) {
    for (CategoryList::iterator start = __categories.begin(); start != __categories.end(); start++) {
        if (start->Name() == key)
            return (true);
    }
    return (false);
}

bool ConfigParser::Category::HasValue(const std::string &key) {
    FieldMap::iterator element = __fields.find(key);
    if (element != __fields.end())
        return (true);
    std::cout <<element->first;
    return (false);
}

void ConfigParser::Category::AddCategory(const Category &new_category) {
    __categories.push_back(new_category);
}

std::string                 &ConfigParser::Category::Name() {
    return (__name);
}