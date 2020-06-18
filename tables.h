enum {
	LexEmpty, LexIdent, LexValInt, LexValReal, LexStr, LexBirth, LexDeath,
	LexDie, LexInt, LexReal, LexString, 
	LexIf, LexElse, LexWhile, LexPut, LexGet, LexComma, LexSemicolon,
	LexAssign, LexMultiAssign,
	LexLB, LexRB, LexLSB, LexRSB, LexLParen, LexRParen,
	LexOr, LexAnd, LexLT, LexLE, LexGT, LexGE, LexEq, LexNotEq, LexAdd, LexSub,
	LexMul, LexDiv, LexNeg,	LexError
};

const char *TableOfWords[] = 
{
	"", "", "", "", "", "birth", "death", "die", "int", "real", "string",
	"if", "else", "while", "put", "get", ",", ";", "=", ":=",
	"{", "}", "[", "]", "(", ")",
	"or", "and", "<", "<=", ">", ">=", "==", "=!", "+", "-", "*", "/", "!",
	NULL
};

enum {
	InvalLex, BirthExpect, DeathExpect, NothingExpect, SemicolonExpect,
	IdentAfterInt,
	RpArrFstInt, ValArrFstInt, RpArrScdInt, ValArrScdInt,
	RParenInitValIntExp, ValOrIdentInitValIntExp,
	LParenInt, ValIntOrIdent, RParenInt,
	IdentAfterReal,
	RpArrFstReal, ValArrFstReal, RpArrScdReal, ValArrScdReal,
	RParenInitValRealExp, ValOrIdentInitValRealExp,
	LParenReal, ValOrIdentReal, RParenReal,
	IdentAfterStr,	
	IdentAfterCommaStr, LParenStr, StrExpect, 
	RParenStr, ValIntOrIdentArray,
	ValIntOrIdentArraySec, RSBExpect, LParenWhile, RParenWhileExp, 
	ValOrIdentWhileExp, RParenWhile, LBWhile, RBWhile, LParenGet, IdentGet,
	IdentGetComma, RParenGet, LParenPut, RParenPutExp, ValOrIdentPutExp,
	RParenPut,
	AssignExpect, IdentExpectAssign, RParenAssignExp, ValOrIdentAssignExp,
	LParenIf,
	RParenIfExp, ValOrIdentIfExp, RParenIf, LBIfExpect, RBIfExpect,
	LBElseExpect, RBElseExpect,
	RpArrFstExp, ValArrFstExp, RpArrScdExp, ValArrScdExp,
	RpArrFstAsgn, ValArrFstAsgn, RpArrScdAsgn, ValArrScdAsgn
};

const char *TableOfErrors[] = 
{
	"",
	"\"birth\" expected in the beginning of the program",
	"\"death\" expected in the end of the program",
	"lexeme after end of the program",
	"\";\" expected in the end of statement",
	"identifier expected in description of integer",
	"\")\" expected in the condition of first dimension of array "
	"in description of integer",
	"identifier or int or real value expected in the condition of "
	"first dimension of array in description of integer",
	"\")\" expected in the condition of second dimension of array "
	"in description of integer",
	"identifier or int or real value expected in the condition of "
	"second dimension of array in description of integer",
	"\")\" expected in the condition of initial value "
	"in description of integer",
	"identifier or int or real value expected in the condition of "
	"initial value in description of integer",
	"all variables must be initialized: "
	"\"(\" expected after identifier in description of integer",
	"identifier or int value expected after \"(\" in description of integer",
	"\")\" expected after initial value in description of integer",
	"identifier expected in description of real",
	"\")\" expected in the condition of first dimension of array "
	"in description of real",
	"identifier or int or real value expected in the condition of "
	"first dimension of array in description of real",
	"\")\" expected in the condition of second dimension of array "
	"in description of real",
	"identifier or int or real value expected in the condition of "
	"second dimension of array in description of real",
	"\")\" expected in the condition of initial value "
	"in description of real",
	"identifier or int or real value expected in the condition of "
	"initial value in description of real",
	"all variables must be initialized: "
	"\"(\" expected after identifier in description of real",
	"identifier or real or int value expected after \"(\" "
	"in description of real",
	"\")\" expected after initial value in description of real",
	"identifier expected after \"string\"",
	"identifier expected after \",\" in description of string",
	"all variables must be initialized: "
	"\"(\" expected after identifier in description of string",
	"string expected after \"(\" in description of string",
	"\")\" expected after initial value in description of string",
	"identifier or int value expected after \"[\" in description of array",
	"identifier or int value expected after \",\" in description of array",
	"\"]\" expected after the dimension in the end of description of array",
	"\"(\" expected after \"while\"",
	"\")\" expected in the condition of \"while\"",
	"identifier or int or real value expected in the condition of \"while\"",
	"\")\" expected after the condition of \"while\"",
	"\"{\" expected after \")\" in the beginning of \"while\" body",
	"\"}\" expected in the end of \"while\" body",
	"\"(\" expected after \"get\"",
	"identifier expected after \"(\" as the argument of \"get\"",
	"identifier expected after \",\" as the argument of \"get\"",
	"\")\" expected after the argument of \"get\"",
	"\"(\" expected after \"put\"",
	"\")\" expected in the argument of \"put\"",
	"identifier or int or real value expected in the argument of \"put\"",
	"\")\" expected after the argument of \"put\"",
	"\"=\" expected after identifier in the assignment",
	"identifier expected after \":=\" in the assignment",
	"\")\" expected in the assignment",
	"identifier or int or real value expected in the assignment",
	"\"(\" expected after \"if\"",
	"\")\" expected in the condition of \"if\"",
	"identifier or int or real value expected in the condition of \"if\"",
	"\")\" expected after the condition of \"if\"",
	"\"{\" expected after \")\" in the beginning of \"if\" body",
	"\"}\" expected in the end of \"if\" body",
	"\"{\" expected after \"else\" in the beginning of \"else\" body",
	"\"}\" expected in the end of \"else\" body",
	"\")\" expected in the condition of first dimension of array "
	"in description of expression",
	"identifier or int or real value expected in the condition of "
	"first dimension of array in description of expression",
	"\")\" expected in the condition of second dimension of array "
	"in description of expression",
	"identifier or int or real value expected in the condition of "
	"second dimension of array in description of expression"
	"\")\" expected in the condition of first dimension of array "
	"in description of assignment",
	"identifier or int or real value expected in the condition of "
	"first dimension of array in description of assignment",
	"\")\" expected in the condition of second dimension of array "
	"in description of assignment",
	"identifier or int or real value expected in the condition of "
	"second dimension of array in description of assignment"
};
