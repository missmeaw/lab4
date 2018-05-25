#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>

void generate(void)
{
	int c;

	srand(time(NULL));
	while (1) {
		c = rand() % 128;
		putchar(c);
	}
}

void main(void)
{
	FILE *key;//ключ
	FILE *res_f;//результат шифрования
	int p_chiper[2];//труба для ключа
	int p_file[2];//труба для файла
	char data, chip, result, buf, buf1, name[256];
	size_t n;//для проверки на чтение

	write(1, "Ecrypt(1) или decrypt(2)?\n", 29);
	read(0, &buf, 1);
	read(0, &buf1, 1);
	if ((buf != '1') && (buf != '2')) {
		printf("Error reading1");
		exit(1);
	}
	write(1, "What file name?\n", 16);
	n = read(0, name, sizeof(name));
	if (n < 1) {
		printf("Error reading2");
		exit(1);
	}
	name[n-1] = '\0';

	if (buf == '1') {
		key = fopen("chip", "w");//key-file
		if (key == NULL) {
			printf("Could not create encrypt-file\n");
			exit(1);
		}
		res_f = fopen("rezult", "w");
		if (res_f == NULL) {
			printf("Could not create key-file\n");
			exit(1);
		}
	}
	if (buf == '2') {
		key = fopen(name, "w");//key-file
		if (key == NULL) {
			printf("Could not create decrypt-file\n");
			exit(1);
		}
		res_f = fopen("rezult", "r");
		if (res_f == NULL) {
			printf("Could not create key-file\n");
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

	switch (fork()) {
	case -1:
		printf("fork() failed\n");
		exit(1);
	case 0://поток детё
		close(1);//Закрыли поток
		dup(p_chiper[1]);//дублируем файл дескр
		close(p_chiper[0]);//Перенаправление
		close(p_chiper[1]);//вывода одного
		close(p_file[0]);//конца
		close(p_file[1]);//трубы-ключа

		if (buf == '1')
			generate();//send  gen-key on standr vivod
		if (buf == '2')
			execlp("cat", "cat", "chip", NULL);
	default://поток родитель
		switch (fork()) {
		case -1:
			printf("fork() failed\n");
			exit(1);
		case 0://поток дитё в родителе
			close(1);
			dup(p_file[1]);//Перенаправление
			close(p_file[0]);//вывода
			close(p_file[1]);
			close(p_chiper[0]);
			close(p_chiper[1]);
			if (buf == '1')
				execlp("cat", "cat", name, NULL);
			if (buf == '2')
				execlp("cat", "cat", "rezult", NULL);
		default://поток родитель в родителе
			close(p_file[1]);
			close(p_chiper[1]);
			while (read(p_file[0], &data, 1) > 0) {
				read(p_chiper[0], &chip, 1);
				result = data ^ chip;
				if (buf == '2')
					fputc(result, key);
				if (buf == '1') {
					fputc(chip, key);
					fputc(result, res_f);
				}
			}
			fclose(key);
			fclose(res_f);
		}
	}
}
