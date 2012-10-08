#ifndef BMVC_JSON
#define BMVC_JSON

//because of the cyclic dependence of the class nature, using generic
//programming to generalize the use of this function is almost impossible
//therefore, I will leave 2 macro definitions here and it will expand to
//customize the container for array and the container for map
//Macros are evil, so use it with care
//NOTE: the container HAS to accept incomplete types, so STL container is not
//going to work. 


#include <string>
#include <cstring>    // for memset 

#define BOOST_SETTING_2

//one possible setting for boost without boost fusion
#if defined BOOST_SETTING_1
#include <boost/container/vector.hpp>
#include <boost/container/list.hpp>
#include <boost/container/map.hpp>
#include <boost/unordered_map.hpp>
#define JSON_ARRAY_TYPE boost::container::vector<JSON>
#define JSON_MAP_TYPE boost::unordered_map<std::string, JSON>

#elif defined BOOST_SETTING_2
#include <boost/container/vector.hpp>
#include <boost/unordered/unordered_map.hpp>
#define JSON_ARRAY_TYPE boost::container::vector<JSON>
#define JSON_MAP_TYPE boost::unordered_map<std::string, JSON>

#endif





#include <iostream>
#include <cassert>

using namespace std;
enum storageType
{
    no_type,
    type_str,
    type_obj,
    type_array,
    type_double,
    type_bool,
};

class JSONSyntaxError : public std::exception
{
public:
    JSONSyntaxError(std::string msg = "JSON syntax error!",
        int p_pos = -1, string p_JSONString = string() ) 
        :message(msg), pos(p_pos), JSONString(p_JSONString) {}
    virtual ~JSONSyntaxError() throw(){}
    const virtual char * what () const throw() { 
        if(pos == -1)
            return message.c_str(); 
        else
            return string(message.c_str() + JSONString.substr(pos, 30)).c_str();
    }
private:
    std::string message;
    int pos;
    std::string JSONString;
};

class JSONOperationException : public std::exception
{
public:
    JSONOperationException(std::string msg="Unexpected operation!"):message(msg) {}
    virtual ~JSONOperationException() throw() {}
    const virtual char * what () const throw() { return message.c_str(); }
private:
    std::string message;
};


class JSON
{
  public:
    //typedefs
    typedef JSON_MAP_TYPE Map_t;
    typedef JSON_ARRAY_TYPE Arr_t;

    //Constructors and destructors
    JSON ();
    JSON(std::string &content);
    // boolean initialization has to be explicit because of the C++'s
    // often implicit conversion to boolean values
    explicit JSON(bool content);
    JSON(int content);
    JSON(double content);
    JSON(const Arr_t & rhs);
    JSON(const Map_t & rhs);
    JSON(const JSON & rhs);
    ~JSON();

    //assignment operator overload
    //different from the constructor, it will return a reference
    JSON & operator=(const JSON & rhs);
	JSON & operator=(int u_int);
    JSON & operator=(std::string & content);
    JSON & operator=(double curDouble);
    JSON & operator=(const char * content);
    // boolean assignment operator is disabled because there will be a lot
    // of ambiguity. It has to be loaded using an () operator
    void operator()(bool u_bool);

    //index operator overloads
    JSON & operator[](int i);
    JSON & operator[](std::string key);
    JSON & operator[](const char *key);

    // very important information that will provide the STL like functions
    void clear();
    bool empty() const;
    int  type() const;
    void swap(JSON & rhs);
    void push_back(JSON rhs);
    void push_back(const string & key, JSON & rhs);
    void append(const Arr_t & arr);
    void append(const Map_t & map);

    // need to change to queue-based implementation next for better
    // performance; right now recursion is used to solve the problem
    // which requires a lot of temporary variables
    std::string json_encode(int level = 0, bool noTab = false) const;

    //move constructor and move assignment operator overload
    JSON(JSON && rhs);
    JSON & operator=(JSON && rhs);

    //explicit conversion operator, which allow users to obtain data from
    //a JSON in various format
    explicit operator const char * () const;
    explicit operator string () const;
    explicit operator double() const;
    explicit operator int() const;
    explicit operator const Map_t() const;
    explicit operator const Arr_t() const;


    // iterators
    Arr_t::iterator arr_begin();
    Arr_t::const_iterator arr_begin() const;
    Arr_t::iterator arr_end();
    Arr_t::const_iterator arr_end() const;

    Map_t::iterator map_begin();
    Map_t::const_iterator map_begin() const;
    Map_t::iterator map_end();
    Map_t::const_iterator map_end() const;

    // arithmatic operator
    // arithmatic operator should NEVER work with arrays and map
    // if you want to combine maps or arrays, use append
    JSON & operator+=(const JSON &rhs);
    JSON & operator*=(const JSON &rhs);

    // using unconstrained union
    // always remember to manually destruct the object before initializing
    union Data_t
    {
        char * u_data; // 64 byte on a 64 bit machine
        double u_double;
        bool u_bool;
        std::string u_str;
        Arr_t u_arr;
        Map_t u_map;
        //always set the data to 0 during construction and desctruction
        Data_t  (){} 
        ~Data_t (){}
        Data_t (int val) : u_double(val){}
        Data_t (double val):u_double(val){}
        Data_t (bool val):u_bool(val){}
    };
    Data_t m_data;
    // use a int for better alignment
    int m_type;
};

// non-member functions for the JSON format
// this is preferred over using member functions because it is symmetrical
// which a lot of std functions require
bool operator==(const JSON & lhs, const JSON rhs);
bool operator!=(const JSON & lhs, const JSON rhs);
bool operator<(const JSON & lhs, const JSON rhs);
bool operator>(const JSON & lhs, const JSON rhs);
bool operator<=(const JSON & lhs, const JSON rhs);
bool operator>=(const JSON & lhs, const JSON rhs);

JSON json_decode(const string & rhs)
{
    return JSON();
}

/******************Constructors and destructors begin************/
inline JSON::JSON()
  :m_type(no_type)
{}

inline JSON::JSON(std::string & content)
    :m_type(type_str)
{
    new (&(m_data.u_str)) std::string(content);
}
inline JSON::JSON(bool content)
  :m_type(type_bool)
{
    m_data.u_bool = content;
}
inline JSON::JSON(int content)
    :m_type(type_double)
{
    m_data.u_double= content;
}
inline JSON::JSON(double content)
    :m_type(type_double)
{
    m_data.u_double= content;
}

inline JSON::JSON(const Arr_t &param)
    :m_type(type_array)
{
    new (&(m_data.u_arr)) Arr_t(param);
    //allocate an empty vector and swap content with the current vector 
}

inline JSON::JSON(const Map_t &param)
    :m_type(type_obj)
{
    new (&(m_data.u_map)) Map_t(param);
    //allocate an empty vector and swap content with the current vector 
}

inline
JSON::JSON(const JSON & rhs)
{
    this->m_type = rhs.m_type;
    switch(m_type)
    {
        // for classes that copies memory dynamically, make a deep copy
        // using placement new instead of using memcpy otherwise things will
        // be deallocated as expected when the last object dies
        case type_array:
            new (&(m_data.u_arr)) Arr_t(rhs.m_data.u_arr);
            break;
        case type_obj:
            new (&(m_data.u_map)) Map_t(rhs.m_data.u_map);
            break;
        case type_str:
            new (&(m_data.u_str)) std::string(rhs.m_data.u_str);
            break;
        case type_double:
            m_data.u_double = rhs.m_data.u_double;
            break;
        default:
            m_data.u_bool = rhs.m_data.u_bool; 
            break;
    }
}

inline
JSON::~JSON()
{
    switch(m_type)
    {
        case type_str:
            m_data.u_str.~string();
            break;
        case type_array:
            m_data.u_arr.~Arr_t();
            break;
        case type_obj:
            m_data.u_map.~Map_t();
            break;
    }
}
////////////////////////////////////////////////////////////////////////
/////////////////Constructors and destructors end
////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//Operator Overload begin
///////////////////////////////////////////////////////////////////////

JSON & JSON::operator=(const JSON & rhs)
{
    this->clear();
    this->m_type = rhs.m_type;
    switch(m_type)
    {
        case type_array:
            new (&(m_data.u_arr)) Arr_t(rhs.m_data.u_arr);
            break;
        case type_obj:
            new (&(m_data.u_map)) Map_t(rhs.m_data.u_map);
            break;
        case type_str:
            new (&(m_data.u_str)) std::string(rhs.m_data.u_str);
            break;
        case type_double:
            m_data.u_double = rhs.m_data.u_double;
            break;
        default:
            m_data.u_bool = rhs.m_data.u_bool; 
            break;
    }
    return *this;
}
inline
JSON & JSON::operator=(int u_int)
{
    if(m_type != type_double)
    {
        this->clear();
        this->m_type = type_double;
    }
    this->m_data.u_double = u_int;
    return *this;
}
inline
JSON & JSON::operator=(std::string & content)
{
    if(this->m_type != type_str)
    {
        this->clear();
        this->m_type = type_str;
    }
    new (&(m_data.u_str)) std::string(content);
    return *this;
}
inline
JSON & JSON::operator=(double u_double)
{
    if(m_type != type_double)
    {
        this->clear();
        this->m_type = type_double;
    }
    this->m_data.u_double = u_double;
    return *this;
}

inline
JSON & JSON::operator=(const char * content)
{
    if(this->m_type != type_str)
    {
        this->clear();
        this->m_type = type_str;
    }
    new (&(m_data.u_str)) std::string(content);
    return *this;
}
void JSON::operator()(bool curBool)
{
    this->clear();
    this->m_type = type_bool;
    m_data.u_bool = curBool; 
}

////////////////////////////////////////////////////////////////
//Index operator overload
///////////////////////////////////////////////////////////////

//return a copy of that object from the array
inline
JSON & JSON::operator[](int i)
{
    //turn itself into an array 
    if(m_type != type_array)
    {
        this->clear();
        m_type = type_array;
        new (&(m_data.u_arr)) Arr_t(); 
    }
    //need to return the element at the given index
    int arrayOldSize = m_data.u_arr.size(); 
    //make sure the size of the arrray is big enough 
    if(i >= arrayOldSize) 
    {
        Arr_t append ( i-arrayOldSize +1);
        m_data.u_arr.insert(m_data.u_arr.end(),append.begin(),append.end()); 
    }
    return m_data.u_arr[i];
}

inline
JSON & JSON::operator[](string key)
{
    //turn itself into an array 
    if(m_type != type_obj)
    {
        this->clear();
        m_type = type_obj;
        new (&(m_data.u_map)) Map_t();
    }
    auto it = m_data.u_map.find(key);
    if(it == m_data.u_map.end())
        m_data.u_map[key] = JSON();
    return m_data.u_map[key];
}
inline
JSON & JSON::operator[](const char *key)
{
    //turn itself into an array 
    if(m_type != type_obj)
    {
        this->clear();
        m_type = type_obj;
        new (&(m_data.u_map)) Map_t();
    }
    auto it = m_data.u_map.find(key);
    if(it == m_data.u_map.end())
        m_data.u_map[key] = JSON();
    return m_data.u_map[key];
}
////////////////////////////////////////////////////////////////
//Index operator overload end
///////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
//Functions that provide an STL like interface
////////////////////////////////////////////////////////////////////
inline
void JSON::clear()
{
    switch(m_type)
    {
        case type_array:
            m_data.u_arr.~Arr_t();
            break;
        case type_obj:
            m_data.u_map.~Map_t();
            break; 
        case type_str:
            m_data.u_str.~string();
            break;
        default:
            break;
    }
}
inline
bool JSON::empty() const
{
    switch(m_type)
    {
        case type_array:
            return m_data.u_arr.empty();
        case type_obj:
            return m_data.u_map.empty();
        case no_type:
            return true;
    } 
    return false;
}
inline
void JSON::swap(JSON & rhs)
{
    // store rhs data
    Data_t curData;
    int curType = rhs.m_type;
    memcpy(&curData, &rhs.m_data, sizeof(m_data));

    // copy this to rhs
    rhs.m_type = m_type;
    memcpy(&rhs.m_data, &m_data, sizeof(m_data));

    // copy this to rhs
    m_type = curType;
    memcpy(&m_data, &curData, sizeof(m_data));
}
inline
int JSON::type() const
{
    return this->m_type;
}
inline
void JSON::push_back(JSON rhs)
{
    if(m_type == type_array)
        m_data.u_arr.push_back(rhs);
    else
        throw JSONOperationException();
}
inline
void JSON::push_back(const string & key, JSON & rhs)
{
    if(m_type == type_obj)
        m_data.u_map[key] = rhs;
    else
        throw JSONOperationException();
}
inline
void JSON::append(const Arr_t & arr)
{
    if(m_type == type_array)
    {
        //relocate enough space once off so that the system does not 
        //take too much time to relocate later
        m_data.u_arr.reserve(m_data.u_arr.size() + arr.size());
        m_data.u_arr.insert(m_data.u_arr.end(), arr.begin(), arr.end());
    }
    else
        throw JSONOperationException();
}
inline
void JSON::append(const Map_t & map)
{
    if(m_type == type_obj)
        m_data.u_map.insert(map.begin(), map.end());
    else
        throw JSONOperationException();
}
inline
string JSON::json_encode(int level, bool noTab) const
{
    string current;
    string suffix; 
    string tab(level, '\t');
    switch(this->m_type)
    {
        case type_array:
        {
            current += '\n' + tab+"[\n";
            auto back = --(this->arr_end());
			level++;
            for(auto it = this->arr_begin(); it != back; it++)
            {
                current += tab + '\t' + (*it).json_encode(level) + ",\n";
            }
			current += tab +'\t'+  (*back).json_encode(level) + "\n";
            suffix += tab+"]";
            break;
        }
        case type_obj:
        {
			current += '\n' + tab +"{\n";
            auto end = this->map_end();
			level++;
            auto it = this->map_begin();
            auto next = it;
            next++;
            while(next != end)
            {
				if(noTab)
				{
					current += tab + '"'+it->first+'"'+" : "
                        +it->second.json_encode(level, true)+",\n";
					noTab = true;
				}
				else
				{
					current += tab+'\t' + '"'+it->first+'"'+" : "
                        +it->second.json_encode(level, true)+",\n";
				}
				it++;
                next++;
            }
			if(noTab)
				current += tab + '"'+ it->first+'"'+" : "
                    +it->second.json_encode(level, true)+"\n";
			else
				current += tab+ '\t' + '"'+it->first+'"'+" : "
                    +it->second.json_encode(level, true)+"\n";
			suffix +=tab + "}";
            break;
        }
        case type_str:
        {
			if(noTab)
				current += '"' + this->m_data.u_str + '"';
			else
				current += tab + '"' + this->m_data.u_str + '"';
            break;
        }
        case type_double:
        {
			if(noTab)
				current += to_string(this->m_data.u_double); 
			else
				current += tab + to_string(this->m_data.u_double); 
            break;
        }
        case type_bool:
        {
			if(noTab)
			{
				if(m_data.u_bool)
					current += "true";
				else
					current += "false";
			}
			else
			{
				if(m_data.u_bool)
					current += tab+"true";
				else
					current += tab+"false";
			}
			break;
        }
		case no_type:
			current += tab + "null";
			break;
        default:
            break;
    }
    return current + suffix;
}


//////////////////////////////////////////////////////////////////////
//move constructor and move assignment operator begin
/////////////////////////////////////////////////////////////////////
inline
JSON::JSON(JSON && rhs)
  :m_data(0), m_type(no_type)  
{
    // copy the resource from rhs to current data
    memcpy(&m_data, &(rhs.m_data), sizeof(m_data));
    m_type = rhs.m_type;

    // set the rhs data to zero
    memset(&(rhs.m_data), 0, sizeof(rhs.m_data));
    rhs.m_type = no_type;
}
inline
JSON & JSON::operator=(JSON && rhs)
{
    // make sure it is not itself
    if(this!= &rhs)
    {
        //clear all current content;
        this->clear();
        //copy the resource over
        memcpy(&m_data, &(rhs.m_data), sizeof(m_data)); 
        m_type = rhs.m_type;
        // set the rhs data to zero
        memset(&(rhs.m_data), 0, sizeof(rhs.m_data));
        rhs.m_type = no_type;
    }
    return *this;
}

///////////////////////////////////////////////////////////////////////////
//--------------move constructor and move assignment operator end----------
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
//--------------explicit conversion operator start----------
///////////////////////////////////////////////////////////////////////////
inline
JSON::operator const char * () const
{
    if(this->m_type == type_str)
        return m_data.u_str.c_str();;
    throw JSONOperationException();
}
inline
JSON::operator string ()const
{
    if(this->m_type == type_str)
        return m_data.u_str.c_str();;
    throw JSONOperationException("Not a string type");
}
inline
JSON::operator double()const
{
    switch(m_type)
    {
        case type_double:
            return m_data.u_double;
        case type_bool:
            return (double)m_data.u_bool;
    }
    throw JSONOperationException("Not a double type");
}
inline
JSON::operator int()const
{
    switch(m_type)
    {
        case type_double:
            return (int)m_data.u_double;
        case type_bool:
            return (int)m_data.u_bool;
    }
    throw JSONOperationException("Not an int type");
}
inline
JSON::operator const Map_t()const
{
    if(m_type == type_obj)
        return m_data.u_map;
    throw JSONOperationException("Not a map type");
}

inline
JSON::operator const Arr_t()const
{
    if(m_type == type_array)
        return m_data.u_arr;
    throw JSONOperationException("Not an array type");
}

///////////////////////////////////////////////////////////////////////////
//--------------explicit conversion operator end----------
///////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// iterators begin
////////////////////////////////////////////////////////////////////
inline
JSON::Arr_t::iterator JSON::arr_begin()
{
    if(m_type == type_array)
        return m_data.u_arr.begin();
    throw JSONOperationException("Not an array type");
}
inline
JSON::Arr_t::const_iterator JSON::arr_begin() const
{
    if(m_type == type_array)
        return m_data.u_arr.begin();
    throw JSONOperationException("Not an array type");
}
inline
JSON::Arr_t::iterator JSON::arr_end()
{
    if(m_type == type_array)
        return m_data.u_arr.end();
    throw JSONOperationException("Not an array type");
}
inline
JSON::Arr_t::const_iterator JSON::arr_end() const
{
    if(m_type == type_array)
        return m_data.u_arr.end();
    throw JSONOperationException("Not an array type");
}
inline
JSON::Map_t::iterator JSON::map_begin()
{
    if(m_type == type_obj)
        return m_data.u_map.begin();
    throw JSONOperationException("Not a map type");
}
inline
JSON::Map_t::const_iterator JSON::map_begin() const
{
    if(m_type == type_obj)
        return m_data.u_map.begin();
    throw JSONOperationException("Not a map type");
}
inline
JSON::Map_t::iterator JSON::map_end()
{
    if(m_type == type_obj)
        return m_data.u_map.end();
    throw JSONOperationException("Not a map type");
}
inline
JSON::Map_t::const_iterator JSON::map_end() const
{
    if(m_type == type_obj)
        return m_data.u_map.end();
    throw JSONOperationException("Not a map type");
}
////////////////////////////////////////////////////////////////////
// iterators end 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// non-member helper function begin 
////////////////////////////////////////////////////////////////////

//return the end of the current section of JSON
//if the root is an array, get the key and the value, and insert that into root
int JSONDecode(const string & JSONString, JSON & root, 
    unsigned int begin = 0)
{
	if(JSONString.size() < begin)
		throw JSONSyntaxError("JSON string size less than index to begin with");
	const unsigned int rootType = root.m_type;
	int i = begin;
	const int stringSize = JSONString.size();
	for(; i < stringSize; i ++)
		if(!isspace(JSONString[i]))
			break;
	//now find the real beginChar
	char beginChar = JSONString[i];

	//now seek for the terminator
	if(beginChar == '"')//within a string
	{
		int curBegin = i;
		//check if the root JSON is an object; if it is, then also get the value
		i++;
		//seek for the end of the current string
		for(;i < stringSize; i++)
			if(JSONString[i] == '"' && JSONString[i-1] != '\\')
				break;
		string curStr (JSONString, curBegin+1, i-curBegin-1);
		//i++;
		switch(rootType)
		{
		case type_array:
			{
				root.push_back(JSON(curStr));
				break;//return the end of the current position
			}
		case type_obj: //need to identify the key as well as the value
			{
				//it needs to be matched with a value:
                i++;
				JSON curArr;
				for(; i < stringSize; i++)
					if(!isspace(JSONString[i]))
						break;
				if(JSONString[i] != ':') 
					throw JSONSyntaxError("key is not mapped to a value!");
				i++;
				i = JSONDecode(JSONString, curArr, i);
				root.push_back(curStr, curArr);
				break;
			}
		case no_type: //just fill the root element and return
			{
				root = curStr;
				break;
			}
		}
		return i;
	}
	else if(isdigit(beginChar)) //exponential or beginChar
	{
		char * endPtr;
		double result = strtod( & JSONString[i], &endPtr);
		switch(rootType)
		{
		case type_array:
			{
				root.push_back(JSON(result));
				break;//return the end of the current position
			}
		case no_type:
			{
				root = result;
				break;
			}
		}
        unsigned int j = i + (endPtr - & JSONString[i]-1);
        for(; j < JSONString.size(); j++)
            if(!isspace(JSONString[j]))
                break;
		return j;
	}
	else if(beginChar == 't' || beginChar == 'n')
	{
		if(beginChar == 't' && JSONString[i+1] == 'r' && JSONString[i+2] == 'u' && JSONString[i+3] == 'e')
		{
			switch(rootType)
			{
			case type_array:
                {
                    JSON temp;
                    temp(true);
					root.push_back(temp);
					break;//return the end of the current position
                }
			case no_type:
					root(true);
					break;
			}
			return i + 3;
		}
		else if(beginChar == 'n' && JSONString[i+1] == 'u' && JSONString[i+2] == 'l' && JSONString[i+3] == 'l')
		{
			if(rootType == type_array)
			{
				root.push_back(JSON());
			}
			//case no_type:  do nothing, because the array would have been initialized to no type anyways.
			return i + 3;
		}
		else
			throw JSONSyntaxError("It is not true and not null");
	}
	else if(beginChar == 'f') //false
	{
		if(JSONString[i+1] == 'a' && JSONString[i+2] == 'l' && JSONString[i+3] == 's'
			 && JSONString[i+4] == 'e')
		{
			switch(rootType)
			{
			case type_array:
            {
                JSON temp;
                temp(false);
				root.push_back(temp);
				break;//return the end of the current position
            }
			case no_type:
				root(false);
				break;
			}
			return i + 4;
		}
		else
			throw JSONSyntaxError("It is not false");
	}
	else if (beginChar == '{') // it is an object
	{
		//proceed until the end has been reached
		JSON curObject = JSON(JSON::Map_t());
		int curEnd = i;
		while(curEnd <stringSize)
		{
			curEnd++;
			curEnd = JSONDecode(JSONString,  curObject, curEnd);
			curEnd++;
			for(; curEnd < stringSize; curEnd++)
				if(!isspace(JSONString[curEnd]))
					break;
			if(stringSize <= curEnd)
				throw JSONSyntaxError("Cannot find the end of the bracket");
			if(JSONString[curEnd] == '}')// that is the end of it, return
			{
				switch(rootType)
				{
				case type_array:
					root.push_back(curObject);
					break;
				case no_type:
					root = curObject;
					break;
				}
				return curEnd;
			}
			if(JSONString[curEnd] == ',')
				continue;//curEnd++; //increase by one and continue;
		}
		//now we have the content, make sure that the content is root
		throw JSONSyntaxError("End bracket of object is not found");
	}
	else if (beginChar == '[') // it is an array
	{
		//proceed until the end has been reached
		JSON curArray = JSON(JSON::Arr_t());
		int curEnd = i;
		while(curEnd <stringSize)
		{
			curEnd++;
			curEnd = JSONDecode(JSONString,  curArray, curEnd);
			curEnd++;
			for(; curEnd < stringSize; curEnd++)
				if(!isspace(JSONString[curEnd]))
					break;
			if(stringSize <= curEnd)
				throw JSONSyntaxError("Cannot find end bracket for the array");
			if(JSONString[curEnd] == ']')// that is the end of it, return
			{
				switch(rootType)
				{
				case type_array:
					root.push_back(curArray);
					break;
				case no_type:
					root = curArray;
					break;
				}
				return curEnd;
			}
			if(JSONString[curEnd] == ',')
				continue;//curEnd++; //increase by one and continue;
		}
		//if reached here, something funny happened
		throw JSONSyntaxError("End bracket of array is not found");
	}
	else
		throw JSONSyntaxError("none of the cases matched", begin, JSONString);
}

////////////////////////////////////////////////////////////////////
// non-member helper function end 
////////////////////////////////////////////////////////////////////

#endif
