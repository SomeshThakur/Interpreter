#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define MAXBUF 100
struct data{
	char name;
	int address;
	int size;
	int const_value;
};
struct label{
	char name;
	int address;
}**label_table;

char *opcode[] = {
	"MOV",//1(R-M),2(M-R)
	"ADD",//3
	"SUB",//4
	"MUL",//5
	"JUMP",//6
	"IF",//7
	"EQ",//8
	"LT",//9
	"GT",//10
	"LTEQ",//11
	"GTEQ",//12
	"PRINT",//13(R),16(M)
	"READ",//14
	"ENDIF"//15
};
void strip(char *text, char delim){
	size_t i = 0;
	for (i = 0; text[i] != delim&&i < strlen(text); i++);
	text[i] = '\0';
}

int get_register(char *regs){
	/*if (regs[0] == 'A')return 0;
	if (regs[0] == 'B')return 1;
	if (regs[0] == 'C')return 2;
	if (regs[0] == 'D')return 3;
	if (regs[0] == 'E')return 4;
	if (regs[0] == 'F')return 5;
	if (regs[0] == 'G')return 6;
	if (regs[0] == 'H')return 7;*/
	return ('A' - regs[0])*-1;
	return -1;
}

int get_symbol_address(char* symbol, data **symbol_table, int symbol_table_size){
	for (int i = 0; i < symbol_table_size; i++){
		if (symbol_table[i]->name == symbol[0])
		{
			if (symbol[1] == '[')
				return symbol_table[i]->address + (symbol[2] - '0');
			return symbol_table[i]->address;
		}
	}
	return -1;
}

int ends_with_colon(char *token){
	int i = 0;
	for (i = 0; token[i] != '\0'; i++);
	i--;
	return token[i] == ':';
}

int get_label_index(char *lb, label **label_table, int label_index){
	for (int i = 0; i <= label_index; i++){
		if (label_table[i]->name == lb[0])return i;
	}
	return -1;
}

int **intermediate_code_gen(FILE *fp, int *ints_count, int *label_index, data **symbol_table, int symbol_table_size){
	int **intermediate_code = (int **)malloc(sizeof(int*)*MAXBUF);
	char *line = (char *)malloc(sizeof(char)*MAXBUF);
	char *token = (char*)malloc(sizeof(char)*MAXBUF);
	char *tmp = (char*)malloc(sizeof(char)*MAXBUF);
	int size, index = 0, ino = 0;
	int flag = 1;
	int if_stack[100];
	int if_top = -1;
	size = sizeof(opcode) / sizeof(opcode[0]);

	while (fgets(line, MAXBUF, fp) != NULL){
		//Loading Token
		strcpy(token, strtok(line, " "));
		if (token[0] == '\t')token = token + 1;
		//Finding opcode
		for (int i = 0; i < size; i++)
		{
			index = 0;
			if (!strcmp(opcode[i], token))
			{
				intermediate_code[ino] = (int*)malloc(sizeof(int)* 6);
				intermediate_code[ino][index] = i + 2;
				index++;
				if (!strcmp(token, "MOV"))
				{
					strcpy(tmp, strtok(NULL, " "));
					//Register to Memory
					int offset = 0;
					if (tmp[1] == ',' || strlen(tmp) == 5){
						intermediate_code[ino][index - 1] -= 1;
						strip(tmp, ',');
						intermediate_code[ino][index] = get_symbol_address(tmp, symbol_table, symbol_table_size);
						index++;
						strcpy(tmp, strtok(NULL, " "));
						strip(tmp, '\n');
						intermediate_code[ino][index++] = get_register(tmp);
					}
					//Memory to Register
					else if (tmp[1] == 'X')
					{
						intermediate_code[ino][index - 1] -= 1;
						strip(tmp, ',');
						intermediate_code[ino][index++] = get_register(tmp);
						strcpy(tmp, strtok(NULL, " "));
						strip(tmp, '\n');
						intermediate_code[ino][index] = get_symbol_address(tmp, symbol_table, symbol_table_size);
						index++;
					}
				}
				else if (!strcmp(token, "READ"))
				{
					strcpy(tmp, strtok(NULL, " "));
					strip(tmp, '\n');
					intermediate_code[ino][index] = get_register(tmp);
					index++;
				}
				else if (!strcmp(token, "ADD") || !strcmp(token, "SUB") || !strcmp(token, "MUL"))
				{
					//Destination
					strcpy(tmp, strtok(NULL, " "));
					strip(tmp, ',');
					intermediate_code[ino][index] = get_register(tmp);
					index++;
					//Source 1
					strcpy(tmp, strtok(NULL, " "));
					strip(tmp, ',');
					intermediate_code[ino][index] = get_register(tmp);
					index++;
					//Source 2
					strcpy(tmp, strtok(NULL, " "));
					strip(tmp, '\n');
					intermediate_code[ino][index] = get_register(tmp);
					index++;
				}
				else if (!strcmp(token, "IF"))
				{
					// Pushing if into stack
					if_stack[++if_top] = ino;
					strcpy(tmp, strtok(NULL, " "));

					// 1st Operand
					intermediate_code[ino][index] = get_register(tmp);
					index++;
					strcpy(tmp, strtok(NULL, " "));

					// Operator 
					for (int p = 6; p < 11; i++)
					if (!strcmp(opcode[p], tmp))
					{
						intermediate_code[ino][index] = p + 2;
						index++;
						break;
					}
					strcpy(tmp, strtok(NULL, " "));
					// 2nd Operand
					intermediate_code[ino][index] = get_register(tmp);
					index++;

				}
				else if (!strcmp(token, "ENDIF"))
				{
					// Poping from stack 
					intermediate_code[if_stack[if_top]][1] = ino + 1;
					//	if_stack[if_top] = ino;
				}
				else if (!strcmp(token, "JUMP"))
				{
					strcpy(tmp, strtok(NULL, " "));
					strip(tmp, '\n');
					intermediate_code[ino][index] = label_table[get_label_index(tmp, label_table, *label_index)]->address;
					index++;
				}
				else if (!strcmp(token, "PRINT"))
				{
					strcpy(tmp, strtok(NULL, " "));
					strip(tmp, '\n');
					if (strlen(tmp) == 2)
					{
						intermediate_code[ino][index++] = get_register(tmp);
					}
					else{
						intermediate_code[ino][index - 1] = 16;
						intermediate_code[ino][index++] = get_symbol_address(tmp, symbol_table, symbol_table_size);
					}
				}
				ino++;
				break;
			}
			else
			{
				strip(token, '\n');
				if (ends_with_colon(token))
				{
					flag = 0;
					strip(token, ':');
					if (label_table == NULL){
						label_table = (label**)malloc(sizeof(label*)*MAXBUF);
						for (int i = 0; i < MAXBUF; i++)
							label_table[i] = (label*)malloc(sizeof(label));
					}
					//intermediate_code[ino] = (int*)malloc(sizeof(int)*MAXBUF);
					//	intermediate_code[ino][index] = ino;
					label_table[*label_index]->name = token[0];
					label_table[(*label_index)++]->address = ino;
				}
				else
				{
					if (!strcmp(token, "ELSE"))
					{
						intermediate_code[ino] = (int*)malloc(sizeof(int)*MAXBUF);
						// Poping from stack and storing if's break point
						intermediate_code[if_stack[if_top]][4] = ino + 1;
						if_top--;
						for (int k = 0; k < size; k++)
						if (!strcmp(opcode[k], "JUMP"))
						{
							intermediate_code[ino][index] = k + 2; break;
						}
						//intermediate_code[if_stack[if_top]][4] = ino;
						if_stack[++if_top] = ino;
						//if_top;
						ino++;
						break;
					}
				}
			}
		}
	}
	*ints_count = ino;
	free(line);
	free(tmp);
	return intermediate_code;
}

int lookup(int variable_index, int *memory){
	return memory[variable_index];
}

void print_intermediate_code(int **intermediate_code, int ints_count){
	int opt;
	printf("------------------ INSTRUCTIONS ---------------------\n");
	for (int i = 0; i < ints_count; i++){
		opt = intermediate_code[i][0];
		switch (opt)
		{
		case 1:
		case 2:
			printf("| %d | %d %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1], intermediate_code[i][2]);
			break;
		case 3:
		case 4:
		case 5:
			printf("| %d | %d %d %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1], intermediate_code[i][2], intermediate_code[i][3]);
			break;
		case 6:
			printf("| %d | %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1]);
			break;
		case 7:
			printf("| %d | %d %d %d %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1], intermediate_code[i][2], intermediate_code[i][3], intermediate_code[i][4]);
			break;
		case 13:
			printf("| %d | %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1]);
			break;
		case 14:
		case 16:
			printf("| %d | %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1]);
			break;
		default:
			break;
		}
	}
	printf("-----------------------------------------------------\n");
	system("pause");
}

data ** load_symbol_table(FILE *fp, int *symbol_table_size){
	data **symbol_table = (data**)malloc(sizeof(data*)*MAXBUF);
	char *line = (char *)malloc(sizeof(char)*MAXBUF);
	char *token = (char*)malloc(sizeof(char)*MAXBUF);
	char *tmp = (char*)malloc(sizeof(char)*MAXBUF);
	int flag = 0;
	int address = 0;
	int sindex = 0;
	while (fgets(line, MAXBUF, fp) != NULL)
	{
		strip(line, '\n');
		symbol_table[sindex] = (data*)malloc(sizeof(data));
		//Loading Token
		strcpy(token, strtok(line, " "));
		if (!strcmp(token, "DATA")){
			strcpy(tmp, strtok(NULL, " "));
			if (strlen(tmp) == 1){
				symbol_table[sindex]->name = tmp[0];
				symbol_table[sindex]->size = 1;
				symbol_table[sindex]->address = address;
				address++;
			}
			else{
				symbol_table[sindex]->address = address;
				symbol_table[sindex]->size = tmp[2] - '0';
				symbol_table[sindex]->name = tmp[0];
				address += tmp[2] - '0';
			}
		}
		if (!strcmp("CONST", token)){
			strcpy(tmp, strtok(NULL, " "));
			symbol_table[sindex]->address = sindex;
			symbol_table[sindex]->name = tmp[0];
			symbol_table[sindex]->size = 0;
			//to skip =
			strtok(NULL, " ");
			// Getting number
			strcpy(tmp, strtok(NULL, " "));
			//Saving it in memory
			//memory[address] = atoi(tmp);
			symbol_table[sindex]->const_value = atoi(tmp);
			address++;
		}
		//symbol_table[sindex]->isconst = strcmp(token, "DATA") ? 0 : 1,memory[address]=;
		if (!strcmp(token, "START:")){
			break;
		}
		sindex++;
	}
	*symbol_table_size = sindex;
	free(line);
	free(token);
	free(tmp);
	return symbol_table;
}

void execute(int *memory, data **symbol_table, int **intermediate_code, int ints_count, int *registers){
	memory = (int *)malloc(sizeof(int)*MAXBUF);
	int opt;
	printf("-------------------- EXECUTION ------------------------\n");
	for (int pc = 0; pc < ints_count; pc++){
		opt = intermediate_code[pc][0];
		switch (opt)
		{
		case 1:
			memory[intermediate_code[pc][1]] = registers[intermediate_code[pc][2]];
			break;
		case 2:
			registers[intermediate_code[pc][1]] = memory[intermediate_code[pc][2]];
			break;
		case 3:
			registers[intermediate_code[pc][1]] = registers[intermediate_code[pc][2]] + registers[intermediate_code[pc][3]];
			break;
		case 4:
			registers[intermediate_code[pc][1]] = registers[intermediate_code[pc][2]] - registers[intermediate_code[pc][3]];
			break;
		case 5:
			registers[intermediate_code[pc][1]] = registers[intermediate_code[pc][2]] * intermediate_code[pc][3];
			break;
		case 6:
			pc = intermediate_code[pc][1] - 1;
			break;
		case 7:
			switch (intermediate_code[pc][2])
			{
			case 8:
				if (registers[intermediate_code[pc][1]] == registers[intermediate_code[pc][3]])
					continue;
				else{
					pc = intermediate_code[pc][4] - 1;
					break;
				}
			case 9:
				if (registers[intermediate_code[pc][1]] < registers[intermediate_code[pc][3]])
					continue;
				else{
					pc = intermediate_code[pc][4] - 1;
					break;
				}
			case 10:
				if (registers[intermediate_code[pc][1]] > registers[intermediate_code[pc][3]])
					continue;
				else{
					pc = intermediate_code[pc][4] - 1;
					break;
				}
			case 11:
				if (registers[intermediate_code[pc][1]] <= registers[intermediate_code[pc][3]])
					continue;
				else{
					pc = intermediate_code[pc][4] - 1;
					break;
				}
			case 12:
				if (registers[intermediate_code[pc][1]] >= registers[intermediate_code[pc][3]])
					continue;
				else{
					pc = intermediate_code[pc][4] - 1;
					break;
				}
			}break;
		case 13:
			printf("%d\n", registers[intermediate_code[pc][1]]);
			break;
		case 14:
			printf("Enter value : ");
			scanf("%d", &registers[intermediate_code[pc][1]]);
			break;
		case 16:
			if (symbol_table[intermediate_code[pc][1]]->size == 0)
			{
				printf("%d\n", symbol_table[intermediate_code[pc][1]]->const_value);
			}
			else{
				printf("%d\n", memory[intermediate_code[pc][1]]);
			}
			break;

		default:
			break;
		}
	}
	printf("------------------------------------------------------\n");
	system("pause");
}

void save_intermediate_code(int **intermediate_code, int ints_count, data**symbol_table, int symbol_table_size, char *filename){
	int opt;
	FILE *fp = fopen(filename, "w");
	if (fp == NULL){
		perror("Error");
		system("pause");
		exit(1);
	}
	fprintf(fp, "%d\n", symbol_table_size);
	for (int i = 0; i < symbol_table_size; i++)
	{
		fprintf(fp, "%c %d %d %d\n", symbol_table[i]->name, symbol_table[i]->size, symbol_table[i]->address, symbol_table[i]->const_value);
	}

	fprintf(fp, "%d\n", ints_count);
	for (int i = 0; i < ints_count; i++){
		opt = intermediate_code[i][0];
		switch (opt)
		{
		case 1:
		case 2:
			fprintf(fp, "%d %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1], intermediate_code[i][2]);
			break;
		case 3:
		case 4:
		case 5:
			fprintf(fp, "%d %d %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1], intermediate_code[i][2], intermediate_code[i][3]);
			break;
		case 6:
			fprintf(fp, "%d %d\n", i, intermediate_code[i][0], intermediate_code[i][1]);
			break;
		case 7:
			fprintf(fp, "%d %d %d %d %d\n", i, intermediate_code[i][0], intermediate_code[i][1], intermediate_code[i][2], intermediate_code[i][3], intermediate_code[i][4]);
			break;
		case 13:
			fprintf(fp, "%d %d\n", i, intermediate_code[i][0], intermediate_code[i][1]);
			break;
		case 14:
		case 16:
			fprintf(fp, "%d %d\n", i, intermediate_code[i][0], intermediate_code[i][1]);
			break;
		default:
			break;
		}
	}
	fclose(fp);
}

void load_intermediate_code(char *filename)
{

}

int main(){
	data **symbol_table = NULL;
	int *memory = NULL;
	int **intermediate_code = NULL;
	label **label_table = NULL;
	int ints_count = 0, symbol_table_size = 0, label_index = 0;
	int registers[8];
	char ch;
	FILE *fp = NULL;
	char filename[50];

	do {
		system("cls");
		printf("\n\t--Compiler/Interpreter--\n");
		printf("\t1. Compile Asem Code\n");
		printf("\t2. Execute Compiled Code\n");
		printf("\t3. Save Compiled Code\n");
		printf("\t4. Load Intermediate Code and execute\n");
		printf("\tEnter your choice : ");
		scanf("%c", &ch);
		switch (ch)
		{
		case '1':
			system("cls");
			printf("Enter Source file Name : ");
			scanf("%s", filename);
			fp = fopen(filename, "r");
			if (fp == NULL){
				perror("Error");
				system("pause");
			}
			symbol_table = load_symbol_table(fp, &symbol_table_size);
			intermediate_code = intermediate_code_gen(fp, &ints_count, &label_index, symbol_table, symbol_table_size);
			print_intermediate_code(intermediate_code, ints_count);
			break;

		case '2':
			system("cls");
			execute(memory, symbol_table, intermediate_code, ints_count, registers);
			break;

		case '3':
			printf("Enter file Name to save : ");
			scanf("%s", filename);
			save_intermediate_code(intermediate_code, ints_count, symbol_table, symbol_table_size, filename);
			break;

		case '4':
			system("cls");
			printf("Enter Intermediate code file Name : ");
			scanf("%s", filename);
			fp = fopen(filename, "r");
			if (fp == NULL){
				perror("Error");
				system("pause");
			}
			goto load_intermediate_code;
		back:
			execute(memory, symbol_table, intermediate_code, ints_count, registers);
		default:
			break;
		}

	} while (ch != 'E');
	fclose(fp);
	system("pause");

load_intermediate_code:
	FILE *fp1 = fopen(filename, "r");
	if (fp == NULL){
		perror("ERROR");
		system("pause");
		exit(1);
	}
	symbol_table = (data**)malloc(sizeof(data*)*MAXBUF);
	fscanf(fp, " %d", &symbol_table_size);
	for (int i = 0; i < symbol_table_size; i++)
	{
		symbol_table[i] = (data*)malloc(sizeof(data));
		fscanf(fp, " %c %d %d %d", &symbol_table[i]->name, &symbol_table[i]->size, &symbol_table[i]->address, &symbol_table[i]->const_value);
	}
	fscanf(fp, " %d", &ints_count);
	int opt;
	intermediate_code = (int**)malloc(sizeof(100));
	for (int i = 0; i < ints_count; i++){
		intermediate_code[i] = (int*)malloc(sizeof(int)* 6);
		fscanf(fp, " %d", &intermediate_code[i][0]);
		opt = intermediate_code[i][0];
		switch (opt)
		{
		case 1:
		case 2:
			fscanf(fp, " %d %d", &intermediate_code[i][1], &intermediate_code[i][2]);
			break;
		case 3:
		case 4:
		case 5:
			fscanf(fp, " %d %d %d", &intermediate_code[i][1], &intermediate_code[i][2], &intermediate_code[i][3]);
			break;
		case 6:
			fscanf(fp, " %d", &intermediate_code[i][1]);
			break;
		case 7:
			fscanf(fp, " %d %d %d %d", &intermediate_code[i][1], &intermediate_code[i][2], &intermediate_code[i][3], &intermediate_code[i][4]);
			break;
		case 13:
			fscanf(fp, " %d\n", &intermediate_code[i][1]);
			break;
		case 14:
		case 16:
			fscanf(fp, " %d\n", &intermediate_code[i][1]);
			break;
		default:
			break;
		}
	}
	fclose(fp);
	printf("\n\t Loaded successfully!\n");
	system("pause");
	goto back;
	return 0;
}
