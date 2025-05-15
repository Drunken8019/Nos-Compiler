#include "Lexer.h"

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
			tokenBuffer.push({ TokenType::Colon, ":", {0, 0}});
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
				tokenBuffer.push({ TokenType::Number, temp, {0, 0} });
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
				if (it != keywords.end()) tokenBuffer.push({ it->second, temp, {0, 0} });
				else tokenBuffer.push({ TokenType::Identifier, temp, {0, 0} });
			}
			break;
		}
	}
	return true;
}