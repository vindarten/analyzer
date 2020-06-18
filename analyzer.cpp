#include <stdio.h>
#include <string.h>
#include <stdlib.h>
enum {
	LexEmpty, LexIdent, LexConst, LexStr, LexCom, LexBirth, LexDeath, LexDie,
	LexInt,	LexReal, LexString, LexIter, LexIf, LexElse, LexWhile, LexDo,
	LexPut, LexGet, LexLB, LexRB, LexLSB, LexRSB, LexLParen, LexRParen,
	LexComma, LexColon, LexSemicolon, LexPlus, LexMinus, LexMul, LexDiv,
	LexAssign, LexLT, LexLE, LexGT, LexGE, LexEq, LexNotEq, LexError,
};

class Lexeme {
	char *value;
	int LexNum, LineNum;
public:
	Lexeme(): LexNum(0), LineNum(0) { value = NULL; }
	Lexeme(char buf[], int &BufSize, int line, int num)
	{
		LexNum = num;
		LineNum = line;
//		if (LexNum == LexIdent || LexNum == LexConst || LexNum == LexCom) {
			value = new char[BufSize + 1];
			for(int i = 0; i <= BufSize; i++)
				value[i] = buf[i];
//		}
		buf[0] = 0;
		BufSize = 0;
	}
	int NotEmpty() { return LexNum; }
	int CheckCorrect() { return LexNum != LexError; }  
	void Print() 
	{
		printf("%d: %d ", LineNum, LexNum);
//		if (LexNum == LexIdent || LexNum == LexConst || LexNum == LexCom)
			printf("%s", value);
		printf("\n");
	}
	void PrintError()
	{
		printf("Error in line %d: invalid lexeme %s\n", LineNum, value);
	}
};

class Automat {
	enum {H, String, Ident, Int, Real, Equal, lg, Comment, Error, S, SResend};
	enum {MaxBuf = 4096};
	static const char *TableOfWords[];
	Lexeme *lex;
	char buf[MaxBuf];
	int BufSize, state, line;
	Lexeme *StateS(char c, int begin);
	void ChangeState(char c);
	void StateH(char c);
	void StateString(char c);
	void StateIdent(char c);
	void StateInt(char c);
	void StateReal(char c);
	void StateEqual(char c);
	void StateLessGreater(char c);
	void StateComment(char c);
	void AddBuf(char c);
	int GetLexNum();
	int Letter(char c); 
	int Digit(char c);
	int EndLine(char c);
	int Brace(char c);
	int Compare(char c);
	int Single(char c);
	int Other(char c);
	int All(char c);
	int Delimiter(char c);
public:
	Automat(): BufSize(0), state(H), line(1) { buf[0] = 0; lex = NULL; }
	Lexeme *FeedChar(char c);
};

const char *Automat::TableOfWords[] = 
{
	"", "", "", "", "", "birth", "death", "die", "int", "real", "string", "iterator",
	"if", "else", "while", "do", "put", "get", "{", "}", "[", "]", "(", ")",
	",", ":", ";", "+", "-", "*", "/", "=", "<", "<=", ">", ">=", "==", "=!",
	NULL
};

int Automat::Letter(char c) 
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

int Automat::Digit(char c)
{
	return c >= '0' && c <= '9';
}

int Automat::EndLine(char c)
{
	return c == '\r' || c == '\n' || c == EOF;
}

int Automat::Brace(char c)
{
	return c=='(' || c==')' || c=='[' || c==']' || c=='{' || c=='}';
}

int Automat::Compare(char c)
{
	return c == '>' || c == '<' || c == '=';
}

int Automat::Single(char c)
{
	return c==':'||c==';'||c==','||c=='+'||c=='-'||c=='*'||c=='/';
}

int Automat::Other(char c)
{
	return c=='!'||c =='@'||c=='#'||c=='$'||c=='%'||c=='?'||c=='&';
}

int Automat::All(char c)
{
	return Letter(c)||Digit(c)||Brace(c)||Compare(c)||Single(c)||Other(c); 
}

int Automat::Delimiter(char c)
{
	return EndLine(c)||Brace(c)||Compare(c)||Single(c)||c=='&'||c=='#'||c==' ';
}

Lexeme *Automat::FeedChar(char c)
{
	if (state == S) {
		return StateS(c, 1);
	} else if (state == Error) {
		return new Lexeme(buf, BufSize, line, LexError);
	}
	ChangeState(c);
	if (state == S) {
		return StateS(c, 0);
	} else if (state == SResend) {
		return StateS(c, 1);
	} else if (state == Error) {
		return new Lexeme(buf, BufSize, line, LexError);
	} else {
		AddBuf(c);
	}
	return NULL;
}

Lexeme *Automat::StateS(char c, int begin)
{
	if (!begin)
		AddBuf(c);
	state = H;
	lex = new Lexeme(buf, BufSize, line, GetLexNum());
	if (c == '\n')
		line++;
	if (begin) {
		ChangeState(c);
		AddBuf(c);
	}
	if ((*lex).NotEmpty()) {
		return lex;
	} else {
		return NULL;
	}
}

void Automat::ChangeState(char c)
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
	} else if (state == lg) {
		StateLessGreater(c);
	} else if (state == Comment) {
		StateComment(c);
	}
}

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
		state = lg;
	} else if (c == '/') {
		state = Comment;
	} else if (EndLine(c) || Brace(c) || Single(c) || c == ' ' || c == '\t') {
		state = S;
	} else {
		state = Error;
	}
}

void Automat::StateString(char c)
{
	if (c == '\"') {
		state = S;
	} else if (All(c) || c == ' ' || c == '\t') {
	} else {
		state = Error;
	}
}

void Automat::StateIdent(char c)
{
	if (Letter(c) || Digit(c)) {
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateInt(char c)
{
	if (Digit(c)) {
	} else if (c == '.') {
		state = Real;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateReal(char c)
{
	if (Digit(c)) {
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateEqual(char c)
{
	if (c == '=' || c == '!') {
		state = S;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateLessGreater(char c)
{
	if (c == '=') {
		state = S;
	} else if (Delimiter(c)) {
		state = SResend;
	} else {
		state = Error;
	}
}

void Automat::StateComment(char c)
{
	if (EndLine(c)) {
		state = S;
	}
}

void Automat::AddBuf(char c)
{
	if (((c != ' ' && c != '\t') || BufSize != 0) && !EndLine(c)) {
		buf[BufSize++] = c;
		buf[BufSize] = 0;
	}
}

int Automat::GetLexNum()
{
	int i = 0;
	if (buf[0] == '&')
		return LexIdent;
	if (buf[0] == '#')
		return LexConst;
	if (buf[0] == '/')
		return LexCom;
	if (buf[0] == '"')
		return LexStr;
	while(TableOfWords[i] != NULL) {
		if (!strcmp(buf, TableOfWords[i]))
			return i;
		i++;
	}
	return LexError;
}

FILE *OpenFile(int argc, char **argv)
{
	if (argc < 2)
		return 0;
	return fopen(argv[1], "r");
}
	 
int main(int argc, char **argv)
{
	Automat automat;
	Lexeme *lex;
	char c;
	FILE *f = OpenFile(argc, argv);
	if (!f)
		return 0;
	do {
		c = fgetc(f);
		if ((lex = automat.FeedChar(c)) != NULL) {
			if ((*lex).CheckCorrect()) {
				(*lex).Print();
			} else {
				(*lex).PrintError();
				return 0;
			}
		}
	} while(c != EOF);
	printf("Correct\n");
	return 0;
}
