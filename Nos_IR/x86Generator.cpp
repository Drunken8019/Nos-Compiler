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


std::string x86Generator::x86Code(CodeTemplate CTemplate)
{
	/*if (CTemplate.isVarDef)
	{
		varTable.insert({ tokens.at(1).value, variableCounter });
		variableCounter++;
	}*/

	std::string result = "";
	Token curToken;
	for (Option<std::string, Token> o : CTemplate.indexedx86Code)
	{
		switch (o.isLeft)
		{
		case true:
			result.append(o.getLeft());
			break;
		case false:
			curToken = o.getRight();
			result.append(curToken.value);
			/*if (curToken.type == TokenType::Identifier && CTemplate.isVarDef)
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
			}*/
			break;
		}
	}
	printAsm(result);
	return result;
}