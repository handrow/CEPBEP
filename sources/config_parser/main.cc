#include "config_parser.h"

class ll
{
    private:
    std::string &name;
    public:
    ll(std::string &n) : name(n)
    {}
    void setname(std::string &n){name=n;}
    void getname(){std::cout << name << "\n";}
};

int main()
{
    std::string file = "default.cfg";
    ConfigParser::Category conf(file);
    ConfigParser::Category::Category& cat = conf.category("server1").category("location");
    cat.SetField(std::string("A"), std::string("B"));
    std::cout << cat.HasValue("A") << "\n";
}



/*
A=gggg
B=aaaa

[server]
a=1
b=2
b=3

[server_2]
z=1
x=2
y=3

[server:http]
param=1
key1=
key2=22

*/

/*
int main() {

    config::Category conf = config::parse_from_ini("/path/to/config.ini");

    {
        config::Category& cat = conf.category("server").category("http");
        cat.set_field("param", "2");
        cat.set_field(std::string("key3"), std::string("Some text"));
        cat.field("key3").value(); // returns reference string("Some text")
    }

    {
        conf.set_field("global_param", "value"); // new field
        conf.set_field("A", "1234"); // change value of A
    }

    {
        conf.category("server").has_category("http"); // returns true
        conf.category("server").has_category("No category"); // returns false
        conf.category("server").has_value("a"); // returns true
        conf.category("server").has_value("No value"); // returns false
    }

    {
        conf.remove_field("global_param"); // removes field
    }

    {
        config::Field& field = conf.field("A");

        field.value() = "1234";
        field.value(); // returns std::string reference to value ("1234")
        filed.key(); // returns const std::string reference to key ("A");
    }

    {
        config::Category::FieldIterator f_iter = conf.get_field_iter_begin();

        *f_iter; // returs Field reference
        f_iter->value(); // returns std::string reference to value ("1234")
        f_iter->key(); // returns const std::string reference to key ("A");

        f_iter.next();
        f_iter->value(); // returns std::string reference to value ("aaaa")
        f_iter->key(); // returns const std::string reference to key ("B");

        f_iter.is_end(); // false
        f_iter.next();
        f_iter.is_end(); // true
        f_iter.prev();
        f_iter.is_end(); // false
        f_iter->value(); // returns std::string reference to value ("aaaa")
        f_iter->key(); // returns const std::string reference to key ("B");

        f_iter.prev();
        f_iter.is_rend(); // false
        f_iter.prev();
        f_iter.is_rend(); // true
    }

    {
        config::Category::CategoryIterator c_iter = conf.get_category_iter_begin();

        *c_iter; // returs Category reference
        c_iter->name(); // returns reference to string "server";
        c_iter->name() = "Booo"; // renames category "server" to "Booo";
                                 //      NOTE: it is category's method:  name() -> std::string&

        c_iter.next();
        c_iter->name(); // returns "server_2";
        c_iter.is_end(); // false

        c_iter.next();
        c_iter.is_end(); // true

        c_iter.prev();
        c_iter.is_rend(); // false
        c_iter.prev();
        c_iter.is_rend(); // false
        c_iter.prev();
        c_iter.is_rend(); // true
    }

    {
        config::Category cat;
        config::Field field1("Hero", "ABC");
        config::Field field2("boo", "daaaaa");

        cat.name() = "new_category"; 
        cat.set_field("boo", "fooo");
        cat.set_field("doo", "rooo");
        cat.add_field(field1);
        cat.add_field(field2); // returns and error code, such field already exists

        conf.add_category(cat); // adds new category

        conf.category("new_category").count_fields() // returns 3;
        conf.category("new_category").add_category(cat);
        conf.category("new_category").count_categories(); // returns 1;
        conf.remove_category("new_category"); // removes this category and all it's fields and subcategories
    }

    config::dump_to_ini(conf, "/path/to/new/ini_file.ini");

}

*/