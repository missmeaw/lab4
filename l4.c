#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

void generate(void)
{
	srand(time(NULL));
        while (1) {
		int c = rand() % 128;
		putchar(c);
	}
}

void main(void)
{
	FILE *key;//ключ
	FILE *res_f = fopen("lll","w");//результат шифрования
	int p_chiper[2];//труба для ключа
	int p_file[2];//труба для файла
	int chip, result = 0;//хм, дескрипторы для ключа и результа?????
	char data, buf, name[256];//чар?0 зачем??
	size_t n, n1, n2;//для размеров???

	write(1, "Ecrypt(1) или decrypt(2)?\n", 34);
	read(0, &buf, 1);
	if ((buf != '1')||(buf != '2')) {
		printf("Error reading");
		exit(1);
	}
	write(1, "What file name?\n", 16);
	n = read(0, name, sizeof(name));
	if (n < 1) {
		printf("Error reading");
		exit(1);
	}
	name[n-1] = '\0';

	if (buf == '1') {
		key = fopen("chip","w");//key-file
		if (key == NULL) {
			printf("Could not create encrypt-file\n");
			exit(1);
		}
	}
	if (pipe(p_chiper) != 0) {//open key-pipe
		printf("Could not create one of the pipes\n");
		exit(1);
	}
	if (pipe(p_file) != 0) {//open file-pipe
		printf("Could not create one of the pipes\n");
		exit(1);
	}

	switch(fork())
	{
	case -1:
		printf("fork() failed\n");
		exit(1);
	case 0://поток детё
		close(1);
		//Закрыли поток, будем перенаправлять
		dup(p_chiper[1]);//дублируем файловый дескриптор
		//Перенаправление вывода одного конца трубы-ключа
		close(p_chiper[0]);
		close(p_chiper[1]);
		close(p_file[0]);
		close(p_file[1]);
		
		if (buf == '1')
			generate();
		//Данная функция передает сгенерированный ключ
		//На стандартный вывод, коем является p_chiper[1]
		if (buf == '1') {
			execlp("cat", "cat", "chip", NULL);
	default://поток родитель
		switch(fork())
		{
		case -1:
			printf("fork() failed\n");
			exit(1);
		case 0://поток дитё в родителе
			close(1);
			dup(p_file[1]);
			//Перенаправление вывода
			close(p_file[0]);
			close(p_file[1]);
			close(p_chiper[0]);
			close(p_chiper[1]);
			execlp("cat", "cat", name, NULL);
		default://поток родитель в родителе
			close(p_file[1]);
			close(p_chiper[1]);
			n2 = read(p_file[0], &data, 1);
			while (n2 > 0) {
				n1 = read(p_chiper[0], &chip, 1);
				result = data ^ chip;
				fputc(result, res_f);
				if (buf == '1')
					fputc(chip, key);
			}
			if (buf == '1') 
				fclose(key);					
			fclose(res_f);
		}
	}
}
