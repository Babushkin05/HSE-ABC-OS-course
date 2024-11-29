.include "load-text.s"
.include "string_handler.asm"
.global main

.text
	main:
		call read_string_from_file
		
		call counting_vowels_and_consonants
		
		call create_string
		
		call save_string_to_file
				
		exit