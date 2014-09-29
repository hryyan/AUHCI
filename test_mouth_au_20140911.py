#!/usr/bin/env python
# coding=utf-8

def fun(filepath):
	f = open(filepath)
	lines = f.readlines()

	total = 0
	c_mouth = 0
	c_eye = 0
	m_mouth = 0
	m_eye = 0
	w_mouth = 0
	w_eye = 0

	for line in lines:
	    s = line.split(' ')
	    if s[0][0] == '\t':
	        s[0] = s[0][1:]
	    if s[0] == 'wrong:':
	    	for a in s[1:]:
	    		if a != '\n' and int(a) < 10:
	    			w_eye += 1
	    		elif a != '\n' and int(a) >= 10:
	    			w_mouth += 1
	    if s[0] == 'miss:':
	    	for a in s[1:]:
	    		if a != '\n' and int(a) < 10:
	    			m_eye += 1
	    		elif a != '\n' and int(a) >= 10:
	    			m_mouth += 1
	    if s[0] == 'correct:':
	    	for a in s[1:]:
	    		if a != '\n' and int(a) < 10:
	    			c_eye += 1
	    		elif a != '\n' and int(a) >= 10:
	    			c_mouth += 1
	    if s[0] == 'Num:':
	    	total = total + int(s[-1])

	total_eye = w_eye + c_eye + m_eye
	total_mouth = w_mouth + c_mouth + m_mouth

	print "mouth_correct: ", float(c_mouth) / total_mouth
	print "eye_correct:   ", float(c_eye)   / total_eye
	print "mouth_miss:    ", float(m_mouth) / total_mouth
	print "eye_miss:      ", float(m_eye)   / total_eye
	print "mouth_wrong:   ", float(w_mouth) / total_mouth
	print "eye_wrong:     ", float(w_eye)   / total_eye

	print total_mouth
	print total_eye

print "gaussian_kernel_0"
fun("build-AUHCI_crossplatform-Desktop-Debug/gaussian_kernel_0")
print "gaussian_kernel_1"
fun("build-AUHCI_crossplatform-Desktop-Debug/gaussian_kernel_1")
print "gaussian_kernel_2"
fun("build-AUHCI_crossplatform-Desktop-Debug/gaussian_kernel_2")
print "gaussian_kernel_3"
fun("build-AUHCI_crossplatform-Desktop-Debug/gaussian_kernel_3")
# print "gaussian_kernel_4"
# fun("build-AUHCI_crossplatform-Desktop-Debug/gaussian_kernel_4")
# print "gaussian_kernel_5"
# fun("build-AUHCI_crossplatform-Desktop-Debug/gaussian_kernel_5")
# print "gaussian_kernel_6"
# fun("build-AUHCI_crossplatform-Desktop-Debug/gaussian_kernel_6")
