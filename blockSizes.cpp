#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

string blocksFolder = "/mnt/f/Bitcoin/Data/blocks/";

vector<string> getFileContent(string path), split(string s, string delimiter = " ");
void print(string s);
int convertStrToInt(string str);

template<typename T>
string convertNbToStr(const T& number)
{
    ostringstream convert;
    convert << number;
    return convert.str();
}

int main()
{
	vector<string> lines = getFileContent("../piX.txt");
	unsigned int linesSize = lines.size();
	unsigned long long totalSize = 0;
	for(unsigned int linesIndex = 0; linesIndex < linesSize; linesIndex++)
	{
		//if(linesIndex % 10 == 0)
		//	print(convertNbToStr(linesIndex) + " / " + convertNbToStr(linesSize));
		string line = lines[linesIndex];
		vector<string> lineParts = split(line);
		unsigned short linePartsSize = lineParts.size();
		if(linePartsSize != 2)
			print("format problem");
		unsigned short blockNumber = convertStrToInt(lineParts[0]);
		unsigned int blockPosition = convertStrToInt(lineParts[1]);
		string blockNumberStr = lineParts[0];
		unsigned short blockNumberStrLength = blockNumberStr.length();
		//print("blockNumberStr (def - " + convertNbToStr(blockNumberStrLength) + "): " + blockNumberStr);
		for(unsigned short i = 0; i < 5 - blockNumberStrLength; i++) // it seems that I was using blockNumberStr.length() and it was recomputing each round
		{
			blockNumberStr = "0" + blockNumberStr;
			//print("blockNumberStr (add): " + blockNumberStr);
		}
		string filePath = blocksFolder + "blk" + blockNumberStr + ".dat";
		FILE* f = fopen(filePath.c_str(), "rb");
		if(f == NULL)
		{
			print("Failed to open file (" + filePath + ") !");
			//continue;
			return 1;
		}
		fseek(f, blockPosition + 4, SEEK_SET);
		char blockSizeChars[4];
		for(unsigned short i = 0; i < 4; i++)
			blockSizeChars[i] = fgetc(f);
		fclose(f);
		unsigned int blockSize = *(reinterpret_cast<unsigned int*>(blockSizeChars));
		totalSize += blockSize + 8;
		print(convertNbToStr(blockSize + 8));
	}
	//print("totalSize (bytes): " + convertNbToStr(totalSize)); // 1 779 414 479 - now 963 261 638
	return 0;
}

int convertStrToInt(string str)
{
    int number;
    sscanf(str.c_str(), "%d", &number);
    return number;
}

vector<string> getFileContent(string path)
{
    vector<string> vec;
    ifstream infile(path.c_str());
    string line;
    while(getline(infile, line))
        vec.push_back(line);
    return vec;
}

vector<string> split(string s, string delimiter)
{
    vector<string> toReturn;
    size_t pos = 0;
    while((pos = s.find(delimiter)) != string::npos)
    {
        toReturn.push_back(s.substr(0, pos));
        s.erase(0, pos + delimiter.length());
    }
    toReturn.push_back(s);
    return toReturn;
}

void print(string s)
{
	cout << s << endl;
}
