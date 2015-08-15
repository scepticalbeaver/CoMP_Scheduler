import os
import numpy as np


def print_line(line):
	line_size = line.shape[0]
	assert line_size == 18

	strline = ""
	for i in range(10):
		x = round(line[i], 4) if i < 2 else line[i]
		strline += str(x) + "\t"
	print strline


def print_diff():
	EPS = 0.0001
	dlRlcKPI_1 = np.loadtxt("compAlgo/input/DlRlcStats.txt", comments = '%')
	dlRlcKPI_2 = np.loadtxt("compAlgo/output/DlRlcStats.txt", comments = '%')

	lines_num1 = dlRlcKPI_1.shape[0]
	lines_num2 = dlRlcKPI_2.shape[0]

	print lines_num1, lines_num2

	lpointer = 0
	rpointer = 0

	#% 0start	1end	2CellId	3IMSI	4RNTI	LCID	nTxPDUs	TxBytes	nRxPDUs	RxBytes	delay	stdDev	min	max	PduSize	stdDev	min	max

	while lpointer < lines_num1 and rpointer < lines_num2:
		cell1 = dlRlcKPI_1[lpointer, 2]
		cell2 = dlRlcKPI_2[rpointer, 2]

		assert (cell2 == 1)

		if cell1 != 1:
			lpointer += 1
			continue

		if abs(dlRlcKPI_1[lpointer, 0] - dlRlcKPI_2[rpointer, 0]) > EPS or abs(dlRlcKPI_1[lpointer, 1] - dlRlcKPI_2[rpointer, 1]) > EPS:
			print "diff:"
			print "line 1: ", lpointer + 2, "/", lines_num1 + 1 , "\tline 2: ", rpointer + 2, "/", lines_num2 + 1
			print ""
			print_line(dlRlcKPI_1[lpointer])
			print_line(dlRlcKPI_2[rpointer])
			return False
		else:
			lpointer += 1
			rpointer += 1

	return True


def main():
	if print_diff():
		print "done"
	else:
		print "Problem detected"


main()
