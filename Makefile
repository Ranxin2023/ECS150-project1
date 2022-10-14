# compile sshell
sshell: sshell.c 
	gcc -Wall -Werror -Wextra -o sshell sshell.c

# recompile
rc: sshell.c
	$(RM) sshell
	gcc -Wall -Werror -Wextra -o sshell sshell.c

# recompile and run 
rcr: sshell.c
	$(RM) sshell
	gcc -Wall -Werror -Wextra -o sshell sshell.c
	./sshell
	
# run the shell
run: sshell.c
	./sshell

# clean environment
clean: 
	$(RM) sshell
