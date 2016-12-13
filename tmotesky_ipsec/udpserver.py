#!/usr/bin/python

from datetime import datetime
from datetime import timedelta
import signal
import os, socket, sys, time

def do_alarm(sig, stack):
	print "Timeout"
	sys.exit(1)

def timeDeltaToMillis(time):
	return time.days * 24 * 3600 * 1000 + time.seconds * 1000 + time.microseconds / 1000

def main():
	signal.signal(signal.SIGALRM, do_alarm)
	host = sys.argv[1]
	port = 1234
	s = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
	s.connect((host, port))

	data = "Req"
	num_measurements = 10
	i = 1

	for i in range(num_measurements):
		start = datetime.now()
		signal.alarm(4)

		s.sendall(data)
		resp = s.recv(1024)

		end = datetime.now()
		signal.alarm(0)
		duration = timeDeltaToMillis(end - start)

		print "\nDuration: %d ms" %(duration)
		print "Request:  %s" %(data)
		print "Response: %s\n" %(resp)

		if i == num_measurements:
			print "End measurements request\n"
			break;

		time.sleep(5)

main()
