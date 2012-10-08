#include "json.h"
void dummyFunc(JSON & val)
{
    val["HAPPY"] = "customer";
}

JSON getAJSON()
{
    return JSON(1);
}
void populate(JSON & rhs)
{
    JSON data;
    data["id"] = 15;
    data["secondID"] = getAJSON();
    data["name"](false);
    dummyFunc(data);
    data["person"][0]["name"] = "Benny";
    data["person"][0]["id"] = 3294;
    data["person"][1]["name"] = "Cunpu";
    data["person"][1]["id"] = 321312;
    rhs = data;
}
int main()
{
    JSON data;
    populate(data);
    int ID = (int)data["id"];
    assert(((int)data["id"]) == ID);
    assert(((string)data["person"][0]["name"]) == "Benny");
    assert(((int)data["secondID"]) == 1);
    std::cout << data.json_encode() << std::endl;
    JSON decoded;
    JSONDecode(data.json_encode(), decoded);
    std::cout << decoded.json_encode() << std::endl;
    std::string test_data = "{ \"glossary\": { \"title\": \"example glossary\","
		"\"GlossDiv\": { \"title\": \"S\", \"GlossList\": { \"GlossEntry\": {"
        "\"ID\": \"SGML\",\"SortAs\": \"SGML\","
	    "\"GlossTerm\": \"Standard Generalized Markup Language\","
        "\"Acronym\": \"SGML\",\"Abbrev\": \"ISO 8879:1986\","
        "\"GlossDef\": {\"para\": \"A meta-markup language, "
        "used to create markup languages such as DocBook.\","
        "\"GlossSeeAlso\": [\"GML\", \"XML\"]},\"GlossSee\": \"markup\""
        "} } } } }";
    JSON decoded2;
    JSONDecode(test_data, decoded2);
    std::cout << decoded2.json_encode() << std::endl;
    return 0;
}
