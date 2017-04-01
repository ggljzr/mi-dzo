import cv2
import numpy as np

ein = cv2.imread('ein.png')
ein = cv2.cvtColor(ein, cv2.COLOR_RGB2GRAY)

f = np.fft.fft2(ein)

m = np.ones(ein.shape)
m[0:(m.shape[0] // 2)] = -1

mf = np.fft.fft2(m)

res = f * mf
res = np.fft.ifft2(res)
res = np.log(np.abs(res))

cv2.imwrite('res.png', res * 15)