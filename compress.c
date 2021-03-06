#include "compress.h"


// 압축할 파일에 cpd 확장자를 붙이는 함수 
char *get_encode_name(char *path)
{
	// 길이를 받아와서 끝부터 시작해서 처음 나타나는 . 을 기준으로 뒤에 cpd를 붙인다. 
	int len = strlen(path);
	char *ret = (char *)malloc(len + 1);
	int i;
	strcpy(ret,path);
	for(i=len-1;ret[i] != '.';i--);
	strcpy(ret+i,".cpd");
	
	// 이름 리턴 
	return ret;
}

// 압축해제할 파일에 dcpd 확장자를 붙이는 함수 
char *get_decode_name(char *path)
{
	// 길이를 받아와서 끝부터 시작해서 처음 나타나는 . 을 기준으로 뒤에 dcpd를 붙인다. 
	int len = strlen(path);
	char *ret = (char *)malloc(len + 1);
	int i;
	strcpy(ret,path);
	for(i=len-1;ret[i] != '.';i--);
	strcpy(ret+i,".dcpd");
	
	// 이름 리턴 
	return ret;
}

// 파일 마다 다른 알고리즘을 적용하기 위해 파일의 이름을 가져오는 함수 
int get_name(char *path)
{
	// 끝에서부터 첫 . 이 발견되면 그 앞 문자를 리턴 
	int len = strlen(path);
	for(;path[len--]!='.';);
	return path[len] - '0';
}

// 파일로부터 버퍼를 가져오는 함수 
void get_buffer(FILE *fp,char *buffer)
{
	for(;*(buffer++)=fgetc(fp),!feof(fp););
}

// 버퍼를 파일에 저장하는 함수 
void set_buffer(FILE *fp,char *buffer,int size)
{
	for(;size--;)fputc(*(buffer++),fp);	
}

// 문자열을 복사하는 함수 
void buffer_cpy(char *buffer, int size, char *result)
{
	int i;
	
	// size 길이만큼 복사 
	for(i=0;i<size;i++)buffer[i]=result[i];
}

void compress(char *path, int option)
// 압축밑 해제를 담당하는 함수 
{
	FILE *fp;
	char *buffer, *result;
	int size,rsize,i;
	
	// 파일로부터 버퍼를 읽어옴 
	
	fp = fopen(path,"rb");
	if(fp == 0){puts("[!] fopen error");return;}
	
	// 사이즈를 받아와서 할당 
	size = get_size(fp); 
	buffer = (char *)malloc(size+0xFFFFFFF);
	get_buffer(fp,buffer);
	fclose(fp);
	
	int name = get_name(path);
	
	result = (char *)malloc(size+0xFFFFFFF);
	// encode
	if (option == 0)
	{
		rsize = encode(buffer,size,result,name);
		fp = fopen(get_encode_name(path),"wb");
	}
	
	// decode
	else
	{
		rsize = decode(buffer,size,result,name);
		fp = fopen(get_decode_name(path),"wb");
	}
	// 버퍼를 파일에 저장
	set_buffer(fp,buffer,rsize);
	fclose(fp);  
}


// 압축하는 알고리즘을 정하는 함수 
int encode(char *buffer, int size, char *result, int name)
{
	int rsize = size;	
	
	// LZSS, Huffman, Substitution 중에서 고르고 압축을 수행. 
	switch(name)
	{
	case 1:
		rsize = LZSS_encode(buffer,rsize,result);
		rsize = huffman_encode(result,rsize,buffer);
	break;
	case 2:
		rsize = LZSS_encode(buffer,rsize,result);
		rsize = huffman_encode(result,rsize,buffer);
	break;
	case 3:
		rsize = substitution_encode(buffer,rsize,result);
		rsize = huffman_encode(result,rsize,buffer);
	break;
	case 4:
		rsize = huffman_encode(buffer,rsize,result);
		
		// 압축결과는 buffer에 저장되게 구현해서 result를 buffer에 복사 
		buffer_cpy(buffer,rsize,result);
	break;
	case 5:
		rsize = LZSS_encode(buffer,rsize,result);
		rsize = huffman_encode(result,rsize,buffer);
	break;
	default:
		return -1;	
	}
	
	// 길이 리턴 
	return rsize;
}

// 압축해제하는 알고리즘을 정하는 함수 
int decode(char *buffer, int size, char *result, int name)
{
	int rsize = size;
	// LZSS, Huffman, Substitution 중에서 고르고 압축을 수행. 
	switch(name)
	{
	case 1:
		rsize = huffman_decode(buffer,rsize,result);
		rsize = LZSS_decode(result,rsize,buffer);
	break;
	case 2:
		rsize = huffman_decode(buffer,rsize,result);
		rsize = LZSS_decode(result,rsize,buffer);
	break;
	case 3:
		rsize = huffman_decode(buffer,rsize,result);
		rsize = substitution_decode(result,rsize,buffer);
	break;
	case 4:
		rsize = huffman_decode(buffer,rsize,result);
		buffer_cpy(buffer,rsize,result);
	break;
	case 5:
		rsize = huffman_decode(buffer,rsize,result);
		rsize = LZSS_decode(result,rsize,buffer);
	break;
	default:
		return -1;	
	}
	return rsize;
}
