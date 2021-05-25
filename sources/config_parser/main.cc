#include "config_parser.h"

int main()
{
	
	ft::Config test("default.cfg");

	std::vector<std::string> category = test.getCategory();

	std::cout << "Category:" << std::endl;
	for (size_t i = 0; i < category.size(); i++)
		std::cout << "  " << category[i] << std::endl;
	std::cout << std::endl;

	std::vector<std::string> subcategory = test.getSubcategory("server1");

	std::cout << "Subcategory server1:" << std::endl;
	for (size_t i = 0; i < subcategory.size(); i++)
		std::cout << "  " << subcategory[i] << std::endl;
	std::cout << std::endl;
	
	std::vector<std::string> fields = test.getCategoryInteriorFields("server1:location /directory");

	std::cout << "Fields server1:location /directory:" << std::endl;
	for (size_t i = 0; i < fields.size(); i++)
		std::cout << "  " << fields[i] << std::endl;
	std::cout << std::endl;

	std::string fieldsData = test.fieldData("server1:location /directory", "index");

	std::cout << "fieldsdata \"index\":" << std::endl << "  ";
	for (size_t i = 0; i < fieldsData.size(); i++)
		std::cout << fieldsData[i];
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;





	ft::Config::categoryObject *object = test.getCategoryObject("server1:location /directory:location *.bla");

	fields = test.getCategoryInteriorFields(object);

	std::cout << "Fields server1:location /directory:location *.bla:" << std::endl;
	for (size_t i = 0; i < fields.size(); i++)
		std::cout << "  " << fields[i] << std::endl;
	std::cout << std::endl;

	fieldsData = test.fieldData(object, "cgi_pass");

	std::cout << "fieldsdata \"cgi_pass\":" << std::endl << "  ";
	for (size_t i = 0; i < fieldsData.size(); i++)
		std::cout << fieldsData[i];
	std::cout << std::endl;

}