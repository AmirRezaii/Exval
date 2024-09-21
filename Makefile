##
# Exval
#
# @file
# @version 0.1

output: main.o
	gcc -o app main.o

main.o: main.c
	gcc -c main.c

# end
