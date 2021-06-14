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

bool                  Category::HasField(const std::string& fname) const {
    FieldsConstIter element = __fields.find(fname);
    if (element != __fields.end())
        return true;
    return false;
}

bool                  Category::HasCategory(const std::string& cname) const {
    SubcategoryConstIter element = __subs.find(cname);
    if (element != __subs.end())
        return true;
    return false;
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

std::pair<Category, usize> Category::ParseFromCategory(std::ifstream &file, std::string& buffer) {
    Category subcat;
    std::string fild_name;
    std::string fild_value;
    usize check_end_file = 0;
    if (getline(file, buffer))
        check_end_file = 1;
    buffer = EditLine(buffer, file);
    while (check_end_file) {
        if (buffer.size()) {
            trim(fild_name = buffer.substr(0, buffer.find('=')), ' ');
            trim(fild_value = buffer.substr(buffer.find('=') + 1, buffer.size() - 1), ' ');
            subcat.SetField(fild_name, fild_value);
        }
        if (!getline(file, buffer))
            check_end_file = 0;
        buffer = EditLine(buffer, file);
        if (buffer[0] == '[')
            break;
    }
    return (std::pair<Category, usize>(subcat, check_end_file));
}

Category Category::ParseFromINI(const std::string& filepath, Error *err) {
    Category global;
    std::string buffer;
    std::ifstream file(filepath);
    if (!file.is_open())
        *err = Error(CONF_FILE_NOT_FOUND_ERR, "File open error");
    else
        *err = Error(CONF_NO_ERR, "No error");
    if (err->IsError())
        return global;
    std::pair<Category, usize> subcat_and_count = global.ParseFromCategory(file, buffer);
    usize check_end_file = subcat_and_count.second;
    global = subcat_and_count.first;
    while (check_end_file) {
        if (buffer[0] == '[' && buffer.find(':') > buffer.size()) {
            std::string category_name = buffer.substr(1, buffer.size() - 2);
            std::pair<Category, usize> subcat_and_count = global.ParseFromCategory(file, buffer);
            global.SetSubcategory(category_name, subcat_and_count.first);
            check_end_file = subcat_and_count.second;
        } else if (buffer[0] == '[') {
            std::vector<std::string> path = stringSplit(buffer, ':');
            Category *subcat = &global.GetSubcategoryRef(path[0]);
            for (size_t count = 1; count < path.size() - 1; count++) {
                subcat = &subcat->GetSubcategoryRef(path[count]);
            }
            std::string category_name = path[path.size() - 1];
            subcat_and_count = global.ParseFromCategory(file, buffer);
            subcat->SetSubcategory(category_name, subcat_and_count.first);
            check_end_file = subcat_and_count.second;
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
