/*
 * bbc_tokens.h - BBC BASIC token definitions and keyword table
 *
 * Extracted from BBCSDL: BBC.h and bbmain.c (R.T. Russell)
 * Adapted for standalone tokenizer/detokenizer.
 */

#ifndef BBC_TOKENS_H
#define BBC_TOKENS_H

/* ── Token values (signed char: -128..+16) ─────────────────────────── */

#define TAND      (-128)
#define TDIV      (-127)
#define TEOR      (-126)
#define TMOD      (-125)
#define TOR       (-124)
#define TERROR    (-123)
#define TLINE     (-122)
#define TOFF      (-121)
#define TSTEP     (-120)
#define TSPC      (-119)
#define TTAB      (-118)
#define TELSE     (-117)
#define TTHEN     (-116)
#define TLINO     (-115)
#define TOPENIN   (-114)
#define TPTRR     (-113)
#define TPAGER    (-112)
#define TTIMER    (-111)
#define TLOMEMR   (-110)
#define THIMEMR   (-109)
#define TABS      (-108)
#define TACS      (-107)
#define TADVAL    (-106)
#define TASC      (-105)
#define TASN      (-104)
#define TATN      (-103)
#define TBGET     (-102)
#define TCOS      (-101)
#define TCOUNT    (-100)
#define TDEG       (-99)
#define TERL       (-98)
#define TERR       (-97)
#define TEVAL      (-96)
#define TEXP       (-95)
#define TEXTR      (-94)
#define TFALSE     (-93)
#define TFN        (-92)
#define TGET       (-91)
#define TINKEY     (-90)
#define TINSTR     (-89)
#define TINT       (-88)
#define TLEN       (-87)
#define TLN        (-86)
#define TLOG       (-85)
#define TNOT       (-84)
#define TOPENUP    (-83)
#define TOPENOUT   (-82)
#define TPI        (-81)
#define TPOINT     (-80)
#define TPOS       (-79)
#define TRAD       (-78)
#define TRND       (-77)
#define TSGN       (-76)
#define TSIN       (-75)
#define TSQR       (-74)
#define TTAN       (-73)
#define TTO        (-72)
#define TTRUE      (-71)
#define TUSR       (-70)
#define TVAL       (-69)
#define TVPOS      (-68)
#define TCHR       (-67)
#define TGETS      (-66)
#define TINKEYS    (-65)
#define TLEFT      (-64)
#define TMID       (-63)
#define TRIGHT     (-62)
#define TSTR       (-61)
#define TSTRING    (-60)
#define TEOF       (-59)
#define TSUM       (-58)
#define TWHILE     (-57)
#define TCASE      (-56)
#define TWHEN      (-55)
#define TOF        (-54)
#define TENDCASE   (-53)
#define TOTHERWISE (-52)
#define TENDIF     (-51)
#define TENDWHILE  (-50)
#define TPTRL      (-49)
#define TPAGEL     (-48)
#define TTIMEL     (-47)
#define TLOMEML    (-46)
#define THIMEML    (-45)
#define TSOUND     (-44)
#define TBPUT      (-43)
#define TCALL      (-42)
#define TCHAIN     (-41)
#define TCLEAR     (-40)
#define TCLOSE     (-39)
#define TCLG       (-38)
#define TCLS       (-37)
#define TDATA      (-36)
#define TDEF       (-35)
#define TDIM       (-34)
#define TDRAW      (-33)
#define TEND       (-32)
#define TENDPROC   (-31)
#define TENVEL     (-30)
#define TFOR       (-29)
#define TGOSUB     (-28)
#define TGOTO      (-27)
#define TGCOL      (-26)
#define TIF        (-25)
#define TINPUT     (-24)
#define TLET       (-23)
#define TLOCAL     (-22)
#define TMODE      (-21)
#define TMOVE      (-20)
#define TNEXT      (-19)
#define TON        (-18)
#define TVDU       (-17)
#define TPLOT      (-16)
#define TPRINT     (-15)
#define TPROC      (-14)
#define TREAD      (-13)
#define TREM       (-12)
#define TREPEAT    (-11)
#define TREPORT    (-10)
#define TRESTOR     (-9)
#define TRETURN     (-8)
#define TRUN        (-7)
#define TSTOP       (-6)
#define TCOLOUR     (-5)
#define TTRACE      (-4)
#define TUNTIL      (-3)
#define TWIDTH      (-2)
#define TOSCLI      (-1)

/* Positive (shifted) tokens: preceded by TELSE in token stream */
#define TCIRCLE      1
#define TELLIPSE     2
#define TFILL        3
#define TMOUSE       4
#define TORIGIN      5
#define TQUIT        6
#define TRECT        7
#define TSWAP        8
#define TSYS         9
#define TTINT       10
#define TWAIT       11
#define TINSTALL    12
#define TPRIVATE    14
#define TBY         15
#define TEXIT       16

/* Ranges */
#define FUNTOK  TLINO         /* first function token */
#define TOKLO   TPTRR         /* -113 */
#define TOKHI   THIMEMR       /* -109 */
#define OFFSIT  (TPTRL-TPTRR) /* 64: offset for l-value variants */

/* Bit names */
#define BIT0    0x01
#define BIT1    0x02
#define BIT2    0x04
#define BIT3    0x08
#define BIT4    0x10
#define BIT5    0x20
#define BIT6    0x40
#define BIT7    0x80

/* ── Keyword table ─────────────────────────────────────────────────── */
/* Format: token_byte, chars..., next_token_byte (acts as terminator)
 * Table is sorted alphabetically. Last entry terminated by 0x00, 0x7F. */

static const signed char keywds[] = {
	TAND,'A','N','D',
	TABS,'A','B','S',
	TACS,'A','C','S',
	TADVAL,'A','D','V','A','L',
	TASC,'A','S','C',
	TASN,'A','S','N',
	TATN,'A','T','N',
	TBGET,'B','G','E','T',' ',
	TBPUT,'B','P','U','T',' ',
	TBY,'B','Y',' ',
	TCOLOUR,'C','O','L','O','U','R',
	TCOLOUR,'C','O','L','O','R',
	TCALL,'C','A','L','L',
	TCASE,'C','A','S','E',
	TCHAIN,'C','H','A','I','N',
	TCHR,'C','H','R','$',
	TCLEAR,'C','L','E','A','R',' ',
	TCLOSE,'C','L','O','S','E',' ',
	TCLG,'C','L','G',' ',
	TCLS,'C','L','S',' ',
	TCOS,'C','O','S',
	TCOUNT,'C','O','U','N','T',' ',
	TCIRCLE,'C','I','R','C','L','E',
	TDATA,'D','A','T','A',
	TDEG,'D','E','G',
	TDEF,'D','E','F',
	TDIV,'D','I','V',
	TDIM,'D','I','M',
	TDRAW,'D','R','A','W',
	TENDPROC,'E','N','D','P','R','O','C',' ',
	TENDWHILE,'E','N','D','W','H','I','L','E',' ',
	TENDCASE,'E','N','D','C','A','S','E',' ',
	TENDIF,'E','N','D','I','F',' ',
	TEND,'E','N','D',' ',
	TENVEL,'E','N','V','E','L','O','P','E',
	TELSE,'E','L','S','E',
	TEVAL,'E','V','A','L',
	TERL,'E','R','L',' ',
	TERROR,'E','R','R','O','R',
	TEOF,'E','O','F',' ',
	TEOR,'E','O','R',
	TERR,'E','R','R',' ',
	TEXIT,'E','X','I','T',' ',
	TEXP,'E','X','P',
	TEXTR,'E','X','T',' ',
	TELLIPSE,'E','L','L','I','P','S','E',
	TFOR,'F','O','R',
	TFALSE,'F','A','L','S','E',' ',
	TFILL,'F','I','L','L',
	TFN,'F','N',
	TGOTO,'G','O','T','O',
	TGETS,'G','E','T','$',
	TGET,'G','E','T',
	TGOSUB,'G','O','S','U','B',
	TGCOL,'G','C','O','L',
	THIMEMR,'H','I','M','E','M',' ',
	TINPUT,'I','N','P','U','T',
	TIF,'I','F',
	TINKEYS,'I','N','K','E','Y','$',
	TINKEY,'I','N','K','E','Y',
	TINT,'I','N','T',
	TINSTR,'I','N','S','T','R','(',
	TINSTALL,'I','N','S','T','A','L','L',
	TLINE,'L','I','N','E',
	TLOMEMR,'L','O','M','E','M',' ',
	TLOCAL,'L','O','C','A','L',
	TLEFT,'L','E','F','T','$','(',
	TLEN,'L','E','N',
	TLET,'L','E','T',
	TLOG,'L','O','G',
	TLN,'L','N',
	TMID,'M','I','D','$','(',
	TMODE,'M','O','D','E',
	TMOD,'M','O','D',
	TMOVE,'M','O','V','E',
	TMOUSE,'M','O','U','S','E',
	TNEXT,'N','E','X','T',
	TNOT,'N','O','T',
	TON,'O','N',
	TOFF,'O','F','F',' ',
	TOF,'O','F',' ',
	TORIGIN,'O','R','I','G','I','N',
	TOR,'O','R',
	TOPENIN,'O','P','E','N','I','N',
	TOPENOUT,'O','P','E','N','O','U','T',
	TOPENUP,'O','P','E','N','U','P',
	TOSCLI,'O','S','C','L','I',
	TOTHERWISE,'O','T','H','E','R','W','I','S','E',
	TPRINT,'P','R','I','N','T',
	TPAGER,'P','A','G','E',' ',
	TPRIVATE,'P','R','I','V','A','T','E',
	TPTRR,'P','T','R',' ',
	TPI,'P','I',' ',
	TPLOT,'P','L','O','T',
	TPOINT,'P','O','I','N','T','(',
	TPROC,'P','R','O','C',
	TPOS,'P','O','S',' ',
	TQUIT,'Q','U','I','T',' ',
	TRETURN,'R','E','T','U','R','N',' ',
	TREPEAT,'R','E','P','E','A','T',
	TREPORT,'R','E','P','O','R','T',' ',
	TREAD,'R','E','A','D',
	TREM,'R','E','M',
	TRUN,'R','U','N',' ',
	TRAD,'R','A','D',
	TRESTOR,'R','E','S','T','O','R','E',
	TRIGHT,'R','I','G','H','T','$','(',
	TRND,'R','N','D',' ',
	TRECT,'R','E','C','T','A','N','G','L','E',
	TSTEP,'S','T','E','P',
	TSGN,'S','G','N',
	TSIN,'S','I','N',
	TSQR,'S','Q','R',
	TSPC,'S','P','C',
	TSTR,'S','T','R','$',
	TSTRING,'S','T','R','I','N','G','$','(',
	TSOUND,'S','O','U','N','D',
	TSTOP,'S','T','O','P',' ',
	TSUM,'S','U','M',
	TSWAP,'S','W','A','P',
	TSYS,'S','Y','S',
	TTAN,'T','A','N',
	TTAB,'T','A','B','(',
	TTHEN,'T','H','E','N',
	TTIMER,'T','I','M','E',' ',
	TTINT,'T','I','N','T',
	TTO,'T','O',
	TTRACE,'T','R','A','C','E',
	TTRUE,'T','R','U','E',' ',
	TUNTIL,'U','N','T','I','L',
	TUSR,'U','S','R',
	TVDU,'V','D','U',
	TVAL,'V','A','L',
	TVPOS,'V','P','O','S',' ',
	TWHILE,'W','H','I','L','E',
	TWHEN,'W','H','E','N',
	TWAIT,'W','A','I','T',' ',
	TWIDTH,'W','I','D','T','H',
	/* L-value variants (duplicates for left-mode matching) */
	THIMEML,'H','I','M','E','M',' ',
	TLOMEML,'L','O','M','E','M',' ',
	TPAGEL,'P','A','G','E',' ',
	TPTRL,'P','T','R',' ',
	TTIMEL,'T','I','M','E',' ',
	0x00,0x7F
};

/* Tokens that can be followed by an encoded line number */
static const signed char list1[] = {
	TGOTO, TGOSUB, TRESTOR, TTRACE, TTHEN, TELSE, 0
};

/* Tokens that switch the lexical analyser to 'left' mode */
static const signed char list2[] = {
	TTHEN, TELSE, TREPEAT, TERROR, TCLOSE, TMOUSE, TMOVE, TSYS, ':', 0
};

/* ── Helper functions ──────────────────────────────────────────────── */

/* Test a character for valid in, or terminating, a variable name */
static __attribute__((unused)) int range0(char c)
{
	return (((c >= '_') && (c <= '{')) ||
	        ((c >= '@') && (c <= 'Z')) ||
	        ((c >= '0') && (c <= '9')) ||
	        ((c >= '#') && (c <= '&')) ||
	         (c == '(') || (c == '.'));
}

/* Test a character for valid in a variable name */
static __attribute__((unused)) int range1(char c)
{
	return (((c >= '_') && (c <= 'z')) ||
	        ((c >= '@') && (c <= 'Z')) ||
	        ((c >= '0') && (c <= '9')));
}

/* Test a character for valid as the first character of a variable name */
static __attribute__((unused)) int range2(char c)
{
	return (((c >= '_') && (c <= 'z')) ||
	        ((c >= '@') && (c <= 'Z')));
}

#endif /* BBC_TOKENS_H */
