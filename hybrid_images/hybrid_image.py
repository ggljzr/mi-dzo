import numpy as np
from numpy.fft import fft2, ifft2, fftshift, ifftshift
import math
import cv2

#src: https://jeremykun.com/2014/09/29/hybrid-images/
def makeGaussianFilter(numRows, numCols, sigma):
   centerI = int(numRows/2) + 1 if numRows % 2 == 1 else int(numRows/2)
   centerJ = int(numCols/2) + 1 if numCols % 2 == 1 else int(numCols/2)
 
   def gaussian(i,j):
      coefficient = math.exp(-1.0 * ((i - centerI)**2 + (j - centerJ)**2) / (2 * sigma**2))
      return coefficient
 
   return np.array([[gaussian(i,j) for j in range(numCols)] for i in range(numRows)])

def get_spec(img):
   spec = fft2(img)
   spec = fftshift(spec)
   return spec

if __name__ == "__main__":
   mon = cv2.imread("mon250.png", 0)
   ein = cv2.imread("ein250.png", 0)

   n,m = mon.shape

   gaussian = makeGaussianFilter(n, m, 20)

   ein_spec = get_spec(ein)
   mon_spec = get_spec(mon)
   mon_spec = mon_spec * gaussian

   diff = 0

   low = ifft2(mon_spec)
   high = ifft2(ein_spec * (1 - gaussian))

   res = low + high
   
   cv2.imshow('aaa', np.abs(res) / 255)
   cv2.waitKey()