#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include "Parser.h"
using namespace std;

void temp_compile(char* src);
bool checkExtension(char* s);

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "No source file (.nir) provided.";
	}
	else if (argc == 2)
	{
		if (checkExtension(argv[1]))
		{
			time_t stamp = time(NULL);
			tm start;
			localtime_s(&start, &stamp);
			cout << "Started at " << start.tm_hour << ":" << start.tm_min << ":" << start.tm_sec << endl;
			temp_compile(argv[1]);
			stamp = time(NULL);
			tm end;
			localtime_s(&end, &stamp);
			int took = end.tm_sec - start.tm_sec;
			cout << "Finished at " << end.tm_hour << ":" << end.tm_min << ":" << end.tm_sec << " and took " << took << " seconds" << endl;
		}
		else cout << "Wrong file type. Correct extension: .nir";
	}
	else cout << "Too many arguments.";
	return 0;
}

void temp_compile(char* src)
{
	ifstream in;
	in.open(src);
	/*Lexer l = {&in};
	Token temp;
	while(l.hasNext)
	{
		temp = l.nextToken();
		cout << temp.value << "\t" << temp.type << "\t" << temp.loc.line << " : " << temp.loc.column << endl;
	}*/
	ofstream out;
	out.open("E:/Nos_IR/default.asm");
	
	Parser p = { &in, &out };
	p.parse();
}

bool checkExtension(char* s)
{
	char cur;
	unsigned int counter = 0;
	do
	{
		cur = s[counter];
		if(cur == '.')
		{
			if(s[counter+1] != '\0' && s[counter + 1] == 'n')
			{
				if (s[counter + 2] != '\0' && s[counter + 2] == 'i')
				{
					if (s[counter + 3] != '\0' && s[counter + 3] == 'r')
					{
						return true;
					}
				}
			}
		}
		counter++;
	} while (cur != '\0');
	return false;
}