#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tables.h"

enum {Int, Real, Iterator, String};

class Lexeme {
	void *value;
	int LexNum, LineNum, TableIDNum, IdentType;
public:
	Lexeme()
		: value(NULL), LexNum(LexEmpty), LineNum(0),
		TableIDNum(0), IdentType(Int) {}
	Lexeme(char buf[], int &BufSize, int line, int num);
	Lexeme(int line, int num);
	int Empty() { return !LexNum; }
	int CheckCorrect() { return LexNum != LexError; } 
	int GetLexNum() { return LexNum; } 
	int GetLineNum() { return LineNum; }
	void SetTableNum(int i) { TableIDNum = i; }
	void SetIdentType(int i) { IdentType = i; }
	void Print(); 
	~Lexeme();
};

Lexeme::Lexeme(char buf[], int &BufSize, int line, int num)
	:TableIDNum(0), IdentType(Int)
{
	LexNum = num;
	LineNum = line;
	if (LexNum == LexIdent || LexNum == LexError) {
		value = new char[BufSize + 1];
		for(int i = 0; i <= BufSize; i++)
			((char *)value)[i] = buf[i];
	} else if (LexNum == LexStr) {
		value = new char[BufSize - 1];
		for(int i = 0; i < BufSize - 2; i++)
			((char *)value)[i] = buf[i+1];
		((char *)value)[BufSize - 1] = 0;
	} else if (LexNum == LexValInt) {
		value = new long long(strtoll(buf, NULL, 10));
	} else if (LexNum == LexValReal) {
		value = new double(strtod(buf, NULL));
	}
	buf[0] = 0;
	BufSize = 0;
}

Lexeme::Lexeme(int line, int num)
	:TableIDNum(0), IdentType(Int)
{
	LexNum = num;
	LineNum = line;
}

void Lexeme::Print() 
{
	if (LexNum == LexIdent || LexNum == LexStr) {
		printf("%s", (char *)value);
	} else if (LexNum == LexValInt) {
		printf("%lld", *(long long *)value);
	} else if (LexNum == LexValReal) {
		printf("%f", *(double *)value);
	} else {
		printf("%s", TableOfWords[LexNum]);
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
	if (Digit(c)) {
		state = Int;
	} else if (c == '\"') {
		state = String;
	} else if (Letter(c)) {
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
	if (Digit(buf[0])) { 
		if (SearchPoint()) {
			return LexValReal;
		} else {
		return LexValInt;
		}
	} else if (buf[0] == '/') {
		return LexEmpty;
	} else if (buf[0] == '"') {
		return LexStr;
	}
	while(TableOfWords[i] != NULL) {
		if (!strcmp(buf, TableOfWords[i]))
			return i;
		i++;
	}
	if (Letter(buf[0]))
		return LexIdent;
	return LexError;
}

class BadLex {
	int ErrNum;
public:
	BadLex(int e): ErrNum(e) {}
	int GetErrNum() const { return ErrNum; }
};

class Parser {
	Automat automat;
	Lexeme *current, *last;
	int LexNum;
	FILE *f;
	void next();
	void S();
	void Action();
	void IntDesc(int AfterComma);
	void RealDesc(int AfterComma);
	void StringDesc(int AfterComma);
	void IteratorDesc(int AfterComma);
	void WhileDesc();
	void GetDesc();
	void PutDesc();
	void AssignOrForDesc();
	void AssignDesc();
	void ForDesc();
	void IfElseDesc();
	void ArrayDesc();
	void ExpOr(int RParenErr, int ValOrIdentErr);
	void ExpAnd(int RParenErr, int ValOrIdentErr);
	void ExpComp(int RParenErr, int ValOrIdentErr);
	void ExpAddSub(int RParenErr, int ValOrIdentErr);
	void ExpMulDiv(int RParenErr, int ValOrIdentErr);
	void ExpLast(int RParenErr, int ValOrIdentErr);
	int CheckComp(int n);
	int CheckInExp(int ErrNum);
	void CheckIfIdent(int AfterComma, int ErrIfComma, int ErrElse); 
	void CheckIfIdentOrVal(int val, int err);
public:
	Parser(): automat(), current(0), last(0), LexNum(0), f(0) {}
	void analyze(FILE *file);
};

void Parser::analyze(FILE *file)
{
	f = file;
	try {
		next();
		S();
	}
	catch(const BadLex &err) {
		int ErrNum = err.GetErrNum();
		printf("Error in line %d: ", (*current).GetLineNum());
		if (ErrNum == NothingExpect) {
			printf("%s\n", TableOfErrors[ErrNum]);
			return;
		} else if (ErrNum == InvalLex) {
			printf("invalid lexeme \"");
		} else if (CheckInExp(ErrNum)) {
			printf("%s after \"", TableOfErrors[ErrNum]);
			(*last).Print();
			printf("\" before \"");
		} else {
			printf("%s before \"", TableOfErrors[ErrNum]);
		}
		(*current).Print();
		printf("\"\n");
		return;
	}
	printf("Correct\n");
}

void Parser::next()
{
	char c;
	if (last != NULL)
		delete last;
	last = current;
	current = NULL;
	do {
		c = fgetc(f);
		if ((current = automat.FeedChar(c)) != NULL) {
			if (!((*current).CheckCorrect())) 
				throw BadLex(InvalLex);
		}
	} while(current == NULL);
	LexNum = (*current).GetLexNum();
}

void Parser::S()
{
	if (LexNum != LexBirth)
		throw BadLex(BirthExpect);
	next();
	Action();
	if (LexNum != LexDeath)
		throw BadLex(DeathExpect);
	next();
	if (LexNum != LexEmpty)
		throw BadLex(NothingExpect);
}

void Parser::Action()
{
	if (LexNum == LexDie) {
		next();
	} else if (LexNum == LexInt) {
		next();
		IntDesc(0);
	} else if (LexNum == LexReal) {
		next();
		RealDesc(0);
	} else if (LexNum == LexString) {
		next();
		StringDesc(0);
	} else if (LexNum == LexIter) {
		next();
		IteratorDesc(0);
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
		throw BadLex(SemicolonExpect);
	next();
	Action();
}

int Parser::CheckInExp(int ErrNum)
{
	return (
		ErrNum == RParenWhileExp || ErrNum == ValOrIdentWhileExp ||
		ErrNum == RParenPutExp || ErrNum == ValOrIdentPutExp ||
		ErrNum == RParenAssignExp || ErrNum == ValOrIdentAssignExp ||
		ErrNum == RParenIfExp || ErrNum == ValOrIdentIfExp
	);
}

void Parser::CheckIfIdent(int AfterComma, int ErrIfComma, int ErrElse) 
{
	if (LexNum != LexIdent) {
		if (AfterComma) {
			throw BadLex(ErrIfComma);
		} else {
			throw BadLex(ErrElse);
		}
	}
}

void Parser::CheckIfIdentOrVal(int val, int err)
{
	if (LexNum == val) {
		next();
	} else if (LexNum == LexIdent) {
		next();
		ArrayDesc();
	} else {
		throw BadLex(err);
	}
}

void Parser::IntDesc(int AfterComma)
{
	CheckIfIdent(AfterComma, IdentAfterCommaInt, IdentAfterInt);  
	next();
	ArrayDesc();
	if (LexNum != LexLParen)
		throw BadLex(LParenInt);
	next();
	CheckIfIdentOrVal(LexValInt, ValIntOrIdent);
	if (LexNum != LexRParen)
		throw BadLex(RParenInt);
	next();
	if (LexNum == LexComma) {
		next();
		IntDesc(1);
	}
}

void Parser::RealDesc(int AfterComma)
{
	CheckIfIdent(AfterComma, IdentAfterCommaReal, IdentAfterReal);  
	next();
	ArrayDesc();
	if (LexNum != LexLParen)
		throw BadLex(LParenReal);
	next();
	if (LexNum == LexValReal || LexNum == LexValInt) {
		next();
	} else if (LexNum == LexIdent) {
		next();
		ArrayDesc();
	} else {
		throw BadLex(ValOrIdentReal);
	}
	if (LexNum != LexRParen)
		throw BadLex(RParenReal);
	next();
	if (LexNum == LexComma) {
		next();
		RealDesc(1);
	}
}

void Parser::StringDesc(int AfterComma)
{
	CheckIfIdent(AfterComma, IdentAfterCommaStr, IdentAfterStr);  
	next();
	if (LexNum != LexLParen)
		throw BadLex(LParenStr);
	next();
	if (LexNum != LexStr)
		throw BadLex(StrExpect);
	next();
	if (LexNum != LexRParen)
		throw BadLex(RParenStr);
	next();
	if (LexNum == LexComma) {
		next();
		StringDesc(1);
	}
}

void Parser::IteratorDesc(int AfterComma)
{
	CheckIfIdent(AfterComma, IdentAfterCommaIter, IdentAfterIter);  
	next();
	if (LexNum == LexComma) {
		next();
		IteratorDesc(1);
	}
}

void Parser::ArrayDesc()
{
	if (LexNum == LexLSB) {
		next();
		CheckIfIdentOrVal(LexValInt, ValIntOrIdentArray);
		if (LexNum == LexComma) {
			next();
			CheckIfIdentOrVal(LexValInt, ValIntOrIdentArraySec);
		}
		if (LexNum != LexRSB)
			throw BadLex(RSBExpect);
		next();
	}
}
		
void Parser::WhileDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenWhile);
	next();
	ExpOr(RParenWhileExp, ValOrIdentWhileExp);
	if (LexNum != LexRParen)
		throw BadLex(RParenWhile);
	next();
	if (LexNum != LexLB)
		throw BadLex(LBWhile);
	next();
	Action();
	if (LexNum != LexRB)
		throw BadLex(RBWhile);
	next();
}

void Parser::GetDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenGet);
	next();
	if (LexNum != LexIdent)
		throw BadLex(IdentGet);
	next();
	ArrayDesc();
	while(LexNum == LexComma) {
		next();
		if (LexNum != LexIdent)
			throw BadLex(IdentGetComma);
		next();
		ArrayDesc();
	}
	if (LexNum != LexRParen)
		throw BadLex(RParenGet);
	next();
}

void Parser::PutDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenPut);
	next();
	ExpOr(RParenPutExp, ValOrIdentPutExp);
	while(LexNum == LexComma) {
		next();
		ExpOr(RParenPutExp, ValOrIdentPutExp);
	}
	if (LexNum != LexRParen)
		throw BadLex(RParenPut);
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
		throw BadLex(AssignExpect);
	}
}

void Parser::AssignDesc()
{
	if (LexNum != LexAssign)
		throw BadLex(AssignExpect);
	next();
	while(LexNum == LexIdent) {
		next();
		ArrayDesc();
		if (LexNum != LexAssign)
			throw BadLex(AssignExpect);
		next();
	}
	ExpOr(RParenAssignExp, ValOrIdentAssignExp);
}

void Parser::ForDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenFor);
	next();
	CheckIfIdentOrVal(LexValInt, ValIntOrIdentForFirst);
	if (LexNum != LexComma)
		throw BadLex(CommaForFirst);
	next();
	CheckIfIdentOrVal(LexValInt, ValIntOrIdentForSecond);
	if (LexNum != LexComma)
		throw BadLex(CommaForSecond);
	next();
	CheckIfIdentOrVal(LexValInt, ValIntOrIdentForThird);
	if (LexNum != LexRParen)
		throw BadLex(RParenFor);
	next();
	if (LexNum != LexLB)
		throw BadLex(LBFor);
	next();
	Action();
	if (LexNum != LexRB)
		throw BadLex(RBFor);
	next();
}
		
void Parser::IfElseDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenIf);
	next();
	ExpOr(RParenIfExp, ValOrIdentIfExp);
	if (LexNum != LexRParen)
		throw BadLex(RParenIf);
	next();
	if (LexNum != LexLB)
		throw BadLex(LBIfExpect);
	next();
	Action();
	if (LexNum != LexRB)
		throw BadLex(RBIfExpect);
	next();
	if (LexNum == LexElse) {
		next();
		if (LexNum != LexLB)
			throw BadLex(LBElseExpect);
		next();
		Action();
		if (LexNum != LexRB)
			throw BadLex(RBElseExpect);
		next();
	}
}	

void Parser::ExpOr(int RParenErr, int ValOrIdentErr)
{
	ExpAnd(RParenErr, ValOrIdentErr);
	while(LexNum == LexOr) {
		next();
		ExpAnd(RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpAnd(int RParenErr, int ValOrIdentErr)
{
	ExpComp(RParenErr, ValOrIdentErr);
	while(LexNum == LexAnd) {
		next();
		ExpComp(RParenErr, ValOrIdentErr);
	}
}

int Parser::CheckComp(int n)
{
	return n==LexLT||n==LexLE||n==LexGT||n==LexGE||n==LexEq||n==LexNotEq;
}

void Parser::ExpComp(int RParenErr, int ValOrIdentErr)
{
	ExpAddSub(RParenErr, ValOrIdentErr);
	while(CheckComp(LexNum)) {
		next();
		ExpAddSub(RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpAddSub(int RParenErr, int ValOrIdentErr)
{
	if (LexNum == LexAdd || LexNum == LexSub) {
		next();
	}
	ExpMulDiv(RParenErr, ValOrIdentErr);
	while(LexNum == LexAdd || LexNum == LexSub) {
		next();
		ExpMulDiv(RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpMulDiv(int RParenErr, int ValOrIdentErr)
{
	ExpLast(RParenErr, ValOrIdentErr);
	while(LexNum == LexMul || LexNum == LexDiv) {
		next();
		ExpLast(RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpLast(int RParenErr, int ValOrIdentErr)
{
	if (LexNum == LexIdent) {
		next();
		ArrayDesc();
	} else if (LexNum == LexValInt || LexNum == LexValReal) {
		next();
	} else if (LexNum == LexNeg) {
		next();
		ExpLast(RParenErr, ValOrIdentErr);
	} else if (LexNum == LexLParen) {
		next();
		ExpOr(RParenErr, ValOrIdentErr);
		if (LexNum != LexRParen)
			throw BadLex(RParenErr);
		next();
	} else {
		throw BadLex(ValOrIdentErr);
	}
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
