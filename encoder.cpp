#include <vector>
#include <iterator>
#include <fstream>
#include <iostream>

using namespace std;

typedef unsigned char uchar;


void encode(vector<uchar>& data) {
	const int start = 0xcf0;
	const uchar key = 56;

    uchar buffer[3] = {0, 0, 0};

    int count = 0;

    while ((count < 3) || not ((buffer[0] == 0xc3) && (buffer[1] == 0x66) && (buffer[2] == 0x66))) {
        buffer[0] = buffer[1];
        buffer[1] = buffer[2];

        uchar instruction = data[start + count];
        data[start + count] = instruction ^ key;

        buffer[2] = instruction;
        count++;
    }
    cout << "last address: " << start + count << endl;
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

	encode(data);

	cout << "encoding success" << endl;

	ofstream fout(argv[1], ios::out | ios::binary);
 	fout.write((char*)&data[0], data.size() * sizeof(unsigned char));
 	fout.close();

 	cout << "writing success" << endl;

	return 0;
}