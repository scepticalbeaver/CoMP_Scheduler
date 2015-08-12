# get throughput
import os
import numpy as np

def toThroughputKbps(bytes, seconds):
	return bytes / 1000.0 * 8 / seconds
#-------------------------------------------------------------------------------
#  RLC
#-------------------------------------------------------------------------------


dlRlcKPIs = np.loadtxt("build/debug/bin/DlRlcStats.txt", comments = '%', skiprows = 1)

# rows: 0start 1end 2CellId 3IMSI 4RNTI 5LCID 6nTxPDUs 7TxBytes 8nRxPDUs 9RxBytes delay stdDev min max PduSize stdDev min max

totalBytesRx = [.0, .0, .0]
maxThroughput = [.0, .0, .0]

rlcStartTime = dlRlcKPIs[0 , 0]
totalDuration = dlRlcKPIs[-1, 1] - dlRlcKPIs[0 , 0]
timestep = dlRlcKPIs[0, 1] - dlRlcKPIs[0, 0]

for i in range(dlRlcKPIs.shape[0]):
	ueId = np.uint32(dlRlcKPIs[i, 3]) - 1
	rxBytes = dlRlcKPIs[i, 9]
	totalBytesRx[ueId] += rxBytes
	curThroughput = toThroughputKbps(rxBytes, timestep)
	maxThroughput[ueId] = curThroughput if curThroughput > maxThroughput[ueId] else maxThroughput[ueId]




print "DlThroughput (RLC) [Kbps]:"
print "ueId    max         average"
for i in range(3):
	aveThroughput = toThroughputKbps(totalBytesRx[i], totalDuration)
	print "{0:<8}{1:<12}{2:<16}".format(i + 1, maxThroughput[i], aveThroughput)


#-------------------------------------------------------------------------------
#  MAC
#-------------------------------------------------------------------------------
epochDuration = .500


dlMacKPIs = np.loadtxt("build/debug/bin/DlMacStats.txt", comments = '%', skiprows = 1)

# rows: 0time 1cellId 2IMSI 3frame 4sframe 5RNTI 6mcsTb1 7sizeTb1 mcsTb2 sizeTb2

macStartTime = dlMacKPIs[0, 0]

totalBytesRx = [.0, .0, .0]
totalBytesRxPrev = [.0, .0, .0]

maxThroughput = [.0, .0, .0]
epochStartTime = [macStartTime, macStartTime, macStartTime]


for i in range(dlMacKPIs.shape[0]):
	time = dlMacKPIs[i, 0]
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
print "ueId    max         average"
for i in range(3):
	aveThroughput = toThroughputKbps(totalBytesRx[i], totalDuration)
	print "{0:<8}{1:<12}{2:<16}".format(i + 1, maxThroughput[i], aveThroughput)