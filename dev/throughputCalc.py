#!/usr/bin/env python
# get throughput
# author: Ivan Senin 
import os
import sys
import re
import numpy as np

def toThroughputKbps(bytes, seconds):
	return bytes / 1000.0 * 8 / seconds
#-------------------------------------------------------------------------------
#  RLC
#-------------------------------------------------------------------------------
def processRlcStats(filename, start_time):
	dlRlcKPIs = np.loadtxt(filename, comments = '%')
	# rows: 0start 1end 2CellId 3IMSI 4RNTI 5LCID 6nTxPDUs 7TxBytes 8nRxPDUs 9RxBytes delay stdDev min max PduSize stdDev min max

	totalBytesRx = [.0, .0, .0]
	maxThroughput = [.0, .0, .0]
	rlcStartTime = dlRlcKPIs[0 , 0]
	totalDuration = dlRlcKPIs[-1, 1] - dlRlcKPIs[0 , 0]
	timestep = dlRlcKPIs[0, 1] - dlRlcKPIs[0, 0]

	for i in range(dlRlcKPIs.shape[0]):
		time = dlRlcKPIs[i, 0]
		if time < start_time:
			continue
		ueId = np.uint32(dlRlcKPIs[i, 3]) - 1
		rxBytes = dlRlcKPIs[i, 9]
		totalBytesRx[ueId] += rxBytes
		curThroughput = toThroughputKbps(rxBytes, timestep)
		maxThroughput[ueId] = curThroughput if curThroughput > maxThroughput[ueId] else maxThroughput[ueId]

	print "DlThroughput (RLC) [Kbps]:"
	print "Ue Id   Max         Average"
	for i in range(3):
		aveThroughput = toThroughputKbps(totalBytesRx[i], totalDuration)
		print "{0:<8}{1:<12}{2:<16}".format(i + 1, maxThroughput[i], aveThroughput)


#-------------------------------------------------------------------------------
#  MAC
#-------------------------------------------------------------------------------
def processMacStats(filename, start_time):
	epochDuration = .500
	dlMacKPIs = np.loadtxt(filename, comments = '%')

	# rows: 0time 1cellId 2IMSI 3frame 4sframe 5RNTI 6mcsTb1 7sizeTb1 mcsTb2 sizeTb2

	macStartTime = dlMacKPIs[0, 0]
	totalBytesRx = [.0, .0, .0]
	totalBytesRxPrev = [.0, .0, .0]
	maxThroughput = [.0, .0, .0]
	epochStartTime = [macStartTime, macStartTime, macStartTime]

	for i in range(dlMacKPIs.shape[0]):
		time = dlMacKPIs[i, 0]
		if time < start_time:
			continue
		ueId = np.uint32(dlMacKPIs[i, 2]) - 1
		mcs = np.uint32(dlMacKPIs[i, 6])
		totalBytesRx[ueId] += dlMacKPIs[i, 7]

		if time >= epochStartTime[ueId] + epochDuration:
			curThroughput = toThroughputKbps(totalBytesRx[ueId] - totalBytesRxPrev[ueId], time - epochStartTime[ueId])
			maxThroughput[ueId] = curThroughput if curThroughput > maxThroughput[ueId] else maxThroughput[ueId]
			epochStartTime[ueId] = time
			totalBytesRxPrev[ueId] = totalBytesRx[ueId]

	totalDuration = dlMacKPIs[-1, 0] - macStartTime

	print "DlThroughput (MAC) [Kbps]:"
	print "Ue Id   Max         Average"
	for i in range(3):
		aveThroughput = toThroughputKbps(totalBytesRx[i], totalDuration)
		print "{0:<8}{1:<12}{2:<16}".format(i + 1, maxThroughput[i], aveThroughput)

	print "\nTotal received (MAC):"
	print "Ue Id   Received [Mb]"
	for i in range(3):
		print "{0:<8}{1:<12}".format(i + 1, totalBytesRx[i] / 1024 / 1024)
	print ""

def is_float(value):
  try:
    float(value)
    return True
  except ValueError:
    return False

def usage():
	print "File name must contain words mac or rlc to process them appropriate\n"
	print "Usage:  throughputCalc.py filepath [start time in same resolution as in file]"
	print "Examples:"
	print "throughputCalc.py DlRlcStats.txt" 
	print "throughputCalc.py ./some_directory/DlMacStats.txt 0.11\n"

def main():
	argc = len(sys.argv)
	if argc < 2:
		usage()
		return

	full_path = sys.argv[1]
	filename = os.path.basename(full_path)

	is_mac_stats = re.search("mac", filename, re.IGNORECASE)
	is_rlc_stats = re.search("rlc", filename, re.IGNORECASE)

	if len(filename) == 0 or (is_mac_stats and is_rlc_stats) or (not is_mac_stats and not is_rlc_stats):
		usage()
		return;

	start_time = float(sys.argv[2]) if argc > 2 and is_float(sys.argv[2]) else float("-NaN")


	if is_mac_stats:
		processMacStats(full_path, start_time)
	else:
		processRlcStats(full_path, start_time)

main()