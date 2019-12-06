Adam Anderson
aander10

To run:
	make
	./main <-size width height> <-threads number> <-output>

Timting test results:
	Machine:
		Ryzen 2500x
		6 cores 12 logical processors
		4.00 GHz
		Running windows with mingw

	Threads	Time
	1		155.074
	2		87.5647
	4		50.071
	8		29.5948
	16		22.5003