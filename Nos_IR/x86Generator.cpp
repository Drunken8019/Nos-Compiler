#include "x86Generator.h"

x86Generator::x86Generator()
{
	this->out = NULL;
}

x86Generator::x86Generator(std::ofstream* out)
{
	this->out = out;
}

void x86Generator::printDefaultHeader()
{
	*out <<
		"global main\n"
		"extern ExitProcess\n"
		"section .bss\n"
		"section .data\n"
		"section .text\n";
}

void x86Generator::printAsm(std::string s)
{
	*out << s;
	//out->write(s.c_str(), s.length()-1);
}

void x86Generator::startMatching(Lexer l)
{
	Token curToken = l.nextToken();
	std::cout << curToken.value << "\t" << curToken.type << "\t" << curToken.loc.line << " : " << curToken.loc.column << std::endl; //----DEBUG
	Token prevToken;
	std::string toPrint = "";
	std::vector<Token> tkInCurExpr;
	bool syntaxMatching = true;
	bool useSaved = false;
	bool lookBack = false;
	std::vector<Token> saved;
	int count = 0;

	while (curToken.type != TokenType::COMPILER_EOF)
	{
		//prevToken = curToken;
		saved.push_back(curToken);

		for(SyntaxTemplate t : syntax)
		{
			for (Snippet s : t.def)
			{
				if (s.isToken)
				{
					if (curToken.type == s.getType())
					{
						lookBack = true;
						syntaxMatching = true;
						tkInCurExpr.push_back(curToken);
						if (useSaved && !saved.size()>count) { curToken = saved.at(count); count++; }
						else { curToken = l.nextToken(); saved.push_back(curToken); }
						std::cout << curToken.value << "\t" << curToken.type << "\t" << curToken.loc.line << " : " << curToken.loc.column << std::endl; //----DEBUG
					}
					else
					{
						if (curToken.type != TokenType::COMPILER_EOF)
						{
							//curToken = prevToken;
							if (saved.size() > count && lookBack)
							{
								useSaved = true;
								curToken = saved.at(count);
								count++;
							}
							if (saved.size() < count) useSaved = false;
						}
						syntaxMatching = false;
						break;
					}
				}
				count = 0;
			}
			if (syntaxMatching) toPrint = x86Generator::x86Code(tkInCurExpr, t);
			tkInCurExpr.clear();
			if (!toPrint.empty() && syntaxMatching) { *out << toPrint; toPrint.clear(); break; }
		}
		saved.clear();
		useSaved = false;
	}
}

std::string x86Generator::x86Code(std::vector<Token> tokens, SyntaxTemplate t)
{
	if (t.type == SyntaxType::VarDef)
	{
		varTable.insert({ tokens.at(1).value, variableCounter });
		variableCounter++;
	}
	std::string result = "";
	Token curToken;
	for (Option o : t.CTemplate.indexedx86Code)
	{
		switch (o.isString)
		{
		case true:
			result.append(o.getx86Code());
			break;
		case false:
			curToken = tokens.at(o.getIndex());
			if(curToken.type == TokenType::Identifier && t.type != SyntaxType::Function)
			{
				auto varRes = varTable.find(curToken.value);
				if (varRes != varTable.end())
				{
					int varOffset = (variableCounter - varRes->second) * 4;
					result.append("[ESP+");
					result.append(std::to_string(varOffset));
					result.append("]");
				}
			}
			else
			{
				result.append(curToken.value);
			}
			break;
		}
	}
	return result;
}