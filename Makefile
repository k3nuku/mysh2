all:
	gcc -o mysh ./src/mysh.c ./src/commands.c ./lib/parser.c ./lib/fs.c -I./include

unit_test:
	gcc -o unit_test ./lib/parser.c ./tests/parser_test.c -I./include
	./unit_test

func_test:
	touch scoring.txt
	rm scoring.txt
	./mysh < sample1.input > your_answer1.txt
	diff your_answer1.txt answer1.txt >> scoring.txt
	./mysh < sample2.input > your_answer2.txt
	diff your_answer2.txt answer2.txt >> scoring.txt
	./mysh < sample3.input > your_answer3.txt
	diff your_answer3.txt answer3.txt >> scoring.txt
	./mysh < sample4.input > your_answer4.txt
	diff your_answer4.txt answer4.txt >> scoring.txt
	./mysh < sample5.input > your_answer5.txt
	diff your_answer5.txt answer5.txt >> scoring.txt
