#include <vector>
#include <iterator>
#include <fstream>
#include <stdexcept>
#include <stdio.h>
#include <iostream>
#include <cstdlib>

using namespace std;

typedef unsigned char uchar;


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
	i++;
	// find function end
	int j = i + 1;
	while (dump[j] != "")
		j++;
	j += 2;
	return pair<int, int>(i, j);
}


int hex_to_int(string hex) {
	unsigned long hex_value = std::strtoul(hex.c_str(), 0, 16);
    return (int) hex_value;
}


int extract_address(string line) {
	string address = split(line, ':')[0];
	address = address.substr(1);
	return hex_to_int(address);
}


void encode(vector<uchar>& data, int start, int end) {
	const uchar key = 56;

    for (int i = start; i < end; ++i)
    	data[i] ^= key;
}


int main(int argc, char const *argv[])
{
	ifstream orig_file(argv[1], ios_base::binary);

	if(orig_file.fail())
    {
    	cout << "Error Opening Input File [" << argv[1] << "]. " << strerror(errno) << "\n";
    	return 0;
    }

	orig_file.seekg(0, std::ios::end);
	unsigned int len = orig_file.tellg();
	orig_file.seekg(0, std::ios::beg);

	std::vector<unsigned char> data(len);
	orig_file.read((char*) &data[0], len);
	orig_file.close();

	cout << "reading success" << endl;

	string cmd = "objdump -section=__text -d " + string(argv[1]);
 	string objdump = exec(cmd.c_str()); 	
 	vector<string> dump = split(objdump);

 	pair<int, int> be = get_function_be(dump, "foo");
 	cout << dump[be.first] << endl << dump[be.second] << endl;
 	cout << extract_address(dump[be.first]) << endl << extract_address(dump[be.second]) << endl;

 	encode(data, extract_address(dump[be.first]), extract_address(dump[be.second])); 

	cout << "encoding success" << endl;

	ofstream fout(argv[1], ios::out | ios::binary);
 	fout.write((char*)&data[0], data.size() * sizeof(unsigned char));
 	fout.close();

 	cout << "writing success" << endl;

	return 0;
}