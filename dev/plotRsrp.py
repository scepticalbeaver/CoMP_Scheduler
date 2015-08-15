#!/usr/bin/env python
# measurements plotter
# author: Ivan Senin 
import sys
import re
import numpy as np
import matplotlib.pyplot as plt
import scipy.interpolate as ip
import scipy.fftpack


def reject_outliers_at(x_vals, y_vals, m, beg, end):
	x_vals_new = []
	y_vals_new = []


	diff = np.abs(y_vals[beg : end] - np.median(y_vals[beg : end]))
	mdev = np.median(diff)
	offset = 3
	if len(x_vals) > end + offset:
		diff2 = np.abs(y_vals[beg + offset : end + offset] - np.median(y_vals[beg + offset : end + offset]))
		mdev = (mdev + np.median(diff2)) / 2.0

	s = diff / mdev if mdev else diff
	for j in range(beg, end):
		if s[j - beg] < m:
			x_vals_new.append(x_vals[j])
			y_vals_new.append(y_vals[j])
	return x_vals_new, y_vals_new

def reject_outliers(x_vals, y_vals, m = 2.5):
	x_vals_new = []
	y_vals_new = []

	chunk_size = 5
	chunk_num = len(y_vals) / chunk_size
	i = 0

	for i in range(0, chunk_num):
		beg = i * chunk_size
		end = (i + 1) * chunk_size
		x_new_part, y_new_part = reject_outliers_at(x_vals, y_vals, m, beg, end)
		x_vals_new = x_vals_new + x_new_part
		y_vals_new = y_vals_new + y_new_part

	if chunk_size * chunk_num < len(y_vals):
		beg = chunk_num * chunk_size
		end = len(y_vals)
		x_new_part, y_new_part = reject_outliers_at(x_vals, y_vals, m, beg, end)
		x_vals_new = x_vals_new + x_new_part
		y_vals_new = y_vals_new + y_new_part

	return x_vals_new, y_vals_new

def load_measurements(filepath, used_to_same_cell_id=False):
	rsrp_data = np.loadtxt(filepath, comments = '%')
	# % 0time[usec]	1srcCellId	2targetCellId	3RSRP

	timings = { 1 : [], 2 : [], 3 : []}
	measurements = { 1 : [], 2 : [], 3 : []}

	for i in range(rsrp_data.shape[0]):
		time = float(rsrp_data[i, 0]) / 1000 / 1000 # into [sec]
		scell_id = rsrp_data[i, 1]
		tcell_id = rsrp_data[i, 2]
		idx = 1 if used_to_same_cell_id else tcell_id
		if used_to_same_cell_id and scell_id != tcell_id:
			continue
		rsrp = rsrp_data[i, 3]

		timings[idx].append(time)
		measurements[idx].append(rsrp)

	return timings, measurements

def find_bounds(measurements):
	rmin = +float('Inf')
	rmax = -float('Inf')
	for i in range(1,4):
		imin = min(measurements[i])
		imax = max(measurements[i])
		rmin = min(rmin, imin)
		rmax = max(rmax, imax)
	return rmin, rmax

def plot_comp_ue_measures(may_rm_outliers=False):
	timing, measurement = load_measurements("compAlgo/output/measurements.log", used_to_same_cell_id=True)
	timing = timing[1]
	measurement = measurement[1]

	if may_rm_outliers:
		timing, measurement = reject_outliers(timing, measurement)

	plt.plot(timing, measurement, '--^', label='Ue', linewidth=3)


def plot_as_is(xs, ys, lw_offset=2.0):
	plt.plot(xs[1], ys[1], 'b', label='Cell 1', linewidth=lw_offset + .6)  # 'ro'
	plt.plot(xs[2], ys[2], 'r', label='Cell 2', linewidth=lw_offset + .4)  # 'b--'
	plt.plot(xs[3], ys[3], 'c', label='Cell 3', linewidth=lw_offset + .2)


def smooth(xs, ys):
	N = len(xs) * 3

	box_pts = 3
	box = np.ones(box_pts)/box_pts
	y_smooth = np.convolve(ys, box, mode='same')
    
	return xs, y_smooth


def main():
	timings, measurements = load_measurements("compAlgo/input/measurements.log")

	if "--row" in sys.argv:
		plot_as_is(timings, measurements, 0.1)

	may_rm_outliers = "--rm-outliers" in sys.argv
	if may_rm_outliers:
		for i in range(1,4):
			timings[i], measurements[i] = reject_outliers(timings[i], measurements[i])

	"""if "--spline" in sys.argv:
		count = 50
		xnew = { 1 : np.linspace(timings[1][0], timings[1][-1], count), \
		    	 2 : np.linspace(timings[2][0], timings[2][-1], count), \
			     3 : np.linspace(timings[3][0], timings[3][-1], count)}
		for i in range(1,4):
			measurements[i] = spline(timings[i], measurements[i], xnew[i])
		timings = xnew"""

	if "--smooth" in sys.argv:
		timings[1], measurements[1] = smooth(timings[1], measurements[1])
		timings[2], measurements[2] = smooth(timings[2], measurements[2])
		timings[3], measurements[3] = smooth(timings[3], measurements[3])

	plot_as_is(timings, measurements)

	if "--ue" in sys.argv:
		plot_comp_ue_measures(may_rm_outliers)

	mmin, mmax = find_bounds(measurements)
	ydelta = abs(float(mmax - mmin) / 10.0)

	xmin, xmax = find_bounds(timings)
	xdelta = abs(float(xmin - xmax) / 30.0)

	plt.ylim((mmin - ydelta, mmax + ydelta))
	plt.xlim((xmin - xdelta, xmax + xdelta))
	plt.legend(loc=4)
	plt.grid(True)
	plt.xlabel('Time [s]')
	plt.ylabel('RSRP')
	plt.suptitle('CSI')

	plt.savefig("measurements_plot.png")
	
	if "--show" in sys.argv:
		plt.show()



main()

