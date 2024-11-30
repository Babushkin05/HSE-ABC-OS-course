.include "io_handler.asm"
.include "string_handler.asm"


.text
	program:
		# prolog
		addi sp sp -4
		sw ra (sp)
		
		# read file, return string adres in a0 and string size in a1
		call read_string_from_file
		
		# count vowels and consonants in file, 
		# takes string adres in a0 and strin size in a1, 
		# return vowels count in a0 consonants count in a1
		call counting_vowels_and_consonants

		# create string from counts
		# takes vowels count in a0 consonants count in a0
		# return string adress in a0, string size in a1
		call create_string

		# write string to file or to terminal
		# gets string adres in a0, string size in a1
		# return nothing
		call output

		# epilog
		lw ra (sp)
		addi sp sp 4

		ret