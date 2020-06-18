#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum {
	LexEmpty, LexIdent, LexValInt, LexValReal, LexStr, LexBirth, LexDeath,
	LexDie, LexInt, LexReal, LexString, LexIter, LexIf, LexElse, LexWhile,
	LexOr, LexAnd, LexPut, LexGet, LexLB, LexRB, LexLSB, LexRSB, LexLParen,
	LexRParen, LexComma, LexColon, LexSemicolon, LexAdd, LexSub, LexMul,
	LexDiv, LexAssign, LexLT, LexLE, LexGT, LexGE, LexEq, LexNotEq, LexNeg,
	LexError
};

const char *TableOfWords[] = 
{
	"", "", "", "", "", "birth", "death", "die", "int", "real", "string",
	"iterator",	"if", "else", "while", "or", "and", "put", "get", "{", "}",
	"[", "]", "(", ")", ",", ":", ";", "+", "-", "*", "/", "=", "<", "<=",
	">", ">=", "==", "=!", "!", NULL
};

class Lexeme {
	void *value;
	int LexNum, LineNum;
public:
	Lexeme(): value(NULL), LexNum(LexEmpty), LineNum(0) {}
	Lexeme(char buf[], int &BufSize, int line, int num);
	Lexeme(int line, int num);
	int Empty() { return !LexNum; }
	int CheckCorrect() { return LexNum != LexError; } 
	int GetLexNum() { return LexNum; } 
	int GetLineNum() { return LineNum; }
	void Print(); 
	void PrintInvalLex();
	~Lexeme();
};

Lexeme::Lexeme(char buf[], int &BufSize, int line, int num)
{
	LexNum = num;
	LineNum = line;
	if (LexNum == LexIdent) {
		value = new char[BufSize];
		for(int i = 0; i < BufSize; i++)
			((char *)value)[i] = buf[i+1];
	}
	if (LexNum == LexStr) {
		value = new char[BufSize - 1];
		for(int i = 0; i < BufSize - 2; i++)
			((char *)value)[i] = buf[i+1];
		((char *)value)[BufSize - 1] = 0;
	}
	if (LexNum == LexValInt)
		value = new long long(strtoll(buf+1, NULL, 10));
	if (LexNum == LexValReal)
		value = new double(strtod(buf+1, NULL));
	if (LexNum == LexError) {
		value = new char[BufSize + 1];
		for(int i = 0; i <= BufSize; i++)
			((char *)value)[i] = buf[i];
	}
	buf[0] = 0;
	BufSize = 0;
}

Lexeme::Lexeme(int line, int num)
{
	LexNum = num;
	LineNum = line;
}

void Lexeme::Print() 
{
	printf("%d: %d ", LineNum, LexNum);
	if (LexNum == LexIdent || LexNum == LexStr) {
		printf("%s\n", (char *)value);
	} else if (LexNum == LexValInt) {
		printf("%lld\n", *(long long *)value);
	} else if (LexNum == LexValReal) {
		printf("%f\n", *(double *)value); 
	} else {
		printf("\n");
	}
}

void Lexeme::PrintInvalLex()
{
	printf("Error in line %d: invalid lexeme ", LineNum);
	if (LexNum == LexIdent || LexNum == LexStr) {
		printf("%s\n", (char *)value);
	} else if (LexNum == LexValInt) {
		printf("%lld\n", *(long long *)value);
	} else if (LexNum == LexValReal) {
		printf("%f\n", *(double *)value);
	} else {
		printf("%s\n", (char *)value);
	}
}

Lexeme::~Lexeme()
{
	if (LexNum == LexIdent || LexNum == LexStr)
		delete [] (char *)value;
	if (LexNum == LexValInt)
		delete (long long *)value;
	if (LexNum == LexValReal)
		delete (double *)value;
}

class Automat {
	enum {H, String, Ident, Int, Real, Equal, lg, Comment, Error, S, SResend};
	enum {MaxBuf = 4096};
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
	int SearchPoint();
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
	Automat(): lex(NULL), BufSize(0), state(H), line(1) { buf[0] = 0; }
	Lexeme *FeedChar(char c);
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
	return c==':'||c==';'||c==','||c=='+'||c=='-'||c=='*'||c=='/'||c=='!';
}

int Automat::Other(char c)
{
	return c =='@'||c=='#'||c=='$'||c=='%'||c=='?'||c=='&';
}

int Automat::All(char c)
{
	return Letter(c)||Digit(c)||Brace(c)||Compare(c)||Single(c)||Other(c); 
}

int Automat::Delimiter(char c)
{
	int a = c == ' ' || c == '\t';
	return EndLine(c)||Brace(c)||Compare(c)||Single(c)||c=='&'||c=='#'||a;
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
	if ((*lex).Empty()) {
		delete lex;
		if (c == EOF) {
			lex = new Lexeme(line, 0);
		} else {
			lex = NULL;
		}
	}
	return lex;
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

int Automat::SearchPoint()
{
	for(int i = 0; i < BufSize; i++)
		if (buf[i] == '.')
			return 1;
	return 0;
}

int Automat::GetLexNum()
{
	int i = 0;
	if (buf[0] == '&')
		return LexIdent;
	if (buf[0] == '#') { 
		if (SearchPoint()) {
			return LexValReal;
		} else {
		return LexValInt;
		}
	}
	if (buf[0] == '/')
		return LexEmpty;
	if (buf[0] == '"')
		return LexStr;
	while(TableOfWords[i] != NULL) {
		if (!strcmp(buf, TableOfWords[i]))
			return i;
		i++;
	}
	return LexError;
}

enum {
	InvalLex, BirthExpect, DeathExpect, NothingExpect, SemicolonExpect,
	IdentExpect, ValIntOrIdentExpect, ValRealExpect, StrExpect, RSBExpect,
	LParenExpect, RParenExpect, LBExpect, RBExpect, AssignExpect, CommaExpect
};

const char *TableOfErr[] = 
{
	"", "\"birth\"", "\"death\"", "nothing", "\";\"", "identifier",
	"integer or identifier", "real", "string", "\"]\"", "\"(\"", "\")\"",
	"\"{\"", "\"}\"", "\"=\"", "\",\""
};

class FlyingBug {
public:
	Lexeme *lex;
	int ErrNum;
	FlyingBug(Lexeme *l, int n) { lex = l; ErrNum = n; } 
};

void PrintExpect(int LineNum, const char *s)
{
	printf("Error in line %d: %s expected\n", LineNum, s);
}

class Parser {
	Automat automat;
	Lexeme *current;
	int LexNum;
	FILE *f;
	void next();
	void S();
	void Action();
	void ValIntOrIdentDesc();
	void IntDesc();
	void RealDesc();
	void StringDesc();
	void IteratorDesc();
	void WhileDesc();
	void GetDesc();
	void PutDesc();
	void AssignOrForDesc();
	void AssignDesc();
	void ForDesc();
	void IfElseDesc();
	void ArrayDesc();
	void ExpOr();
	void ExpAnd();
	void ExpComp();
	void ExpAddSub();
	void ExpMulDiv();
	void ExpLast();
	int CheckComp(int n);
public:
	Parser(): automat(), current(NULL), LexNum(0), f(0) {}
	void analyze(FILE *file)
	{
		f = file;
		try {
			next();
			S();
		}
		catch(FlyingBug &bug) {
			if (bug.ErrNum == InvalLex) {
				(*bug.lex).PrintInvalLex();
			} else {
				PrintExpect((*bug.lex).GetLineNum(), TableOfErr[bug.ErrNum]);
			}
			delete bug.lex;
			return;
		}
		printf("Correct\n");
	}
};

void Parser::S()
{
	if (LexNum != LexBirth)
		throw FlyingBug(current, BirthExpect);
	next();
	Action();
	if (LexNum != LexDeath)
		throw FlyingBug(current, DeathExpect);
	next();
	if (LexNum != LexEmpty)
		throw FlyingBug(current, NothingExpect);
}

void Parser::Action()
{
	if (LexNum == LexDie) {
		next();
	} else if (LexNum == LexInt) {
		next();
		IntDesc();
	} else if (LexNum == LexReal) {
		next();
		RealDesc();
	} else if (LexNum == LexString) {
		next();
		StringDesc();
	} else if (LexNum == LexIter) {
		next();
		IteratorDesc();
	} else if (LexNum == LexWhile) {
		next();
		WhileDesc();
	} else if (LexNum == LexGet) {
		next();
		GetDesc();
	} else if (LexNum == LexPut) {
		next();
		PutDesc();
	} else if (LexNum == LexIdent) {
		next();
		AssignOrForDesc();
	} else if (LexNum == LexIf) {
		next();
		IfElseDesc();
	} else {
		return;
	}
	if (LexNum != LexSemicolon)
		throw FlyingBug(current, SemicolonExpect);
	next();
	Action();
}

void Parser::ValIntOrIdentDesc()
{
	if (LexNum == LexValInt) {
		next();
	} else if (LexNum == LexIdent) {
		next();
		ArrayDesc();
	} else {
		throw FlyingBug(current, ValIntOrIdentExpect);
	}
}

void Parser::IntDesc()
{
	if (LexNum != LexIdent)
		throw FlyingBug(current, IdentExpect);
	next();
	ArrayDesc();
	if (LexNum != LexLParen)
		throw FlyingBug(current, LParenExpect);
	next();
	ValIntOrIdentDesc();
	if (LexNum != LexRParen)
		throw FlyingBug(current, RParenExpect);
	next();
	if (LexNum == LexComma) {
		next();
		IntDesc();
	}
}

void Parser::RealDesc()
{
	if (LexNum != LexIdent)
		throw FlyingBug(current, IdentExpect);
	next();
	ArrayDesc();
	if (LexNum != LexLParen)
		throw FlyingBug(current, LParenExpect);
	next();
	if (LexNum != LexValReal)
		throw FlyingBug(current, ValRealExpect);
	next();
	if (LexNum != LexRParen)
		throw FlyingBug(current, RParenExpect);
	next();
	if (LexNum == LexComma) {
		next();
		RealDesc();
	}
}

void Parser::StringDesc()
{
	if (LexNum != LexIdent)
		throw FlyingBug(current, IdentExpect);
	next();
	if (LexNum != LexLParen)
		throw FlyingBug(current, LParenExpect);
	next();
	if (LexNum != LexStr)
		throw FlyingBug(current, StrExpect);
	next();
	if (LexNum != LexRParen)
		throw FlyingBug(current, RParenExpect);
	next();
	if (LexNum == LexComma) {
		next();
		StringDesc();
	}
}

void Parser::IteratorDesc()
{
	if (LexNum != LexIdent)
		throw FlyingBug(current, IdentExpect);
	next();
	if (LexNum == LexComma) {
		next();
		IteratorDesc();
	}
}

void Parser::ArrayDesc()
{
	if (LexNum == LexLSB) {
		next();
		ValIntOrIdentDesc();
		if (LexNum == LexComma) {
			next();
			ValIntOrIdentDesc();
		}
		if (LexNum != LexRSB)
			throw FlyingBug(current, RSBExpect);
		next();
	}
}
		
void Parser::WhileDesc()
{
	if (LexNum != LexLParen)
		throw FlyingBug(current, LParenExpect);
	next();
	ExpOr();
	if (LexNum != LexRParen)
		throw FlyingBug(current, RParenExpect);
	next();
	if (LexNum != LexLB)
		throw FlyingBug(current, LBExpect);
	next();
	Action();
	if (LexNum != LexRB)
		throw FlyingBug(current, RBExpect);
	next();
}

void Parser::GetDesc()
{
	if (LexNum != LexLParen)
		throw FlyingBug(current, LParenExpect);
	next();
	if (LexNum != LexIdent)
		throw FlyingBug(current, IdentExpect);
	next();
	ArrayDesc();
	while(LexNum == LexComma) {
		next();
		if (LexNum != LexIdent)
			throw FlyingBug(current, IdentExpect);
		next();
		ArrayDesc();
	}
	if (LexNum != LexRParen)
		throw FlyingBug(current, RParenExpect);
	next();
}

void Parser::PutDesc()
{
	if (LexNum != LexLParen)
		throw FlyingBug(current, LParenExpect);
	next();
	ExpOr();
	while(LexNum == LexComma) {
		next();
		ExpOr();
	}
	if (LexNum != LexRParen)
		throw FlyingBug(current, RParenExpect);
	next();
}

void Parser::AssignOrForDesc()
{
	if (LexNum == LexLSB || LexNum == LexAssign) {
		ArrayDesc();
		AssignDesc();
	} else if (LexNum == LexColon) {
		next();
		ForDesc();
	} else {
		throw FlyingBug(current, AssignExpect);
	}
}

void Parser::AssignDesc()
{
	if (LexNum != LexAssign)
		throw FlyingBug(current, AssignExpect);
	next();
	while(LexNum == LexIdent) {
		next();
		ArrayDesc();
		if (LexNum != LexAssign)
			throw FlyingBug(current, AssignExpect);
		next();
	}
	ExpOr();
}

void Parser::ForDesc()
{
	if (LexNum != LexLParen)
		throw FlyingBug(current, LParenExpect);
	next();
	ValIntOrIdentDesc();
	if (LexNum != LexComma)
		throw FlyingBug(current, CommaExpect);
	next();
	ValIntOrIdentDesc();
	if (LexNum != LexComma)
		throw FlyingBug(current, CommaExpect);
	next();
	ValIntOrIdentDesc();
	if (LexNum != LexRParen)
		throw FlyingBug(current, RParenExpect);
	next();
	if (LexNum != LexLB)
		throw FlyingBug(current, LBExpect);
	next();
	Action();
	if (LexNum != LexRB)
		throw FlyingBug(current, RBExpect);
	next();
}
		
void Parser::IfElseDesc()
{
	if (LexNum != LexLParen)
		throw FlyingBug(current, LParenExpect);
	next();
	ExpOr();
	if (LexNum != LexRParen)
		throw FlyingBug(current, RParenExpect);
	next();
	if (LexNum != LexLB)
		throw FlyingBug(current, LBExpect);
	next();
	Action();
	if (LexNum != LexRB)
		throw FlyingBug(current, RBExpect);
	next();
	if (LexNum == LexElse) {
		next();
		if (LexNum != LexLB)
			throw FlyingBug(current, LBExpect);
		next();
		Action();
		if (LexNum != LexRB)
			throw FlyingBug(current, RBExpect);
		next();
	}
}	

void Parser::ExpOr()
{
	ExpAnd();
	while(LexNum == LexOr) {
		next();
		ExpAnd();
	}
}

void Parser::ExpAnd()
{
	ExpComp();
	while(LexNum == LexAnd) {
		next();
		ExpComp();
	}
}

int Parser::CheckComp(int n)
{
	return n==LexLT||n==LexLE||n==LexGT||n==LexGE||n==LexEq||n==LexNotEq;
}

void Parser::ExpComp()
{
	ExpAddSub();
	while(CheckComp(LexNum)) {
		next();
		ExpAddSub();
	}
}

void Parser::ExpAddSub()
{
	ExpMulDiv();
	while(LexNum == LexAdd || LexNum == LexSub) {
		next();
		ExpMulDiv();
	}
}

void Parser::ExpMulDiv()
{
	ExpLast();
	while(LexNum == LexMul || LexNum == LexDiv) {
		next();
		ExpLast();
	}
}

void Parser::ExpLast()
{
	if (LexNum == LexIdent) {
		next();
		ArrayDesc();
	} else if (LexNum == LexValInt || LexNum == LexValReal) {
		next();
	} else if (LexNum == LexNeg) {
		next();
		ExpLast();
	} else if (LexNum == LexLParen) {
		next();
		ExpOr();
		if (LexNum != LexRParen)
			throw FlyingBug(current, RParenExpect);
		next();
	} else {
		throw FlyingBug(current, ValIntOrIdentExpect);
	}
}

void Parser::next()
{
	char c;
	if (current != NULL)
		delete current;
	current = NULL;
	do {
		c = fgetc(f);
		if ((current = automat.FeedChar(c)) != NULL) {
			if (!((*current).CheckCorrect())) 
				throw FlyingBug(current, InvalLex);
		}
	} while(current == NULL);
	LexNum = (*current).GetLexNum();
}

int main(int argc, char **argv)
{
	Parser parser;
	FILE *f;
	if (argc < 2)
		return 0;
	if (!(f = fopen(argv[1], "r")))
		return 0;
	parser.analyze(f);
	return 0;
}
