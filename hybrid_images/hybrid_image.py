import cv2
import numpy as np

ein = cv2.imread('ein.png', 0)

print(ein.shape)
f = np.fft.fft2(ein, (407, 407))
fshift = np.fft.fftshift(f)
fspec = np.log(np.abs(fshift))

m = np.ones(ein.shape)
m[0:(m.shape[0] // 2)] = -1

mf = np.fft.fft2(m)

cv2.imwrite('res.png', fspec * 15)