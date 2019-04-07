#include <vector>
#include <iterator>
#include <fstream>
#include <iostream>

using namespace std;


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

	cout << len << endl;

	data[3659] = 22;

	cout << "success" << endl;

	ofstream fout(argv[1], ios::out | ios::binary);
 	fout.write((char*)&data[0], data.size() * sizeof(unsigned char));
 	fout.close();

 	cout << "success" << endl;

	return 0;
}