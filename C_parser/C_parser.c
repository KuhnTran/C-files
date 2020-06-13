#include<stdio.h>
#include<ctype.h>
#include<string.h>

#define STRMAX 1024
char peekc(FILE* fp)
{
	char i;
	i = fgetc(fp);
	ungetc(i, fp);
	return i;	
}

void parseWord(FILE* fp)
{
	char input = fgetc(fp);
	while(isalnum(peekc(fp)) || peekc(fp) == '_')
	{
		printf("%c",input);
		input = fgetc(fp);
	}
	printf("%c", input); 	
}

void parseNumber(FILE* fp)
{
	char input = fgetc(fp);
	while(isdigit(peekc(fp)))
	{
		printf("%c",input);
		input = fgetc(fp);
	}
	printf("%c", input);	
}

void parseString(char in, FILE* fp)
{
	//function with 7 lines
	printf("%c", in);
	if ((isdigit(in) && isdigit(peekc(fp))))
		parseNumber(fp);
	else if( (isalpha(in) || in == '_') && 
		( isalnum(peekc(fp)) || peekc(fp) == '_' ))
		parseWord(fp);	
	printf("\n");		
}	

void parseComparison(char in, FILE* fp, char* sign)
{
	int i = 0;
	while (peekc(fp) != sign[i] && sign[i] != 0) i++;
	if (sign[i] == 0) printf("%c\n", in);
	else printf("%c%c\n", in , fgetc(fp));
}
			

void isComment(char in, FILE* fp)
{	
	if (peekc(fp) == '/') while (fgetc(fp) != '\n');
	else if (peekc(fp) == '*')
	{	
		 while((fgetc(fp) != '*') || peekc(fp)!='/');
		 fgetc(fp);
	}
}	

void parseComment(char in, FILE *fp)
{
	if (peekc(fp) != '=') isComment(in, fp);
	else printf("%c%c\n", in, fgetc(fp)); 
}

void printSpecialChar(FILE* fp)
{	
	for (int i = 0; i < 3; i++)
		printf("%c", fgetc(fp));	
}
void parseSingleQuote(char in, FILE *fp)
{
	printf("%c", in);
	if( peekc(fp) == '\\') printSpecialChar(fp);
	else for(int i = 0; i < 2; i++) printf("%c", fgetc(fp));
	printf("\n");
}

void parseQuotation(char in, FILE*fp)
{
	// function with 7 lines
	do 
	{
		if (in != '\\' && in != '%') printf("%c", in);
		else printf("%c%c", in, fgetc(fp));
		in = fgetc(fp);
	} while(in != '"');
	printf("%c\n", in);
}

void getToken(FILE* fp)
{
	char in;
	in = fgetc(fp);
	while (in != EOF)
	{
		switch(in)
		{
			case ' ':
			case '\n':
			case '\t':
			break;
			case ',':
			case '#':		
			case '.':
			case '^':
			case ':':
			case '~':
			case '?':
			case '(':
			case ')':
			case '[':
			case ']':
			case '{':
			case '}':
			case ';':
			case '%':
			printf("%c\n", in);
			break;
			case 39:
			parseSingleQuote(in, fp);
			break;
			case '/':
			parseComment(in, fp);
			break;
			case '"':
			parseQuotation(in, fp);
			break;
			case '+':
			parseComparison(in, fp, "+=");
			break;
			case '-':
			parseComparison(in, fp, "-=>");
			break;
			case '*':
			case '=':
			case '!':
			case '<':
			case '>':
			parseComparison(in, fp, "=");
			break;
			case '&':
			parseComparison(in, fp, "&");
			break;
			case '|':
			parseComparison(in, fp, "|=");
			break;
			default:
			parseString(in, fp);
			break;
		}
		in = fgetc(fp); 
	}
}

int main()
{
	FILE* fp;
	fp = stdin;
	getToken(fp);
	return 0;
}
