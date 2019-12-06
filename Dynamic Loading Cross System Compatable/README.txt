See DESIGN-RATIONALE.txt for design and testing documentation

Compiling and running:
	Linux:
		make -f Makefile.Linux
		./modFormatter <Options>

	Mac:
		make -f Makefile.Darwin
		./modFormatter <Options>

	Windows:
		nmake /f Makefile.Windows
		modFormatter <Options>

		*NOTE: GetProcAddress always returns NULL on Windows

	<Options> =	-v				: be more verbose
				-o <file>		: place output into <file>
				-P <path>		: Set search path for modules
				-D <modules>	: Set modules to be used seperated by commas default,quote,caps,crlf,barelf,native
