#include <vector>
#include <iterator>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <iostream>
#include <cstdlib>

using namespace std;

typedef unsigned char uchar;

const string SUCCESS = "[ OK ] ";
const string FAIL    = "[FAIL] ";
const pair<int, int> FUNC_NOT_FOUND = pair<int, int>(-1, -1);


vector<string> split(string s, char sep='\n') {
	vector<string> res = vector<string>();
	int idx = 0;
	while (idx < s.length()) {
		string temp = "";
		while (s[idx] != sep) {
			temp += s[idx];
			idx++;
		}
		res.push_back(temp);
		idx++;
	}
	return res;
}


string exec(const char* cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}


pair<int, int> get_function_be(vector<string> dump, string func_name) {
	// find function begin
	int i = 0;
	for (i = 0; i < dump.size(); ++i) {
		if (dump[i] == "_" + func_name + ":")
			break;
	}
	if (i == dump.size()) 
		return FUNC_NOT_FOUND;
	i++;
	// find function end
	int j = i + 1;
	while (dump[j] != "")
		j++;
	j += 2;
	return pair<int, int>(i, j);
}


int hex_to_int(string hexx) {
	unsigned long hex_value = std::strtoul(hexx.c_str(), 0, 16);
    return (int) hex_value;
}


int extract_address(string line) {
	string address = split(line, ':')[0];
	address = address.substr(1);
	return hex_to_int(address);
}


void encode(vector<uchar>& data, uchar key, int start, int end) {
    for (int i = start; i < end; ++i)
    	data[i] ^= key;
}


int main(int argc, char const *argv[])
{
	ifstream orig_file(argv[1], ios_base::binary);

	if(orig_file.fail())
    {
    	cout << FAIL << "Error Opening Input File [" << argv[1] << "]. " << strerror(errno) << "\n";
    	return 0;
    }

	orig_file.seekg(0, std::ios::end);
	unsigned int len = orig_file.tellg();
	orig_file.seekg(0, std::ios::beg);

	std::vector<unsigned char> data(len);
	orig_file.read((char*) &data[0], len);
	orig_file.close();

	cout << SUCCESS << "reading" << endl;

	string cmd = "objdump -section=__text -d " + string(argv[1]);
 	string objdump = exec(cmd.c_str()); 	
 	vector<string> dump = split(objdump);

 	if (argc < 3) 
 		cout << FAIL << "no function names to encode" << endl;

 	uchar key;
 	unsigned int key_int;
 	cout << "Enter key: ";
 	cin >> key_int;
 	key = key_int;

 	for (int i = 2; i < argc; ++i) {
 		string func_name = string(argv[i]);
 		pair<int, int> be = get_function_be(dump, func_name);
 		if (be == FUNC_NOT_FOUND) {
 			cout << FAIL << "function " << func_name << " wasn't found" << endl;
 			continue;
 		}
 		encode(data, key, extract_address(dump[be.first]), extract_address(dump[be.second]));
 		cout << SUCCESS << "encoding " << func_name << endl;
 	} 

	ofstream fout(argv[1], ios::out | ios::binary);
 	fout.write((char*)&data[0], data.size() * sizeof(unsigned char));
 	fout.close();

 	cout << SUCCESS << "writing" << endl;

	return 0;
}