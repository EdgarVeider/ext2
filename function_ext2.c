#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "ext2_fs.h"

#define BASE_OFFSET 1024                   /* locates beginning of the super block (first group) */
#define FD_DEVICE "myext2image.img"              /* the floppy disk device */
#define BLOCK_OFFSET(block) (BASE_OFFSET + (block-1)*block_size)

unsigned int ROOT_DIRECTORY = 2;
unsigned int CURRENT_DIRECTORY = 2;

static unsigned int block_size = 0;        /* block size (to be calculated) */

static void read_dir(int, const struct ext2_inode*, const struct ext2_group_desc*);
static void read_inode(int, int, const struct ext2_group_desc*, struct ext2_inode*);

struct ext2_super_block super;
int fd;
char *pwd;

#define EXT2_S_IROTH 0x0004 // O read
#define EXT2_S_IWOTH 0x0002 // O write
#define EXT2_S_IXOTH 0x0001 // O execute
#define EXT2_S_IRUSR 0x0100 // U read
#define EXT2_S_IWUSR 0x0080 // U write
#define EXT2_S_IXUSR 0x0040 // U execute
#define EXT2_S_IRGRP 0x0020 // G read
#define EXT2_S_IWGRP 0x0010 // G write
#define EXT2_S_IXGRP 0x0008 // G execute



//Funções Auxiliares 4.c

static 
void read_inode(int fd, int inode_no, const struct ext2_group_desc *group, struct ext2_inode *inode)
{
	lseek(fd, BLOCK_OFFSET(group->bg_inode_table)+(inode_no-1)*sizeof(struct ext2_inode), 
	      SEEK_SET);
	read(fd, inode, sizeof(struct ext2_inode));
} /* read_inode() */


static void read_dir(int fd, const struct ext2_inode *inode, const struct ext2_group_desc *group);

static unsigned int Read_Inode_Number(int fd, const struct ext2_inode *inode, const struct ext2_group_desc *group, char* nome_arquivo)
{
	void *block;

	if (S_ISDIR(inode->i_mode)) {
		struct ext2_dir_entry_2 *entry;
		unsigned int size = 0;

		if ((block = malloc(block_size)) == NULL) { /* allocate memory for the data block */
			fprintf(stderr, "Memory error\n");
			close(fd);
			exit(1);
		}

		lseek(fd, BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
		read(fd, block, block_size);                /* read block from disk*/

		entry = (struct ext2_dir_entry_2 *) block;  /* first entry in the directory */
                /* Notice that the list may be terminated with a NULL
                   entry (entry->inode == NULL)*/
		while((size < inode->i_size) && entry->inode) {
			char file_name[EXT2_NAME_LEN+1];
			memcpy(file_name, entry->name, entry->name_len);
			file_name[entry->name_len] = 0;     /* append null character to the file name */

			if(strcmp(nome_arquivo, entry->name) == 0){
				return entry->inode;
			}

			entry = (void*) entry + entry->rec_len;
			size += entry->rec_len;
		}

		free(block);
	}
} /* retorna numero inode */

void read_arq(int fd, struct ext2_inode *inode){
	int size = inode->i_size;
	char *bloco = malloc(block_size);

	lseek(fd, BLOCK_OFFSET(inode->i_block[0]),SEEK_SET);
	read(fd, bloco, block_size);

	for (int i = 0; i < size; i++)
	{
		printf("%c", bloco[i]);
	}
}

void change_group(unsigned int* inode, struct ext2_group_desc* groupToGo, int* currentGroup) {	
	unsigned int block_group = ((*inode) - 1) / super.s_inodes_per_group; // Cálculo do grupo do Inode
	if (block_group != (*currentGroup))
	{
		*currentGroup = block_group;

		lseek(fd, BASE_OFFSET + block_size + sizeof(struct ext2_group_desc) * block_group, SEEK_SET);
		read(fd, groupToGo, sizeof(struct ext2_group_desc));
	}
}

//Entra em um novo grupo

int entrar_grupo(int i_number, struct ext2_super_block *super, struct ext2_group_desc *group, int fd){
	int group_number;
	group_number = (i_number - 1) / super->s_inodes_per_group;
	lseek(fd, BASE_OFFSET + block_size + sizeof(struct ext2_group_desc)*group_number, SEEK_SET);
	read(fd, &(*group), sizeof(*group));
}

void change_path(char *pwd, char *dir_name){
	int pwd_init = strlen(pwd);
	//printf("%d", pwd_init);

	if(strcmp(dir_name, "..") == 0){
		int j = strlen(pwd);
		j = j-2;
		while (pwd[j] != '/')
		{
			j--;
		}
		pwd[j+1] = '\0';
		
	}else{
		char *new_name = calloc(100, sizeof(char));
		strcpy(new_name, dir_name);
		int tam = strlen(dir_name);
		new_name[tam] = '/';
		new_name[tam+1] = '\0';


		int i;
		for (i = 0; new_name[i] != '\0'; i++)
		{
			pwd[pwd_init + i] = new_name[i];
		}
		pwd[pwd_init + i] = '\0';
	}

}

//Funções Finais 
static void read_dir(int fd, const struct ext2_inode *inode, const struct ext2_group_desc *group)
{
	void *block;

	if (S_ISDIR(inode->i_mode)) {
		struct ext2_dir_entry_2 *entry;
		unsigned int size = 0;

		if ((block = malloc(block_size)) == NULL) { /* allocate memory for the data block */
			fprintf(stderr, "Memory error\n");
			close(fd);
			exit(1);
		}

		lseek(fd, BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
		read(fd, block, block_size);                /* read block from disk*/

		entry = (struct ext2_dir_entry_2 *) block;  /* first entry in the directory */
                /* Notice that the list may be terminated with a NULL
                   entry (entry->inode == NULL)*/
		while((size < inode->i_size) && entry->inode) {
			char file_name[EXT2_NAME_LEN+1];
			memcpy(file_name, entry->name, entry->name_len);
			file_name[entry->name_len] = 0;     /* append null character to the file name */
			printf("%10u %s\n", entry->inode, file_name);
			entry = (void*) entry + entry->rec_len;
			size += entry->rec_len;
		}

		free(block);
	}
} /* LS() */

void change_directory(char* dirName, struct ext2_inode *inode, struct ext2_group_desc *group, int *currentGroup) {

	void *block;

	if (S_ISDIR(inode->i_mode)) {
		struct ext2_dir_entry_2 *entry;
		unsigned int size = 0;

		if ((block = malloc(block_size)) == NULL) { /* allocate memory for the data block */
			fprintf(stderr, "Memory error\n");
			close(fd);
			exit(1);
		}

		lseek(fd, BLOCK_OFFSET(inode->i_block[0]), SEEK_SET);
		read(fd, block, block_size);                /* read block from disk*/

		entry = (struct ext2_dir_entry_2 *) block;  /* first entry in the directory */

		while((size < inode->i_size) && entry->inode) {
			char file_name[EXT2_NAME_LEN+1];
			memcpy(file_name, entry->name, entry->name_len);
			file_name[entry->name_len] = 0;     /* append null character to the file name */

			// PARA RETORNAR INODE
			if((strcmp(dirName, entry->name)) == 0){
				change_path(pwd, dirName);
				//parametros do cd 
				printf("%s\n"
				       "inode: %10u\n"
					   "Record lenght: %hu\n"
				       "Name lenght: %d\n"
					   "File type: %d\n\n",
						file_name,
						entry->inode,
						entry->rec_len,
						entry->name_len,
						entry->file_type);
				break;
			}

			entry = (void*) entry + entry->rec_len;
			size += entry->rec_len;
		}

		free(block);
	}
	
	printf("\n\n");

	unsigned int inodeRetorno = Read_Inode_Number(fd, inode, group, dirName);

	change_group(&inodeRetorno, group, currentGroup);

	unsigned int index = ((int)inodeRetorno) % super.s_inodes_per_group;

	read_inode(fd, index, group, inode);
	
}



void CAT_Ext2(int fd, struct ext2_inode *inode, struct ext2_group_desc *group, char *arquivoNome, int *currentGroup){
	struct ext2_group_desc *grupoTemp = (struct ext2_group_desc *)malloc(sizeof(struct ext2_group_desc));
	struct ext2_inode* inodeEntryTemp = (struct ext2_inode*)malloc(sizeof(struct ext2_inode));
	
	memcpy(grupoTemp, group, sizeof(struct ext2_group_desc));
	memcpy(inodeEntryTemp, inode, sizeof(struct ext2_inode));
	unsigned int inodeRetorno = Read_Inode_Number(fd, inodeEntryTemp, grupoTemp, arquivoNome);
	// READ_DIR RETORNA INODE ERRADO (ela consegue encontrar o certo, mas RETORNA errado)
	// inodeRetorno = 12290;
	printf("\ninodeRetorno: %u\n", inodeRetorno);

	change_group(&inodeRetorno, grupoTemp, currentGroup);

	
	unsigned int index = inodeRetorno % super.s_inodes_per_group;
	
	read_inode(fd, index, grupoTemp, inodeEntryTemp);
	

	char *block = (char*)malloc(sizeof(char)*block_size);

	lseek(fd, BLOCK_OFFSET(inodeEntryTemp->i_block[0]), SEEK_SET);
	read(fd, block, block_size);

	int arqSize = inodeEntryTemp->i_size;
	
	int singleIndirection[256];
	int doubleIndirection[256];

	printf("ARQ SIZE: %d ", arqSize);

	// Percorrendo pelos blocos de dados sem indireção
	for(int i = 0; i < 12; i++) {

		lseek(fd, BLOCK_OFFSET(inodeEntryTemp->i_block[i]), SEEK_SET);
		read(fd, block, block_size); // Lê bloco i em block
		
		// Exibindo conteúdo do primeiro bloco
		for(int i = 0; i < 1024; i++) {
			printf("%c", block[i]);
			
			arqSize = arqSize - sizeof(char); // Quantidade de dados restantes

			if(arqSize <= 0) {
				break;
			}
		}
		if(arqSize <= 0) {
			break;
		}
	}

	// Se após os blocos sem indireção ainda existirem dados,
	// percorre o bloco 12 (uma indireção)
	if(arqSize > 0) {
		
		lseek(fd, BLOCK_OFFSET(inodeEntryTemp->i_block[12]), SEEK_SET);
		read(fd, singleIndirection, block_size);
		
		for(int i = 0; i < 256; i++) {
			
			lseek(fd, BLOCK_OFFSET(singleIndirection[i]), SEEK_SET);
			read(fd, block, block_size);

			for(int j = 0; j < 1024; j++) {
				
				printf("%c", block[j]);
				arqSize = arqSize - 1;
				
				if (arqSize <= 0) {
					break;
				}
			}
			if (arqSize <= 0) {
				break;
			}
		}
	}

	// Se depois dos blocos com uma indireção ainda existirem dados,
	// percorre o bloco 13 (dupla indireção)
	if(arqSize > 0){

		lseek(fd, BLOCK_OFFSET(inodeEntryTemp->i_block[13]), SEEK_SET);
		read(fd, doubleIndirection, block_size);

		for(int i = 0; i < 256; i++){
			
			//não entendi
			if(arqSize <= 0){
				break;
			}

			lseek(fd, BLOCK_OFFSET(doubleIndirection[i]), SEEK_SET);
			read(fd, singleIndirection, block_size);

			for(int j = 0; j < 256; j++) {
				
				if (arqSize <= 0){
					break;
				}

				lseek(fd, BLOCK_OFFSET(singleIndirection[j]), SEEK_SET);
				read(fd, block, block_size);

				for(int k = 0; k < 1024; k++){
					
					printf("%c", block[k]);
					
					arqSize = arqSize - 1;
					
					if (arqSize <= 0){
						break;
					}
				}
			}
		}
	}

	// unsigned int entry_inode = read_dir(fd, &inode, &group, second_param);

	// read_inode(fd, entry_inode, &group, &inode);


	free(block);
	free(grupoTemp);
	free(inodeEntryTemp);

}

void info(){
	printf(
		   "Volume name.............: %s\n"
		   "Image size..............: %u bytes\n" 
		   "Free space..............: %u KiB\n" 
	       "Free inodes ............: %u\n"
		   "Free blocks ............: %u\n"
		   "Block size..............: %u bytes\n"
		   "Inode size..............: %hu bytes\n"
		   "Groups count............: %u\n"
	       "Groups size.............: %u blocks\n"
	       "Groups inodes...........: %u inodes\n"
		   "Inodetable size.........: %lu blocks\n"
	       ,  
		   super.s_volume_name, //nome do volume 
		   (super.s_blocks_count * block_size /* super.s_blocks_per_group*/), //tamanho da imagem
		   ((super.s_free_blocks_count - super.s_r_blocks_count) * block_size) / 1024, // espaço livre //bug???
	       super.s_free_inodes_count, //free inodes 
		   super.s_free_blocks_count, //free blocks
		   block_size, // tamanho do bloco
		   super.s_inode_size, //inode size 
		   (super.s_blocks_count/super.s_blocks_per_group), //bug??
		   super.s_blocks_per_group,
		   super.s_inodes_per_group,
		   (super.s_inodes_per_group/(block_size/sizeof(struct ext2_inode)))
	       );
}

void attr(struct ext2_inode *inode, struct ext2_group_desc *group, char *arquivoNome, int* currentGroup){
	struct ext2_inode* entry = (struct ext2_inode*)malloc(sizeof(struct ext2_inode));
	struct ext2_group_desc *grupoTemp = (struct ext2_group_desc *)malloc(sizeof(struct ext2_group_desc));
	memcpy(entry, inode, sizeof(struct ext2_inode));
	memcpy(grupoTemp, group, sizeof(struct ext2_group_desc));
	unsigned int inodeRetorno = Read_Inode_Number(fd, inode, group, arquivoNome);
	change_group(&inodeRetorno, grupoTemp, currentGroup);
	read_inode(fd, inodeRetorno, grupoTemp, entry);

		char fileOrDir;
	if(S_ISDIR(entry->i_mode)){
		fileOrDir = 'd';
	}else {
		fileOrDir = 'f';
	}

	/*verificar as permisões do usuario*/
	char uRead; 
	char uWrite;
	char uExec;
	if((entry->i_mode) & (EXT2_S_IRUSR)){
		uRead = 'r';
	} else{
		uRead = '-';
	}
	if ((entry->i_mode) & (EXT2_S_IWUSR)){
		uWrite = 'w';
	}else{
		uWrite = '-';
	}
	if ((entry->i_mode) & (EXT2_S_IXUSR)){
		uExec = 'x';
	}else{
		uExec = '-';
	}

	/*verificar as permisões do grupo*/
	char gRead; 
	char gWrite;
	char gExec;

	if ((entry->i_mode) & (EXT2_S_IRGRP)){
		gRead = 'r';
	}else{
		gRead = '-';
	}
	if ((entry->i_mode) & (EXT2_S_IWGRP)){
		gWrite = 'w';
	}else{
		gWrite = '-';
	}
	if ((entry->i_mode) & (EXT2_S_IXGRP)){
		gExec = 'x';
	}else{
		gExec = '-';
	}

	/*verificar as permisões do grupo*/
	char oRead; 
	char oWrite;
	char oExec;
	if ((entry->i_mode) & (EXT2_S_IROTH)){
		oRead = 'r';
	}else{
		oRead = '-';
	}
	if ((entry->i_mode) & (EXT2_S_IWOTH))
		oWrite = 'w';
	else
		oWrite = '-';
	if ((entry->i_mode) & (EXT2_S_IXOTH))
		oExec = 'x';
	else
		oExec = '-';

	printf(
			"permissões\t"
			"uid \t"
			"gid \t"
			"tamanho \t"
			"modificado em\t\n"
			"%c"
			"%c"
			"%c"
			"%c"
			"%c"
			"%c"
			"%c"
			"%c"
			"%c"
			"%c\t"
			"%d\t"
			"%d  ",
			fileOrDir,
			uRead,
			uWrite,
			uExec,
			gRead,
			gWrite,
			gExec,
			oRead,
			oWrite,
			oExec,
			entry->i_uid,
			entry->i_gid
			);

	
	if (entry->i_size > 1024){
		printf("   %.1f KiB\t\t", (((float)entry->i_size) / 1024));
	}else{
		printf("    %d B\t\t", (entry->i_size));
	}

			struct tm *mtime;
			time_t segundos = entry->i_mtime;
			mtime = localtime(&segundos); 
			printf(
					"%d/"
					"%d/"
					"%d"
					" %d:" 
					"%d\n",
		   			mtime->tm_mday, 	
					mtime->tm_mon + 1, 
					(mtime->tm_year + 1900),
		   			mtime->tm_hour, 
					mtime->tm_min
				);
}

int main(){

	struct ext2_inode inode;
	struct ext2_group_desc group;
	int currentGroup = 0;

	//Inicialiando PWD
	pwd = calloc(200, sizeof(char));
	char dir_raiz[] = "root/";
	strcpy(pwd, dir_raiz);

	/* open floppy device */
 	if ((fd = open(FD_DEVICE, O_RDONLY)) < 0) {
 		perror(FD_DEVICE);
 		exit(1);  /* error while opening the floppy device */
 	}

	// 	/****** read super-block *******/
	// 	/******************************/

 	lseek(fd, BASE_OFFSET, SEEK_SET); 
 	read(fd, &super, sizeof(super));

 	if (super.s_magic != EXT2_SUPER_MAGIC) {
 		fprintf(stderr, "Not a Ext2 filesystem\n");
 		exit(1);
 	}
		
 	block_size = 1024 << super.s_log_block_size;

	// 	/********* read group descriptor ***********/
	// 	/******************************************/
	lseek(fd, BASE_OFFSET + block_size, SEEK_SET);
 	read(fd, &group, sizeof(group));

	read_inode(fd, 2, &group, &inode);  // read inode 2 (root directory)
	read_dir(fd, &inode, &group);
	printf("\n\n");

	//testes LER BIBLIA

	// CAT_Ext2(fd, &inode, &group, "hello.txt", &currentGroup);
	// printf("\n\n");

	// change_directory("livros", &inode, &group, &currentGroup);
	// read_dir(fd, &inode, &group);

	// change_directory("religiosos", &inode, &group, &currentGroup);
	// read_dir(fd, &inode, &group);
	// CAT_Ext2(fd, &inode, &group, "Biblia.txt", &currentGroup);

	char* comand = calloc(100, sizeof(char));

	char* comand_1 = calloc(100, sizeof(char));
	char* comand_2 = calloc(100, sizeof(char));

	int comand_flag;

	while (1)
	{
		fgets(comand, 50, stdin);
		comand[strcspn(comand, "\n")] = 0;
		
		
		if (strchr(comand, ' ') != NULL)
		{
			comand_flag = 1;	
			for (int i = 0; comand[i] != ' '; i++)
			{
				comand_1[i] = comand[i];
			}

			char* temp_str = calloc(100, sizeof(char));
			temp_str = strchr(comand, ' ');

			for (int i = 0; temp_str[i] != '\0'; i++)
			{
				comand_2[i] = temp_str[i+1];
			}

		}else{
			comand_flag = 0;
			strcpy(comand_1, comand);
		}

		if (comand_flag == 0)
		{

			if(strcmp(comand_1, "ls") == 0){

				read_dir(fd, &inode, &group);
			}

			if(strcmp(comand_1, "pwd") == 0){

				printf("%s\n", pwd);
			}

			if(strcmp(comand_1, "info") == 0){

				info();
			}


		}

		if (comand_flag = 1){
			if (strcmp(comand_1, "cd") == 0)
			{
				change_directory(comand_2, &inode, &group, &currentGroup);
			}

			if (strcmp(comand_1, "cat") == 0)
			{
				CAT_Ext2(fd, &inode, &group, comand_2, &currentGroup);
			}
			
			if(strcmp(comand_1, "attr") == 0){

				attr(&inode, &group, comand_2, &currentGroup);
			}
			
		}
		
	}
	



    return 0;
}