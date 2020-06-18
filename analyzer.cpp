#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tables.h"

class Lexeme {
	void *value;
	int LexNum, LineNum, TableIDNum, IdentType;
public:
	Lexeme(): value(NULL), LexNum(LexEmpty), LineNum(0) {}
	Lexeme(char buf[], int &BufSize, int line, int num);
	Lexeme(int line, int num);
	~Lexeme();
	int Empty() { return !LexNum; }
	int CheckCorrect() { return LexNum != LexError; } 
	int GetLexNum() { return LexNum; } 
	int GetLineNum() { return LineNum; }
	char *GetName() { return (char *)value; }
	long long GetInt() { return *((long long *)value); }
	double GetReal() { return *((double *)value); }
	char *GetString() { return (char *)value; }
	void Print(); 
};

Lexeme::Lexeme(char buf[], int &BufSize, int line, int num)
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
	enum {
		H, String, Ident, Int, Real, Equal, lg, 
		Comment, MultiAssign, Error, S, SResend
	};
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
	void StateMultiAssign(char c);
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
	int a = c == ' ' || c == '\t' || Digit(c);
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
	} else if (state == MultiAssign) {
		StateMultiAssign(c);
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
	} else if (c == '\\') {
		state = Comment;
	} else if (c == ':') {
		state = MultiAssign;
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

void Automat::StateMultiAssign(char c)
{
	if (c == '=') {
		state = S;
	} else {
		state = Error;
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
	} else if (buf[0] == '\\') {
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

class IpnElem;

enum {InFstDim, InScdDim, InVarDesc, InAssign, InAddFst, InAddScd,
	InSubFst, InSubScd, InMulFst, InMulScd, InDivFst, InDivScd,
	InOrFst, InOrScd, InAndFst, InAndScd, InLessFst, InLessScd,
	InLessEFst, InLessEScd, InGreatFst, InGreatScd,
	InGreatEFst, InGreatEScd, InEqualFst, InEqualScd,
	InNotEqualFst, InNotEqualScd, InNeg, InOpGoFalse, InPut,
	InSemicolon, InVar, InOpGo
};

class IpnEx {
	int LineNum;
public:
	IpnEx(int l = 0): LineNum(l) {}
	virtual ~IpnEx() {}
	int GetLineNum() { return LineNum; }
	virtual void PrintError() const = 0;
	void HandleError() const
	{
		printf("Error in line %d: ", LineNum);
		PrintError();
	}
};

class IpnExNotInt: public IpnEx {
	int where;
public:
	IpnExNotInt(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotInt() {}
	virtual void PrintError() const
	{
		if (where == InFstDim) {
			printf("integer value expected ");
			printf("as first dimension of array ");
		} else if (where == InScdDim) {
			printf("integer value expected ");
			printf("as second dimension of array ");
		} else if (where == InVarDesc) {
			printf("integer value expected ");
			printf("as initial value in description of integer ");
		} else if (where == InAssign) {
			printf("expression after \"=\" is not integer type ");
		}
		printf("\n");
	}
};

class IpnExNotIntOrReal: public IpnEx {
	int where;
public:
	IpnExNotIntOrReal(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotIntOrReal() {}
	virtual void PrintError() const
	{
		if (where == InAssign) {
			printf("expression after \"=\" is not integer or real type ");
		} else if (where == InVarDesc) {
			printf("integer or real value expected ");
			printf("as initial value in description of real ");
		} else if (where == InAddFst) {
			printf("first operand in addition is not integer or real ");
		} else if (where == InAddScd) {
			printf("second operand in addition is not integer or real ");
		} else if (where == InSubFst) {
			printf("first operand in subtraction is not integer or real ");
		} else if (where == InSubScd) {
			printf("second operand in subtraction is not integer or real ");
		} else if (where == InMulFst) {
			printf("first operand in multiplication is not integer or real ");
		} else if (where == InMulScd) {
			printf("second operand in multiplication is not integer or real ");
		} else if (where == InDivFst) {
			printf("first operand in division is not integer or real ");
		} else if (where == InDivScd) {
			printf("second operand in division is not integer or real ");
		} else if (where == InOrFst) {
			printf("first operand in \"or\" is not integer or real ");
		} else if (where == InOrScd) {
			printf("second operand in \"or\" is not integer or real ");
		} else if (where == InAndFst) {
			printf("first operand in \"and\" is not integer or real ");
		} else if (where == InAndScd) {
			printf("second operand in \"and\" is not integer or real ");
		} else if (where == InLessFst) {
			printf("first operand in \"<\" is not integer or real ");
		} else if (where == InLessScd) {
			printf("second operand in \"<\" is not integer or real ");
		} else if (where == InLessEFst) {
			printf("first operand in \"<=\" is not integer or real ");
		} else if (where == InLessEScd) {
			printf("second operand in \"<=\" is not integer or real ");
		} else if (where == InGreatFst) {
			printf("first operand in \">\" is not integer or real ");
		} else if (where == InGreatScd) {
			printf("second operand in \">\" is not integer or real ");
		} else if (where == InGreatEFst) {
			printf("first operand in \">=\" is not integer or real ");
		} else if (where == InGreatEScd) {
			printf("second operand in \">=\" is not integer or real ");
		} else if (where == InEqualFst) {
			printf("first operand in \"==\" is not integer or real ");
		} else if (where == InEqualScd) {
			printf("second operand in \"==\" is not integer or real ");
		} else if (where == InNotEqualFst) {
			printf("first operand in \"=!\" is not integer or real ");
		} else if (where == InNotEqualScd) {
			printf("second operand in \"=!\" is not integer or real ");
		} else if (where == InNeg) {
			printf("operand in \"!\" is not integer or real ");
		} else if (where == InOpGoFalse) {
			printf("operand in conditional jump is not integer or real ");
		}
		printf("\n");
	}
};

class IpnExNotStr: public IpnEx {
	int where;
public:
	IpnExNotStr(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotStr() {}
	virtual void PrintError() const
	{
		if (where == InAssign) {
			printf("expression after \"=\" is not string type ");
		} else if (where == InVarDesc) {
			printf("string value expected ");
			printf("as initial value in description of string ");
		}
		printf("\n");
	}
};

class IpnExNotIntOrRealOrString: public IpnEx {
	int where;
public:
	IpnExNotIntOrRealOrString(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotIntOrRealOrString() {}
	virtual void PrintError() const
	{
		if (where == InPut) {
			printf("integer or real or string value expected ");
			printf("as argument of put ");
		} else if (where == InSemicolon) {
			printf("integer or real or string value expected before \";\" ");
		}
		printf("\n");
	}
};

class IpnExNotFound: public IpnEx {
	int where;
	char *name;
public:
	IpnExNotFound(int l, int w, char *n): IpnEx(l), where(w)
		{ name = strdup(n); }
	virtual ~IpnExNotFound() { delete [] name; }
	virtual void PrintError() const
	{
		if (where == InVar || where == InAssign) {
			printf("variable \"%s\" was not declared in this scope ", name);
		}
		printf("\n");
	}
};

class IpnExRedec: public IpnEx {
	int where;
	char *name;
public:
	IpnExRedec(int l, int w, char *n): IpnEx(l), where(w)
		{ name = strdup(n); }
	virtual ~IpnExRedec() { delete [] name; }
	virtual void PrintError() const
	{
		if (where == InVarDesc) {
			printf("variable \"%s\" is already declared ", name);
		}
		printf("\n");
	}
};

class IpnExStackEmpty: public IpnEx {
public:
	IpnExStackEmpty(int l): IpnEx(l) {}
	virtual ~IpnExStackEmpty() {}
	virtual void PrintError() const { printf("Stack is empty\n"); }
};

class IpnExSegFault: public IpnEx {
	char *name;
public:
	IpnExSegFault(int l): IpnEx(l), name(0) {}
	IpnExSegFault(int l, char *n): IpnEx(l) { name = strdup(n); }
	virtual ~IpnExSegFault()
	{
		if (name)
			delete [] name;
	}
	virtual void PrintError() const
	{
		printf("%s: Segmentation fault\n", name);
	}
};

class IpnExNotLabel: public IpnEx {
	int where;
public:
	IpnExNotLabel(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotLabel() {}
	virtual void PrintError() const
	{
		if (where == InOpGo) {
			printf("operand in unconditional jump is not label ");
		} else if (where == InOpGoFalse) {
			printf("operand in conditional jump is not label ");
		}
		printf("\n");
	}
};

class IpnExNotVarAddr: public IpnEx {
	int where;
public:
	IpnExNotVarAddr(int l, int w): IpnEx(l), where(w) {}
	virtual ~IpnExNotVarAddr() {}
	virtual void PrintError() const
	{
		if (where == InAssign) {
			printf("operand in assignment is not address of variable ");
		}
		printf("\n");
	}
};

enum {NotFound, Int, Real, String};

struct VarElem {
	int type, FstDim, ScdDim, level;
	char *name;
	void *value;
	VarElem *next;
	VarElem(char *n, int t, void *v, int f, int s, int l)
		: type(t), FstDim(f), ScdDim(s), level(l), name(n), value(v)
	{
#ifdef DEBUGGING
		printf("%s %d %d\n", name, FstDim, ScdDim);
#endif
	}
};

struct IpnItem {
	IpnElem *elem;
	IpnItem *next;
	IpnItem() {}
	IpnItem(IpnElem *e, IpnItem *n): elem(e), next(n) {}
};

class IpnElem {
	int LineNum;
public:
	IpnElem(int l = 0): LineNum(l) {}
	virtual ~IpnElem() {}
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList
	) const = 0;
	int GetLineNum() const { return LineNum; }
protected:
	void Push(IpnItem **stack, IpnElem *elem) const
	{
		*stack = new IpnItem(elem, *stack);
	}
	IpnElem *Pop(IpnItem **stack) const
	{
		IpnElem *elem;
		IpnItem *help;
		if (*stack == 0) {
			throw new IpnExStackEmpty(GetLineNum());
		} else {
			help = *stack;
			elem = (*stack)->elem;
			*stack = (*stack)->next;
			delete help;
		}
		return elem;
	}
};

class IpnConst: public IpnElem {
public:
	IpnConst(int l = 0): IpnElem(l) {}
	virtual IpnElem *Clone() const = 0;
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList
	) const
	{
		Push(stack, Clone());
		*CurCmd = (*CurCmd)->next;
	}
};

template <class T>
class IpnGenericConst: public IpnConst {
	T value;
public:
	IpnGenericConst(const T v, int l = 0): IpnConst(l), value(v) {}
	virtual ~IpnGenericConst() {}
	virtual IpnElem *Clone() const
		{ return new IpnGenericConst<T>(value, GetLineNum()); }
	T Get() const { return value; }
};

typedef IpnGenericConst<long long> IpnInt;
typedef IpnGenericConst<double> IpnReal;
typedef IpnGenericConst<IpnItem *> IpnLabel;

class IpnEndOfArg: public IpnConst {
public:
	IpnEndOfArg(int l = 0): IpnConst(l) {}
	virtual ~IpnEndOfArg() {}
	virtual IpnEndOfArg *Clone() const
		{ return new IpnEndOfArg(GetLineNum()); }
};

class IpnString: public IpnConst {
	char *str;
public:
	IpnString(const char *s, int l = 0): IpnConst(l) { str = strdup(s); }
	IpnString(const IpnString &other) { str = strdup(other.str); }
	virtual ~IpnString() { delete [] str; }
	virtual IpnString *Clone() const
		{ return new IpnString(str, GetLineNum()); }
	char *Get() const { return str; }
};

class IpnVarAddr: public IpnConst {
	char *str;
public:
	IpnVarAddr(const char *s, int l = 0): IpnConst(l) { str = strdup(s); }
	IpnVarAddr(const IpnVarAddr &other) { str = strdup(other.str); }
	virtual ~IpnVarAddr() { delete [] str; }
	virtual IpnVarAddr *Clone() const
		{ return new IpnVarAddr(str, GetLineNum()); }
	char *Get() const { return str; }
};

class IpnVarOrAssign: public IpnElem {
protected:
	int GetType(VarElem **VarList, char *search) const
	{
		VarElem *cur = *VarList;
		while(cur != 0) {
			if (!strcmp(cur->name, search))
				return cur->type;
			cur = cur->next;
		}
		return NotFound;
	}
	void GetDims(IpnItem **stack, int &FstDim, int &ScdDim, int inDesc) const
	{
		IpnElem *operand2 = Pop(stack), *operand1 = Pop(stack);
		IpnInt *iInt2 = dynamic_cast<IpnInt*>(operand2);
		IpnInt *iInt1 = dynamic_cast<IpnInt*>(operand1);
		if (!iInt2) {
			throw new IpnExNotInt(operand2->GetLineNum(), InScdDim);
		} else if (!iInt1) {
			throw new IpnExNotInt(operand1->GetLineNum(), InScdDim);
		} else {
			ScdDim = iInt2->Get();
			FstDim = iInt1->Get();
		}
		delete operand1;
		delete operand2;
#ifdef DEBUGGING
		printf("?%d %d\n", FstDim, ScdDim);
#endif
		if (FstDim < 0 || ScdDim < 0)
			throw new IpnExSegFault(GetLineNum());
		if (inDesc) {
			if (FstDim == 0 || ScdDim == 0)
				throw new IpnExSegFault(GetLineNum());
		}
	}
public:
	IpnVarOrAssign(int l = 0): IpnElem(l) {}
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const = 0;
	void Evaluate(IpnItem **stack, IpnItem **CurCmd, VarElem **VarList) const
	{
		IpnElem *res = EvaluateFun(stack, VarList);
		if (res)
			Push(stack, res);
		*CurCmd = (*CurCmd)->next;
	}
};

class IpnVar: public IpnVarOrAssign {
	char *name;
	template <class T>
	T GetValue(VarElem **VarList, int FstDim, int ScdDim) const
	{
		VarElem *cur = *VarList;
		while(cur != 0) {
			if (!strcmp(cur->name, name)) {
				if (FstDim >= cur->FstDim || ScdDim >= cur->ScdDim) {
					throw new IpnExSegFault(GetLineNum(), name);
				} else {
					int dim = FstDim*(cur->ScdDim) + ScdDim;
					return ((T *)(cur->value))[dim];
				}
			}
			cur = cur->next;
		}
		return 0;
	}
	char *GetString(VarElem **VarList) const
	{
		VarElem *cur = *VarList;
		while(cur != 0) {
			if (!strcmp(cur->name, name))
				return (char *)(cur->value);
			cur = cur->next;
		}
		return 0;
	}
public:
	IpnVar(char *n, int l = 0): IpnVarOrAssign(l) { name = strdup(n); }
	virtual ~IpnVar() { delete [] name; }
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const
	{
		int FstDim, ScdDim;
		GetDims(stack, FstDim, ScdDim, 0);
		int type = GetType(VarList, name);
		if (type == NotFound) {
			throw new IpnExNotFound(GetLineNum(), InVar, name);
		} else if (type == Int) {
			return new IpnInt(GetValue<long long>(VarList, FstDim, ScdDim),
				GetLineNum());
		} else if (type == Real) {
			return new IpnReal(GetValue<double>(VarList, FstDim, ScdDim),
				GetLineNum());
		} else {
			return new IpnString(GetString(VarList), GetLineNum());
		}
	}
};

class IpnAssign: public IpnVarOrAssign {
	template <class T, class IpnType>
	void SetValue(IpnItem **stack, VarElem **VarList, IpnVarAddr *VarAddr,
		IpnType IpnName) const
	{
		int FstDim, ScdDim;
		VarElem *cur = *VarList;
		char *name = VarAddr->Get();
		GetDims(stack, FstDim, ScdDim, 0);
		while(cur != 0) {
			if (!strcmp(cur->name, name)) {
				if (FstDim >= cur->FstDim || ScdDim >= cur->ScdDim) {
					throw new IpnExSegFault(GetLineNum(), name);
				} else {
					int dim = FstDim*(cur->ScdDim) + ScdDim;
					((T *)(cur->value))[dim] = IpnName->Get();
					delete VarAddr;
					delete IpnName;
					return;
				}
			}
			cur = cur->next;
		}
	}
	void SetString(VarElem **VarList, IpnVarAddr *VarAddr,
		IpnString *iString) const
	{
		VarElem *cur = *VarList;
		char *name = VarAddr->Get();
		while(cur != 0) {
			if (!strcmp(cur->name, name)) {
				delete [] (char *)(cur->value);
				cur->value = strdup(iString->Get());
#ifdef DEBUGGING
				printf("%s\n", (char *)(cur->value));
#endif
				delete VarAddr;
				delete iString;
				return;
			}
			cur = cur->next;
		}
	}
public:
	IpnAssign(int l = 0): IpnVarOrAssign(l) {}
	virtual ~IpnAssign() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const
	{
		IpnElem *operand2 = Pop(stack), *operand1 = Pop(stack);
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand2);
		IpnReal *iReal = dynamic_cast<IpnReal*>(operand2);
		IpnString *iString = dynamic_cast<IpnString*>(operand2);
		IpnVarAddr *iVarAddr = dynamic_cast<IpnVarAddr*>(operand1);
		if (!iVarAddr)
			throw new IpnExNotVarAddr(operand1->GetLineNum(), InAssign);
		int type = GetType(VarList, iVarAddr->Get());
		if (type == NotFound) {
			int line = iVarAddr->GetLineNum();
			throw new IpnExNotFound(line, InAssign, iVarAddr->Get());
		} else if (type == Real) {
			if (iInt) {
				SetValue<double, IpnInt *>(stack, VarList, iVarAddr, iInt);
				return new IpnInt(iInt->Get(), GetLineNum());
			} else if (iReal) {
				SetValue<double, IpnReal *>(stack, VarList, iVarAddr, iReal);
				return new IpnReal(iReal->Get(), GetLineNum());
			} else {
				throw new IpnExNotIntOrReal(operand2->GetLineNum(), InAssign);
			}
		} else if (type == Int) {
			if (iInt) {
				SetValue<long long, IpnInt *>(stack, VarList, iVarAddr, iInt);
				return new IpnInt(iInt->Get(), GetLineNum());
			} else {
				throw new IpnExNotInt(operand2->GetLineNum(), InAssign);
			}
		} else if (type == String) {
			if (iString) {
				SetString(VarList, iVarAddr, iString);
				return new IpnString(iString->Get(), GetLineNum());
			} else {
				throw new IpnExNotStr(operand2->GetLineNum(), InAssign);
			}
		}
		return 0;
	}
};

class IpnVarDesc: public IpnVarOrAssign {
	int type, FstDim, ScdDim, considered;
	char *name;
	void *value;
	int SearchName(VarElem *VarList)
	{
		while(VarList != 0) {
			if (!strcmp(VarList->name, name))
				return 1;
			VarList = VarList->next;
		}
		return 0;
	}
	void AddInVarList(VarElem **VarList, int level)
	{
		VarElem *NewElem;
		NewElem = new VarElem(name, type, value, FstDim, ScdDim, level);
		NewElem->next = *VarList;
		*VarList = NewElem;
		considered = 1;
	}
public:
	IpnVarDesc(char *n, int t, int l = 0)
		: IpnVarOrAssign(l), type(t), FstDim(0), ScdDim(0), 
		considered(0), value(0)
	{
		name = strdup(n);
	}
	~IpnVarDesc()
	{
		delete [] name;
		if (type == Int) {
			delete [] (long long *)value;
		} else if (type == Real) {
			delete [] (double *)value;
		} else if (type == String) {
			delete [] (char *)value;
		}
	}
	virtual IpnElem *EvaluateFun(IpnItem **stack, VarElem **VarList) const
		{ return 0; }
	void AddVar(IpnItem **stack, VarElem **VarList, int level)
	{
		IpnElem *operand3 = Pop(stack);
		GetDims(stack, FstDim, ScdDim, 1);
		if (!considered) {
			if (type == Int) {
				IpnInt *iInt = dynamic_cast<IpnInt*>(operand3);
				if (!iInt)
					throw new IpnExNotInt(operand3->GetLineNum(), InVarDesc);
				value = new long long[FstDim * ScdDim];
#ifdef DEBUGGING
				printf("%lld\n", iInt->Get());
#endif
				for(int i = 0; i < FstDim * ScdDim; i++)
					((long long *)value)[i] = iInt->Get();
			} else if (type == Real) {
				IpnInt *iInt = dynamic_cast<IpnInt*>(operand3);
				IpnReal *iReal = dynamic_cast<IpnReal*>(operand3);
				double InitValue;
				if (iInt) {
					InitValue = iInt->Get();
#ifdef DEBUGGING
					printf("%lld\n", iInt->Get());
#endif
				} else if (iReal) {
					InitValue = iReal->Get();
#ifdef DEBUGGING
					printf("%f\n", iReal->Get());
#endif
				} else {
					int line = operand3->GetLineNum();
					throw new IpnExNotIntOrReal(line, InVarDesc);
				}
				value = new double[FstDim * ScdDim];
				for(int i = 0; i < FstDim * ScdDim; i++)
					((double *)value)[i] = InitValue;
			} else if (type == String) {
				IpnString *iString = dynamic_cast<IpnString*>(operand3);
				if (!iString) {
					int line = operand3->GetLineNum();
					throw new IpnExNotStr(line, InVar);
				}
				value = strdup(iString->Get());
#ifdef DEBUGGING
				printf("%s\n", (char*)value);
#endif
			}
			if (SearchName(*VarList))
				throw new IpnExRedec(GetLineNum(), InVarDesc, name);
			AddInVarList(VarList, level);
		}
		delete operand3;
	}
};

class IpnFunction: public IpnElem {
public:
	IpnFunction(int l = 0): IpnElem(l) {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const = 0;
	void Evaluate(IpnItem **stack, IpnItem **CurCmd, VarElem **VarList) const
	{
		IpnElem *res = EvaluateFun(stack);
		if (res)
			Push(stack, res);
		*CurCmd = (*CurCmd)->next;
	}
};

#define CLASSARITHMETICOPERATION(ClassName, oper, ExOper1, ExOper2)\
class ClassName: public IpnFunction {\
	void DelAndThrow(IpnElem *OperDel, IpnElem *OperThrow, int ExOper) const\
	{\
		delete OperDel;\
		throw new IpnExNotIntOrReal(OperThrow->GetLineNum(), ExOper);\
	}\
	double Check(IpnInt *iInt, IpnElem *OpDel, IpnElem *OpTh, int ExOp) const\
	{\
		if (iInt) {\
			return iInt->Get();\
		} else {\
			DelAndThrow(OpDel, OpTh, ExOp);\
		}\
		return 0;\
	}\
public:\
	ClassName(int l = 0): IpnFunction(l) {}\
	virtual ~ClassName() {}\
	virtual IpnElem *EvaluateFun(IpnItem **stack) const\
	{\
		IpnElem *operand2 = Pop(stack), *operand1 = Pop(stack);\
		long long OneIsReal = 0, ResInt;\
		double ResReal;\
		IpnInt *iInt1 = dynamic_cast<IpnInt*>(operand1);\
		IpnInt *iInt2 = dynamic_cast<IpnInt*>(operand2);\
		IpnReal *iReal1 = dynamic_cast<IpnReal*>(operand1);\
		IpnReal *iReal2 = dynamic_cast<IpnReal*>(operand2);\
		if (iReal1 || iReal2) {\
			OneIsReal = 1;\
			if (iReal1 && iReal2) {\
				ResReal = iReal1->Get() oper iReal2->Get();\
			} else if (iReal1) {\
				double r = iReal1->Get();\
				ResReal = r oper Check(iInt2,operand1,operand2,ExOper2);\
			} else if (iReal2) {\
				double r = iReal2->Get();\
				ResReal = r oper Check(iInt1,operand2,operand1,ExOper1);\
			}\
		} else if (iInt1 && iInt2) {\
			ResInt = iInt1->Get() oper iInt2->Get();\
		} else if (!iInt1) {\
			DelAndThrow(operand2, operand1, ExOper1);\
		} else {\
			DelAndThrow(operand1, operand2, ExOper2);\
		}\
		delete operand1;\
		delete operand2;\
		if (OneIsReal) {\
			return new IpnReal(ResReal, GetLineNum());\
		} else {\
			return new IpnInt(ResInt, GetLineNum());\
		}\
	}\
};\

CLASSARITHMETICOPERATION(IpnAdd, +, InAddFst, InAddScd);
CLASSARITHMETICOPERATION(IpnSub, -, InSubFst, InSubScd);
CLASSARITHMETICOPERATION(IpnMul, *, InMulFst, InMulScd);
CLASSARITHMETICOPERATION(IpnDiv, /, InDivFst, InDivScd);

#define CLASSLOGICALOPERATION(ClassName, oper, ExOper1, ExOper2)\
class ClassName: public IpnFunction {\
public:\
	ClassName(int l = 0): IpnFunction(l) {}\
	virtual ~ClassName() {}\
	virtual IpnElem *EvaluateFun(IpnItem **stack) const\
	{\
		IpnElem *operand2 = Pop(stack);\
		long long ret;\
		double res;\
		IpnInt *iInt2 = dynamic_cast<IpnInt*>(operand2);\
		IpnReal *iReal2 = dynamic_cast<IpnReal*>(operand2);\
 		if (iReal2 || iInt2) {\
			if (iReal2) {\
				res = iReal2->Get();\
			} else {\
				res = iInt2->Get();\
			}\
		} else {\
			throw new IpnExNotIntOrReal(operand2->GetLineNum(), ExOper2);\
		}\
		delete operand2;\
		IpnElem *operand1 = Pop(stack);\
		IpnInt *iInt1 = dynamic_cast<IpnInt*>(operand1);\
		IpnReal *iReal1 = dynamic_cast<IpnReal*>(operand1);\
 		if (iReal1 || iInt1) {\
			if (iReal1) {\
				ret = iReal1->Get() oper res;\
			} else {\
				ret = iInt1->Get() oper res;\
			}\
		} else {\
			throw new IpnExNotIntOrReal(operand1->GetLineNum(), ExOper1);\
		}\
		delete operand1;\
		return new IpnInt(ret, GetLineNum());\
	}\
};\

CLASSLOGICALOPERATION(IpnOr, ||, InOrFst, InOrScd);
CLASSLOGICALOPERATION(IpnAnd, &&, InAndFst, InAndScd);
CLASSLOGICALOPERATION(IpnLess, <, InLessFst, InLessScd);
CLASSLOGICALOPERATION(IpnLessE, <=, InLessEFst, InLessEScd);
CLASSLOGICALOPERATION(IpnGreat, >, InGreatFst, InGreatScd);
CLASSLOGICALOPERATION(IpnGreatE, >=, InGreatEFst, InGreatEScd);
CLASSLOGICALOPERATION(IpnEqual, ==, InEqualFst, InEqualScd);
CLASSLOGICALOPERATION(IpnNotEqual, !=, InNotEqualFst, InNotEqualScd);

class IpnNeg: public IpnFunction {
public:
	IpnNeg(int l = 0): IpnFunction(l) {}
	virtual ~IpnNeg() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const
	{
		IpnElem *operand = Pop(stack);
		long long res;
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand);
		IpnReal *iReal = dynamic_cast<IpnReal*>(operand);
 		if (iReal || iInt) {
			if (iReal) {
				res = !(iReal->Get());
			} else {
				res = !(iInt->Get());
			}
		} else {
			throw new IpnExNotIntOrReal(operand->GetLineNum(), InNeg);
		}
		delete operand;
		return new IpnInt(res, GetLineNum());
	}
};

class IpnPut: public IpnFunction {
public:
	IpnPut(int l = 0): IpnFunction(l) {}
	virtual ~IpnPut() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const
	{
		IpnItem *HelpList = 0, *HelpItem;
		IpnElem *operand = Pop(stack);
		while(!dynamic_cast<IpnEndOfArg*>(operand)) {
			HelpList = new IpnItem(operand, HelpList);
			operand = Pop(stack);
		}
		delete operand;
		while(HelpList != 0) {
			operand = HelpList->elem;
			HelpItem = HelpList;
			HelpList = HelpList->next;
			delete HelpItem;
			IpnInt *iInt = dynamic_cast<IpnInt*>(operand);
			IpnReal *iReal = dynamic_cast<IpnReal*>(operand);
			IpnString *iString = dynamic_cast<IpnString*>(operand);
			if (iInt) {
				printf("%lld ", iInt->Get());
			} else if (iReal) {
				printf("%f ", iReal->Get());
			} else if (iString) {
				printf("%s ", iString->Get());
			} else {
				int line = operand->GetLineNum();
				throw new IpnExNotIntOrRealOrString(line, InPut);
			}
			delete operand;
		}
		printf("\n");
		return 0;
	}
};

class IpnSemicolon: public IpnFunction {
public:
	IpnSemicolon(int l = 0): IpnFunction(l) {}
	virtual ~IpnSemicolon() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const
	{
		IpnElem *operand = Pop(stack);
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand);
		IpnReal *iReal = dynamic_cast<IpnReal*>(operand);
		IpnString *iString = dynamic_cast<IpnString*>(operand);
 		if (!iReal && !iInt && !iString) {
			int line = operand->GetLineNum();
			throw new IpnExNotIntOrRealOrString(line, InSemicolon);
		}
		delete operand;
		return 0;
	}
};

class IpnNoOp: public IpnFunction {
public:
	IpnNoOp(int l = 0): IpnFunction(l) {}
	virtual ~IpnNoOp() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const { return 0; }
};

class IpnBraceL: public IpnFunction {
public:
	IpnBraceL(int l = 0): IpnFunction(l) {}
	virtual ~IpnBraceL() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const { return 0; }
};

class IpnBraceR: public IpnFunction {
public:
	IpnBraceR(int l = 0): IpnFunction(l) {}
	virtual ~IpnBraceR() {}
	virtual IpnElem *EvaluateFun(IpnItem **stack) const { return 0; }
	void DelVar(VarElem **VarList, int &NestingLevel)
	{
		VarElem *help;
		while(*VarList != 0 && (*VarList)->level == NestingLevel) {
			help = *VarList;
			*VarList = (*VarList)->next;
			delete help;
		}
		NestingLevel--;
	}
};

class IpnSell: public IpnFunction {
};

class IpnEndTurn: public IpnFunction {
};

class IpnOpGo: public IpnElem {
public:
	IpnOpGo(int l = 0): IpnElem(l) {}
	virtual ~IpnOpGo() {}
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList
	) const
	{
		IpnElem *operand = Pop(stack);
		IpnLabel *label = dynamic_cast<IpnLabel*>(operand);
		if (!label)
			throw new IpnExNotLabel(operand->GetLineNum(), InOpGo);
		*CurCmd = label->Get();
		delete operand;
	}
};

class IpnOpGoFalse: public IpnElem {
public:
	IpnOpGoFalse(int l = 0): IpnElem(l) {}
	virtual ~IpnOpGoFalse() {}
	virtual void Evaluate(
		IpnItem **stack, IpnItem **CurCmd, VarElem **VarList
	) const
	{
		double res;
		IpnElem *operand1 = Pop(stack);
		IpnLabel *label = dynamic_cast<IpnLabel*>(operand1);
		if (!label)
			throw new IpnExNotLabel(operand1->GetLineNum(), InOpGoFalse);
		IpnElem *operand2 = Pop(stack);
		IpnInt *iInt = dynamic_cast<IpnInt*>(operand2);
		IpnReal *iReal = dynamic_cast<IpnReal*>(operand2);
		if (iInt) {
			res = iInt->Get();
		} else if (iReal) {
			res = iReal->Get();
		} else {
			delete operand1;
			throw new IpnExNotIntOrReal(operand2->GetLineNum(), InOpGoFalse);
		}
		if (!res) {
			*CurCmd = label->Get();
		} else {
			*CurCmd = (*CurCmd)->next;
		}
		delete operand1;
	}
};

class Ipn {
	IpnItem *FstItem, *LstItem;
	IpnBraceL *BraceL;
	IpnBraceR *BraceR;
	IpnVarDesc *VarDesc;
public:
	Ipn(): FstItem(0), LstItem(0), BraceL(0), BraceR(0) {}
	void Add(IpnElem *NewElem);
	void Add(IpnElem *NewElem1, IpnElem *NewElem2);
	void AddNoShift(IpnElem *NewElem);
	void AddNoShift(IpnElem *NewElem1, IpnElem *NewElem2);
	void AddInEnd(IpnElem *NewElem)
		{ LstItem->next->next = new IpnItem(NewElem, LstItem->next->next); }
	IpnItem* Get() { return LstItem; }
	IpnItem* GetLast() { return LstItem->next; }
	IpnItem* GetLastLast() { return LstItem->next->next; }
	void Shift() { LstItem = LstItem->next; }
	void Shift(int n);
	void Perform();
	void Print(IpnElem *elem);
	~Ipn()
	{
		IpnItem *help;
		while(FstItem != 0) {
			help = FstItem;
			FstItem = FstItem->next;
			delete help->elem;
			delete help;
		}
	}
};

void Ipn::Add(IpnElem *NewElem)
{
	if (FstItem == 0) {
		FstItem = new IpnItem(NewElem, 0);
		LstItem = FstItem;
	} else {
		LstItem->next = new IpnItem(NewElem, LstItem->next);
		LstItem = LstItem->next;
	}
}

void Ipn::Add(IpnElem *NewElem1, IpnElem *NewElem2)
{
	Add(NewElem1);
	Add(NewElem2);
}

void Ipn::AddNoShift(IpnElem *NewElem)
{
	LstItem->next = new IpnItem(NewElem, LstItem->next);
}

void Ipn::AddNoShift(IpnElem *NewElem1, IpnElem *NewElem2)
{
	AddNoShift(NewElem2);
	AddNoShift(NewElem1);
}

void Ipn::Shift(int n)
{
	for(int i = 0; i < n; i++)
		Shift();
}

void Ipn::Perform()
{
	IpnItem *stack = 0, *CurCmd = FstItem;
	VarElem *VarList = 0;
	int NestingLevel = 0;
	try
	{
		while(CurCmd != 0) {
#ifdef DEBUGGING
			Print(CurCmd->elem);
#endif
			BraceL = dynamic_cast<IpnBraceL*>(CurCmd->elem);
			BraceR = dynamic_cast<IpnBraceR*>(CurCmd->elem);
			VarDesc = dynamic_cast<IpnVarDesc*>(CurCmd->elem);
			if (BraceL) {
				NestingLevel++;
#ifdef DEBUGGING
				printf("%d\n", NestingLevel);
#endif
			}
			if (BraceR) {
				BraceR->DelVar(&VarList, NestingLevel);
#ifdef DEBUGGING
				printf("%d!\n", NestingLevel);
#endif
			}
			if (VarDesc) {
				VarDesc->AddVar(&stack, &VarList, NestingLevel);
			}
			CurCmd->elem->Evaluate(&stack, &CurCmd, &VarList);
		}
	}
	catch(IpnEx *ex)
	{
		ex->HandleError();
	}
}

void Ipn::Print(IpnElem *elem)
{
	if (dynamic_cast<IpnInt*>(elem)) {
		printf("IpnInt\n");
	} else if (dynamic_cast<IpnReal*>(elem)) {
		printf("IpnReal\n");
	} else if (dynamic_cast<IpnLabel*>(elem)) {
		printf("IpnLabel\n");
	} else if (dynamic_cast<IpnEndOfArg*>(elem)) {
		printf("IpnEndOfArg\n");
	} else if (dynamic_cast<IpnString*>(elem)) {
		printf("IpnString\n");
	} else if (dynamic_cast<IpnVarAddr*>(elem)) {
		printf("IpnVarAddr\n");
	} else if (dynamic_cast<IpnVar*>(elem)) {
		printf("IpnVar\n");
	} else if (dynamic_cast<IpnAssign*>(elem)) {
		printf("IpnAssign\n");
	} else if (dynamic_cast<IpnAdd*>(elem)) {
		printf("IpnAdd\n");
	} else if (dynamic_cast<IpnSub*>(elem)) {
		printf("IpnSub\n");
	} else if (dynamic_cast<IpnMul*>(elem)) {
		printf("IpnMul\n");
	} else if (dynamic_cast<IpnDiv*>(elem)) {
		printf("IpnDiv\n");
	} else if (dynamic_cast<IpnOr*>(elem)) {
		printf("IpnOr\n");
	} else if (dynamic_cast<IpnAnd*>(elem)) {
		printf("IpnAnd\n");
	} else if (dynamic_cast<IpnLess*>(elem)) {
		printf("IpnLess\n");
	} else if (dynamic_cast<IpnLessE*>(elem)) {
		printf("IpnLessE\n");
	} else if (dynamic_cast<IpnGreat*>(elem)) {
		printf("IpnOr\n");
	} else if (dynamic_cast<IpnGreatE*>(elem)) {
		printf("IpnAnd\n");
	} else if (dynamic_cast<IpnEqual*>(elem)) {
		printf("IpnEqual\n");
	} else if (dynamic_cast<IpnNotEqual*>(elem)) {
		printf("IpnNotEqual\n");
	} else if (dynamic_cast<IpnNeg*>(elem)) {
		printf("IpnNeg\n");
	} else if (dynamic_cast<IpnSemicolon*>(elem)) {
		printf("IpnSemicolon\n");
	} else if (dynamic_cast<IpnVarDesc*>(elem)) {
		printf("IpnVarDesc\n");
	} else if (dynamic_cast<IpnNoOp*>(elem)) {
		printf("IpnNoOp\n");
	} else if (dynamic_cast<IpnBraceL*>(elem)) {
		printf("IpnBraceL\n");
	} else if (dynamic_cast<IpnBraceR*>(elem)) {
		printf("IpnBraceR\n");
	} else if (dynamic_cast<IpnOpGo*>(elem)) {
		printf("IpnOpGo\n");
	} else if (dynamic_cast<IpnOpGoFalse*>(elem)) {
		printf("IpnOpGoFalse\n");
	}
}

struct IntItem {
	int value;
	IntItem *next;
	IntItem(int v, IntItem *n): value(v), next(n) {}
};

class IntStack {
	IntItem *first;
public:
	IntStack(int Init) { first = new IntItem(Init, 0); }
	void Add(int NewElem) { first = new IntItem(NewElem, first); }
	void AddOr(Ipn *ipn, int LineNum);
	void AddAnd(Ipn *ipn, int LineNum);
	void AddComp(Ipn *ipn, int oper, int LineNum);
	void AddAddSub(Ipn *ipn, int oper, int LineNum);
	void AddMulDiv(Ipn *ipn, int oper, int LineNum);
	void AddNeg(Ipn *ipn, int LineNum);
	void MetRParen(Ipn *ipn, int LineNum);
	void AddIpn(Ipn *ipn, int AddValue, int LineNum);
	int Get();
	~IntStack();
};

void IntStack::AddOr(Ipn *ipn, int LineNum)
{
	if (first->value == LexLParen) {
		Add(LexOr);
	} else {
		AddIpn(ipn, Get(), LineNum);
		AddOr(ipn, LineNum);
	}
};

void IntStack::AddAnd(Ipn *ipn, int LineNum)
{
	int v = first->value;
	if (v==LexLParen || v==LexOr) {
		Add(LexAnd);
	} else {
		AddIpn(ipn, Get(), LineNum);
		AddAnd(ipn, LineNum);
	}
};

void IntStack::AddComp(Ipn *ipn, int oper, int LineNum)
{
	int v = first->value;
	if (v==LexLParen || v==LexOr || v==LexAnd) {
		Add(oper);
	} else {
		AddIpn(ipn, Get(), LineNum);
		AddComp(ipn, oper, LineNum);
	}
}

void IntStack::AddAddSub(Ipn *ipn, int oper, int LineNum)
{
	int v = first->value;
	if (v==LexAdd || v==LexSub || v==LexMul || v==LexDiv || v==LexNeg) {
		AddIpn(ipn, Get(), LineNum);
		AddAddSub(ipn, oper, LineNum);
	} else {
		Add(oper);
	}
}

void IntStack::AddMulDiv(Ipn *ipn, int oper, int LineNum)
{
	int v = first->value;
	if (v==LexMul || v==LexDiv || v==LexNeg) {
		AddIpn(ipn, Get(), LineNum);
		AddMulDiv(ipn, oper, LineNum);
	} else {
		Add(oper);
	}
}

void IntStack::AddNeg(Ipn *ipn, int LineNum)
{
	if (first->value == LexNeg) {
		AddIpn(ipn, Get(), LineNum);
		AddNeg(ipn, LineNum);
	} else {
		Add(LexNeg);
	}
}

void IntStack::MetRParen(Ipn *ipn, int LineNum)
{
	int oper = Get();
	while(oper != LexLParen && oper != 0) {
		AddIpn(ipn, oper, LineNum);
		oper = Get();
	}
}

void IntStack::AddIpn(Ipn *ipn, int AddValue, int LineNum)
{
	if (AddValue == LexOr) {
		ipn->Add(new IpnOr(LineNum));
	} else if (AddValue == LexAnd) {
		ipn->Add(new IpnAnd(LineNum));
	} else if (AddValue == LexLT) {
		ipn->Add(new IpnLess(LineNum));
	} else if (AddValue == LexLE) {
		ipn->Add(new IpnLessE(LineNum));
	} else if (AddValue == LexGT) {
		ipn->Add(new IpnGreat(LineNum));
	} else if (AddValue == LexGE) {
		ipn->Add(new IpnGreatE(LineNum));
	} else if (AddValue == LexEq) {
		ipn->Add(new IpnEqual(LineNum));
	} else if (AddValue == LexNotEq) {
		ipn->Add(new IpnNotEqual(LineNum));
	} else if (AddValue == LexNeg) {
		ipn->Add(new IpnNeg(LineNum));
	} else if (AddValue == LexAdd) {
		ipn->Add(new IpnAdd(LineNum));
	} else if (AddValue == LexSub) {
		ipn->Add(new IpnSub(LineNum));
	} else if (AddValue == LexMul) {
		ipn->Add(new IpnMul(LineNum));
	} else if (AddValue == LexDiv) {
		ipn->Add(new IpnDiv(LineNum));
	}
}

int IntStack::Get()
{
	if (first) {
		int value = first->value;
		IntItem *help = first;
		first = first->next;
		delete help;
		return value;
	}
	return 0;
}

IntStack::~IntStack()
{
	while(first != 0) {
		IntItem *help = first;
		first = first->next;
		delete help;
	}
}

class BadLex {
	int ErrNum;
public:
	BadLex(int e): ErrNum(e) {}
	int GetErrNum() const { return ErrNum; }
};

class Parser {
	Automat automat;
	Ipn *ipn;
	Lexeme *current, *last;
	int LexNum, LineNum;
	FILE *f;
	void next();
	void S();
	void Action();
	void IntDesc();
	void RealDesc();
	void StringDesc();
	void WhileDesc();
	void GetDesc();
	void PutDesc();
	void AssignDesc();
	void IfElseDesc();
	void ArrayDesc(int DefVal, int RFst, int RScd, int ValFst, int ValScd);
	void ExpOr(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpAnd(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpComp(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpAddSub(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpMulDiv(IntStack *stack, int RParenErr, int ValOrIdentErr);
	void ExpLast(IntStack *stack, int RParenErr, int ValOrIdentErr);
	int CheckComp(int n);
	int CheckInExp(int ErrNum);
public:
	Parser()
		: automat(), current(0), last(0), LexNum(0), f(0) { ipn = new Ipn; }
	Ipn *analyze(FILE *file);
};

Ipn *Parser::analyze(FILE *file)
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
			return 0;
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
		return 0;
	}
	return ipn;
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
	LexNum = current->GetLexNum();
	LineNum = current->GetLineNum();
}

void Parser::S()
{
	if (LexNum != LexBirth)
		throw BadLex(BirthExpect);
	ipn->Add(new IpnBraceL(current->GetLineNum()));
	next();
	Action();
	if (LexNum != LexDeath)
		throw BadLex(DeathExpect);
	ipn->Add(new IpnBraceR(current->GetLineNum()));
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
		IntDesc();
	} else if (LexNum == LexReal) {
		next();
		RealDesc();
	} else if (LexNum == LexString) {
		next();
		StringDesc();
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
		AssignDesc();
	} else if (LexNum == LexIf) {
		next();
		IfElseDesc();
	} else {
		return;
	}
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

void Parser::ArrayDesc(int DefVal, int RFst, int RScd, int ValFst, int ValScd)
{
	if (LexNum == LexLSB) {
		next();
		ExpOr(0, RFst, ValFst);
		if (LexNum == LexComma) {
			next();
			ExpOr(0, RScd, ValScd);
		} else {
			ipn->Add(new IpnInt(DefVal, current->GetLineNum()));
		}
		if (LexNum != LexRSB)
			throw BadLex(RSBExpect);
		next();
	} else {
		ipn->Add(new IpnInt(DefVal, current->GetLineNum()));
		ipn->Add(new IpnInt(DefVal, current->GetLineNum()));
	}
}
		
void Parser::IntDesc()
{
	if (LexNum != LexIdent)
		throw BadLex(IdentAfterInt);  
	char *name = strdup(current->GetName());
	int line = current->GetLineNum();
	next();
	ArrayDesc(1, RpArrFstInt, ValArrFstInt, RpArrScdInt, ValArrScdInt);
	if (LexNum != LexLParen)
		throw BadLex(LParenInt);
	next();
	ExpOr(0, RParenInitValIntExp, ValOrIdentInitValIntExp);
	if (LexNum != LexRParen)
		throw BadLex(RParenInt);
	next();
	ipn->Add(new IpnVarDesc(name, Int, line));
	delete [] name;
	if (LexNum == LexComma) {
		next();
		IntDesc();
	}
}

void Parser::RealDesc()
{
	if (LexNum != LexIdent)
		throw BadLex(IdentAfterReal);
	char *name = strdup(current->GetName());
	int line = current->GetLineNum();
	next();
	ArrayDesc(1, RpArrFstReal, ValArrFstReal, RpArrScdReal, ValArrScdReal);
	if (LexNum != LexLParen)
		throw BadLex(LParenReal);
	next();
	ExpOr(0, RParenInitValRealExp, ValOrIdentInitValRealExp);
	if (LexNum != LexRParen)
		throw BadLex(RParenReal);
	next();
	ipn->Add(new IpnVarDesc(name, Real, line));
	delete [] name;
	if (LexNum == LexComma) {
		next();
		RealDesc();
	}
}

void Parser::StringDesc()
{
	if (LexNum != LexIdent)
		throw BadLex(IdentAfterStr);
	char *name = strdup(current->GetName());
	int line = current->GetLineNum();
	next();
	ipn->Add(new IpnInt(1, line), new IpnInt(1, line));
	if (LexNum != LexLParen)
		throw BadLex(LParenStr);
	next();
	if (LexNum != LexStr)
		throw BadLex(StrExpect);
	ipn->Add(new IpnString(current->GetString(), LineNum));
	next();
	if (LexNum != LexRParen)
		throw BadLex(RParenStr);
	next();
	ipn->Add(new IpnVarDesc(name, String, line));
	delete [] name;
	if (LexNum == LexComma) {
		next();
		StringDesc();
	}
}

void Parser::WhileDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenWhile);
	ipn->Add(new IpnBraceL(LineNum));
	next();
	ipn->Add(new IpnNoOp(LineNum));
	IpnLabel *label = new IpnLabel(ipn->Get(), LineNum);
	ExpOr(0, RParenWhileExp, ValOrIdentWhileExp);
	ipn->AddNoShift(new IpnNoOp(LineNum));
	ipn->Add(new IpnLabel(ipn->GetLast(), LineNum));
	ipn->Add(new IpnOpGoFalse(LineNum));
	ipn->AddNoShift(label, new IpnOpGo(LineNum));
	if (LexNum != LexRParen)
		throw BadLex(RParenWhile);
	next();
	if (LexNum != LexLB)
		throw BadLex(LBWhile);
	next();
	Action();
	ipn->Shift(3);
	if (LexNum != LexRB)
		throw BadLex(RBWhile);
	ipn->Add(new IpnBraceR(LineNum));
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
//	ArrayDesc();
	while(LexNum == LexComma) {
		next();
		if (LexNum != LexIdent)
			throw BadLex(IdentGetComma);
		next();
//		ArrayDesc();
	}
	if (LexNum != LexRParen)
		throw BadLex(RParenGet);
	next();
}

void Parser::PutDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenPut);
	ipn->Add(new IpnEndOfArg(LineNum));
	next();
	ExpOr(0, RParenPutExp, ValOrIdentPutExp);
	while(LexNum == LexComma) {
		next();
		ExpOr(0, RParenPutExp, ValOrIdentPutExp);
	}
	if (LexNum != LexRParen)
		throw BadLex(RParenPut);
	ipn->Add(new IpnPut(LineNum));
	next();
}

void Parser::AssignDesc()
{
	int AssignNum = 1, line = last->GetLineNum();
	char *name = strdup(last->GetName());
	ArrayDesc(0, RpArrFstAsgn, ValArrFstAsgn, RpArrScdAsgn, ValArrScdAsgn);
	ipn->Add(new IpnVarAddr(name, line));
	delete [] name;
	while(LexNum == LexMultiAssign) {
		next();
		if (LexNum != LexIdent)
			throw BadLex(IdentExpectAssign);
		AssignNum++;
		char *name = strdup(current->GetName());
		line = LineNum;
		next();
		ArrayDesc(0, RpArrFstAsgn, ValArrFstAsgn, RpArrScdAsgn, ValArrScdAsgn);
		ipn->Add(new IpnVarAddr(name, line));
		delete [] name;
	}
	if (LexNum != LexAssign)
		throw BadLex(AssignExpect);
	line = LineNum;
	next();
	ExpOr(0, RParenAssignExp, ValOrIdentAssignExp);
	for(int i = 0; i < AssignNum; i++)
		ipn->Add(new IpnAssign(line));
	if (LexNum != LexSemicolon)
		throw BadLex(SemicolonExpect);
	ipn->Add(new IpnSemicolon(LineNum));
	next();
}

void Parser::IfElseDesc()
{
	if (LexNum != LexLParen)
		throw BadLex(LParenIf);
	ipn->Add(new IpnBraceL(LineNum));
	next();
	ExpOr(0, RParenIfExp, ValOrIdentIfExp);
	ipn->AddNoShift(new IpnNoOp(LineNum));
	ipn->Add(new IpnLabel(ipn->GetLast(), LineNum));
	ipn->Add(new IpnOpGoFalse(LineNum));
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
		ipn->AddInEnd(new IpnNoOp(LineNum));
		ipn->Add(new IpnLabel(ipn->GetLastLast(), LineNum));
		ipn->Add(new IpnOpGoFalse(LineNum));
		ipn->Shift();
		if (LexNum != LexLB)
			throw BadLex(LBElseExpect);
		next();
		Action();
		if (LexNum != LexRB)
			throw BadLex(RBElseExpect);
		next();
	}
	ipn->Shift();
	ipn->Add(new IpnBraceR(last->GetLineNum()));
}	

void Parser::ExpOr(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	int Declared = 0;
	if (stack == 0) {
		stack = new IntStack(LexLParen);
		Declared = 1;
	}
	ExpAnd(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexOr) {
		stack->AddOr(ipn, LineNum);
		next();
		ExpAnd(stack, RParenErr, ValOrIdentErr);
	}
	if (Declared) {
		stack->MetRParen(ipn, LineNum);
		delete stack;
	}
}

void Parser::ExpAnd(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	ExpComp(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexAnd) {
		stack->AddAnd(ipn, LineNum);
		next();
		ExpComp(stack, RParenErr, ValOrIdentErr);
	}
}

int Parser::CheckComp(int n)
{
	return n==LexLT||n==LexLE||n==LexGT||n==LexGE||n==LexEq||n==LexNotEq;
}

void Parser::ExpComp(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	ExpAddSub(stack, RParenErr, ValOrIdentErr);
	while(CheckComp(LexNum)) {
		stack->AddComp(ipn, LexNum, LineNum);
		next();
		ExpAddSub(stack, RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpAddSub(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	if (LexNum == LexAdd || LexNum == LexSub) {
		ipn->Add(new IpnInt(0, LineNum));
		stack->AddAddSub(ipn, LexNum, LineNum);
		next();
	}
	ExpMulDiv(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexAdd || LexNum == LexSub) {
		stack->AddAddSub(ipn, LexNum, LineNum);
		next();
		ExpMulDiv(stack, RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpMulDiv(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	ExpLast(stack, RParenErr, ValOrIdentErr);
	while(LexNum == LexMul || LexNum == LexDiv) {
		stack->AddMulDiv(ipn, LexNum, LineNum);
		next();
		ExpLast(stack, RParenErr, ValOrIdentErr);
	}
}

void Parser::ExpLast(IntStack *stack, int RParenErr, int ValOrIdentErr)
{
	if (LexNum == LexIdent) {
		char *name = strdup(current->GetName());
		int line = current->GetLineNum();
		next();
		ArrayDesc(0, RpArrFstExp, ValArrFstExp, RpArrScdExp, ValArrScdExp);
		ipn->Add(new IpnVar(name, line));
		delete [] name;
	} else if (LexNum == LexValInt) {
		ipn->Add(new IpnInt(current->GetInt(), LineNum));
		next();
	} else if (LexNum == LexValReal) {
		ipn->Add(new IpnReal(current->GetReal(), LineNum));
		next();
	} else if (LexNum == LexStr) {
		ipn->Add(new IpnString(current->GetString(), LineNum));
		next();
	} else if (LexNum == LexNeg) {
		stack->AddNeg(ipn, LineNum);
		next();
		ExpLast(stack, RParenErr, ValOrIdentErr);
	} else if (LexNum == LexLParen) {
		stack->Add(LexLParen);
		next();
		ExpOr(stack, RParenErr, ValOrIdentErr);
		if (LexNum != LexRParen)
			throw BadLex(RParenErr);
		stack->MetRParen(ipn, LineNum);
		next();
	} else {
		throw BadLex(ValOrIdentErr);
	}
}

int main(int argc, char **argv)
{
	Parser parser;
	Ipn *ipn;
	FILE *f;
	if (argc < 2)
		return 0;
	if (!(f = fopen(argv[1], "r")))
		return 0;
	ipn = parser.analyze(f);
	if (ipn) {
		ipn->Perform();
		delete ipn;
	}
	return 0;
}
