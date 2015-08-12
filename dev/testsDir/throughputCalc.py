# get throughput
import os
import numpy as np

def toThroughputKbps(bytes, seconds):
	return bytes * 8 / 1000.0 / seconds

dlRlcKPIs = np.loadtxt("build/debug/bin/DlRlcStats.txt", comments = '%', skiprows = 1)

# rows: 0start 1end 2CellId 3IMSI 4RNTI 5LCID 6nTxPDUs 7TxBytes 8nRxPDUs 9RxBytes delay stdDev min max PduSize stdDev min max

totalBytesRx = [.0, .0, .0]
maxThroughput = [.0, .0, .0]
timestep = dlRlcKPIs[0, 1] - dlRlcKPIs[0, 0]

for i in range(dlRlcKPIs.shape[0]):
	ueId = np.uint32(dlRlcKPIs[i, 3]) - 1
	rxBytes = dlRlcKPIs[i, 9]
	totalBytesRx[ueId] += rxBytes
	curThroughput = toThroughputKbps(rxBytes, timestep)
	maxThroughput[ueId] = curThroughput if curThroughput > maxThroughput[ueId] else maxThroughput[ueId]


totalDuration = dlRlcKPIs[-1, 1] - dlRlcKPIs[0 , 0]

print "DlThroughput [Kbps]:"
print "ueId    max         average"
for i in range(3):
	aveThroughput = toThroughputKbps(totalBytesRx[i], totalDuration)
	print "{0:<8}{1:<12}{2:<16}".format(i + 1, maxThroughput[i], aveThroughput)





