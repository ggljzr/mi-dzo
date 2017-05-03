import numpy as np
from numpy.fft import fft2, ifft2, fftshift, ifftshift
import cv2

def get_gaussian(rows, cols, sigma):
    c_y = rows // 2 + (rows % 2)
    c_x = cols // 2 + (cols % 2)

    def gaussian(x, y):
        exponent = ((x - c_x)**2 + (y - c_y)**2)/ (-2*sigma**2)
        return np.exp(exponent)

    return np.fromfunction(lambda x, y : gaussian(x,y), (rows, cols))

def get_spec(img):
    spec = fft2(img)
    spec = fftshift(spec)
    return spec

if __name__ == '__main__':
    mon = cv2.imread('mon350.png', 0)
    ein = cv2.imread('ein350.png', 0)

    n,m = mon.shape

    gaussian = get_gaussian(n, m, 20)

    ein_spec = get_spec(ein)
    mon_spec = get_spec(mon)

    mon_spec = mon_spec * gaussian
    #ein_spec = ein_spec - mon_spec
    ein_spec = ein_spec * (1 - gaussian)

    low = ifft2(mon_spec)
    high = ifft2(ein_spec)

    res = low + (high * 1.2)

    cv2.imshow('gauss', gaussian)
    cv2.imshow('res', np.abs(res) / 255)
    cv2.imshow('low', np.abs(low) / 255)
    cv2.imshow('high', np.abs(high) / 255)
    cv2.waitKey()