#include <fstream>
#include <iostream>

#include "config/config.h"
#include "config/utils.h"

namespace Config {

std::vector<std::string> stringSplit(std::string str, char sign) {
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

void trim(std::string &str, char sing) {
    while (str[0] == sing)
        str.erase(0, 1);
    while (str[str.length() - 1] == sing)
        str.erase(str.length() - 1, 1);
}

void Category::SetField(const std::string& fname, const std::string& fvalue) {
    __fields.insert(Field(fname, fvalue));
}

void Category::RemoveField(const std::string& fname) {
    __fields.erase(fname);
}

Category::Field& Category::GetFieldRef(const std::string& fname) {
    FieldsIter element = __fields.find(fname);
    return (*element);
}

const Category::Field& Category::GetFieldRef(const std::string& fname) const {
    FieldsConstIter element = __fields.find(fname);
    return (*element);
}

std::string         Category::GetFieldValue(const std::string& fname) const {
    return (__fields.find(fname)->second);
}

void Category::SetSubcategory(const std::string& cname, const Category& subcat) {
    __subs.insert(std::pair<std::string, Category>(cname, subcat));
}

void Category::RemoveSubcategory(const std::string& fname) {
    __subs.erase(fname);
}

Category& Category::GetSubcategoryRef(const std::string& fname) {
    SubcategoryIter element = __subs.find(fname);
    return (element->second);
}

const Category& Category::GetSubcategoryRef(const std::string& fname) const {
    SubcategoryConstIter element = __subs.find(fname);
    return (element->second);
}

usize               Category::CountSubcategories() const {
    return(__subs.size());
}
usize               Category::CountFields() const {
    return(__fields.size());
}

Category::SubcategoryRange    Category::GetSubcatoryIterRange() {
    return (SubcategoryRange(__subs.begin(), __subs.end()));
}

Category::SubcategoryConstRange Category::GetSubcatoryIterRange() const {
    return (SubcategoryConstRange(__subs.begin(), __subs.end()));
}

Category::FieldsRange         Category::GetFieldsIterRange() {
    return (FieldsRange(__fields.begin(), __fields.end()));
}

Category::FieldsConstRange    Category::GetFieldsIterRange() const {
    return (FieldsConstRange(__fields.begin(), __fields.end()));
}

std::string EditLine(std::string line, std::ifstream& file) {
    if (line.size() == 0) {
        getline(file, line);
        return (EditLine(line, file));
    }
    if (line.find(';') <= line.size())
        line = line.substr(0, line.find(';'));
    trim(line, ' ');
    return (line);
}

std::vector<std::string> FileReading(const std::string& filepath, Error *err) {
    std::ifstream file(filepath);
    std::vector<std::string> data;

    if (!file.is_open())
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File open error");
    else
        *err = Error(CONF_NO_ERR, "No error");
    for (std::string buffer; getline(file, buffer); ) {
        buffer = EditLine(buffer, file);
        if (buffer.size())
            data.push_back(buffer);
    }
    return (data);
}

std::pair<Category, usize> Category::ParseFromCategory(const std::vector<std::string>& data, usize start) {
    Category subcat;
    std::string fild_name;
    std::string fild_value;
    while (data[start][0] != '[' && start < data.size()) {
        trim(fild_name = data[start].substr(0, data[start].find('=')), ' ');
        trim(fild_value = data[start].substr(data[start].find('=') + 1, data[start].size() - 1), ' ');
        subcat.SetField(fild_name, fild_value);
        if (data[start + 1][0] == '[')
            break;
        start++;
    }
    return (std::pair<Category, usize>(subcat, start));
}

Category Category::ParseFromINI(const std::string& filepath, Error *err) {
    std::vector<std::string> data = FileReading(filepath, err);
    Category global;
    if (err->IsError())
        return global;
    std::pair<Category, usize> subcat_and_count = global.ParseFromCategory(data, 0);
    global = subcat_and_count.first;
    for (usize i = subcat_and_count.second; i < data.size(); i++) {
        if (data[i][0] == '[' && data[i].find(':') > data.size()) {
            std::string category_name = data[i].substr(1, data[i].size() - 2);
            std::pair<Category, usize> subcat_and_count = global.ParseFromCategory(data, i + 1);
            global.SetSubcategory(category_name, subcat_and_count.first);
            i = subcat_and_count.second;
        } else if (data[i][0] == '[') {
            std::vector<std::string> path = stringSplit(data[i], ':');
            Category *subcat = &global.GetSubcategoryRef(path[0]);
            for (size_t count = 1; count < path.size() - 1; count++) {
                subcat = &subcat->GetSubcategoryRef(path[count]);
            }
            std::string category_name = path[path.size() - 1];
            subcat_and_count = global.ParseFromCategory(data, i + 1);
            subcat->SetSubcategory(category_name, subcat_and_count.first);
            i = subcat_and_count.second;
        }
    }
    return (global);
}

void Category::WriteToFile(const Category& subcat, std::ofstream& out, std::string &path) const {
    FieldsConstRange fields_range = subcat.GetFieldsIterRange();
    out << std::endl;
    if (path.size() > 0) {
        out << "[" << path << "]" << std::endl;
    }
    for (FieldsConstIter start = fields_range.first; start != fields_range.second; start++) {
        out << start->first << " = " << start->second << std::endl;
    }
    SubcategoryConstRange subs_range = subcat.GetSubcatoryIterRange();
    for (SubcategoryConstIter start = subs_range.first; start != subs_range.second; start++) {
        if (path.size() > 0)
            path = path + ':';
        for (usize i = 0; i < start->first.size(); i++)
            path = path + start->first[i];
        subcat.WriteToFile(start->second, out, path);
        for (usize count = path.size(); path[count - 1] != ':' && count > 1; ) {
            path.pop_back();
            count = path.size();
        }
        path.pop_back();
    }
}

void           Category::DumpToINI(const Category& config_obj, const std::string& filepath, Error *err) {
    std::ofstream out(filepath);
    std::string path;

    if (!out.is_open()) {
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File creation error");
        return;
    }
    *err = Error(CONF_NO_ERR, "File creation error");
    config_obj.WriteToFile(config_obj, out, path);
}

}  // namespace Config
