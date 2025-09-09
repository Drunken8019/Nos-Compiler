#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include "Parser.h"
#include <chrono>
using namespace std;

void temp_compile(char* src);
bool checkExtension(char* s);

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		cout << "No source file (.nos) provided.";
	}
	else if (argc == 2)
	{
		if (checkExtension(argv[1]))
		{
			std::chrono::time_point<std::chrono::system_clock> start, end;
			start = std::chrono::system_clock::now();
			temp_compile(argv[1]);
			end = std::chrono::system_clock::now();
			std::chrono::duration<double> elapsed_seconds = end - start;
			cout << std::endl << "Finished afer " << elapsed_seconds <<" seconds" << endl;
		}
		else cout << "Wrong file type. Correct extension: .nos";
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
				if (s[counter + 2] != '\0' && s[counter + 2] == 'o')
				{
					if (s[counter + 3] != '\0' && s[counter + 3] == 's')
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