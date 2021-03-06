#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
	char *ch=strtok(args," ");
	if(ch==NULL){
	 cpu_exec(1); return 0;}
	int num;
	sscanf(ch,"%d",&num);
	int i;
	for(i=0;i<num;i++)
	cpu_exec(1);
	return 0;
}

static int cmd_info(char *args){
	args=strtok(args," ");
	if(strcmp(args,"r")==0){
	printf("eax: %x\n",cpu.eax);
	printf("edx: %x\n",cpu.edx);
	printf("ecx: %x\n",cpu.ecx);
	printf("ebx: %x\n",cpu.ebx);
	printf("ebp: %x\n",cpu.ebp);
	printf("esi: %x\n",cpu.esi);
	printf("edi: %x\n",cpu.edi);
	printf("esp: %x\n",cpu.esp);
	return 0;
	}
	if(strcmp(args,"w")==0){
		print_wp();
		return 0;
	}
	return 1;
}

static int cmd_x(char *args){
	if(args==NULL){
	printf("too few arguments!");
	return 1;
  }
	char *ch=strtok(args," ");
	if(ch==NULL){
	printf("too few arguments!");
	return 1;
  }
	int num; sscanf(ch,"%d",&num);
	char *expr=strtok(NULL," ");
	if(expr==NULL){
	printf("too few arguments!");
	return 1;
  }
	if(strtok(NULL," ")!=NULL){
	printf("too many arguments!");
	return 1;
  }
	char *str;	
	swaddr_t addr=strtol(expr,&str,16);
	int i;
	printf("0x%08x: ",addr);
	for(i=0;i<num;i++){
	uint32_t data=swaddr_read(addr+i*4,4);
	printf("0x%x ",data);
	}
	printf("\n");
  
	return 0;

}

static int cmd_p(char *args){
	bool success;
	int result=expr(args,&success);
	printf("%d\n",result);
	return 0;
}

static int cmd_w(char *args){
	WP *f;
	f=new_wp();
	bool suc;
	f->val=expr(args,&suc);
	strcpy(f->expr,args);
	if(!suc) Assert(1,"Wrong Input!");
	printf("New Watchpoint: %d  %s  0x%x\n",f->NO,f->expr,f->val);
	return 0;
}

static int cmd_d(char *args){
	int num;
	sscanf(args,"%d",&num);
	free_wp(num);
	return 0;
}
	

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si","Single step",cmd_si },
	{ "info","Print the rigisters",cmd_info},
	{ "x","Scanf the memory",cmd_x},
	{ "p","Expression Evaluation",cmd_p},
	{ "w","Set a watchpoint",cmd_w},
	{ "d","Delete a watchpoint",cmd_d},
	
	
	

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
