aander10
0973066

CIS 3050 a4

run with:
	./mgServer

	in another terminal:
	echo <command> > /tmp/server-$USER-fifo
	or
	./mgSubmitJob <command>

assumptions:
	-sending "__server-$USER-decrement-jobs" to the grid will decrement the running jobs counter
		this is just how I keep track of how many jobs are running
	-sending "__socket-kill" through the socket interface will close the socket system
		this is how I close the socket at cleanup

	-logs are refreshed each time the server is run, they won't retain contents between runs
	-giving the command exit to the grid will end one of the processors
	-if all processors are ended with exit the server will shutdown
	-server outputs messages as it does things, eg. "processor 0: <command>"

test cases:
	determine server accepts jobs through fifo and assigns to a processor
		echo ls > /tmp/server-$USER-fifo
		expected output:
			processor log contains output from ls
			if it doesn't the test failed
		result: PASS	
	determine server accepts jobs through socket
		./mgSubmitJob ls
		expected output:
			processor log contains output from ls
			if it doesn't the test failed
		result: PASS
	determine server cleans up properly
		./mgSubmitJob shutdown
		expected output:
			server stops running
			ps shows no processors running
			/tmp contains no fifos or sockets
			all of the above must be true to pass
		result: PASS
		
		./mgSubmitJob exit (xNumProcessors)
		expected output:
			server stops running
			ps shows no processors running
			/tmp contains no fifos or sockets
			all of the above must be true to pass
		result: PASS

		^C
		expected output:
			server stops running
			ps shows no processors running
			/tmp contains no fifos or sockets
			all of the above must be true to pass
		result: PASS
	determine status displays correct information
		./mgSubmitJob status
		expected output:
			same # of processors as it said it initialized
			0 jobs
			all of the above must be true to pass
		result: PASS

		./mgSubmitJob exit
		./mgSubmitJob status
		expected output:
			1 less than # of processors as it said it initialized
			0 jobs
			all of the above must be true to pass
		result: PASS

		./mgSubmitJob timedCountdown 10
		./mgSubmitJob status (within 10 seconds)
		expected output:
			however many processors should be running at this point
			1 jobs
			all of the above must be true to pass
		result: PASS
	determine exit works
		interlace ./mgSubmitJob exit and ./mgSubmitJob echo foo arbitrarily numProcessors times
		./mgSubmitJob echo foo over the set of now running processors
		expected output:
			processors told to exit exit without any errors
			the ./mgSubmitJob echo foo blitz over the set skips over exited processors
			/tmp contains no fifos for the exited processors
			./mgSubmitJob status shows correct number of currently running processors
			all of the above must be true to pass
		result: PASS