#include <stdio.h>

enum {MaxName = 128};
enum {NotReady, Finish, FinishResend, Error};

int Letter(char c) 
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int Digit(char c)
{
	return c >= '0' && c <= '9';
}

int EndLine(char c)
{
	return c == '\t' || c == '\r' || c == '\n';
}

int Brace(char c)
{
	return c=='(' || c==')' || c=='[' || c==']' || c=='{' || c=='}';
}

int Compare(char c)
{
	return c == '>' || c == '<' || c == '=';
}

int Single(char c)
{
	return c==':'||c==';'||c==','||c=='+'||c=='-'||c=='*'||c=='/';
}

int Other(char c)
{
	return c=='!'||c =='@'||c=='#'||c=='$'||c=='%'||c=='?'||c=='&';
}

int All(char c)
{
	return Letter(c)||Digit(c)||Brace(c)||Compare(c)||Single(c)||Other(c); 
}

int Delimiter(char c)
{
	return EndLine(c)||Brace(c)||Compare(c)||Single(c)||c=='&'||c=='#'||c==' ';
}

class Lexeme {
public:
	char name[MaxName];
	int NameSize, LexNum, LineNum;
	Lexeme()
	{
		name[0] = 0;
		NameSize = LexNum = LineNum = 0;
	}
	void AddChar(char c)
	{
		if ((c != ' ' || NameSize != 0) && !EndLine(c)) {
			name[NameSize] = c;
			name[++NameSize] = 0;
		}
	}
	int NotEmpty() { return NameSize; }
	void SetLineNum(int n) { LineNum = n; }  
	void PrintLexeme()
	{
		if (NotEmpty()) {
			printf("%s\n", name);
		}
	}
};

struct ListOfLexeme {
	Lexeme lex;
	ListOfLexeme *next;
	ListOfLexeme (): lex(), next(NULL) {}
};

ListOfLexeme *AddLex(ListOfLexeme *list, int n)
{
	if ((*list).lex.NotEmpty()) {
		ListOfLexeme *help;
		(*list).lex.SetLineNum(n);
		help = new ListOfLexeme;
		(*help).next = list;
		list = help;
	}
	return list;
}

void PrintListOfLex(ListOfLexeme *list)
{
	while(list != NULL) {
		(*list).lex.PrintLexeme();
		list = (*list).next;
	}
}

class Automat {
	enum {H, String, Ident, Int, Real, Equal, LessGreater, Comment}; 
	Lexeme *lex;	
	int state;
	int StateH(char c);
	int StateString(char c);
	int StateIdent(char c);
	int StateInt(char c);
	int StateReal(char c);
	int StateEqual(char c);
	int StateLessGreater(char c);
	int StateComment(char c);
public:
	Automat()
	{
		state = H;
		lex = new Lexeme;
	}
	int FeedChar(char c);
};

int Automat::StateH(char c)
{
	if (c == '#') {
		state = Int;
	} else if (c == '\"') {
		state = String;
	} else if (c == '&' || Letter(c)) {
		state = Ident;
	} else if (c == '=') {
		state = Equal;
	} else if (c == '>' || c == '<') {
		state = LessGreater;
	} else if (c == '/') {
		state = Comment;
	} else if (EndLine(c) || Brace(c) || Single(c) || c == ' ') {
		stae;
	} else {
		return Error;
	}
	return NotReady;
}

int Automat::StateString(char c)
{
	if (c == '\"') {
		state = H;
		return Finish;
	} else if (All(c) || c == ' ' || c == '\t') {
		return NotReady;
	} else {
		return Error;
	}
}

int Automat::StateIdent(char c)
{
	if (Letter(c) || Digit(c)) {
		return NotReady;
	} else if (Delimiter(c)) {
		state = H;
		return FinishResend;
	} else {
		return Error;
	}
}

int Automat::StateInt(char c)
{
	if (Digit(c)) {
	} else if (c == '.') {
		state = Real;
	} else if (Delimiter(c)) {
		state = H;
		return FinishResend;
	} else {
		return Error;
	}
	return NotReady;
}

int Automat::StateReal(char c)
{
	if (Digit(c)) {
	} else if (Delimiter(c)) {
		state = H;
		return FinishResend;
	} else {
		return Error;
	}
	return NotReady;
}

int Automat::StateEqual(char c)
{
	if (c == '=' || c == '!') {
		state = H;
		return Finish;
	} else if (Delimiter(c)) {
		state = H;
		return FinishResend;
	} else {
		return Error;
	}
}

int Automat::StateLessGreater(char c)
{
	if (c == '=') {
		state = H;
		return Finish;
	} else if (Delimiter(c)) {
		state = H;
		return FinishResend;
	} else {
		return Error;
	}
}

int Automat::StateComment(char c)
{
	if (EndLine(c)) {
		state = H;
		return Finish;
	} else {
		return NotReady;
	}
}

int Automat::FeedChar(char c)
{
	if (state == H) {
		return StateH(c);
	} else if (state == String) {
		return StateString(c);
	} else if (state == Ident) {
		return StateIdent(c);
	} else if (state == Int) {
		return StateInt(c);
	} else if (state == Real) {
		return StateReal(c);
	} else if (state == Equal) {
		return StateEqual(c);
	} else if (state == LessGreater) {
		return StateLessGreater(c);
	} else if (state == Comment) {
		return StateComment(c);
	} else {
		return Error;
	}
}

char GetChar(FILE *f, int &LineNumber)
{
	char c = fgetc(f);
	if (c == '\n')
		LineNumber++;
	return c;
}

int main(int argc, char **argv)
{
	ListOfLexeme *list;
	Automat automat;
	int LineNumber = 1, res;
	FILE *f;
	char c;
	if (argc < 2)
		return 0;
	if (!(f = fopen(argv[1], "r")))
		return 0;
	c = GetChar(f, LineNumber);
	while(c != EOF) {
		res = automat.FeedChar(c);
		if (res == Finish) {
			(*list).lex.AddChar(c);
			(*list).lex.PrintLexeme();
			list = AddLex(list, LineNumber);
			c = GetChar(f, LineNumber);
		} else if (res == FinishResend) {
			(*list).lex.PrintLexeme();
			list = AddLex(list, LineNumber);
		} else if (res == Error) {
			printf("Error in line %d: invalid character %c\n", LineNumber, c);
			return 0;
		} else if (res == NotReady) {
			(*list).lex.AddChar(c);
			c = GetChar(f, LineNumber);
		}
	}
	
	return 0;
}
