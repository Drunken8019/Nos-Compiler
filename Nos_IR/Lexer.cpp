#include "Lexer.h"
bool isOnlyWhitespace(const std::string& str);

Lexer::Lexer()
{
	this->in = NULL;
}

Lexer::Lexer(std::ifstream* i)
{
	this->in = i;
}

void Lexer::initialize()
{
	std::string curLine;
	do
	{
		do
		{
			if (!std::getline(*in, curLine))
			{
				tokenBuffer.push_back({ TokenType::COMPILER_EOF, "EOF!", {loc.line, 0} });
				return;
			}
			loc.line++;
		} while (curLine.empty() || isOnlyWhitespace(curLine));
		loadTokens(curLine);
	} while (tokenBuffer.back().type != COMPILER_EOF);
}

bool Lexer::stepBack(unsigned int len)
{
	if (index - len < 0) return false;
	index -= len;
	return true;
}

bool Lexer::setTo(unsigned int index)
{
	if (index >= tokenBuffer.size()) return false;
	Lexer::index = index;
	return true;
}

unsigned int Lexer::getIndex()
{
	return index;
}

Token Lexer::nextToken()
{
	Token result = { TokenType::COMPILER_EOF, "EOF!", {loc.line, 0} };

	if(index < tokenBuffer.size())
	{
		result = tokenBuffer[index];
		index++;
	}
	return result;
}

Token Lexer::peek()
{
	Token result = { TokenType::COMPILER_EOF, "EOF!", {loc.line, 0} };

	if (index < tokenBuffer.size())
	{
		result = tokenBuffer[index];
	}
	return result;
}

Token Lexer::peek_behind()
{
	Token result = { TokenType::COMPILER_EOF, "EOF!", {loc.line, 0} };

	if (index-1 >= 0)
	{
		result = tokenBuffer[index-1];
	}
	return result;
}

bool Lexer::loadTokens(std::string curLine)
{
	std::string stringLiteral = "";
	bool literalOpen = false;
	for (int i = 0; i < curLine.length(); i++)
	{
		if(curLine[i] == '\'')
		{
			if(i+2 < curLine.length())
			{
				i++;
				std::string temp = "";
				temp = curLine[i];
				tokenBuffer.push_back({ Character, temp, {loc.line, i}});
				i++;
				if(curLine[i] != '\'')
				{
					std::cout << "Missing closing quote\n";
					return false;
				}
			}
		}

		if(curLine[i] == '"') 
		{
			if(literalOpen)
			{
				tokenBuffer.push_back({ String, stringLiteral, {loc.line, i} });
				stringLiteral.clear();
			}

			literalOpen == !literalOpen;
		}

		if(literalOpen)
		{
			stringLiteral.append(1, curLine[i]);
			continue;
		}

		auto sFound = symbols.find(curLine[i]);
		if (sFound != symbols.end()) 
		{
			if (i+1 < curLine.length())
			{
				std::string cs = "";
				cs.append(1, curLine[i]);
				cs.append(1, curLine[i + 1]);
				auto csFound = compoundSymbols.find(cs);
				if(csFound != compoundSymbols.end())
				{
					tokenBuffer.push_back({ csFound->second, {csFound->first}, {loc.line, i} });
					i++;
				}
				else
				{
					tokenBuffer.push_back({ sFound->second, {sFound->first}, {loc.line, i + 1} });
				}
			}
			else
			{
				tokenBuffer.push_back({ sFound->second, {sFound->first}, {loc.line, i + 1} });
			}
		}
		else
		{
			if (std::isdigit(curLine[i]))
			{
				std::string temp;
				while (std::isdigit(curLine[i]))
				{
					temp.append(1, curLine[i]);
					i++;
					if (i >= curLine.length()) break;
				}
				i--;
				tokenBuffer.push_back({ TokenType::Number, temp, {loc.line, i + 1} });
			}
			else if (std::isalpha(curLine[i]))
			{
				std::string temp;
				while (std::isalpha(curLine[i]) || std::isdigit(curLine[i]) || curLine[i] == '_')
				{
					temp.append(1, curLine[i]);
					i++;
					if (i >= curLine.length()) break;
				}
				i--;
				auto kFound = keywords.find(temp);
				if (kFound != keywords.end()) tokenBuffer.push_back({ kFound->second, kFound->first, {loc.line, i + 1} });
				else tokenBuffer.push_back({ TokenType::Identifier, temp, {loc.line, i + 1} });
			}
			else if(!std::isspace(curLine[i]))
			{
				std::cout << "Illegal character \"" << curLine[i] << "\"\n";
				return false;
			}
		}
	}
	return true;
}

bool isOnlyWhitespace(const std::string& str) {
	
	for(char c : str)
	{
		if (!std::isspace(c)) { return false; }
	}
	return true;
}