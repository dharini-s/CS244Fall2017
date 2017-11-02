#
# Team 4 - Assignment 3
#
import numpy as np

from scipy.signal import butter, lfilter
import matplotlib.pyplot as plt

import math

data = np.genfromtxt("takashin_Homework_sample.csv",delimiter=",", skip_header=1, dtype=float)
formattedData = np.empty((data.shape[0], 3))
formattedData[:,:] = data[:,0:3]

def findMaxMins(data):

    minimas = []
    maximas = []

    for i in range(1, len(data)-1):
        if (data[i-1] < data[i]) and (data[i] > data[i+1]):
            maximas.append(i)
        if (data[i-1] > data[i]) and (data[i] < data[i+1]):
            minimas.append(i)
    return maximas, minimas

def butterworthLowpass(cutoff, sample_rate, order=5):
    filterCutoff = cutoff / (0.5 * sample_rate)
    return butter(order, filterCutoff, analog=False, btype='low')
    
#Butterworth low pass filter
def butterworthLowpassFilter(data, cutoff, sample_rate, order=5):
    b, a = butterworthLowpass(cutoff, sample_rate, order=order)
    return lfilter(b, a, data)


def butterworth(lowThreshold, highThreshold, sample_rate, order=5):
    nyquist = 0.5 * sample_rate
    low = lowThreshold / nyquist
    high = highThreshold / nyquist
    return butter(order, [low, high], btype='band')

#Butterworth bandpass filter
def butterworthFilter(data, lowThreshold, highThreshold, sample_rate, order=5):
    b, a = butterworth(lowThreshold, highThreshold, sample_rate, order=order)
    return lfilter(b, a, data)


def filterBoundaries(lowBpm, highBpm, freqs):
    higherThreshold = float(highBpm)/60.0   
    lowerThreshold = float(lowBpm)/60.0 

    start = -1
    for i in range(len(freq)):
        if freqs[i] >= lowerThreshold:
            start = i
            break

    end = -1
    for i in range(len(freq)):
        if freqs[i] >= higherThreshold:
            end = i
            break

    return start, end

def findPeak(fftvalue, start, end):
    peak = 0.0
    peakFreq = -1
    for i in range(start, end):
        if fftvalue[i-1]<fftvalue[i]>fftvalue[i+1] and fftvalue[i] > peak:
            peak = fftvalue[i]
            peakFreq = i
    return peakFreq

def calHeartRate(fftvalue, freqs):
    start, end = filterBoundaries(40.0, 220.0, freqs)
    peakFreq = findPeak(fftvalue, start, end)
    return freqs[peakFreq]

def calRespiratoryRate(fftvalue, freqs):
    start, end = filterBoundaries(8, 22, freqs)
    peakFreq = findPeak(fftvalue, start, end)
    return freqs[peakFreq]


#Problem statement solving
heartRate = 0.0
respRate = 0.0
spo2Value = 0.0


#Given as 50Hz
sampleFreq = 50

dataSignal = formattedData[:,2]

timestep = 1.0/sampleFreq
splits = 1



# Calculating SpO2
IR = data[:, 2]
RED = data[:, 1]
IR = butterworthLowpassFilter(IR, 2, sampleFreq, 6)[100:]
RED = butterworthLowpassFilter(RED, 2, sampleFreq, 6)[100:]
xValues = [_ for _ in range(len(IR))]
redMaximums, redMinimums = findMaxMins(RED)
irMaximums, irMinimums = findMaxMins(IR)
redYValues = [RED[i] for i in redMinimums]
redYInterp = np.interp(xValues, redMinimums, redYValues)
irYValues = [IR[i] for i in irMinimums]
irYInterp = np.interp(xValues, irMinimums, irYValues)
acIr = [(IR[i] - irYInterp[i]) for i in irMaximums]
dcIr = [irYInterp[i] for i in irMaximums]
acRed = [(RED[i] - redYInterp[i]) for i in irMaximums]
dcRed = [redYInterp[i] for i in irMaximums]
spo2ans = []    


for i in range(len(acRed)):
    ratioAverage = (acRed[i]*dcIr[i])/(acIr[i]*dcRed[i])
    spo2 = (-45.060* ratioAverage * ratioAverage) + (30.354 * ratioAverage) + 94.845
    spo2ans.append(spo2)
spo2Value = np.mean(spo2ans)
print "The calculated SpO2:- ", spo2Value


#Resp and Heart rate
for item in np.array_split(dataSignal, splits):
    avg = np.mean(item)
    item = item-avg
    fourier = np.fft.fft(item)
    absoluteVal = abs(fourier)**2
    freq = np.fft.fftfreq(item.size, d=timestep)
    heartRateFreq = calHeartRate(absoluteVal, freq)
    filteredDataSignal = butterworthFilter(item, heartRateFreq-0.5, heartRateFreq+0.5, sampleFreq)    
    maximums, mins = findMaxMins(filteredDataSignal[1500:])
    heartrateList = [(60*50/float(maximums[i]-maximums[i-1])) for i in range(1, len(maximums))]
    heartRate += np.mean(heartrateList)/splits

    respRate_freq = calRespiratoryRate(absoluteVal, freq)
    filteredDataSignal = butterworthFilter(item, 0.2, 0.4, sampleFreq, order = 4)    
    maximums, mins = findMaxMins(filteredDataSignal[1500:])
    resprate_arr = [(60*50/float(maximums[i]-maximums[i-1])) for i in range(1, len(maximums))]
    resprate_arr = filter(lambda x: 6<x<24,  resprate_arr)
    respRate += np.mean(resprate_arr)/splits

print "The calculated Respiration Rate:- ", respRate
print "The calculated Heart Rate:- ", heartRate