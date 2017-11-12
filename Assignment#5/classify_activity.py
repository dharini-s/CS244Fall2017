# -*- coding: utf-8 -*-
"""
Created on Sun Nov 12 12:01:10 2017

@author: staslist
"""

import csv
import numpy as np
from sklearn import svm
from sklearn.model_selection import LeaveOneOut
from sklearn.model_selection import cross_val_score

import mltools as ml
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
         
activities = np.genfromtxt("data/sampleXYZ.csv", delimiter =',', skip_header = 1)

#print(activities.size)
#print(activities.shape)

Activity_One = activities[:, 1:4]
Target_One = np.reshape(activities[:, 4], [activities[:, 4].size, 1])
Activity_Two = activities[:, 5:8]
Target_Two =  np.reshape(activities[:, 8], [activities[:, 8].size, 1])
Activity_Three = activities[:, 9:12]
Target_Three =  np.reshape(activities[:, 12], [activities[:, 12].size, 1])
Activity_Four = activities[:, 13:16]
Target_Four =  np.reshape(activities[:, 16], [activities[:, 16].size, 1])
Activity_Five = activities[:, 17:20]
Target_Five =  np.reshape(activities[:, 20], [activities[:, 20].size, 1])

# Visualizing the data
'''
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

xs = Activity_One[:, 0]
ys = Activity_One[:, 1]
zs = Activity_One[:, 2]

ax.scatter(xs, ys, zs, c='b', marker='.')

xs = Activity_Two[:, 0]
ys = Activity_Two[:, 1]
zs = Activity_Two[:, 2]

ax.scatter(xs, ys, zs, c='r', marker='.')

xs = Activity_Three[:, 0]
ys = Activity_Three[:, 1]
zs = Activity_Three[:, 2]

ax.scatter(xs, ys, zs, c='g', marker='.')

xs = Activity_Four[:, 0]
ys = Activity_Four[:, 1]
zs = Activity_Four[:, 2]

ax.scatter(xs, ys, zs, c='w', marker='.')

xs = Activity_Five[:, 0]
ys = Activity_Five[:, 1]
zs = Activity_Five[:, 2]

ax.scatter(xs, ys, zs, c='y', marker='.')

ax.set_xlabel('X Label')
ax.set_ylabel('Y Label')
ax.set_zlabel('Z Label')

plt.show()
'''

def extract_features(A, n, w_step = 1):
    ''' extract min, max, mean, and std features from matrix A using window of size n '''
    # A is a 4500 by 3 matrix with each column representing one direction (X,Y,Z)
    # n is window size
    # return result matrix with rows =  A.rows - n & columns = 12 (X.min, Y.min, Z.min, X.max, ... ,Z.std)
    
    # windows overlap
    # window step = 1
    i = 0
    result = np.empty([12, 1])
    while(i < A.shape[0] - (n)):
        A_window = A[i:n+i, :]
        
        minimum = np.amin(A_window, axis = 0) 
        maximum = np.amax(A_window, axis = 0) 
        mean = np.mean(A_window, axis = 0)
        std = np.std(A_window, axis = 0)
        
        feature_row = np.concatenate([minimum, maximum, mean, std], axis = 0)
        #feature_row = np.concatenate([mean], axis = 0)
        feature_row = np.reshape(feature_row, [1, feature_row.size])
        if (i == 0):
            result = feature_row
        else:
            result = np.concatenate([result, feature_row], axis = 0)
        i = i + w_step
    return result

def calc_error(Y_val, Y_hat_val):
    i = 0
    mistakes = 0
    while(i < Y_val.size):
        if(Y_val[i] != Y_hat_val[i]):
            mistakes = mistakes + 1
        i = i + 1
    error_rate = mistakes / Y_val.size
    return error_rate
        

# split data into 80/20 train/validation
AOne_tr, AOne_va, TOne_tr, TOne_va = ml.splitData(Activity_One, Target_One, 0.8)
ATwo_tr, ATwo_va, TTwo_tr, TTwo_va = ml.splitData(Activity_Two, Target_Two, 0.8) 
AThree_tr, AThree_va, TThree_tr, TThree_va = ml.splitData(Activity_Three, Target_Three, 0.8) 
AFour_tr, AFour_va, TFour_tr, TFour_va = ml.splitData(Activity_Four, Target_Four, 0.8) 
AFive_tr, AFive_va, TFive_tr, TFive_va = ml.splitData(Activity_Five, Target_Five, 0.8) 

# transform target arrays into target matrices with 1 column
'''TOne_tr = np.reshape(TOne_tr, [TOne_tr.size, 1])
TOne_va = np.reshape(TOne_va, [TOne_va.size, 1])
TTwo_tr = np.reshape(TTwo_tr, [TTwo_tr.size, 1])
TTwo_va = np.reshape(TTwo_va, [TTwo_va.size, 1])
TThree_tr = np.reshape(TThree_tr, [TThree_tr.size, 1])
TThree_va = np.reshape(TThree_va, [TThree_va.size, 1])
TFour_tr = np.reshape(TFour_tr, [TFour_tr.size, 1])
TFour_va = np.reshape(TFour_va, [TFour_va.size, 1])
TFive_tr = np.reshape(TFive_tr, [TFive_tr.size, 1])
TFive_va = np.reshape(TFive_va, [TFive_va.size, 1])'''

# strategy: 1) use SVM with hold out (80/20) validation
# 2) use SVM with LOO validation

# window the XYZ values (window size n = 100, window step w_step = 1):
n = 100

# extract the following features from each window
# extract mean & STD from each window
# extract min and max values

AOne_tr_extr = extract_features(AOne_tr, n)
AOne_va_extr = extract_features(AOne_va, n)
ATwo_tr_extr = extract_features(ATwo_tr, n)
ATwo_va_extr = extract_features(ATwo_va, n)
AThree_tr_extr = extract_features(AThree_tr, n)
AThree_va_extr = extract_features(AThree_va, n)
AFour_tr_extr = extract_features(AFour_tr, n)
AFour_va_extr = extract_features(AFour_va, n)
AFive_tr_extr = extract_features(AFive_tr, n)
AFive_va_extr = extract_features(AFive_va, n)

# trim each of the target arrays by n values
TOne_tr_extr = TOne_tr[:-n]
TOne_va_extr = TOne_va[:-n]
TTwo_tr_extr = TTwo_tr[:-n]
TTwo_va_extr = TTwo_va[:-n]
TThree_tr_extr = TThree_tr[:-n]
TThree_va_extr = TThree_va[:-n]
TFour_tr_extr = TFour_tr[:-n]
TFour_va_extr = TFour_va[:-n]
TFive_tr_extr = TFive_tr[:-n]
TFive_va_extr = TFive_va[:-n]

X_train = np.concatenate([AOne_tr_extr, ATwo_tr_extr, AThree_tr_extr, AFour_tr_extr, AFive_tr_extr], axis = 0)
Y_train = np.concatenate([TOne_tr_extr, TTwo_tr_extr, TThree_tr_extr, TFour_tr_extr, TFive_tr_extr], axis = 0)
X_val = np.concatenate([AOne_va_extr, ATwo_va_extr, AThree_va_extr, AFour_va_extr, AFive_va_extr], axis = 0)
Y_val = np.concatenate([TOne_va_extr, TTwo_va_extr, TThree_va_extr, TFour_va_extr, TFive_va_extr], axis = 0)

X_train, Y_train = ml.shuffleData(X_train, Y_train)
X_val, Y_val = ml.shuffleData(X_val, Y_val)


# train on the features + perform hold validation

clf = svm.SVC(gamma=0.001, C=100.)
clf.fit(X_train, Y_train)

Y_hat_val = clf.predict(X_val)
error_rate = calc_error(Y_val, Y_hat_val)
print("Hold out validation error rate:")
print(error_rate)

#np.savetxt('y_val.csv', Y_val, delimiter=',')
#np.savetxt('y_hat_val.csv', Y_hat_val, delimiter=',')

# train on the features + perform cros validation
X_train = np.concatenate([X_train, X_val], axis = 0)
Y_train = np.concatenate([Y_train, Y_val], axis = 0)

clf = svm.SVC(gamma=0.001, C=100.)
print("Cross validation error rate:")
print(cross_val_score(clf, X_train, Y_train, scoring='neg_mean_absolute_error'))

# train on the features + perform LOO validation
# note: LOO validation can take several minutes to run on a single CPU
loo = LeaveOneOut()
loo.get_n_splits(X_train)

mistakes = 0
for train_index, test_index in loo.split(X_train):
    #print("TRAIN:", train_index, "TEST:", test_index)
    X_tr, X_te = X_train[train_index], X_train[test_index]
    y_tr, y_te = Y_train[train_index], Y_train[test_index]
    clf = svm.SVC(gamma=0.001, C=100.)
    clf.fit(X_tr, y_tr)
    Y_hat = clf.predict(X_te)
    if(Y_hat != y_te):
        mistakes = mistakes + 1
        #print("Misclassified!")

error_rate = mistakes / X_train.size
print(error_rate)