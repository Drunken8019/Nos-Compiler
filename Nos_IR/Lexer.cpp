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

Token Lexer::nextToken()
{
	std::string curLine;
	if(tokenBuffer.empty() && hasNext)
	{
		do
		{
			if (!std::getline(*in, curLine)) 
			{
				hasNext = false;
				return { TokenType::COMPILER_EOF, "EOF!", {loc.line, 0} };
			}
			loc.line++;
		} while (curLine.empty() || isOnlyWhitespace(curLine));
		Lexer::loadTokens(curLine);
	}
	Token result = { TokenType::COMPILER_EOF, "EOF!", {loc.line, 0} };
	if(!tokenBuffer.empty())
	{
		result = tokenBuffer.front();
		tokenBuffer.pop();
	}
	if (!in->good() && tokenBuffer.empty())
	{
		hasNext = false;
	}
	return result;
}

Token Lexer::peek()
{
	std::string curLine;
	if (tokenBuffer.empty() && hasNext)
	{
		do
		{
			if (!std::getline(*in, curLine)) 
			{
				hasNext = false;
				return { TokenType::COMPILER_EOF, "EOF!", {loc.line, 0} };
			}
			loc.line++;
		} while (curLine.empty() || isOnlyWhitespace(curLine));
		Lexer::loadTokens(curLine);
	}
	if (!in->good() && tokenBuffer.empty())
	{
		hasNext = false;
	}
	return tokenBuffer.front();
}

bool Lexer::loadTokens(std::string curLine)
{
	for (int i = 0; i < curLine.length(); i++)
	{
		auto sFound = symbols.find(curLine[i]);
		if (sFound != symbols.end()) tokenBuffer.push({ sFound->second, {sFound->first}, {loc.line, i + 1} });
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
				tokenBuffer.push({ TokenType::Number, temp, {loc.line, i + 1} });
			}
			else if (std::isalpha(curLine[i]))
			{
				std::string temp;
				while (std::isalpha(curLine[i]) || std::isdigit(curLine[i]))
				{
					temp.append(1, curLine[i]);
					i++;
					if (i >= curLine.length()) break;
				}
				i--;
				auto kFound = keywords.find(temp);
				if (kFound != keywords.end()) tokenBuffer.push({ kFound->second, kFound->first, {loc.line, i + 1} });
				else tokenBuffer.push({ TokenType::Identifier, temp, {loc.line, i + 1} });
			}
		}
	}
	return true;
}

bool isOnlyWhitespace(const std::string& str) {
	return std::all_of(str.begin(), str.end(), [](unsigned char c) {
		return std::isspace(c);
	});
}