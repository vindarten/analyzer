#include <stdio.h>

enum {MaxName = 128};

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
	char name[MaxName];
	int NameSize, LexNum, LineNum, Correct;
public:
	Lexeme()
	{
		name[0] = 0;
		Correct = 1;
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
	int GetLineNum() { return LineNum; }
	void Incorrect() { Correct = 0; }
	int CheckCorrect() { return Correct; }  
	void PrintLexeme() { printf("%d: %s\n", LineNum, name); }
};

class Automat {
	enum {H, String, Ident, Int, Real, Equal, LessGreater, Comment, Error, S, SResend}; 
	Lexeme *lex, *ReadyLex;	
	int state, LineNumber;
	void StateH(char c);
	void StateString(char c);
	void StateIdent(char c);
	void StateInt(char c);
	void StateReal(char c);
	void StateEqual(char c);
	void StateLessGreater(char c);
	void StateComment(char c);
	void StateS(char c);
public:
	Automat()
	{
		state = H;
		LineNumber = 1;
		ReadyLex = NULL;
		lex = new Lexeme;
	}
	Lexeme *FeedChar(char c);
};

void Automat::StateH(char c)
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
		state = S;
	} else {
		state = Error;
	}
	(*lex).AddChar(c);
}

void Automat::StateString(char c)
{
	if (c == '\"') {
		state = S;
	} else if (All(c) || c == ' ' || c == '\t') {
	} else {
		state = Error;
	}
	(*lex).AddChar(c);
}

void Automat::StateIdent(char c)
{
	if (Letter(c) || Digit(c)) {
	} else if (Delimiter(c)) {
		state = SResend;
		return;
	} else {
		state = Error;
	}
	(*lex).AddChar(c);
}

void Automat::StateInt(char c)
{
	if (Digit(c)) {
	} else if (c == '.') {
		state = Real;
	} else if (Delimiter(c)) {
		state = SResend;
		return;
	} else {
		state = Error;
	}
	(*lex).AddChar(c);
}

void Automat::StateReal(char c)
{
	if (Digit(c)) {
	} else if (Delimiter(c)) {
		state = SResend;
		return;
	} else {
		state = Error;
	}
	(*lex).AddChar(c);
}

void Automat::StateEqual(char c)
{
	if (c == '=' || c == '!') {
		state = S;
	} else if (Delimiter(c)) {
		state = SResend;
		return;
	} else {
		state = Error;
	}
	(*lex).AddChar(c);
}

void Automat::StateLessGreater(char c)
{
	if (c == '=') {
		state = S;
	} else if (Delimiter(c)) {
		state = SResend;
		return;
	} else {
		state = Error;
	}
	(*lex).AddChar(c);
}

void Automat::StateComment(char c)
{
	if (EndLine(c)) {
		state = S;
	} else {
		(*lex).AddChar(c);
	}
}

void Automat::StateS(char c)
{
	state = H;
	ReadyLex = lex;
	(*ReadyLex).SetLineNum(LineNumber);
	if (c == '\n')
		LineNumber++;
	lex = new Lexeme;
}

Lexeme *Automat::FeedChar(char c)
{
	if (state == H) {
		StateH(c);
	} else if (state == String) {
		StateString(c);
	} else if (state == Ident) {
		StateIdent(c);
	} else if (state == Int) {
		StateInt(c);
	} else if (state == Real) {
		StateReal(c);
	} else if (state == Equal) {
		StateEqual(c);
	} else if (state == LessGreater) {
		StateLessGreater(c);
	} else if (state == Comment) {
		StateComment(c);
	}
	if (state == S || state == SResend) {
		StateS(c);
		if (state == SResend)
			FeedChar(c);
		if ((*ReadyLex).NotEmpty())
			return ReadyLex;
	} else if (state == Error) {
		(*lex).Incorrect();
		return lex;
	}
	return NULL;
}

void PrintError(Lexeme *lex)
{
	printf("Error in line %d: invalid lexeme", (*lex).GetLineNum());
	(*lex).PrintLexeme();
}

struct ListOfLexeme {
	Lexeme *lex;
	ListOfLexeme *next;
	ListOfLexeme (): next(NULL) { lex = new Lexeme;}
};

void PrintListOfLex(ListOfLexeme *list)
{
	while(list != NULL) {
		(*(*list).lex).PrintLexeme();
		list = (*list).next;
	}
}

int main(int argc, char **argv)
{
//	ListOfLexeme *list = new ListOfLexeme, *help;
	Automat automat;
	Lexeme *lex;
	FILE *f;
	char c;
	if (argc < 2)
		return 0;
	if (!(f = fopen(argv[1], "r")))
		return 0;
	while((c = fgetc(f)) != EOF) {
		if ((lex = automat.FeedChar(c)) != NULL) {
			if ((*lex).CheckCorrect()) {
				(*lex).PrintLexeme();
			} else {
				PrintError(lex);
				return 0;
			}
		}
	}
	if ((lex = automat.FeedChar(' ')) != NULL) {
		if ((*lex).CheckCorrect()) {
			(*lex).PrintLexeme();
		} else {
			PrintError(lex);
			return 0;
		}
	}
	printf("Correct\n");
	return 0;
}
