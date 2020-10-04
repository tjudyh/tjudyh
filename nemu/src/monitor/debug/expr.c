#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ,UEQ,LOGICAL_AND,LOGICAL_OR,LOGICAL_NOT,Register, Number,Hex,Pointor,Minus

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
	int priority;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE,0},				// spaces
	{"\\+", '+',4},					// plus
	{"==", EQ,3},					// equal
	{"\\(",'(',7},
	{"\\)",')',7},
	{"-",'-',4},
	{"/",'/',5},
	{"\\*",'*',5},
	{"!=",UEQ,6},
	{"&&",LOGICAL_AND,2},
	{"\\|\\|",LOGICAL_OR,1},
	{"!",LOGICAL_NOT,6},
	{"\\$[a-d][hl]|\\$e?(ax|dx|cx|bx|bp|si|di|sp)",Register,0},
	{"[A-Za-z][A-Za-z0-9_]*",Variable,0},
	{"[0-9]{1,10}",Number,0},
	{"0x[a-f0-9]{1,8}",Hex,0},				
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[1000];
	int priority;
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array `tokens'. For certain types
				 * of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
				case 257:
					tokens[nr_token].type=257;
					tokens[nr_token].str="==";
					tokens[nr_token].priority=3;
					break;
				case 43:
					tokens[nr_token].type=43;
					tokens[nr_token].priority=4;
					break;
				case 45:
					tokens[nr_token].type=45;
					tokens[nr_token].priority=4;
					break;
				case 47:
					tokens[nr_token].type=47;
					tokens[nr_token].priority=5;
					break;				
				case 42:
					tokens[nr_token].type=42;
					tokens[nr_token].priority=5;
					break;
				case 40:
					tokens[nr_token].type=40;
					tokens[nr_token].priority=7;
					break;
				case 41:
					tokens[nr_token].type=41;
					tokens[nr_token].priority=7;
					break;
				case 258:
					tokens[nr_token].type=258;
					tokens[nr_token].str="!=";
					tokens[nr_token].priority=6;
					break;
				case 259:
					tokens[nr_token].type=259;
					tokens[nr_token].str="&&";
					tokens[nr_token].priority=2;
					break;
				case 260:
					tokens[nr_token].type=260;
					tokens[nr_token].str="||";
					tokens[nr_token].priority=1;
					break;
				case 261:
					toekns[nr_token].type=261;
					tokens[nr_token].priority=6;
					break;
				case 262:
					tokens[nr_token].type=262;
					strncpy(tokens[nr_token].str,&e[position-substr_len],substr_len);
					tokens[nr_token].priority=0;
					break;
				case 263:
					tokens[nr_token].type=263;
					strncpy(tokens[nr_token],&e[position-substr_len],substr_len);
					tokens[nr_token].priority=0;
					break;
				case 264:
					tokens[nr_token].type=264;
					strncpy(tokens[nr_token],&e[position-substr_len],substr_len);
					tokens[nr_token].priority=0;
					break;
				case 265:
					tokens[nr_token].type=265;
					strncpy(tokens[nr_token],&e[position-substr_len],substr_len);
					tokens[nr_token].priority=0;
					break;
										
					default: 
						nr_token--;
						break;
				}
				nr_token++;
				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}
	nr_token--;
	return true; 
}

bool check_parentheses(int p,int q){
	int i;int flag=0;
	if(tokens[p].type!=40||tokens[q].type!=41) 
		return false;
	for(i=p;i<=q;i++){
		if(tokens[i].type==40) flag++;
		else if(token[i].type==41) flag--;
		if(flag==0&&i<q) return false;
	}
	if(flag!=0) return false;
	return true;
}

int dominant_operator(int p,int q){
	int i;int dominant=p;int L_coun=0;
	for(i=p;i<=q;i++){
		if(tokens[i].type==40){
			L_coun++;i++;
			while(true){
				if(tokens[i].type==40) L_coun++;
				else if(tokens[i].type==41) L_coun--;
				i++;
				if(L_coun==0) break;
			}
			if(i>q) break;
		}
		else if(tokens[i].type==264||tokens[i].type==262||tokens[i].type==263) continue;
		else if(tokens[i].priority>tokens[dominant].priority) dominant=i;
	}
	return dominant;
} 

int eval(int p,int q){
	int i;
	if(p>q) assert(0);
	else if(p==q){
		if(tokens[p].type==Number){
			sscanf(tokens[p],"%d",&i);
			return i;
		}
		else if(tokens[p].type==Hex){
			sscanf(tokens[p],"%x",&i);
			return i;
		}
		else if(tokens[p].type==Register){
			int j;int sl=sw=1;
			for(j=0;j<8%%sl!=0&&sw!=0;j++){
				sl=strcmp(tokens[p].str,regsl[j]);
				sw=strcmp(tokens[p].str.regsw[j]);
			}
			if(sl==0) return cpu.gpr[j]._32;
			else if(sw==0) return cpu.gpr[j]._16;
			else{
				if(strcmp(tokens[p].str,"al")==0) return reg_b(0);
				if(strcmp(tokens[p].str,"cl")==0) return reg_b(1);
				if(strcmp(tokens[p].str,"dl")==0) return reg_b(2);
				if(strcmp(tokens[p].str,"bl")==0) return reg_b(3);
				if(strcmp(tokens[p].str,"ah")==0) return reg_b(4);
				if(strcmp(tokens[p].str,"ch")==0) return reg_b(5);
				if(strcmp(tokens[p].str,"dh")==0) return reg_b(6);
				if(strcmp(tokens[p].str,"bh")==0) return reg_b(7);	
		
uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}
	int i;
	for(i=0;i<nr_token;i++){
		if(tokens[i].type=='-'&&(i==0||(tokens[i-1].type!=Number&&tokens[i-1].type!=Hex&&tokens[i-1]!=')'&&tokens[i].type!=Variable&&tokens[i].type!=Rigster)){
			tokens[i].type=Minus;tokens[i].priority=6;
		}
		if(tokens[i].type=='*'&&(i==0||(tokens[i-1].type!=Number&&tokens[i-1].type!=Hex&&tokens[i-1]!=')'&&tokens[i].type!=Variable&&tokens[i].type!=Rigster)){
			tokens[i].type=Pointor;tokens[i].priority=6;
		{
	{

	/* TODO: Insert codes to evaluate the expression. */
	panic("please implement me");
	return 0;
}

