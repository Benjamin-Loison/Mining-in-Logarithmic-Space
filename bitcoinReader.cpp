#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <dirent.h>
using namespace std;

template<typename T>
string convertNbToStr(const T& number)
{
    ostringstream convert;
    convert << number;
    return convert.str();
}

void print(string s);
vector<string> listFiles(string folder), getFileContent(string path);
bool endsWith(const string&, const string&);

//string blocksFolder = "/mnt/d/
string blocksFolder = "/mnt/c/Users/Benjamin/Desktop/BensFolder/School/ENS/Saclay/Stage/Emmanuelle Anceaume/Stage/important/Bitcoin reader/";

char pathSeparator = '/';
string pathSeparatorStr = "/";

int main()
{
	vector<string> files = listFiles(blocksFolder);
	unsigned int filesSize = files.size();
	for(unsigned int filesIndex = 0; filesIndex < filesSize; filesIndex++)
	{
		string file = files[filesIndex];
		//print(file);
		if(endsWith(file, ".dat"))
		{
			print(file);
			
			vector<string> lines = getFileContent("main.cpp"/*blocksFolder + file*/);
			unsigned int linesSize = lines.size();
			print("linesSize: " + convertNbToStr(linesSize));
			for(unsigned int linesIndex = 0; linesIndex < linesSize; linesIndex++)
			{
				string line = lines[linesIndex];
				print(convertNbToStr(linesIndex) + " " + line + " !");
			}

			/*unsigned short magics[4];
			FILE* f = fopen(file.c_str(), "rb");
			if(f == NULL)
			{
				print("Failed to open file !");
				return 1;
			}
			for(unsigned short i = 0; i < 40; i++)
			{
				//magics[i] = fgetc(f);
				//print(convertNbToStr(i) + " " + convertNbToStr(magics[i]) + " !");
				print(convertNbToStr(i) + " " + convertNbToStr(fgetc(f)) + " !");
			}
			fclose(f);*/
		}
	}
	return 0;
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

bool startsWith(string subject, string test)
{
    return !subject.compare(0, test.size(), test);
}

void listFiles(string direc, vector<string>* str)
{
    DIR* dir;
    struct dirent* ent;
    if((dir = opendir(direc.c_str())) != NULL)
    {
        while((ent = readdir(dir)) != NULL)
        {
            string file = ent->d_name;
            if(!startsWith(file, "."))
            {
                string path = direc;
                if(path[path.length() - 1] != pathSeparator)
                    path += pathSeparator;
                path += file;
                if(file.find_last_of(".") > file.length())
                    listFiles(path, str);
                else
                    str->push_back(path);
            }
        }
        closedir(dir);
    }
}

bool endsWith(string const& value, string const& ending)
{
    if(ending.size() > value.size()) return false;
    return equal(ending.rbegin(), ending.rend(), value.rbegin());
}

string replace(string subject, const string& search, const string& replace = "")
{
    unsigned int s = subject.find(search);
    if(s > subject.length())
        return subject;
    return subject.replace(s, search.length(), replace);
}

vector<string> listFiles(string directory)
{
	vector<string> files;
    listFiles(directory, &files);
	unsigned int filesSize = files.size();
	for(unsigned int filesIndex = 0; filesIndex < filesSize; filesIndex++)
	{
		files[filesIndex] = replace(files[filesIndex], directory);
	}
    return files;
}

void print(string s)
{
	cout << s << endl;
}
