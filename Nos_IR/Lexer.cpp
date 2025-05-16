#include "Lexer.h"

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
				return { TokenType::Identifier, "EOF!", {0, 0} };
			}
			loc.line++;
		} while (curLine.empty());
		Lexer::loadTokens(curLine);
	}
	if(!in->good())
	{
		hasNext = false;
	}
	Token result = tokenBuffer.front();
	tokenBuffer.pop();
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
				return { TokenType::Identifier, "EOF!", {0, 0} };
			}
			loc.line++;
		} while (curLine.empty());
		Lexer::loadTokens(curLine);
	}
	if (!in->good())
	{
		hasNext = false;
	}
	return tokenBuffer.front();
}

bool Lexer::loadTokens(std::string curLine)
{
	for (int i = 0; i < curLine.length(); i++)
	{
		switch (curLine[i])
		{
		case ' ':
			break;

		case ':':
			tokenBuffer.push({ TokenType::Colon, ":", {loc.line, i + 1}});
			break;
		default:
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
				tokenBuffer.push({ TokenType::Number, temp, {loc.line, i+1} });
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
				auto it = keywords.find(temp);
				if (it != keywords.end()) tokenBuffer.push({ it->second, temp, {loc.line, i + 1} });
				else tokenBuffer.push({ TokenType::Identifier, temp, {loc.line, i + 1} });
			}
			break;
		}
	}
	return true;
}