import numpy as np
from numpy.fft import fft2, ifft2, fftshift, ifftshift
import cv2

import sys

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

    sigma = 10

    if len(sys.argv) < 3:
        print('Usage: ./hybrid_image.py low_image high_image (sigma)')
        sys.exit(0)

    if len(sys.argv) >= 4:
        sigma = float(sys.argv[3])

    low_in = cv2.imread(sys.argv[1], 0)
    high_in = cv2.imread(sys.argv[2], 0)

    if low_in.shape != high_in.shape:
        print('Both images have to be the same size')
        sys.exit(0)

    n, m = low_in.shape

    gaussian = get_gaussian(n, m, sigma)

    low_spec = get_spec(low_in)
    high_spec = get_spec(high_in)

    low_spec = low_spec * gaussian
    high_spec = high_spec * (1 - gaussian)

    low = ifft2(low_spec)
    high = ifft2(high_spec)

    res = low + (high)

    cv2.imshow('gauss', gaussian)
    cv2.imshow('res', np.abs(res) / 255)
    cv2.imshow('low', np.abs(low) / 255)
    cv2.imshow('high', np.abs(high) / 255)
    cv2.waitKey()

    cv2.imwrite('res.png', np.abs(res))